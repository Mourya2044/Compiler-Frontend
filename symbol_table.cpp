#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// ── SymbolTable
// ─────────────────────────────────────────────────────────────── Uses a stack
// of hash tables — one per scope — so that inner-scope declarations shadow
// outer ones, and all symbols vanish automatically when exitScope() is called.
//
// Fixes applied:
//   FIX 1 – insert() now detects duplicate declarations within the *same* scope
//            and returns false (error) instead of silently overwriting.
//   FIX 2 – getType() lets callers retrieve the declared type of a name.
//   FIX 3 – printAll() dumps every live scope, useful for debugging.
//   FIX 4 – exitScope() on an empty stack is guarded (was already there, kept).

class SymbolTable {
  // Each entry: name → type  (e.g. "counter" → "int")
  vector<unordered_map<string, string>> scopes;

 public:
  // ── scope management ───────────────────────────────────────────────────────
  void enterScope() { scopes.push_back({}); }

  void exitScope() {
    if (!scopes.empty())
      scopes.pop_back();
    else
      cerr << "SymbolTable error: exitScope() called on empty scope stack\n";
  }

  // ── insert ─────────────────────────────────────────────────────────────────
  // Returns true on success.
  // FIX 1: returns false and prints an error if the name is already declared
  //         in the *current* (innermost) scope.
  bool insert(const string& name, const string& type) {
    if (scopes.empty()) {
      cerr << "SymbolTable error: insert() called with no active scope\n";
      return false;
    }
    auto& top = scopes.back();
    if (top.count(name)) {
      cerr << "Semantic error: '" << name
           << "' already declared in this scope\n";
      return false;  // FIX 1
    }
    top[name] = type;
    return true;
  }

  // ── lookup ─────────────────────────────────────────────────────────────────
  // Returns true if the name is visible in any enclosing scope.
  bool lookup(const string& name) const {
    for (int i = (int)scopes.size() - 1; i >= 0; i--)
      if (scopes[i].count(name)) return true;
    return false;
  }

  // ── getType ────────────────────────────────────────────────────────────────
  // FIX 2: returns the declared type of 'name', or "" if not found.
  string getType(const string& name) const {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
      auto it = scopes[i].find(name);
      if (it != scopes[i].end()) return it->second;
    }
    return "";
  }

  // ── printAll ───────────────────────────────────────────────────────────────
  // FIX 3: debug helper — prints all live scopes from outermost to innermost.
  void printAll() const {
    cout << "\n── Symbol Table ──────────────────────────\n";
    for (int i = 0; i < (int)scopes.size(); i++) {
      cout << "  Scope " << i << ":\n";
      for (auto& entry : scopes[i]) {
        const string& name = entry.first;
        const string& type = entry.second;
        cout << "    " << name << " : " << type << "\n";
      }
    }
    cout << "──────────────────────────────────────────\n\n";
  }

  // ── scopeDepth ─────────────────────────────────────────────────────────────
  int scopeDepth() const { return (int)scopes.size(); }
};
