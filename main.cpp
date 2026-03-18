#include <iostream>
#include <string>
#include <vector>

using namespace std;

extern void initializeGrammar();
extern void computeFirst();
extern void computeFOLLOW();
extern void constructItemSets();
extern void parseInput(vector<string>);
extern vector<string> tokenize(string);
extern void buildParsingTable();

int main() {
  initializeGrammar();

  computeFirst();
  computeFOLLOW();
  constructItemSets();

  cout << "Enter input expression:\n";

  string input;
  getline(cin, input);

  vector<string> tokens = tokenize(input);
  buildParsingTable();

  parseInput(tokens);

  return 0;
}