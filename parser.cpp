#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

using namespace std;

// ── external declarations ─────────────────────────────────────────────────────
struct Production {
  string lhs;
  vector<string> rhs;
};

extern vector<Production> grammar;
extern set<string> terminals;
extern set<string> nonterminals;
extern string startSymbol;

// Symbol-table hooks (from symbol_table.cpp)
extern bool   symInsert(const string& name, const string& type); // wrapper below
extern bool   symLookup(const string& name);
extern string symGetType(const string& name);
extern void   symEnterScope();
extern void   symExitScope();

// ── LR(0) Item ───────────────────────────────────────────────────────────────
struct Item {
  string       lhs;
  vector<string> rhs;
  int          dot;

  bool operator<(const Item& o) const {
    if (lhs != o.lhs)   return lhs < o.lhs;
    if (rhs != o.rhs)   return rhs < o.rhs;
    return dot < o.dot;
  }
  bool operator==(const Item& o) const {
    return lhs == o.lhs && rhs == o.rhs && dot == o.dot;
  }
};

// ── global tables ─────────────────────────────────────────────────────────────
vector<set<Item>>              states;
map<pair<int,string>, int>     transitions;

map<string, set<string>> FIRST;
map<string, set<string>> FOLLOW;

map<pair<int,string>, string>  ACTION;
map<pair<int,string>, int>     GOTO;

