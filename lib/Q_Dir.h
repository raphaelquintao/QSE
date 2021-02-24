#ifndef QSE_Q_DIR_H
#define QSE_Q_DIR_H

#include <string>
#include <list>

class Q_Dir {

public:
    typedef struct Q_File {
        std::string path;
        std::string name;
        std::string extension;

    } Q_File;

    typedef std::list<Q_File> Q_FileList;


    Q_FileList *Read_Dir(std::string base_dir);

    Q_FileList *Read_Dir_Recursive(std::string base_dir);

private:
    void Read_Recursive(const char *base_dir, Q_FileList *files);
};


#endif //QSE_Q_DIR_H
