#include "Indexer.h"
#include <iostream>
#include <sstream>
#include <boost/tokenizer.hpp>
#include "Q_Dir.h"
#include <zlib.h>

//#define MAX_TRIPLE          2048
//#define MAX_TOTAL_TRIPLE    2048
//#define MAX_POSITION        128

using namespace std;
using namespace boost;

void Indexer::Parse_HTML(const string &html) {
    bytes_total += html.size();
    bytes_readed += html.size();

    string text = Parser_HTML->HTML_Extract_Text(html);

    char_separator<char> sep_space(" ");
    tokenizer<char_separator<char>> tokens(text, sep_space);

    int term_count = 0;
    for (const string &term : tokens) {
        term_count++;
        BufferVoc->Insert_Term(term, (int) doc_count, term_count);
    }
    doc_terms_num = term_count;

}

void Indexer::Flush_Index(bool clear) {
    if ((bytes_readed >= max_bytes) || clear) {
        flush_count++;

        for (Q_Buffer_Voc::iterator it_term = BufferVoc->begin(); it_term != BufferVoc->end(); ++it_term) {
            Q_Term &term = it_term->second;
            term.Flush_Triples(writer_index);
        }

        bytes_writed = writer_index.tellp();
        bytes_readed = 0;

    }

}

void Indexer::Flush_Doc(const string &doc_name) {
    Flush_Index(false);

    writer_doc << doc_terms_num << '\n' << doc_name << '\n';

    doc_terms_sum += doc_terms_num;
    doc_terms_num = 0;
}

void Indexer::Flush_Voc() {
    writer_vocab.open(IndexFiles->voc_path, ofstream::binary);

    if (!writer_vocab.is_open()) {
        printf("   -> ERROR: Can't open voc file!\n");
        exit(EXIT_FAILURE);
    }

    for (Q_Buffer_Voc::iterator i = BufferVoc->begin(); i != BufferVoc->end(); ++i) {
        Q_Term &item = i->second;
        list_long &offsets = item.Offsets();

        writer_vocab << i->first << ',';

        for (list_long::iterator j = offsets.begin(); j != offsets.end(); ++j) {
            if (j != offsets.begin()) writer_vocab << ',';
            writer_vocab << *j;
        }
        writer_vocab << '\n';
    }

    writer_vocab.flush();
    writer_vocab.close();
}

void Indexer::Finish() {
    double DocsAvgLength = doc_terms_sum / (double) doc_count;
    writer_doc << DocsAvgLength << '\n';
    writer_doc.flush();

    Flush_Index(true);
    Flush_Voc();

    printf("   * Raw Data Readed: %f MB\n", (double) bytes_total / 1048576);
    printf("   * Index Size: %f MB\n", (double) bytes_writed / 1000000);
    printf("   * Flush Count: %lli!\n", flush_count);

    writer_index.close();
    writer_doc.close();
    reader_file.close();

    delete Parser_HTML;
    delete BufferVoc;

    printf("\n");
}

Indexer::~Indexer() {
    Finish();
}

Indexer::Indexer(IndexHandler *index_handler) {
    IndexFiles = &index_handler->IndexFiles;

    Parser_HTML = new Parser();

    BufferVoc = new Q_Buffer_Voc();

    doc_count = index_handler->Docs->size();

//#ifndef _WIN32
//    tinydir_dir dir;
//        tinydir_open(&dir, IndexFiles->index_dir.c_str());
//        if (!dir.has_next) {
//            printf("   -> WARNING: Directory %s dont exists.\n", IndexFiles->index_dir.c_str());
//            printf("   * Creating directory... ");
//            if (mkdir(IndexFiles->index_dir.c_str(), S_IRWXU | S_IRWXG | S_IROTH | S_IXOTH)) {
//                printf("\n    ->ERROR: Cant't create!\n");
//                exit(EXIT_FAILURE);
//            } else printf("Successful!\n");
//        }
//        tinydir_close(&dir);
//#endif
//
    writer_index.open(IndexFiles->idx_path, ofstream::binary);
    if (!writer_index.is_open()) {
        printf("   -> ERROR: Can't create %s file!\n", IndexFiles->idx_path.c_str());
        exit(EXIT_FAILURE);
    }

    writer_doc.open(IndexFiles->doc_path, ofstream::binary);
    if (!writer_doc.is_open()) {
        printf("   -> ERROR: Can't open docs file!");
        exit(EXIT_FAILURE);
    }


}

void Indexer::SetFlushRate(int megabytes) {
    if (megabytes > 1) max_bytes = megabytes * max_bytes;
}

