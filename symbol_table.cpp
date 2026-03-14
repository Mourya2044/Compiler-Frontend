#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

class SymbolTable {
  vector<unordered_map<string, string>> scopes;

 public:
  void enterScope() { scopes.push_back({}); }

  void exitScope() {
    if (!scopes.empty()) scopes.pop_back();
  }

  void insert(string name, string type) { scopes.back()[name] = type; }

  bool lookup(string name) {
    for (int i = scopes.size() - 1; i >= 0; i--) {
      if (scopes[i].count(name)) return true;
    }

    return false;
  }
};

SymbolTable symbolTable;