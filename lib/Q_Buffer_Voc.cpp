#include "Q_Buffer_Voc.h"

Q_Triples::~Q_Triples() {
    if (triple_map != nullptr)
        delete triple_map;
}

Q_Triples::Q_Triples() {
    triple_map = new Triples_Map();
}

void Q_Triples::Insert_Term_Position(Q_DocID doc_id, int position) {
    Q_Positions &positions = (*triple_map)[doc_id];
    positions.push_back(position);
}

void Q_Triples::Flush(std::ofstream &writer) {
    for (Triples_Map::iterator it_triple = triple_map->begin(); it_triple != triple_map->end(); ++it_triple) {
        if (it_triple != triple_map->begin()) writer << '|';

//            const Q_DocID &doc_id = it_triple->first;
        Q_Positions *positions = &it_triple->second;

        writer << it_triple->first << ',' << (*positions).size(); // doc_id, term_frequency

        for (Q_Positions::iterator it_pos = positions->begin(); it_pos != positions->end(); ++it_pos) {
            writer << ',' << *it_pos;
        }

    }
    delete triple_map;
    triple_map = new Triples_Map();
}

int Q_Triples::size() { return (int) triple_map->size(); }

list_long &Q_Term::Offsets() { return *term_offsets; }

Q_Term::~Q_Term() {
    if (term_occs != nullptr)
        delete term_occs;
    if (term_offsets != nullptr)
        delete term_offsets;
}

Q_Term::Q_Term() {
    term_occs = new Q_Triples();
    term_offsets = new list_long();
}

void Q_Term::Insert_Occ(int doc_id, int position) {
    term_occs->Insert_Term_Position(doc_id, position);

}

void Q_Term::Flush_Triples(std::ofstream &writer) {
    if (term_occs->size() > 0) {
        long long offset = writer.tellp();
        term_occs->Flush(writer);
        writer << '\n';
        term_offsets->push_back(offset);
        writer.flush();
    }
}

Q_Buffer_Voc::iterator Q_Buffer_Voc::begin() { return buffer_map->begin(); }

Q_Buffer_Voc::iterator Q_Buffer_Voc::end() { return buffer_map->end(); }

Q_Buffer_Voc::~Q_Buffer_Voc() {
    if (buffer_map != nullptr)
        delete buffer_map;
}

Q_Buffer_Voc::Q_Buffer_Voc() {
    buffer_map = new Hash_Buffer();

}

void Q_Buffer_Voc::Insert_Term(const std::string &term_str, int doc_id, int position) {
    Q_Term &term = (*buffer_map)[term_str];
    term.Insert_Occ(doc_id, position);

}