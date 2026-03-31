#include <iostream>
#include <string>
#include <vector>
#include "token.h"

using namespace std;

// Grammar / parser
extern void initializeGrammar();
extern void computeFirst();
extern void computeFOLLOW();
extern void constructItemSets();
extern void buildParsingTable();
extern void printParsingTables();
extern void parseInput(vector<Token>);

// Lexer
extern vector<Token> tokenize(const string&);

// Symbol table
extern void enterScope();
extern void exitScope();
extern void printSymbolTable();

int main() {
    // Build grammar and parsing tables
    initializeGrammar();
    computeFirst();
    computeFOLLOW();
    constructItemSets();
    buildParsingTable();

    // Print parsing tables before processing input
    printParsingTables();

    // Read input program (blank line to finish)
    cout << "\nEnter program (blank line to finish):\n";
    string input, line;
    while (getline(cin, line)) {
        if (line.empty()) break;
        input += line + "\n";
    }

    // Tokenize with location tracking
    vector<Token> tokens = tokenize(input);

    // Open global scope, parse, close
    enterScope();
    parseInput(tokens);
    printSymbolTable();
    exitScope();

    return 0;
}
