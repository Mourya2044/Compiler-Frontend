#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>
#include "token.h"

using namespace std;

// ── Symbol table forward declarations (defined in symbol_table.cpp) ──────────
extern void enterScope();
extern void exitScope();
extern void insertSymbol(const string& name, const string& type);
extern bool lookupSymbol(const string& name);

static unordered_set<string> keywords = {"main", "int", "float", "for", "read"};

// Returns the token stream with location information
vector<Token> tokenize(const string& input) {
    vector<Token> tokens;
    int line = 1, col = 1;

    for (int i = 0; i < (int)input.size(); i++) {
        int startLine = line, startCol = col;

        // Skip whitespace and track line/column
        if (isspace((unsigned char)input[i])) {
            if (input[i] == '\n') {
                line++;
                col = 1;
            } else {
                col++;
            }
            continue;
        }

        // Skip single-line comments  //...
        if (i + 1 < (int)input.size() && input[i] == '/' && input[i+1] == '/') {
            col += 2;
            i += 2;
            while (i < (int)input.size() && input[i] != '\n') {
                col++;
                i++;
            }
            continue;
        }

        // Identifier / keyword
        if (isalpha((unsigned char)input[i]) || input[i] == '_') {
            string word;
            while (i < (int)input.size() &&
                   (isalnum((unsigned char)input[i]) || input[i] == '_')) {
                word += input[i++];
                col++;
            }
            i--;
            col--;
            
            if (keywords.count(word)) {
                tokens.push_back(Token(word, word, startLine, startCol));
            } else {
                tokens.push_back(Token("id", word, startLine, startCol));
            }
            continue;
        }

        // Integer or float literal
        if (isdigit((unsigned char)input[i])) {
            string num;
            while (i < (int)input.size() && isdigit((unsigned char)input[i])) {
                num += input[i++];
                col++;
            }
            if (i < (int)input.size() && input[i] == '.') {
                num += input[i++];
                col++;
                while (i < (int)input.size() && isdigit((unsigned char)input[i])) {
                    num += input[i++];
                    col++;
                }
            }
            i--;
            col--;
            tokens.push_back(Token("num", num, startLine, startCol));
            continue;
        }

        // Two-character operators — check before single-char
        if (i + 1 < (int)input.size()) {
            string two = {input[i], input[i+1]};
            if (two == "==" || two == "!=" || two == "++" || two == "--") {
                tokens.push_back(Token(two, two, startLine, startCol));
                col += 2;
                i++;
                continue;
            }
        }

        // Single-character tokens
        string single(1, input[i]);
        const string valid = "+-*/=<>(){};,";
        if (valid.find(input[i]) != string::npos) {
            tokens.push_back(Token(single, single, startLine, startCol));
            col++;
        } else {
            cerr << "Unrecognised character: '" << input[i] << "' at line " << startLine 
                 << ", column " << startCol << "\n";
        }
    }

    tokens.push_back(Token("$", "$", line, col));

    return tokens;
}
