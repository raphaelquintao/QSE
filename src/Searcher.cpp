#include "Searcher.h"
#include <boost/tokenizer.hpp>
#include <boost/algorithm/string.hpp>
#include <sstream>
#include <math.h>
#include <list>
#include <boost/archive/iterators/base64_from_binary.hpp>
#include <boost/archive/iterators/transform_width.hpp>
#include <boost/archive/iterators/ostream_iterator.hpp>
  
using namespace std;
using namespace boost;


Searcher::Searcher(IndexHandler *handler) {
    Handler = handler;
    Voca = Handler->Voca;
    Docs = Handler->Docs;
    Docs_avg = Handler->DocsAvgLength;

    reader_index.open(Handler->IndexFiles.idx_path, ifstream::binary);
    if (!reader_index.is_open()) {
        printf("   -> Can't find Index: %s\n", Handler->IndexFiles.idx_path.c_str());
    }

    reader_doc.open(Handler->IndexFiles.doc_path, ifstream::binary);
    if (!reader_doc.is_open()) {
        printf("   -> Can't find Documents: %s\n", Handler->IndexFiles.doc_path.c_str());
    }
}

Searcher::ReturnDoc Searcher::GetDocument(int doc_id) {
    ReturnDoc doc;
    long long offset = Docs->at((unsigned long long) doc_id);

    string temp;
    reader_doc.seekg(offset);

    getline(reader_doc, temp);
    doc.length = stod(temp.c_str());

    getline(reader_doc, temp);
    doc.url = temp;

    return doc;
}

TermOccs Searcher::GetTermOcc(const std::string &term, int positions) {
    QTriples occ_vec;

    QVoc::iterator found = Voca->find(term);
    if (found != Voca->end()) {
        VocItem *item = &found->second;
        vector<long long> offsets = item->OccsOffsets;

        char_separator<char> sep1("|");
        char_separator<char> sep2(",");

        string temp_line;
        stringstream temp;
        for (int c = 0; c < offsets.size(); c++) {
            reader_index.seekg(offsets[c]);
            getline(reader_index, temp_line);
//            if (c != 0) temp << '|';
//            temp << temp_line;
            tokenizer<char_separator<char>> block_tokens(temp_line, sep1);
            for (const string &block : block_tokens) {
                tokenizer<char_separator<char>> tokens2(block, sep2);
                int control = 0;
                Occurrence occ;
                vector<int> pos;
                for (const string &term : tokens2) {
                    if (control == 0) {
                        occ.doc_id = atoi(term.c_str());
                    } else if (control == 1) {
                        occ.term_frequency = atoi(term.c_str());
                    } else if ((positions == -1) || ((positions != 0) && (positions > control))) {
                        pos.push_back(atoi(term.c_str()));
                    } else break;
                    control++;
                }
                occ_vec[occ.doc_id] = occ;
            }
        }
//        string line = temp.str();
    }

    return occ_vec;
}

int Searcher::GetTermOccs(QTriples &triples, const std::string &term) {
    QVoc::iterator found = Voca->find(term);
    if (found != Voca->end()) {
        VocItem *item = &found->second;
        vector<long long> offsets = item->OccsOffsets;

        char_separator<char> sep1("|");
        char_separator<char> sep2(",");

        string temp_line;
        stringstream temp;
        int seek_count = 0;

        for (int c = 0; c < offsets.size(); c++) {
            reader_index.seekg(offsets[c]);
            getline(reader_index, temp_line);
            seek_count++;

            tokenizer<char_separator<char>> block_tokens(temp_line, sep1);
            for (const string &block : block_tokens) {

                int control = 0;
                QTriple triple;

                tokenizer<char_separator<char>> tokens2(block, sep2);
                for (const string &termm : tokens2) {
                    if (control == 0) {
                        triple.doc_id = atoi(termm.c_str());
                    } else if (control == 1) {
                        triple.term_frequency = atoi(termm.c_str());
                    } else break;
                    control++;
                }

                triples[triple.doc_id] = triple;

            }

        }
        return seek_count;
    } else return 0;

}

bool compare(const TermOccs &first, const TermOccs &second) {
    return first.size() < second.size();
}


/**
 * Weight TF-IDF - Wij
 * @param Fij Frequency of term in document
 * @param N Total documents in collection
 * @param Ni Total number of term occurrence
 */
double TF_IDF(int Fij, int N, int Ni) {
    if (Fij > 0) {
        double TF = 1 + log2(Fij);
        double IDF = log2(N / Ni);
        return TF * IDF;
    } else return 0;
}

typedef unordered_map<int, double> Rank_Temp;

