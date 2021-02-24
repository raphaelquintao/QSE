#include "Q_Dir.h"
#include "tinydir.h"


Q_Dir::Q_FileList *Q_Dir::Read_Dir(std::string base_dir) {
    Q_FileList *files = new Q_Dir::Q_FileList();

    char last = base_dir.c_str()[base_dir.size() - 1];
    if (last == '\\' || last == '/') base_dir.pop_back();

    tinydir_dir dir;
    tinydir_open(&dir, base_dir.c_str());
    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (!file.is_dir) {
            Q_File qFile;
            qFile.path = file.path;
            qFile.name = file.name;
            qFile.extension = file.extension;
            files->push_back(qFile);
        }
        tinydir_next(&dir);
    }
    tinydir_close(&dir);

    return files;
}

void Q_Dir::Read_Recursive(const char *base_dir, Q_FileList *files) {
    tinydir_dir dir;
    tinydir_open(&dir, base_dir);
    while (dir.has_next) {
        tinydir_file file;
        tinydir_readfile(&dir, &file);

        if (file.is_dir) {
            if ((strcmp(file.name, ".") != 0) && (strcmp(file.name, "..") != 0))
                Read_Recursive(file.path, files);
        } else {
            Q_File qFile;
            qFile.path = file.path;
            qFile.name = file.name;
            qFile.extension = file.extension;
            files->push_back(qFile);
        }

        tinydir_next(&dir);
    }
    tinydir_close(&dir);
}

Q_Dir::Q_FileList *Q_Dir::Read_Dir_Recursive(std::string base_dir) {
    Q_FileList *files = new Q_FileList();

    char last = base_dir.c_str()[base_dir.size() - 1];
    if (last == '\\' || last == '/') base_dir.pop_back();

    Read_Recursive(base_dir.c_str(), files);
}
