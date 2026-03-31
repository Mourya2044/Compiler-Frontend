#ifndef TOKEN_H
#define TOKEN_H

#include <string>

using namespace std;

struct Token {
    string type;      // Token type (e.g., "id", "int", "main", "+", etc.)
    string lexeme;    // Actual text of the token
    int line;         // Line number (1-indexed)
    int col;          // Column number (1-indexed)
    
    Token() : type(""), lexeme(""), line(0), col(0) {}
    
    Token(const string& t, const string& l, int ln, int c) 
        : type(t), lexeme(l), line(ln), col(c) {}
    
    string location() const {
        return to_string(line) + ":" + to_string(col);
    }
};

#endif
