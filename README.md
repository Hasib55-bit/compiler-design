# 🛠️ Compiler Design Lab — Lexical Analyser & Parser

# University lab project implementing the
> **Lexical Analysis** and
> **Syntax Analysis** phases of a compiler, built in C.

---

## 📌 Overview

This project covers two foundational components of a compiler front-end:

| Phase | Technique Used | Purpose |
|---|---|---|
| **Lexical Analysis** | DFA (Deterministic Finite Automaton) | Tokenises the input string and recognises valid patterns |
| **Syntax Analysis** | LL(1) Predictive Parser | Validates the token stream against a context-free grammar |

---

## 📂 Project Structure

```
compiler-design/
├── dfa_lexer.c       # Lexical analyser using a DFA transition table
├── ll1_parser.c      # LL(1) top-down predictive parser
└── README.md
```

---

## 🔬 Part 1 — Lexical Analyser (DFA)

### What it does
Reads an input string and classifies it against a set of regular patterns using a **7-state DFA**.

### Recognised Token Patterns

| Pattern | Description |
|---|---|
| `a*` | Zero or more `a`s |
| `a*b+` | Zero or more `a`s followed by one or more `b`s |
| `a*b+, abb` | Combined pattern including the specific string `abb` |

### DFA Transition Table

| State | `a` | `b` | other |
|---|---|---|---|
| D0 | D1 | D2 | DEAD |
| D1 | D3 | D4 | DEAD |
| D2 | DEAD | D2 | DEAD |
| D3 | D3 | D2 | DEAD |
| D4 | DEAD | D5 | DEAD |
| D5 | DEAD | D2 | DEAD |
| DEAD | DEAD | DEAD | DEAD |

### How to Run

```bash
gcc dfa_lexer.c -o dfa_lexer
./dfa_lexer
```

**Sample Interaction:**
```
Enter a string: abb
abb Accepted by DFA: a*b+,abb
```

---

## 🧮 Part 2 — LL(1) Predictive Parser

### What it does
Implements a **top-down, table-driven parser** for a simple arithmetic grammar involving `id` and `+`.

### Grammar

```
E  → T E'
E' → + T E'  |  ε
T  → id
```

This grammar is **left-recursion free** and suitable for LL(1) parsing.

### LL(1) Parsing Table

| Non-Terminal | `id` | `+` | `$` |
|---|---|---|---|
| `E` | `T E'` | — | — |
| `E'` | — | `+ T E'` | `ε` |
| `T` | `id` | — | — |

### How to Run

```bash
gcc ll1_parser.c -o ll1_parser
./ll1_parser
```

**Sample Interaction:**
```
Enter input string (tokens separated by spaces, e.g. 'id + id'):
id + id

Tokens detected:
id + id $

Stack                     Lookahead  Top        Production Applied
-------------------------------------------------------------------
...
ACCEPTED
```

### Parser Output Columns

| Column | Description |
|---|---|
| **Stack** | Current contents of the parse stack |
| **Lookahead** | Current input token being examined |
| **Top** | Symbol at the top of the stack |
| **Production Applied** | Rule used, `match`, or `epsilon` |

---

## ⚙️ Build Requirements

- **C Compiler:** GCC (or any C99-compliant compiler)
- **OS:** Linux / macOS / Windows (with MinGW or WSL)

No external libraries required — standard C only.

---

## 🧠 Key Concepts

- **DFA:** A finite automaton with no ambiguity — each state has exactly one transition per input symbol. Used here for recognising lexical tokens.
- **LL(1) Parsing:** A predictive parsing strategy that looks one token ahead (`1` in LL(1)) and never backtracks. The grammar must be free of left recursion and ambiguity.
- **Parse Stack:** Drives the LL(1) parser — non-terminals are expanded using the parsing table; terminals are matched against input.
- **FIRST & FOLLOW sets:** Used to construct the LL(1) table (see project report / instructions for derivation).

---

## 📖 References

- Aho, Lam, Sethi & Ullman — *Compilers: Principles, Techniques, and Tools* (Dragon Book)
- Course Lab Instructions (University Internal)

---

## 👤 Author

> University Compiler Design Lab Project  
> *Lexical Analysis & Syntax Analysis Phase*
