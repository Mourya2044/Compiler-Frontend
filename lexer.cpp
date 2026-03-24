#include <cctype>
#include <iostream>
#include <string>
#include <vector>
#include <unordered_set>

using namespace std;

// ── Token ────────────────────────────────────────────────────────────────────
// Each token carries both its grammatical type (used by the parser) and its
// original lexeme (used by the symbol table).
struct Token {
  string type;    // e.g. "id", "num", "int", "for", "+", ...
  string value;   // e.g. "counter", "3.14", "int", "for", "+"
};

// ── Keyword set ──────────────────────────────────────────────────────────────
unordered_set<string> keywords = {"main", "int", "float", "for", "read"};

// ── tokenize ─────────────────────────────────────────────────────────────────
// Returns a vector of Tokens.  The last token is always {"$", "$"}.
// FIX 1: actual identifier names are now preserved in Token::value instead of
//         being discarded (old code pushed only the string "id").
// FIX 2: unknown characters are skipped with a warning rather than silently
//         dropped, making lexer errors visible.
vector<Token> tokenize(const string& input) {
  vector<Token> tokens;

  for (int i = 0; i < (int)input.size(); i++) {

    // ── whitespace ───────────────────────────────────────────────────────────
    if (isspace(input[i])) continue;

    // ── identifiers / keywords ───────────────────────────────────────────────
    if (isalpha(input[i]) || input[i] == '_') {
      string word;
      while (i < (int)input.size() && (isalnum(input[i]) || input[i] == '_')) {
        word += input[i++];
      }
      i--;

      if (keywords.count(word))
        tokens.push_back({word, word});   // keyword: type == value
      else
        tokens.push_back({"id", word});   // FIX 1: preserve actual name
      continue;
    }

    // ── numeric literals ─────────────────────────────────────────────────────
    if (isdigit(input[i])) {
      string num;
      bool hasDot = false;
      while (i < (int)input.size() && (isdigit(input[i]) || (input[i] == '.' && !hasDot))) {
        if (input[i] == '.') hasDot = true;
        num += input[i++];
      }
      i--;
      tokens.push_back({"num", num});     // preserve numeric value
      continue;
    }

    // ── two-character operators (must be checked before single-char) ─────────
    if (i + 1 < (int)input.size()) {
      string two = string(1, input[i]) + input[i + 1];
      if (two == "==" || two == "!=" || two == "++" || two == "--") {
        tokens.push_back({two, two});
        i++;
        continue;
      }
    }

    // ── single-character operators / punctuation ─────────────────────────────
    {
      char c = input[i];
      static const string singles = "+-*/=<>(){};,";
      if (singles.find(c) != string::npos) {
        string s(1, c);
        tokens.push_back({s, s});
        continue;
      }
    }

    // ── unrecognised character — FIX 2: report instead of silently skip ──────
    cerr << "Lexer warning: unrecognised character '" << input[i]
         << "' (ASCII " << (int)(unsigned char)input[i] << ") — skipped\n";
  }

  tokens.push_back({"$", "$"});

  // ── print token stream ───────────────────────────────────────────────────
  cout << "\nTokens:\n";
  for (auto& t : tokens)
    cout << "[" << t.type << (t.type != t.value ? ":" + t.value : "") << "] ";
  cout << "\n\n";

  return tokens;
}

// ── helper: extract just the type strings for the parser ─────────────────────
// The parser works on vector<string> of token types; use this after tokenize().
vector<string> tokenTypes(const vector<Token>& tokens) {
  vector<string> types;
  types.reserve(tokens.size());
  for (auto& t : tokens) types.push_back(t.type);
  return types;
}
