#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>
#include <algorithm>
#include "token.h"

using namespace std;

// ── Shared types (also defined in grammar.cpp) ────────────────────────────────
struct Production {
    string lhs;
    vector<string> rhs;
};

extern vector<Production> grammar;
extern set<string> terminals;
extern set<string> nonterminals;
extern string startSymbol;

// ── LR(0) Item ────────────────────────────────────────────────────────────────
struct Item {
    string lhs;
    vector<string> rhs;
    int dot;

    bool operator<(const Item& o) const {
        if (lhs != o.lhs) return lhs < o.lhs;
        if (rhs != o.rhs) return rhs < o.rhs;
        return dot < o.dot;
    }
    bool operator==(const Item& o) const {
        return lhs == o.lhs && rhs == o.rhs && dot == o.dot;
    }
};

// ── Global state ──────────────────────────────────────────────────────────────
static vector<set<Item>>              states;
static map<pair<int,string>, int>     transitions;
static map<string, set<string>>       FIRST;
static map<string, set<string>>       FOLLOW;
static map<pair<int,string>, string>  ACTION;
static map<pair<int,string>, int>     GOTO;

// ═════════════════════════════════════════════════════════════════════════════
//  FIRST
// ═════════════════════════════════════════════════════════════════════════════
void computeFirst() {
    bool changed = true;
    while (changed) {
        changed = false;
        for (auto& p : grammar) {
            bool allEps = true;
            for (auto& sym : p.rhs) {
                allEps = false;
                
                // Handle epsilon production directly
                if (sym == "epsilon") {
                    allEps = true;
                    break;
                }
                
                if (terminals.count(sym)) {
                    changed |= FIRST[p.lhs].insert(sym).second;
                    break;
                }
                
                // For nonterminals, add their FIRST (except epsilon)
                for (auto& s : FIRST[sym]) {
                    if (s != "epsilon") changed |= FIRST[p.lhs].insert(s).second;
                }
                
                if (!FIRST[sym].count("epsilon")) break;
                allEps = true;
            }
            if (allEps) changed |= FIRST[p.lhs].insert("epsilon").second;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  FOLLOW
// ═════════════════════════════════════════════════════════════════════════════
void computeFOLLOW() {
    FOLLOW[startSymbol].insert("$");
    bool changed = true;
    int iter = 0;
    while (changed) {
        changed = false;
        iter++;
        for (auto& p : grammar) {
            for (int i = 0; i < (int)p.rhs.size(); i++) {
                const string& B = p.rhs[i];
                if (!nonterminals.count(B)) continue;

                bool trailEps = true;
                for (int j = i + 1; j < (int)p.rhs.size(); j++) {
                    trailEps = false;
                    const string& beta = p.rhs[j];
                    if (terminals.count(beta)) {
                        changed |= FOLLOW[B].insert(beta).second;
                        break;
                    }
                    for (auto& s : FIRST[beta]) {
                        if (s != "epsilon") changed |= FOLLOW[B].insert(s).second;
                    }
                    if (!FIRST[beta].count("epsilon")) break;
                    trailEps = true;
                }
                if (trailEps) {
                    for (auto& s : FOLLOW[p.lhs])
                        changed |= FOLLOW[B].insert(s).second;
                }
            }
        }
    }
    
    // Debug specific nonterminals
    cout << "\n=== FOLLOW Analysis ===\n";
    cout << "FOLLOW(DeclList) = {";
    for (auto& s : FOLLOW["DeclList"]) cout << s << " ";
    cout << "}\n";
    cout << "FOLLOW(Decl) = {";
    for (auto& s : FOLLOW["Decl"]) cout << s << " ";
    cout << "}\n";
    // Debug FIRST sets too
    cout << "FIRST(StmtList) = {";
    for (auto& s : FIRST["StmtList"]) cout << s << " ";
    cout << "}\n";
    cout << "FIRST(DeclList) = {";
    for (auto& s : FIRST["DeclList"]) cout << s << " ";
    cout << "}\n";
}

// ═════════════════════════════════════════════════════════════════════════════
//  Closure & Goto
// ═════════════════════════════════════════════════════════════════════════════
static set<Item> closure(set<Item> I) {
    bool added = true;
    while (added) {
        added = false;
        for (auto item : vector<Item>(I.begin(), I.end())) {
            if (item.dot >= (int)item.rhs.size()) continue;
            const string& B = item.rhs[item.dot];
            if (!nonterminals.count(B)) continue;
            for (auto& p : grammar) {
                if (p.lhs != B) continue;
                // Handle epsilon productions: add the item with dot at the end
                if (p.rhs.size() == 1 && p.rhs[0] == "epsilon") {
                    Item ni{p.lhs, p.rhs, 1};  // Dot at end for epsilon production
                    if (!I.count(ni)) { I.insert(ni); added = true; }
                } else {
                    Item ni{p.lhs, p.rhs, 0}; // Dot at beginning for normal productions
                    if (!I.count(ni)) { I.insert(ni); added = true; }
                }
            }
        }
    }
    return I;
}

static set<Item> gotoSet(const set<Item>& I, const string& X) {
    set<Item> J;
    for (auto item : I) {
        if (item.dot < (int)item.rhs.size() && item.rhs[item.dot] == X) {
            Item moved = item;
            moved.dot++;
            J.insert(moved);
        }
    }
    return closure(J);
}

// ═════════════════════════════════════════════════════════════════════════════
//  Item-set construction
// ═════════════════════════════════════════════════════════════════════════════
void constructItemSets() {
    // Augmented start production  S' → Program
    set<Item> I0 = closure({{"S'", {startSymbol}, 0}});
    states.push_back(I0);

    bool added = true;
    while (added) {
        added = false;
        int n = states.size();
        for (int i = 0; i < n; i++) {
            // Collect all symbols after a dot in this state
            set<string> symbols;
            for (auto& item : states[i])
                if (item.dot < (int)item.rhs.size())
                    symbols.insert(item.rhs[item.dot]);

            for (auto& X : symbols) {
                set<Item> g = gotoSet(states[i], X);
                if (g.empty()) continue;

                // Find if g already exists
                int idx = -1;
                for (int k = 0; k < (int)states.size(); k++)
                    if (states[k] == g) { idx = k; break; }

                if (idx == -1) {
                    idx = states.size();
                    states.push_back(g);
                    added = true;
                }
                transitions[{i, X}] = idx;
            }
        }
    }
    cout << "Item sets constructed: " << states.size() << " states\n";
}

// ═════════════════════════════════════════════════════════════════════════════
//  Parsing table  (SLR(1))
// ═════════════════════════════════════════════════════════════════════════════
void buildParsingTable() {
    for (int i = 0; i < (int)states.size(); i++) {
        for (auto& item : states[i]) {

            if (item.dot < (int)item.rhs.size()) {
                // ── SHIFT ──────────────────────────────────────────────────
                const string& a = item.rhs[item.dot];
                if (terminals.count(a) && transitions.count({i, a})) {
                    string act = "s" + to_string(transitions[{i, a}]);
                    auto key = make_pair(i, a);
                    if (ACTION.count(key) && ACTION[key] != act)
                        cerr << "Conflict in state " << i
                             << " on '" << a << "': " << ACTION[key]
                             << " vs " << act << "\n";
                    else
                        ACTION[key] = act;
                }
            } else {
                // ── ACCEPT ─────────────────────────────────────────────────
                if (item.lhs == "S'") {
                    ACTION[{i, "$"}] = "acc";
                    continue;
                }
                // ── REDUCE ─────────────────────────────────────────────────
                // Find production index
                int k = -1;
                for (int p = 0; p < (int)grammar.size(); p++) {
                    if (grammar[p].lhs == item.lhs &&
                        grammar[p].rhs == item.rhs) { k = p; break; }
                }
                if (k == -1) continue;   // shouldn't happen

                for (auto& a : FOLLOW[item.lhs]) {
                    string act = "r" + to_string(k);
                    auto key = make_pair(i, a);
                    if (ACTION.count(key) && ACTION[key] != act)
                        cerr << "Conflict in state " << i
                             << " on '" << a << "': " << ACTION[key]
                             << " vs " << act << "\n";
                    else
                        ACTION[key] = act;
                }
            }
        }

        // ── GOTO table ─────────────────────────────────────────────────────
        for (auto& A : nonterminals)
            if (transitions.count({i, A}))
                GOTO[{i, A}] = transitions[{i, A}];
    }    
        // Debug: print ACTION and GOTO for state 32
        cout << "\n=== State 32 entries ===\n";
        cout << "ACTION entries:\n";
        for (auto it = ACTION.begin(); it != ACTION.end(); ++it) {
            if (it->first.first == 32) cout << "  (" << it->first.first << ", '" << it->first.second << "') -> " << it->second << "\n";
        }
        cout << "GOTO entries:\n";
        for (auto it = GOTO.begin(); it != GOTO.end(); ++it) {
            if (it->first.first == 32) cout << "  (" << it->first.first << ", '" << it->first.second << "') -> " << it->second << "\n";
        }
    }

// ═════════════════════════════════════════════════════════════════════════════
//  Semantic value tracking for symbol table population
// ═════════════════════════════════════════════════════════════════════════════
extern void insertSymbol(const string& name, const string& type);
extern void enterScope();
extern void exitScope();

void parseInput(vector<Token> tokens) {
    static int braceDepth = 0;  // Track nesting depth for scope management
    static stack<int> blockDepths;  // Track the brace depth when each block started
    stack<int> stk;
    stack<string> semantic;  // Track semantic values (lexemes and computed values)
    
    stk.push(0);
    semantic.push("");  // dummy initial value
    int i = 0;
    braceDepth = 0;  // Reset for new parse
    while (!blockDepths.empty()) blockDepths.pop();  // Clear block depth tracking

    cout << "\n--- Parsing ---\n";

    while (true) {
        int state = stk.top();
        const string& tokenType = tokens[i].type;
        const string& lexeme = tokens[i].lexeme;

        auto key = make_pair(state, tokenType);
        if (!ACTION.count(key)) {
            cout << "Syntax error at token '" << lexeme << "' (line " 
                 << tokens[i].line << ", col " << tokens[i].col << ")\n";
            return;
        }

        const string& action = ACTION[key];
        cout << "State " << state << "  token '" << tokenType << "'  action " << action << "\n";

        if (action == "acc") {
            cout << "✓ Input accepted\n";
            return;
        }

        if (action[0] == 's') {
            // SHIFT
            int nextState = stoi(action.substr(1));
            stk.push(nextState);
            semantic.push(lexeme);  // Push the lexeme as semantic value
            
            // Handle scope push for opening braces
            if (lexeme == "{") {
                braceDepth++;
                blockDepths.push(braceDepth);  // Record the depth of this block
                if (braceDepth > 1) {  // First { is program level, additional { are nested blocks
                    enterScope();
                }
                cerr << "[DEBUG] shift '{' at depth " << braceDepth << "\n";
            } else if (lexeme == "}") {
                cerr << "[DEBUG] shift '}' at depth " << braceDepth << "\n";
                braceDepth--;
            }
            
            i++;
        } else if (action[0] == 'r') {
            // REDUCE
            int prodIdx = stoi(action.substr(1));
            const Production& p = grammar[prodIdx];

            cout << "  Reduce by  " << p.lhs << " →";
            for (auto& s : p.rhs) cout << " " << s;
            cout << "\n";

            // Pop semantic values and create new semantic value
            vector<string> rhsValues;
            int popCount = (p.rhs.size() == 1 && p.rhs[0] == "epsilon")
                               ? 0 : (int)p.rhs.size();
            
            for (int k = 0; k < popCount; k++) {
                rhsValues.push_back(semantic.top());
                semantic.pop();
                stk.pop();
            }
            reverse(rhsValues.begin(), rhsValues.end());

            // Semantic action: Handle declarations
            string semanticValue = "";
            if (p.lhs == "Decl") {
                // Decl → Type IdList ;
                // rhsValues[0] = type, rhsValues[1] = idlist (comma-separated)
                string type = rhsValues[0];
                string idlist = rhsValues[1];
                
                // Split comma-separated identifiers and insert all
                size_t pos = 0;
                while (pos < idlist.length()) {
                    // Find next comma or end
                    size_t comma = idlist.find(',', pos);
                    if (comma == string::npos) {
                        comma = idlist.length();
                    }
                    string id = idlist.substr(pos, comma - pos);
                    // Trim whitespace
                    while (!id.empty() && isspace(id.front())) id = id.substr(1);
                    while (!id.empty() && isspace(id.back())) id = id.substr(0, id.length() - 1);
                    if (!id.empty()) {
                        insertSymbol(id, type);
                    }
                    pos = comma + 1;
                }
                semanticValue = type;  // Pass type forward
            } else if (p.lhs == "Type") {
                // Type → int | float
                semanticValue = rhsValues[0];  // Just the type itself
            } else if (p.lhs == "IdList") {
                // IdList → id | id , IdList
                if (p.rhs.size() == 1) {
                    // IdList → id
                    semanticValue = rhsValues[0];
                } else {
                    // IdList → id , IdList
                    // rhsValues[0] = id, rhsValues[2] = IdList value (comma-separated)
                    semanticValue = rhsValues[0] + "," + rhsValues[2];
                }
            } else if (p.lhs == "Block") {
                // Block → { StmtList }
                // Pop the scope that was created for this block
                if (!blockDepths.empty()) {
                    int blockDepth = blockDepths.top();
                    blockDepths.pop();
                    cerr << "[DEBUG] reduce Block that was at depth " << blockDepth << "\n";
                    if (blockDepth > 1) {  // Nested block (not program level)
                        exitScope();
                    }
                }
                semanticValue = "";
            } else {
                semanticValue = "";  // Default for other productions
            }

            int top = stk.top();
            if (!GOTO.count({top, p.lhs})) {
                cout << "GOTO error after reducing to " << p.lhs << "\n";
                return;
            }
            
            stk.push(GOTO[{top, p.lhs}]);
            semantic.push(semanticValue);
        } else {
            cout << "Unknown action: " << action << "\n";
            return;
        }
    }
}

// ═════════════════════════════════════════════════════════════════════════════
//  Debug output functions
// ═════════════════════════════════════════════════════════════════════════════
void printParsingTables() {
    cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                    FIRST SETS                                  ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n";
    for (auto& nt : nonterminals) {
        cout << "FIRST(" << nt << ") = { ";
        for (auto& s : FIRST[nt]) cout << s << " ";
        cout << "}\n";
    }

    cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                    FOLLOW SETS                                 ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n";
    for (auto& nt : nonterminals) {
        cout << "FOLLOW(" << nt << ") = { ";
        for (auto& s : FOLLOW[nt]) cout << s << " ";
        cout << "}\n";
    }

    cout << "\n╔════════════════════════════════════════════════════════════════╗\n";
    cout << "║                   PARSING TABLE                                ║\n";
    cout << "╚════════════════════════════════════════════════════════════════╝\n";
    
    cout << "\nACTION TABLE:\n";
    cout << "─────────────────────────────────────────────\n";
    map<int, vector<pair<string, string>>> actionByState;
    for (auto it = ACTION.begin(); it != ACTION.end(); ++it) {
        actionByState[it->first.first].push_back({it->first.second, it->second});
    }
    for (auto it = actionByState.begin(); it != actionByState.end(); ++it) {
        cout << "State " << it->first << ":\n";
        for (auto& entry : it->second) {
            cout << "  " << entry.first << " -> " << entry.second << "\n";
        }
    }

    cout << "\nGOTO TABLE:\n";
    cout << "─────────────────────────────────────────────\n";
    map<int, vector<pair<string, int>>> gotoByState;
    for (auto it = GOTO.begin(); it != GOTO.end(); ++it) {
        gotoByState[it->first.first].push_back({it->first.second, it->second});
    }
    for (auto it = gotoByState.begin(); it != gotoByState.end(); ++it) {
        cout << "State " << it->first << ":\n";
        for (auto& entry : it->second) {
            cout << "  " << entry.first << " -> " << entry.second << "\n";
        }
    }
}
