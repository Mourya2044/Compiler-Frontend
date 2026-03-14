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

string startSymbol = "E";

void initializeGrammar() {
  grammar.push_back({"E", {"E", "+", "T"}});
  grammar.push_back({"E", {"T"}});

  grammar.push_back({"T", {"T", "*", "F"}});
  grammar.push_back({"T", {"F"}});

  grammar.push_back({"F", {"(", "E", ")"}});
  grammar.push_back({"F", {"id"}});

  nonterminals = {"E", "T", "F"};

  terminals = {"+", "*", "(", ")", "id", "$"};
}