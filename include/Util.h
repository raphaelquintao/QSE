#ifndef QSE_UTIL_H
#define QSE_UTIL_H

#include <vector>
#include <map>
#include <unordered_map>
#include <list>
#include <string>
#include <ctime>

#define P_SHORT_NAME "QSE"
#define P_NAME       "Q Search Engine"
#define P_ABOUT      "Raphael Quintao - https://github.com/raphaelquintao"
#define P_VERSION    "0.5"

typedef struct IndexStruct {
    std::string index_dir;
    std::string index_name;
    std::string idx_file_name;
    std::string voc_file_name;
    std::string doc_file_name;
    std::string idx_path;
    std::string voc_path;
    std::string doc_path;

    std::string doc_temp_file_name;
    std::string doc_temp_path;

    void Gen() {
        doc_temp_file_name = "Q_DOC.tmp";
        doc_temp_path = index_dir + doc_temp_file_name;

        idx_file_name = index_name + ".q_IDX";
        voc_file_name = index_name + ".q_VOC";
        doc_file_name = index_name + ".q_DOC";

        idx_path = index_dir + idx_file_name;
        voc_path = index_dir + voc_file_name;
        doc_path = index_dir + doc_file_name;
    }
} IndexStruct;

class QTime {
    clock_t begin = 0, end = 0;
    double elapsed_sec = 0;

public:

    void Start() {
        begin = clock();
        end = 0;
    }

    void Stop(){
        end = clock();
        elapsed_sec = double(end - begin) / CLOCKS_PER_SEC;
    }

    double Get_Time_Sec(){
        if(end == 0) Stop();
        return elapsed_sec;
    }
};


typedef int QDoc_ID;

typedef struct Occurrence {
    int doc_id;
    int term_frequency;
    std::list<int> occ_pos;
} Occurrence, QTriple;

typedef std::unordered_map<QDoc_ID, QTriple> TermOccs, QTriples;




typedef struct VocEntry {
    std::vector<long long> OccsOffsets;
} VocItem;

typedef std::unordered_map<std::string, VocItem> QVoc;

typedef std::vector<long long> QDoc;


#endif //QSE_UTIL_H
