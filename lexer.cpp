#include <cctype>
#include <iostream>
#include <string>
#include <vector>

using namespace std;

vector<string> tokenize(string input) {
  vector<string> tokens;

  for (char c : input) {
    if (isalpha(c))
      tokens.push_back("id");

    else if (c == '+')
      tokens.push_back("+");

    else if (c == '*')
      tokens.push_back("*");

    else if (c == '(')
      tokens.push_back("(");

    else if (c == ')')
      tokens.push_back(")");
  }

  tokens.push_back("$");

  cout << "Tokens: ";
  for (string token : tokens) {
    cout << token << " ";
  }
  cout << endl;

  return tokens;
}