#include <iostream>
#include <getopt.h>
#include "IndexHandler.h"
#include "Indexer.h"


using namespace std;

void Usage() {
    printf(" Usage: %s-%s -d foo/bar/ -n corpus [-s] [-a] [-r] [-t] [-v] [-h] \n", P_SHORT_NAME, "Indexer");
    printf("  Required\n");
    printf("    -d  : Collection base directory.\n");
    printf("    -n  : Database name.\n");
    printf("    -s  : Max raw data read before flush, default 15MB.\n");
    printf("  Optional\n");
    printf("    -a  : Idicates that directory contains raw html files,\n"
                   "          do not set this option means that directory contains\n"
                   "          data in binary file and contains index.txt as reference.\n");
    printf("    -r  : Read collection recursively, use only if using -a .\n");
    printf("    -t  : Use Tidy as HTML parser, drastically increase time and memory usage.\n"
                   "          Default HTMLCXX, faster but worse.\n");
    printf("    -v  : Version and About.\n");
    printf("    -h  : This Help.\n");
}

static struct option long_options[] = {
        {"directory", required_argument, 0, 'd'},
        {"name",      required_argument, 0, 'n'},
        {"size",      optional_argument, 0, 's'},
        {"raw",       no_argument,       0, 'a'},
        {"recursive", no_argument,       0, 'r'},
        {"tidy",      no_argument,       0, 't'},
        {"version",   no_argument,       0, 'v'},
        {"help",      no_argument,       0, 'h'},
        {0,           0,                 0, 0}
};


int main(int argc, char *argv[]) {
//    printf(" %s - %s %s \n\n", P_SHORT_NAME, P_NAME, P_VERSION);

    int option = 0, option_index = 0, o_raw = 0, o_recursive = 0, o_tidy = 0, o_size = 15;
    string base_dir, base_name = "index";


    opterr = 0;
    while ((option = getopt_long(argc, argv, "d:n:s:artvh", long_options, &option_index)) != -1) {
        switch (option) {
            case 0:
                break;
            case 'd':
                if (optarg) base_dir = optarg;
//                printf("     Base dir '%s'.\n\n", base_dir.c_str());
                break;
            case 'n':
                if (optarg) base_name = optarg;
//                printf("     Base name '%s'.\n\n", base_name.c_str());
                break;
            case 's':
                if (optarg) o_size = atoi(optarg);
                o_size = (o_size < 10) ? 10 : o_size;
//                printf("     Flush Rate '%d'.\n\n", o_size);
                break;
            case 'a':
                o_raw = 1;
                break;
            case 'r':
                o_recursive = 1;
                break;
            case 't':
                o_tidy = 1;
                break;
            case 'h':
                Usage();
                exit(EXIT_SUCCESS);
            case 'v':
                printf(" %s %s \n", P_NAME, P_VERSION);
                printf(" %s\n", P_ABOUT);
                exit(EXIT_SUCCESS);
            case '?':
                if (optopt == 'd' || optopt == 'n' || optopt == 's')
                    printf("   -> ERROR: missing argument to '-%c' \n", optopt);
                else
                    printf("   -> ERROR: invalid option '-%c' \n", optopt);
                exit(EXIT_FAILURE);
            default:
                exit(EXIT_FAILURE);
        }
    }

//    printf("%d\n", argc);
//    printf("%d\n", optind);

    if (optind == 1) {
        printf("\n    No option! Type -h to see all options available.\n");
        exit(EXIT_FAILURE);
    } else if (base_dir.empty()) {
        printf("\n    Base directory required! Type -h to see all options available.\n");
        exit(EXIT_FAILURE);
    } else {
        char last = base_dir.c_str()[base_dir.size() - 1];
        if (last != '\\' && last != '/')
            base_dir.push_back('/');
    }


    IndexHandler *Handler = new IndexHandler(base_name);
    Indexer *qIndexer = new Indexer(Handler);
    qIndexer->SetFlushRate(o_size);

    if (o_raw)
        qIndexer->IndexDir(base_dir, o_recursive);
    else
        qIndexer->IndexAlt(base_dir, 0);

    delete qIndexer;

    Handler->ReloadIndex();
    Handler->Print_Info();

//    getchar();

    return 0;
}