void Searcher::Vector(list <string> &query_terms, JSON_Temp &json) {
    typedef unordered_map<int, ReturnDoc> Docs_Temp;

    // BM25 Variables
    double b = 0.75;
    double avg_doclen = Docs_avg;


    Rank_Temp *temp = new Rank_Temp();
    Docs_Temp *docs_temp = new Docs_Temp();

    int seeks_total = 0;
    double larger = 0;
    for (const string &query_term : query_terms) {
        QTriples term_occs;
        seeks_total += GetTermOccs(term_occs, query_term);

        double IDF = log2(Docs->size() / (double) term_occs.size()); //IDF

        for (QTriples::iterator iter = term_occs.begin(); iter != term_occs.end(); ++iter) {
            ReturnDoc &doc = (*docs_temp)[iter->first];
            if(doc.length == 0){
                ReturnDoc doc_tmp = GetDocument(iter->first);
                doc.length = doc_tmp.length;
                doc.url = doc_tmp.url;
                if(doc.length > larger) larger = doc.length;
            }

            QTriple occ = iter->second;

            double TF_doc = 1 + log2(occ.term_frequency);

            (*temp)[iter->first] += (TF_doc * IDF); // TF-IDF

        }

    }

    Rank *rank = new Rank();

    for (Rank_Temp::iterator j = temp->begin(); j != temp->end(); ++j) {
        ReturnDoc &doc = (*docs_temp)[j->first];
//        rank->emplace(j->second / ((1 - b) + b * (doc.length / avg_doclen)), doc.url);
        rank->emplace(j->second / larger, doc.url);
    }
    delete docs_temp;

//    int max_print = 8;
//    for (Rank::reverse_iterator it = rank->rbegin(); it != rank->rend(); ++it) {
//        printf(" %lf - %s\n", it->first, it->second.c_str());
//        if (--max_print <= 0) break;
//    }

//    printf(" Total documents: %llu\n", temp->size());
//    printf(" Total seeks: %d\n", seeks_total);

    delete temp;

    CreateJson(*rank, json);

    delete rank;

}


void Searcher::BM25(list <string> &query_terms, JSON_Temp &json) {
    typedef unordered_map<int, ReturnDoc> Docs_Temp;
// BM25 Variables
    double k1 = 1.2;
    double b = 0.75;
    double avg_doclen = Docs_avg;


    Rank_Temp *temp = new Rank_Temp();
    Docs_Temp *docs_temp = new Docs_Temp();

    int seeks_total = 0;

    for (const string &query_term : query_terms) {
        QTriples term_occs;
        seeks_total += GetTermOccs(term_occs, query_term);

//        double IDF = log2(1 + ((Docs->size() - term_occs.size() + 0.5) / (term_occs.size() + 0.5)));
//        double IDF = log2((Docs->size() + 0.5) / (double) (term_occs.size() + 0.5)); //IDF
        double IDF = log2((Docs->size() - term_occs.size() + 0.5) / (term_occs.size() + 0.5)); //IDF


        for (QTriples::iterator iter = term_occs.begin(); iter != term_occs.end(); ++iter) {
            ReturnDoc &doc = (*docs_temp)[iter->first];
            if(doc.length == 0){
                ReturnDoc doc_tmp = GetDocument(iter->first);
                doc.length = doc_tmp.length;
                doc.url = doc_tmp.url;
            }
            QTriple occ = iter->second;

            double TF_doc = occ.term_frequency;

            double norm = (k1 * ((1 - b) + b * (doc.length / avg_doclen))) + TF_doc;

            double BIJ = ((k1 + 1) * TF_doc) / norm;

            (*temp)[iter->first] += (BIJ * IDF);

        }

    }

    Rank *rank = new Rank();

    for (Rank_Temp::iterator j = temp->begin(); j != temp->end(); ++j) {
        ReturnDoc &doc = (*docs_temp)[j->first];
        rank->emplace(j->second, doc.url);
    }
    delete docs_temp;

//    int max_print = 8;
//    for (Rank::reverse_iterator it = rank->rbegin(); it != rank->rend(); ++it) {
//        printf(" %lf - %s\n", it->first, it->second.c_str());
//        if (--max_print <= 0) break;
//    }

//    printf(" Total documents: %llu\n", temp->size());
//    printf(" Total seeks: %d\n", seeks_total);

    delete temp;

    CreateJson(*rank, json);

    delete rank;

}

std::string Searcher::Search(const std::string &query, const std::string &model, int max) {
    JSON_Trunc = max;
    list <string> terms;

    char_separator<char> sep_space_html("%20");
    tokenizer<char_separator<char>> tokens(query, sep_space_html);
    for (const string &term : tokens) {
        terms.push_back(term);
    }
    terms.unique();

    JSON_Temp json_result;

    QTime qTime;
    qTime.Start();

    if (model == "bm25") {
        BM25(terms, json_result);
    } else {
        Vector(terms, json_result);
    }

    double time = qTime.Get_Time_Sec();
//    printf(" Total time: %f sec\n", time);

    json_result << "\"secs\":" << time << '}';

    return json_result.str();
}

void Searcher::CreateJson(Rank &rank, JSON_Temp &json) {
    json << "{\"docs\":" << rank.size() << ',';
    json << "\"result\":[";

    int count = 0;
    Rank::reverse_iterator it = rank.rbegin();
    while (it != rank.rend()) {
        json << "{\"" << it->first << "\":\"" << base64_encode(it->second) << "\"}";
        if (JSON_Trunc > 0 && ++count == JSON_Trunc) break;
        if (++it != rank.rend()) json << ',';
    }

    json << "],";
}

std::string Searcher::base64_encode(const std::string& s) {
    const std::string base64_padding[] = {"", "==","="};

    namespace bai = boost::archive::iterators;

    std::stringstream os;

    // convert binary values to base64 characters
    typedef bai::base64_from_binary
    // retrieve 6 bit integers from a sequence of 8 bit bytes
    <bai::transform_width<const char *, 6, 8> > base64_enc; // compose all the above operations in to a new iterator

    std::copy(base64_enc(s.c_str()), base64_enc(s.c_str() + s.size()),
              std::ostream_iterator<char>(os));

    os << base64_padding[s.size() % 3];
    return os.str();
}