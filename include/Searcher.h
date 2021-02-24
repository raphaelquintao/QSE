#ifndef QSE_SEARCHER_H
#define QSE_SEARCHER_H

#include "IndexHandler.h"
#include <fstream>

class Searcher {
    typedef struct DocEntry {
        double length;
        std::string url;
    } ReturnDoc;

    typedef std::multimap<double, std::string> Rank;
    typedef std::stringstream JSON_Temp;
    int JSON_Trunc = 0;

    IndexHandler *Handler;
    QVoc *Voca;
    QDoc *Docs;
    double Docs_avg;

    std::ifstream reader_index;
    std::ifstream reader_doc;

    ReturnDoc GetDocument(int doc_id);

    TermOccs GetTermOcc(const std::string &term, int positions);

    int GetTermOccs(QTriples &triples, const std::string &term);

    std::string base64_encode(const std::string& s);

    void CreateJson(Rank &rank, JSON_Temp &json);

    void Vector(std::list<std::string> &query_terms, JSON_Temp &json);

    void BM25(std::list<std::string> &query_terms, JSON_Temp &json);

public:
    Searcher(IndexHandler *handler);


    std::string Search(const std::string &query, const std::string &model, int max);
};


#endif //QSE_SEARCHER_H
