#include <iostream>
#include <map>
#include <set>
#include <stack>
#include <string>
#include <vector>

using namespace std;

struct Production {
  string lhs;
  vector<string> rhs;
};

extern vector<Production> grammar;
extern set<string> terminals;
extern set<string> nonterminals;
extern string startSymbol;

struct Item {
  string lhs;
  vector<string> rhs;
  int dot;

  bool operator<(const Item& other) const {
    if (lhs != other.lhs) return lhs < other.lhs;
    if (rhs != other.rhs) return rhs < other.rhs;
    return dot < other.dot;
  }

  bool operator==(const Item& other) const {
    return lhs == other.lhs && rhs == other.rhs && dot == other.dot;
  }
};

vector<set<Item>> states;
map<pair<int, string>, int> transitions;

map<string, set<string>> FIRST;
map<string, set<string>> FOLLOW;

/* FIRST */

void computeFirst() {
  bool changed = true;

  while (changed) {
    changed = false;

    for (auto& p : grammar) {
      bool epsilonPossible = true;

      for (string symbol : p.rhs) {
        epsilonPossible = false;

        if (terminals.count(symbol)) {
          if (!FIRST[p.lhs].count(symbol)) {
            FIRST[p.lhs].insert(symbol);
            changed = true;
          }

          break;
        }

        for (string s : FIRST[symbol]) {
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

/* FOLLOW */

void computeFOLLOW() {
  FOLLOW[startSymbol].insert("$");

  bool change = true;

  while (change) {
    change = false;

    for (auto& p : grammar) {
      for (int i = 0; i < p.rhs.size(); i++) {
        string B = p.rhs[i];

        if (!nonterminals.count(B)) continue;

        bool epsilonPossible = true;

        for (int j = i + 1; j < p.rhs.size(); j++) {
          epsilonPossible = false;

          string beta = p.rhs[j];

          if (terminals.count(beta)) {
            if (!FOLLOW[B].count(beta)) {
              FOLLOW[B].insert(beta);
              change = true;
            }

            break;
          }

          for (string s : FIRST[beta]) {
            if (s != "epsilon" && !FOLLOW[B].count(s)) {
              FOLLOW[B].insert(s);
              change = true;
            }

            if (s == "epsilon") epsilonPossible = true;
          }

          if (!epsilonPossible) break;
        }

        if (epsilonPossible) {
          for (string s : FOLLOW[p.lhs]) {
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

/* closure */

set<Item> closure(set<Item> I) {
  bool added = true;

  while (added) {
    added = false;

    vector<Item> items(I.begin(), I.end());

    for (auto item : items) {
      if (item.dot < item.rhs.size()) {
        string B = item.rhs[item.dot];

        if (nonterminals.count(B)) {
          for (auto p : grammar) {
            if (p.lhs == B) {
              Item newItem = {p.lhs, p.rhs, 0};

              if (!I.count(newItem)) {
                I.insert(newItem);
                added = true;
              }
            }
          }
        }
      }
    }
  }

  return I;
}

/* goto */

set<Item> GOTOset(set<Item> I, string X) {
  set<Item> J;

  for (auto item : I) {
    if (item.dot < item.rhs.size() && item.rhs[item.dot] == X) {
      Item moved = item;
      moved.dot++;

      J.insert(moved);
    }
  }

  return closure(J);
}

/* item sets */

void constructItemSets() {
  Item start = {"S'", {startSymbol}, 0};

  set<Item> I0 = closure({start});

  states.push_back(I0);

  bool added = true;

  while (added) {
    added = false;

    int n = states.size();

    for (int i = 0; i < n; i++) {
      set<string> symbols;

      for (auto item : states[i])
        if (item.dot < item.rhs.size()) symbols.insert(item.rhs[item.dot]);

      for (string X : symbols) {
        set<Item> g = GOTOset(states[i], X);

        if (g.empty()) continue;

        int index = -1;

        for (int k = 0; k < states.size(); k++)
          if (states[k] == g) index = k;

        if (index == -1) {
          index = states.size();
          states.push_back(g);
          added = true;
        }

        transitions[{i, X}] = index;
      }
    }
  }
}

/* parsing table */

map<pair<int, string>, string> ACTION;
map<pair<int, string>, int> GOTO;

void buildParsingTable() {
  for (int i = 0; i < states.size(); i++) {
    for (auto item : states[i]) {
      // CASE 1: SHIFT
      if (item.dot < item.rhs.size()) {
        string a = item.rhs[item.dot];

        if (terminals.count(a)) {
          int j = transitions[{i, a}];
          ACTION[{i, a}] = "s" + to_string(j);
        }
      }

      // CASE 2: REDUCE / ACCEPT
      else {
        // ACCEPT
        if (item.lhs == "S'") {
          ACTION[{i, "$"}] = "acc";
        } else {
          // REDUCE A → β
          for (int k = 0; k < grammar.size(); k++) {
            if (grammar[k].lhs == item.lhs && grammar[k].rhs == item.rhs) {
              for (string a : FOLLOW[item.lhs]) {
                ACTION[{i, a}] = "r" + to_string(k);
              }
            }
          }
        }
      }
    }

    // GOTO table
    for (string A : nonterminals) {
      if (transitions.count({i, A})) {
        GOTO[{i, A}] = transitions[{i, A}];
      }
    }
  }
}

/* parsing */

void parseInput(vector<string> tokens) {
  stack<int> st;
  st.push(0);

  int i = 0;

  while (true) {
    int state = st.top();
    string a = tokens[i];

    string action = ACTION[{state, a}];

    cout << "State: " << state << "  Input: " << a << "  Action: " << action
         << endl;

    // ACCEPT
    if (action == "acc") {
      cout << "Input accepted\n";
      return;
    }

    // SHIFT
    else if (action[0] == 's') {
      int nextState = stoi(action.substr(1));
      st.push(nextState);
      i++;
    }

    // REDUCE
    else if (action[0] == 'r') {
      int prodIndex = stoi(action.substr(1));

      Production p = grammar[prodIndex];

      int popCount = p.rhs.size();

      for (int k = 0; k < popCount; k++) st.pop();

      int topState = st.top();

      int gotoState = GOTO[{topState, p.lhs}];

      st.push(gotoState);
    }

    else {
      cout << "Parsing error\n";
      return;
    }
  }
}