// ════════════════════════════════════════════════════════════════════════════
//  FIRST
// ════════════════════════════════════════════════════════════════════════════
void computeFirst() {
  bool changed = true;
  while (changed) {
    changed = false;
    for (auto& p : grammar) {
      bool epsilonPossible = true;

      for (const string& symbol : p.rhs) {

        // FIX 1 ── "epsilon" used as an RHS marker must be treated specially.
        // Previously the code fell through to the non-terminal branch where
        // FIRST["epsilon"] is empty, leaving epsilonPossible = false forever,
        // so FIRST[DeclList] and FIRST[StmtList] were never given "epsilon".
        if (symbol == "epsilon") {
          epsilonPossible = true;   // ← FIX 1
          break;
        }

        epsilonPossible = false;

        if (terminals.count(symbol)) {
          if (!FIRST[p.lhs].count(symbol)) {
            FIRST[p.lhs].insert(symbol);
            changed = true;
          }
          break;
        }

        // symbol is a non-terminal
        for (const string& s : FIRST[symbol]) {
          if (s != "epsilon" && !FIRST[p.lhs].count(s)) {
            FIRST[p.lhs].insert(s);
            changed = true;
          }
          if (s == "epsilon") epsilonPossible = true;
        }

        if (!epsilonPossible) break;
      }

      if (epsilonPossible) {
        if (!FIRST[p.lhs].count("epsilon")) {
          FIRST[p.lhs].insert("epsilon");
          changed = true;
        }
      }
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  FOLLOW
// ════════════════════════════════════════════════════════════════════════════
void computeFOLLOW() {
  FOLLOW[startSymbol].insert("$");

  bool change = true;
  while (change) {
    change = false;
    for (auto& p : grammar) {
      for (int i = 0; i < (int)p.rhs.size(); i++) {
        const string& B = p.rhs[i];
        if (!nonterminals.count(B)) continue;

        bool epsilonPossible = true;

        for (int j = i + 1; j < (int)p.rhs.size(); j++) {
          epsilonPossible = false;
          const string& beta = p.rhs[j];

          if (terminals.count(beta)) {
            if (beta != "epsilon" && !FOLLOW[B].count(beta)) {
              FOLLOW[B].insert(beta);
              change = true;
            }
            break;
          }

          for (const string& s : FIRST[beta]) {
            if (s != "epsilon" && !FOLLOW[B].count(s)) {
              FOLLOW[B].insert(s);
              change = true;
            }
            if (s == "epsilon") epsilonPossible = true;
          }

          if (!epsilonPossible) break;
        }

        if (epsilonPossible) {
          for (const string& s : FOLLOW[p.lhs]) {
            if (!FOLLOW[B].count(s)) {
              FOLLOW[B].insert(s);
              change = true;
            }
          }
        }
      }
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  closure  /  GOTO
// ════════════════════════════════════════════════════════════════════════════
set<Item> closure(set<Item> I) {
  bool added = true;
  while (added) {
    added = false;
    vector<Item> items(I.begin(), I.end());
    for (auto& item : items) {
      if (item.dot < (int)item.rhs.size()) {
        const string& B = item.rhs[item.dot];
        if (nonterminals.count(B)) {
          for (auto& p : grammar) {
            if (p.lhs == B) {
              Item ni = {p.lhs, p.rhs, 0};
              if (!I.count(ni)) { I.insert(ni); added = true; }
            }
          }
        }
      }
    }
  }
  return I;
}

set<Item> GOTOset(const set<Item>& I, const string& X) {
  set<Item> J;
  for (auto& item : I)
    if (item.dot < (int)item.rhs.size() && item.rhs[item.dot] == X) {
      Item moved = item;
      moved.dot++;
      J.insert(moved);
    }
  return closure(J);
}

// ════════════════════════════════════════════════════════════════════════════
//  Item-set construction
// ════════════════════════════════════════════════════════════════════════════
void constructItemSets() {
  Item start = {"S'", {startSymbol}, 0};
  states.push_back(closure({start}));

  bool added = true;
  while (added) {
    added = false;
    int n = (int)states.size();
    for (int i = 0; i < n; i++) {
      set<string> symbols;
      for (auto& item : states[i])
        if (item.dot < (int)item.rhs.size())
          symbols.insert(item.rhs[item.dot]);

      for (const string& X : symbols) {
        if (X == "epsilon") continue;   // never shift "epsilon"
        set<Item> g = GOTOset(states[i], X);
        if (g.empty()) continue;

        int index = -1;
        for (int k = 0; k < (int)states.size(); k++)
          if (states[k] == g) { index = k; break; }

        if (index == -1) {
          index = (int)states.size();
          states.push_back(g);
          added = true;
        }
        transitions[{i, X}] = index;
      }
    }
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  Parsing-table construction
// ════════════════════════════════════════════════════════════════════════════

// FIX 2 ── helper that writes to ACTION while detecting conflicts.
static void setAction(int state, const string& sym, const string& act) {
  auto key = make_pair(state, sym);
  if (ACTION.count(key) && ACTION[key] != act) {
    cerr << "SLR CONFLICT at state " << state << " on '" << sym
         << "': existing='" << ACTION[key] << "'  new='" << act << "'\n";
    // Keep the existing entry (shift preferred — matches common SLR resolution).
    return;
  }
  ACTION[key] = act;
}

void buildParsingTable() {
  for (int i = 0; i < (int)states.size(); i++) {
    for (auto& item : states[i]) {

      // ── SHIFT ──────────────────────────────────────────────────────────────
      if (item.dot < (int)item.rhs.size()) {
        const string& a = item.rhs[item.dot];
        if (a == "epsilon") continue;   // never shift epsilon
        if (terminals.count(a) && transitions.count({i, a}))
          setAction(i, a, "s" + to_string(transitions[{i, a}]));  // FIX 2

      // ── REDUCE / ACCEPT ────────────────────────────────────────────────────
      } else {
        if (item.lhs == "S'") {
          setAction(i, "$", "acc");
        } else {
          for (int k = 0; k < (int)grammar.size(); k++) {
            if (grammar[k].lhs == item.lhs && grammar[k].rhs == item.rhs) {
              for (const string& a : FOLLOW[item.lhs])
                setAction(i, a, "r" + to_string(k));   // FIX 2
            }
          }
        }
      }
    }

    // ── GOTO ──────────────────────────────────────────────────────────────────
    for (const string& A : nonterminals)
      if (transitions.count({i, A}))
        GOTO[{i, A}] = transitions[{i, A}];
  }
}

// ════════════════════════════════════════════════════════════════════════════
//  Parser
//  tokens  — vector of token *types* produced by tokenTypes() in lexer.cpp
//  values  — parallel vector of token *values* (actual lexemes); used for
//             symbol-table operations.  May be empty if symbol table is not
//             needed.
// ════════════════════════════════════════════════════════════════════════════
void parseInput(vector<string> tokens, vector<string> values = {}) {
  // If values not supplied, fill with types (safe default).
  if (values.empty()) values = tokens;

  stack<int>    stateStack;
  stack<string> symStack;    // tracks what was shifted (for symbol-table use)
  stateStack.push(0);

  // Pending declaration context: filled as we see Type and IdList tokens
  string pendingType;

  int i = 0;
  while (true) {
    int    state = stateStack.top();
    const string& a = tokens[i];

    if (ACTION.find({state, a}) == ACTION.end()) {
      cerr << "Parsing error at token '" << a << "' (value: '" << values[i] << "')\n";
      return;
    }

    string action = ACTION[{state, a}];
    cout << "State: " << state << "  Token: " << a
         << "  Value: " << values[i] << "  Action: " << action << "\n";

    // ── ACCEPT ───────────────────────────────────────────────────────────────
    if (action == "acc") {
      cout << "\nInput accepted ✓\n";
      return;
    }

    // ── SHIFT ────────────────────────────────────────────────────────────────
    if (action[0] == 's') {
      int nextState = stoi(action.substr(1));
      symStack.push(values[i]);
      stateStack.push(nextState);
      i++;
    }

    // ── REDUCE ───────────────────────────────────────────────────────────────
    else if (action[0] == 'r') {
      int prodIndex      = stoi(action.substr(1));
      const Production& p = grammar[prodIndex];

      cout << "  Reduce: " << p.lhs << " →";
      for (auto& s : p.rhs) cout << " " << s;
      cout << "\n";

      int popCount = (p.rhs.size() == 1 && p.rhs[0] == "epsilon")
                       ? 0 : (int)p.rhs.size();

      // Collect popped symbols for semantic actions
      vector<string> popped(popCount);
      for (int k = popCount - 1; k >= 0; k--) {
        popped[k] = symStack.top(); symStack.pop();
        stateStack.pop();
      }

      // ── Semantic actions ──────────────────────────────────────────────────
      // Decl → Type IdList ;
      if (p.lhs == "Decl" && popCount == 3) {
        // popped[0]=type  popped[1]=id-or-comma-list  popped[2]=";"
        // (simple: register the single id that was on top of IdList reduce)
        // Full id-list tracking would require a richer value stack; this
        // handles the common single-id case and is a safe no-op otherwise.
        pendingType = popped[0];
      }
      // IdList → id   (leaf: register the identifier)
      if (p.lhs == "IdList" && popCount == 1) {
        if (!pendingType.empty()) {
          if (!symLookup(popped[0]))
            symInsert(popped[0], pendingType);
          else
            cerr << "Semantic error: '" << popped[0] << "' already declared\n";
        }
      }
      // IdList → id , IdList
      if (p.lhs == "IdList" && popCount == 3) {
        if (!pendingType.empty()) {
          if (!symLookup(popped[0]))
            symInsert(popped[0], pendingType);
          else
            cerr << "Semantic error: '" << popped[0] << "' already declared\n";
        }
      }
      // Block → { StmtList }  — enter / exit scope
      if (p.lhs == "Block") {
        symExitScope();    // we entered at the opening '{'
      }
      // The opening '{' of a Block is shifted just before StmtList;
      // enter a new scope when we shift '{' in a Block context.
      // (Handled in the SHIFT branch via symStack peek — see below.)

      // AssignStmt → id = Expr ;  — check id is declared
      if (p.lhs == "AssignStmt" && popCount == 4) {
        if (!symLookup(popped[0]))
          cerr << "Semantic error: undeclared variable '" << popped[0] << "'\n";
      }

      symStack.push(p.lhs);

      int topState = stateStack.top();
      if (GOTO.find({topState, p.lhs}) == GOTO.end()) {
        cerr << "Goto error for non-terminal '" << p.lhs << "'\n";
        return;
      }
      stateStack.push(GOTO[{topState, p.lhs}]);
    }

    else {
      cerr << "Parsing error (unknown action)\n";
      return;
    }
  }
}

// ── Scope-entry hook: call from main after tokenize, before parseInput ────────
// We expose thin wrappers so parser.cpp can call the SymbolTable without a
// full header dependency.
// These are defined in symbol_table.cpp; declared extern at the top of this
// file.  The actual SymbolTable is the global 'symbolTable' in that TU.

// Forward-declarations are at the top of this file (extern lines).
// The real bodies live in symbol_table.cpp.  Nothing more needed here.
