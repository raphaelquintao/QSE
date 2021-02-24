#include "IndexHandler.h"
#include <boost/tokenizer.hpp>
#include <fstream>
#include <iostream>

using namespace std;
using namespace boost;

void IndexHandler::LoadVocb(ifstream &reader) {
    long long lines = 0;
    long long lines2 = 0;
    string entry;
    char_separator<char> sep(",");
    while (getline(reader, entry, '\n')) {
        lines++;
        tokenizer<char_separator<char>> tokens(entry, sep);
        VocItem *item = nullptr;
        vector<long long> *occs = nullptr;
        for (const string &term : tokens) {
            if (item == nullptr) {
                lines2++;
                item = &(*Voca)[term];
            } else if (occs == nullptr) {
                occs = &item->OccsOffsets;
//                    item->DF = atoll(term.c_str());
                occs->push_back(atoll(term.c_str()));
            } else occs->push_back(atoll(term.c_str()));
        }
//            if (lines2 == 1)
//                cout << "ovo " << lines << "\n";
    }
//        vector<int> busca = (*Voca)["quintao"];
//        cout << busca.size() << endl;
//        cout << "Vocabulary file lines: " << lines << " - " << lines2 << endl;
//        cout << "Vocabulary Entries: " << Voca->size() << endl;
    reader.close();
}

void IndexHandler::LoadDocs(ifstream &reader) {
    string entry;
    long long offset = 0;
    short skip = 0;
    while (getline(reader, entry)) {
        if (skip++) {
            Docs->push_back(offset);
            offset = reader.tellg();
            skip = 0;
        }
    }

    reader.clear();
    reader.seekg(offset);
    getline(reader, entry);
    DocsAvgLength = atof(entry.c_str());

//    cout << "Last Offset: " << entry << endl;

    reader.close();
}

IndexHandler::~IndexHandler() {
    delete Voca;
    delete Docs;
}

IndexHandler::IndexHandler(string index_name) {
    IndexFiles.index_dir = "_data/";
    IndexFiles.index_name = index_name;
    IndexFiles.Gen();

    Voca = new QVoc();
    Docs = new QDoc();
    Docs->reserve(10000);
}

void IndexHandler::LoadIndex() {
    std::ifstream reader_voc(IndexFiles.voc_path, std::ifstream::binary);
    std::ifstream reader_doc(IndexFiles.doc_path, std::ifstream::binary);

    bool fail = false;

    if (!reader_voc.is_open()) {
        fail = true;
        cout << "Can't find Vocabulary!" << endl;
    }
    if (!reader_doc.is_open()) {
        fail = true;
        cout << "Can't find Documents!" << endl;
    }
    if (!fail) {
        cout << " Loading index..." << endl;

        clock_t begin = clock();
        LoadVocb(reader_voc);
        LoadDocs(reader_doc);
        clock_t end = clock();

        double elapsed_sec = double(end - begin) / CLOCKS_PER_SEC;
        string time = "sec";
        if (elapsed_sec > 60) {
            elapsed_sec = elapsed_sec / 60;
            time = "min";
        }
        printf(" Loading done in %lf %s \n", elapsed_sec, time.c_str());
    } else {
        cout << "\n PRESS ENTER TO EXIT" << endl;
        exit(EXIT_FAILURE);
    }


}

void IndexHandler::ReloadIndex() {
    if(Voca->size() > 0) {
        delete Voca;
        Voca = new QVoc();
    }
    if(Docs->size() > 0) {
        delete Docs;
        Docs = new QDoc();
    }

    LoadIndex();
}

void IndexHandler::Print_Info() {
    cout << "   * Vocabulary Entries: " << Voca->size() << endl;
    cout << "   * Documents  Entries: " << Docs->size() << endl;
    cout << "   * Documents AvgTerms: " << DocsAvgLength << endl << endl;
}

void IndexHandler::Print_Vocab() {
//    for (QVoc::iterator i = Voca->begin(); i != Voca->end(); ++i) {
//        vector<int> occ = i->second;
////            cout << i->first << " - " << occ.qt_int() << " - ";
//        cout << i->first << " - ";
//        for (vector<int>::iterator j = occ.begin(); j != occ.end(); ++j) {
////                cout << *j;
//            ReadOccurrence(*j);
//            if ((j + 1) != occ.end()) cout << ", ";
//        }
//        cout << endl;
//    }
}

void IndexHandler::Print_DocsID() {
    for (int c = 0; c < Docs->size(); c++) {
        printf(" %d - %lli ", c, Docs->at(c));
    }
}