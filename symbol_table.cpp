#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

// Each scope is a hash-map from name → type.
static vector<unordered_map<string, string>> scopes;

void enterScope() {
    scopes.push_back({});
    cerr << "[DEBUG] enterScope: now have " << scopes.size() << " scopes\n";
}

void exitScope() {
    if (!scopes.empty()) {
        cerr << "[DEBUG] exitScope: removed scope with " << scopes.back().size() << " vars, now have " << (scopes.size()-1) << " scopes\n";
        scopes.pop_back();
    }
}

// Insert into the innermost (current) scope.
// Returns false if the name already exists in this scope (redeclaration).
bool insertSymbol(const string& name, const string& type) {
    if (scopes.empty()) {
        cerr << "Symbol table: no active scope\n";
        return false;
    }
    auto& cur = scopes.back();
    if (cur.count(name)) {
        cerr << "Redeclaration of '" << name << "' in same scope\n";
        return false;
    }
    cerr << "[DEBUG] insertSymbol: " << name << " : " << type << " into scope " << (scopes.size()-1) << "\n";
    cur[name] = type;
    return true;
}

// Search from innermost scope outward.
bool lookupSymbol(const string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--)
        if (scopes[i].count(name)) return true;
    return false;
}

// Returns the type of name, or "" if not found.
string getType(const string& name) {
    for (int i = (int)scopes.size() - 1; i >= 0; i--) {
        auto it = scopes[i].find(name);
        if (it != scopes[i].end()) return it->second;
    }
    return "";
}

void printSymbolTable() {
    cout << "\n=== Symbol Table ===\n";
    for (int i = 0; i < (int)scopes.size(); i++) {
        cout << "Scope " << i << ":\n";
        for (auto& pair : scopes[i])
            cout << "  " << pair.first << " : " << pair.second << "\n";
    }
    cout << "====================\n";
}
