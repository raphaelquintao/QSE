#include "Parser.h"
#include <sstream>
#include <htmlcxx/html/ParserDom.h>
#include <tidybuffio.h>

using namespace std;
//using namespace htmlcxx;

std::string Parser::Clear_Text(const std::string &text) {
//                        "ÀÁÂÃÄÅÆÇÈÉÊËÌÍÎÏÐÑÒÓÔÕÖ×ØÙÚÛÜÝÞßàáâãäåæçèéêëìíîïðñòóôõö÷øùúûüýþÿ"  - 64 Chars
//  const char *garbage = "AAAAAAECEEEEIIIIDNOOOOOx0UUUUYPsaaaaaaeceeeeiiiiOnooooo 0uuuuypy";
    const char *garbage = "aaaaaaeceeeeiiiidnooooox0uuuuypsaaaaaaeceeeeiiiionooooo 0uuuuypy";

    string clean_text;
    for (int x = 0; x < text.size(); x++) {
        char c = text[x];
        if (c < 0) {
//                if (c != '\xEF' && c != '\xBB' && c != '\xBF') { // UTF-8 BOOM chars
            if (c != -61) {
                x++;
                continue;
            }
            clean_text.push_back(garbage[c + text[x + 1] + 189]);
            x++;
//                }
        } else if (c <= 8 || (c >= 11 && c <= 31) || c >= 126 || c == 39 || c == 94) {
            continue;
        } else if (c <= 10 || (c >= 32 && c <= 47) || (c >= 58 && c <= 64) || (c >= 91 && c <= 96) ||
                   (c >= 123 && c <= 127)) {
            clean_text.push_back(char(32));
        } else {
            clean_text.push_back((char) std::tolower(c));
        }

    }
    return clean_text;
}

std::string Parser::Parse_Html_CXX(const std::string &html) {
    stringstream text_raw;

    htmlcxx::HTML::ParserDom parser_dom;

    tree<htmlcxx::HTML::Node> dom = parser_dom.parseTree(html);

    tree<htmlcxx::HTML::Node>::iterator it = dom.begin();
    tree<htmlcxx::HTML::Node>::iterator end = dom.end();

    for (; it != end; ++it) {
        if (!strcasecmp(it->tagName().c_str(), "html")) {
            text_raw.str(" ");
        }
        if (strcasecmp(it->tagName().c_str(), "script") == 0 ||
            strcasecmp(it->tagName().c_str(), "style") == 0) {
            it++;
            if (it == end) break;
        } else if ((!it->isTag()) && (!it->isComment())) {
            text_raw << it->text() << (char) (32);
        }
    }

    return Clear_Text(text_raw.str());

}

void Parser::TIDY_ExtractText(TidyDoc doc, TidyNode tnode, std::stringstream &buffer) {
    TidyNode child;
    for (child = tidyGetChild(tnode); child; child = tidyGetNext(child)) {
        ctmbstr name = tidyNodeGetName(child);
        if (!name) {
            TidyBuffer buf;
            tidyBufInit(&buf);
            tidyNodeGetText(doc, child, &buf);
            buffer << buf.bp << ' ';
//                printf("%s", buf.bp);
            tidyBufFree(&buf);
        } else if (!tidyNodeIsSCRIPT(child) && !tidyNodeIsNOSCRIPT(child) && !tidyNodeIsSTYLE(child))
            TIDY_ExtractText(doc, child, buffer);
    }
}

std::string Parser::Parse_Html_TIDY(const std::string &html) {
    const char *input = html.c_str();
    TidyBuffer output = {0};
    TidyBuffer errbuf = {0};

    int rc = 0;
    Bool ok;

    TidyDoc tdoc = tidyCreate();

    ok = tidyOptSetBool(tdoc, TidyHtmlOut, yes);
    ok = tidyOptSetBool(tdoc, TidyEncloseBodyText, yes);
    ok = tidyOptSetBool(tdoc, TidyDropEmptyElems, yes);
    ok = tidyOptSetBool(tdoc, TidyHideComments, yes);
    ok = tidyOptSetBool(tdoc, TidyForceOutput, yes);
    tidySetOutCharEncoding(tdoc, "utf8");

    if (ok)
        rc = tidySetErrorBuffer(tdoc, &errbuf);      // Capture diagnostics
    if (rc >= 0)
        rc = tidyParseString(tdoc, input);           // Parse the input
    if (rc >= 0)
        rc = tidyCleanAndRepair(tdoc);               // Tidy it up!
//        if (rc >= 0)
//            rc = tidyRunDiagnostics(tdoc);               // Kvetch
//        if (rc > 1)                                    // If error, force output.
//            rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
    if (rc >= 0)
        rc = tidySaveBuffer(tdoc, &output);          // Pretty Print
    if (rc >= 0) {
//            if (rc > 0) printf("\nDiagnostics:\n\n%s", errbuf.bp);
    } else printf("A severe error (%d) occurred.\n", rc);

//        TidyNode remove = tidyGetBody(tdoc);
//        remove = tidyGetChild(remove);
//        tidyDiscardElement(tdoc, remove);


    stringstream result;
    TIDY_ExtractText(tdoc, tidyGetRoot(tdoc), result);


    tidyBufFree(&output);
    tidyBufFree(&errbuf);
    tidyRelease(tdoc);

    return Clear_Text(result.str());

//    string clean = Clear_Text(result.str());
//
//    char_separator<char> sep(" ");
//    tokenizer<char_separator<char>> tokens(clean, sep);
//    int pos = 0;
//    for (const string &term : tokens) {
//        pos++;
//        Insert_in_Buffer(term, pos);
//    }

}

Parser::~Parser() {

}

Parser::Parser() {
    this->use_tidy = false;
}

Parser::Parser(bool use_tidy) {
    this->use_tidy = use_tidy;
}


std::string Parser::HTML_Extract_Text(const std::string &html) {
    return (!use_tidy) ? Parse_Html_CXX(html) : Parse_Html_TIDY(html);
}
