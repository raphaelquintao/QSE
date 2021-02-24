#ifndef QSE_Q_BUFFER_VOC_H
#define QSE_Q_BUFFER_VOC_H

#include <list>
#include <vector>
#include <unordered_map>
#include <map>
#include <fstream>


typedef int Q_DocID;

class Q_Triples {
    typedef std::vector<int> Q_Positions;
    typedef std::unordered_map<Q_DocID, Q_Positions> Triples_Map;

    Triples_Map *triple_map;

public:
    ~Q_Triples();

    Q_Triples();

    void Insert_Term_Position(Q_DocID doc_id, int position);

    void Flush(std::ofstream &writer);

    int size();
};

//---------------------------------------------------------------------

typedef std::list<long long> list_long;

class Q_Term {
    Q_Triples *term_occs;
    list_long *term_offsets;

public:

    list_long &Offsets();;

    ~Q_Term();

    Q_Term();

    void Insert_Occ(int doc_id, int position);

    void Flush_Triples(std::ofstream &writer);

};


class Q_Buffer_Voc {
    typedef std::unordered_map<std::string, Q_Term> Hash_Buffer;

    Hash_Buffer *buffer_map;

public:
    typedef Hash_Buffer::iterator iterator;

    iterator begin();

    iterator end();

    ~Q_Buffer_Voc();

    Q_Buffer_Voc();

    void Insert_Term(const std::string &term_str, int doc_id, int position);

};


#endif //QSE_Q_BUFFER_VOC_H
