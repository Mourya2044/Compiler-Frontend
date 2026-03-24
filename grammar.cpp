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

  // Program
  grammar.push_back({"Program", {"main", "(", ")", "{", "DeclList", "StmtList", "}"}});

  // Declarations
  grammar.push_back({"DeclList", {"Decl", "DeclList"}});
  grammar.push_back({"DeclList", {"epsilon"}});   // nullable
  grammar.push_back({"Decl", {"Type", "IdList", ";"}});

  grammar.push_back({"Type", {"int"}});
  grammar.push_back({"Type", {"float"}});

  grammar.push_back({"IdList", {"id"}});
  grammar.push_back({"IdList", {"id", ",", "IdList"}});

  // Statements
  grammar.push_back({"StmtList", {"Stmt", "StmtList"}});
  grammar.push_back({"StmtList", {"epsilon"}});   // nullable

  grammar.push_back({"Stmt", {"AssignStmt"}});
  grammar.push_back({"Stmt", {"ReadStmt"}});
  grammar.push_back({"Stmt", {"ForStmt"}});
  grammar.push_back({"Stmt", {"IncStmt"}});
  grammar.push_back({"Stmt", {"Block"}});

  // Block
  grammar.push_back({"Block", {"{", "StmtList", "}"}});

  // Assignment
  grammar.push_back({"AssignStmt", {"id", "=", "Expr", ";"}});

  // Read
  grammar.push_back({"ReadStmt", {"read", "(", "IdList", ")", ";"}});

  // Increment / Decrement
  grammar.push_back({"IncStmt", {"id", "++", ";"}});
  grammar.push_back({"IncStmt", {"id", "--", ";"}});

  // For Loop
  grammar.push_back({"ForStmt", {"for", "(", "Init", ";", "Cond", ";", "Update", ")", "Block"}});

  grammar.push_back({"Init", {"id", "=", "Expr"}});
  grammar.push_back({"Cond", {"Expr", "Relop", "Expr"}});

  grammar.push_back({"Update", {"id", "++"}});
  grammar.push_back({"Update", {"id", "--"}});
  grammar.push_back({"Update", {"id", "=", "Expr"}});

  // Relational operators
  grammar.push_back({"Relop", {"<"}});
  grammar.push_back({"Relop", {">"}});
  grammar.push_back({"Relop", {"=="}});
  grammar.push_back({"Relop", {"!="}});

  // Expressions  (left-recursive — handled correctly by SLR)
  grammar.push_back({"Expr", {"Expr", "+", "Term"}});
  grammar.push_back({"Expr", {"Expr", "-", "Term"}});
  grammar.push_back({"Expr", {"Term"}});

  grammar.push_back({"Term", {"Term", "*", "Factor"}});
  grammar.push_back({"Term", {"Term", "/", "Factor"}});
  grammar.push_back({"Term", {"Factor"}});

  grammar.push_back({"Factor", {"(", "Expr", ")"}});
  grammar.push_back({"Factor", {"id"}});
  grammar.push_back({"Factor", {"num"}});

  // Non-terminals
  nonterminals = {
    "Program", "DeclList", "Decl", "Type", "IdList",
    "StmtList", "Stmt", "Block", "AssignStmt", "ReadStmt",
    "IncStmt", "ForStmt", "Init", "Cond", "Update",
    "Relop", "Expr", "Term", "Factor"
  };

  // Terminals  — FIX: "epsilon" added so computeFirst() recognises it
  terminals = {
    "main", "(", ")", "{", "}", ";", ",",
    "int", "float", "read", "for",
    "id", "num",
    "+", "-", "*", "/", "=", "++", "--",
    "<", ">", "==", "!=", "$",
    "epsilon"   // ← FIX: must be a terminal so the symbol-loop handles it correctly
  };
}
