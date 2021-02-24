#ifndef QSE_INDEXHANDLER_H
#define QSE_INDEXHANDLER_H

#include "Util.h"

class IndexHandler {
private:
    void LoadVocb(std::ifstream &reader);

    void LoadDocs(std::ifstream &reader);

public:
    IndexStruct IndexFiles;
    QVoc *Voca;
    QDoc *Docs;
    double DocsAvgLength = 0.0;


    ~IndexHandler();

    IndexHandler(std::string index_name);


    void LoadIndex();

    void ReloadIndex();


    void Print_Info();

    void Print_Vocab();

    void Print_DocsID();

};

#endif //QSE_INDEXHANDLER_H