void Indexer::ProcessFile(const string &file_path, const string &file_name) {
    if (reader_file.is_open()) {
        reader_file.close();
        reader_file.clear();
    }
    reader_file.open(file_path.c_str(), std::ifstream::binary);

    if (reader_file.is_open()) {
        stringstream temp;
        temp << reader_file.rdbuf();
        Parse_HTML(temp.str());
        doc_count++;
        Flush_Doc(file_path);
    } else {
        printf("   -> ERROR: Can't open file: %s \n", file_path.c_str());
    }
}

void Indexer::ProcessFileAlt(const string &file_path, const string &file_name, long long offset_beg, long long offset_end, int uncomp_size) {
    if (processing_now != file_path) {
        if (reader_file.is_open()) reader_file.close();
        reader_file.open(file_path.c_str(), std::ifstream::binary);
        processing_now = file_path;
        if (reader_file.is_open()) {
            printf("   -> Processing now: %s \n", file_path.c_str());
        } else {
            printf("   -> ERROR: Can't open file: %s \n", file_path.c_str());
            return;
        }
    }
    long long size = (offset_end - offset_beg);
    if (size > 0) {
        char *buffer = new char[size];
        reader_file.seekg(offset_beg);
        reader_file.read(buffer, size);
        unsigned char *buffer_uncompressed;
        if (uncomp_size > 0) {
            buffer_uncompressed = new unsigned char[uncomp_size + 1];
            uncompress(buffer_uncompressed, (uLongf *) &uncomp_size, (const Bytef *) buffer, (uLong) size);
            buffer_uncompressed[uncomp_size] = '\0';
//            cout << buffer_uncompressed;
//            string temp = (const char *) buffer_uncompressed;
            Parse_HTML((const char *) buffer_uncompressed);
            delete buffer_uncompressed;
//            exit(EXIT_SUCCESS);

        } else {
            Parse_HTML(buffer);
        }
        delete[] buffer;
        doc_count++;
        Flush_Doc(file_name);
    } else {
        printf("   -> ERROR: File has zero bytes: %s \n", file_path.c_str());
    }
}

void Indexer::IndexDir(string base_dir, int recursive) {
    Q_Dir qDir;
    Q_Dir::Q_FileList *qfiles;
    qfiles = (!recursive) ? qDir.Read_Dir(base_dir) : qDir.Read_Dir_Recursive(base_dir);

    printf(" Indexing %zu files... \n", qfiles->size());

    clock_t begin = clock();

    for (Q_Dir::Q_FileList::iterator i = qfiles->begin(); i != qfiles->end(); i++) {
        ProcessFile(i->path, i->name);
    }
    delete qfiles;

    clock_t end = clock();

    double elapsed_sec = double(end - begin) / CLOCKS_PER_SEC;
    string time = "sec";
    if (elapsed_sec > 60) {
        elapsed_sec = elapsed_sec / 60;
        time = "min";
    }
    printf(" Indexing done in %lf %s \n", elapsed_sec, time.c_str());
}

void Indexer::IndexAlt(string base_dir, int max_files) {
    printf(" Indexing files... \n");

    clock_t begin = clock();

    string indice = "index.txt";

    ifstream file((base_dir + indice), ifstream::binary);
    string line;
    int lines = 0;
    char_separator<char> sep(" ");
    if (file.is_open()) {
        while (getline(file, line, '\n')) {
            lines++;
            std::string file_name, file_path = base_dir;
            long long offset_beg = 0, offset_end = 0;
            int uncomp_size = 0, control = 0;
            tokenizer<char_separator<char>> tokens(line, sep);
            for (const string &word : tokens) {
                if (control == 0) {
                    file_name = word;
                } else if (control == 1) {
                    file_path.append(word);
                } else if (control == 2) {
                    offset_beg = atoi(word.c_str());
                } else if (control == 3) {
                    offset_end = atoi(word.c_str());
                } else if (control == 4) {
                    uncomp_size = atoi(word.c_str());
                }
                control++;
            }

            ProcessFileAlt(file_path, file_name, offset_beg, offset_end, uncomp_size);

            if (max_files != 0 && lines == max_files) break;
        }

    } else cout << "  Error opening file: " << base_dir << indice << endl;
    file.close();


    clock_t end = clock();
    double elapsed_sec = double(end - begin) / CLOCKS_PER_SEC;
    string time = "sec";
    if (elapsed_sec > 60) {
        elapsed_sec = elapsed_sec / 60;
        time = "min";
    }
    printf(" Indexing done in %lf %s \n", elapsed_sec, time.c_str());

}
