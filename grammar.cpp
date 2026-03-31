#include <set>
#include <string>
#include <vector>

using namespace std;

struct Production {
    string lhs;
    vector<string> rhs;
};

vector<Production> grammar;
set<string> terminals;
set<string> nonterminals;
string startSymbol = "Program";

void initializeGrammar() {
    // Program structure
    grammar.push_back({"Program",    {"main", "(", ")", "{", "DeclList", "StmtList", "}"}});

    // Declarations
    grammar.push_back({"DeclList",   {"Decl", "DeclList"}});
    grammar.push_back({"DeclList",   {"epsilon"}});
    grammar.push_back({"Decl",       {"Type", "IdList", ";"}});
    grammar.push_back({"Type",       {"int"}});
    grammar.push_back({"Type",       {"float"}});
    grammar.push_back({"IdList",     {"id", ",", "IdList"}});
    grammar.push_back({"IdList",     {"id"}});

    // Statements
    grammar.push_back({"StmtList",   {"Stmt", "StmtList"}});
    grammar.push_back({"StmtList",   {"epsilon"}});
    grammar.push_back({"Stmt",       {"AssignStmt"}});
    grammar.push_back({"Stmt",       {"ReadStmt"}});
    grammar.push_back({"Stmt",       {"IncStmt"}});
    grammar.push_back({"Stmt",       {"ForStmt"}});
    grammar.push_back({"Stmt",       {"Decl"}});  // Allow declarations anywhere

    // Assignment:  id = Expr ;
    grammar.push_back({"AssignStmt", {"id", "=", "Expr", ";"}});

    // Read:  read ( IdList ) ;
    grammar.push_back({"ReadStmt",   {"read", "(", "IdList", ")", ";"}});

    // Increment / Decrement:  id++ ;  |  id-- ;
    grammar.push_back({"IncStmt",    {"id", "++", ";"}});
    grammar.push_back({"IncStmt",    {"id", "--", ";"}});

    // For loop:  for ( Init ; Cond ; Update ) Block
    grammar.push_back({"ForStmt",    {"for", "(", "Init", ";", "Cond", ";", "Update", ")", "Block"}});
    grammar.push_back({"Init",       {"id", "=", "Expr"}});
    grammar.push_back({"Cond",       {"Expr", "Relop", "Expr"}});
    grammar.push_back({"Update",     {"id", "++"}});
    grammar.push_back({"Update",     {"id", "--"}});
    grammar.push_back({"Update",     {"id", "=", "Expr"}});

    // Block:  { StmtList }
    grammar.push_back({"Block",      {"{", "StmtList", "}"}});

    // Relational operators
    grammar.push_back({"Relop", {"<"}});
    grammar.push_back({"Relop", {">"}});
    grammar.push_back({"Relop", {"=="}});
    grammar.push_back({"Relop", {"!="}});

    // Expressions  (left-recursive — handled by LR parser)
    grammar.push_back({"Expr",   {"Expr", "+", "Term"}});
    grammar.push_back({"Expr",   {"Expr", "-", "Term"}});
    grammar.push_back({"Expr",   {"Term"}});
    grammar.push_back({"Term",   {"Term", "*", "Factor"}});
    grammar.push_back({"Term",   {"Term", "/", "Factor"}});
    grammar.push_back({"Term",   {"Factor"}});
    grammar.push_back({"Factor", {"(", "Expr", ")"}});
    grammar.push_back({"Factor", {"id"}});
    grammar.push_back({"Factor", {"num"}});

    nonterminals = {
        "Program", "DeclList", "Decl", "Type", "IdList",
        "StmtList", "Stmt", "AssignStmt", "ReadStmt", "IncStmt",
        "ForStmt", "Init", "Cond", "Update", "Block",
        "Relop", "Expr", "Term", "Factor"
    };

    terminals = {
        "main", "int", "float", "read", "for",
        "(", ")", "{", "}", ";", ",",
        "id", "num",
        "=", "++", "--", "+", "-", "*", "/",
        "<", ">", "==", "!=", "$"
    };
}
