#ifndef QSE_PARSER_H
#define QSE_PARSER_H

#include <string>
#include <tidy.h>


class Parser {
private:
    bool use_tidy;

    /**
     * Parse HTML and extract text using HTMLCXX
     * - http://sourceforge.net/projects/htmlcxx/
     *
     * @param html String with raw html
     */
    std::string Parse_Html_CXX(const std::string &html);

    /**
     * Recursive traverse the TIDY parser tree and extract text.
     *
     * @param doc TidyDoc Source
     * @param tnode TidyNode Root
     * @param buffer std::stringstream to store text.
     */
    void TIDY_ExtractText(TidyDoc doc, TidyNode tnode, std::stringstream &buffer);

    /**
     * Repair and Parse HTML using Tidy-HTML5
     * - https://github.com/htacg/tidy-html5
     *
     * @param html String with raw html
     */
    std::string Parse_Html_TIDY(const std::string &html);

public:
    ~Parser();

    Parser();

    Parser(bool use_tidy);

    /**
     * @brief Remove, replace and lower case string characters.
     * @param text String with raw text
     */
    std::string Clear_Text(const std::string &text);


    std::string HTML_Extract_Text(const std::string &html);
};


#endif //QSE_PARSER_H
