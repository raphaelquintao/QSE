#include <iostream>
#include "IndexHandler.h"
#include "Searcher.h"
#include "WebServer.h"

using namespace std;

int main(int argc, char *argv[]) {
    printf(" %s - %s %s \n\n", P_SHORT_NAME, P_NAME, P_VERSION);

    string base = "index";
    string port = "8000";

    if (argc < 2 || argc > 3) {
        printf(" Usage: %s port base_name \n", argv[0]);
        exit(EXIT_FAILURE);
    } else if (argc == 2) {
        port = argv[1];
    } else if (argc == 3) {
        port = argv[1];
        base = argv[2];
    }


    IndexHandler *Index_Handler = new IndexHandler(base);
    Index_Handler->LoadIndex();
    Index_Handler->Print_Info();

    Searcher *Index_Searcher = new Searcher(Index_Handler);

    WebServer Server(Index_Searcher);
    Server.Start(port);


    return 0;
}

