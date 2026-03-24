#include <iostream>
#include <string>
#include <vector>

using namespace std;

// ── Grammar / parser
// ──────────────────────────────────────────────────────────
extern void initializeGrammar();
extern void computeFirst();
extern void computeFOLLOW();
extern void constructItemSets();
extern void buildParsingTable();
extern void parseInput(vector<string> tokens, vector<string> values);

// ── Lexer
// ─────────────────────────────────────────────────────────────────────
struct Token {
  string type;
  string value;
};  // matches lexer.cpp
extern vector<Token> tokenize(const string& input);
extern vector<string> tokenTypes(const vector<Token>& tokens);

// ── Symbol table
// ────────────────────────────────────────────────────────────── We expose the
// SymbolTable methods through thin free-function wrappers so that parser.cpp
// can call them without a header.  Define the global here too.

#include <unordered_map>

class SymbolTable {
  vector<unordered_map<string, string>> scopes;

 public:
  void enterScope() { scopes.push_back({}); }
  void exitScope() {
    if (!scopes.empty()) scopes.pop_back();
  }
  bool insert(const string& n, const string& t) {
    if (scopes.empty()) return false;
    if (scopes.back().count(n)) {
      cerr << "Semantic error: '" << n << "' already declared in this scope\n";
      return false;
    }
    scopes.back()[n] = t;
    return true;
  }
  bool lookup(const string& n) const {
    for (int i = (int)scopes.size() - 1; i >= 0; i--)
      if (scopes[i].count(n)) return true;
    return false;
  }
  string getType(const string& n) const {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
      auto it = scopes[i].find(n);
      if (it != scopes[i].end()) return it->second;
    }
    return "";
  }
  void printAll() const {
    cout << "\n── Symbol Table ──────────────────────────\n";
    for (int i = 0; i < (int)scopes.size(); i++) {
      cout << "  Scope " << i << ":\n";
      for (auto& entry : scopes[i]) {
        const string& n = entry.first;
        const string& t = entry.second;
        cout << "    " << n << " : " << t << "\n";
      }
    }
    cout << "──────────────────────────────────────────\n\n";
  }
  int depth() const { return (int)scopes.size(); }
};

// Global instance — also referenced by parser.cpp via externs below
SymbolTable symbolTable;

// ── Wrapper free functions (called by parser.cpp via extern)
// ──────────────────
bool symInsert(const string& n, const string& t) {
  return symbolTable.insert(n, t);
}
bool symLookup(const string& n) { return symbolTable.lookup(n); }
string symGetType(const string& n) { return symbolTable.getType(n); }
void symEnterScope() { symbolTable.enterScope(); }
void symExitScope() { symbolTable.exitScope(); }

// ── main
// ──────────────────────────────────────────────────────────────────────
int main() {
  // Part I+II: build grammar, compute sets, build SLR table
  initializeGrammar();
  computeFirst();
  computeFOLLOW();
  constructItemSets();
  buildParsingTable();

  cout << "Enter program (end with an empty line):\n";
  string input, line;
  while (getline(cin, line)) {
    if (line.empty()) break;
    input += line + "\n";
  }

  // Part II: lex
  vector<Token> toks = tokenize(input);
  vector<string> types = tokenTypes(toks);  // parser only needs types
  vector<string> values;
  for (auto& t : toks) values.push_back(t.value);  // symbol table needs values

  // Part III: open the global (program-level) scope before parsing
  symbolTable.enterScope();

  // Part IV: parse  (also performs symbol-table semantic actions)
  parseInput(types, values);

  // Print final symbol table
  symbolTable.printAll();

  symbolTable.exitScope();  // close program scope

  return 0;
}
