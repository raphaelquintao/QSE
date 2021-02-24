#ifndef QSE_INDEXER_H
#define QSE_INDEXER_H

#include "Util.h"
#include "Parser.h"
#include "IndexHandler.h"
#include "Q_Buffer_Voc.h"
#include <fstream>


class Indexer {
private:
    IndexStruct *IndexFiles;
    Parser *Parser_HTML;

    Q_Buffer_Voc *BufferVoc;

    std::string processing_now;

    int doc_terms_num = 0;
    long long doc_terms_sum = 0;
    long long doc_count = 0;

    long long flush_count = 0;


    long long bytes_total = 0;
    long long bytes_writed = 0;
    long long bytes_readed = 0;
    long long max_bytes = 1048576; //1mb - 2^20 - 1024^2

    int max_triples;
    int max_triples_per_term;

    std::ofstream writer_index; //Format: doc_id, term_frequency, [occurrence position]+ |
    std::ofstream writer_vocab; //Format: term | [offset in index file]+
    std::ofstream writer_doc;   //Format: doc_count, doc_name
    std::ifstream reader_file;


    void Parse_HTML(const std::string &html);

//    void Flush_Critical();

    void Flush_Index(bool clear);

    void Flush_Doc(const std::string &doc_name);

    void Flush_Voc();

    void Finish();


    void ProcessFile(const std::string &file_path, const std::string &file_name);

    void ProcessFileAlt(const std::string &file_path, const std::string &file_name, long long offset_beg, long long offset_end, int uncomp_size);

public:

    ~Indexer();

    Indexer(IndexHandler *index_handler);

    void SetFlushRate(int megabytes);


    void IndexDir(std::string base_dir, int recursive);

    void IndexAlt(std::string base_dir, int max_files);

};


#endif //QSE_INDEXER_H
