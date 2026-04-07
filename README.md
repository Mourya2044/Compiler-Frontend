# Mini Compiler Project (Lexer + SLR Parser + Scoped Symbol Table)

This project implements a small compiler front-end for a C-like language subset.
It includes:

- A lexer with line/column tracking
- SLR parsing table construction from grammar rules
- Shift-reduce parsing with diagnostics
- A scoped symbol table for declarations in nested blocks

## Project Files

- `main.cpp` - Program entry point, file input, pipeline orchestration
- `grammar.cpp` - Grammar definition and terminal/nonterminal sets
- `lexer.cpp` - Tokenization logic
- `parser.cpp` - FIRST/FOLLOW, LR(0) items, SLR ACTION/GOTO table, parser
- `symbol_table.cpp` - Scope stack and symbol insertion/lookup/printing
- `token.h` - Token structure with source location info
- `input.c`, `test.c`, `test_scope.c` - Sample input programs
- `challenges.txt` - Notes from grammar design/conflict resolution

## Supported Language Subset

### Entry Point

- `main() { ... }`

### Declarations

- `int a;`
- `float x, y;`

### Statements

- Assignment: `id = Expr;`
- Read: `read(a, b);`
- Increment/decrement: `i++;` / `i--;`
- For loops:
  - `for(i = 0; i < 10; i++) { ... }`
  - `for(i = 0; i != n; i = i + 1) { ... }`
- Nested blocks: `{ ... }`

### Expressions

- Arithmetic: `+`, `-`, `*`, `/`
- Grouping with parentheses
- Identifiers and numeric literals

### Relational Operators (for loop condition)

- `<`, `>`, `==`, `!=`

## Build

### Windows (g++)

From the project folder:

```powershell
g++ -std=c++17 -O2 -Wall -Wextra main.cpp lexer.cpp parser.cpp grammar.cpp symbol_table.cpp -o compiler.exe
```

### Linux/macOS (g++)

```bash
g++ -std=c++17 -O2 -Wall -Wextra main.cpp lexer.cpp parser.cpp grammar.cpp symbol_table.cpp -o compiler
```

## Run

Execute the binary and provide the source file name when prompted.

### Windows

```powershell
.\compiler.exe
```

### Linux/macOS

```bash
./compiler
```

Then enter a file name, for example:

```text
input.c
```

## Example That Parses Successfully

Use a file like this:

```c
main() {
    int i, j, sum;
    read(i, j);
    sum = 0;
    for(i = 0; i < 5; i++) {
        int num;
        for(j = 0; j < 3; j++) {
            sum = sum + j;
        }
    }
}
```

## Output Behavior

During execution, the parser prints:

- Number of LR item sets constructed
- Reduce actions used while parsing
- Success: `[OK] Input accepted`
- Or syntax errors with line/column info

At the end, the symbol table is printed with scope-wise declarations.

## Current Notes / Limitations

- The parser focuses on syntactic analysis and declaration capture. It does not currently enforce full semantic checks for all statement uses.
- Unknown characters are reported as lexical errors with source location.

## Quick Test

After building, create a file with `main() { ... }` and run:

```powershell
.\compiler.exe
```

Enter your test file name and verify:

- Parsing succeeds
- Symbol table shows variables in the expected scopes
