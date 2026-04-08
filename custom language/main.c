#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

// ============================================================
//  SHARED DEFINITIONS
// ============================================================
#define MAXSTACK 300
#define MAXTOK   300
#define MAXSYM   60

// ============================================================
//  LEXER
// ============================================================

int get_col(char c) {
    if (isalpha(c)) return 0;
    if (isdigit(c)) return 1;
    switch (c) {
        case '_': return 2;
        case '.': return 3;
        case '/': return 4;
        case '#': return 5;
        case '[': return 6;
        case ']': return 7;
        case '(': return 8;
        case ')': return 9;
        case '=': case '+': case '-': case '*':
        case '<': case '!': case '>': return 10;
        default:  return 11;
    }
}

const char* refine_id(char *s, int state) {
    if (state == 2) return "VARIABLE";
    if (strcmp(s, "dec") == 0 || strcmp(s, "int") == 0) return "DATATYPE";
    if (strcmp(s, "return") == 0 || strcmp(s, "while") == 0 || strcmp(s, "break") == 0) return "KEYWORD";
    if (strcmp(s, "mainFn") == 0) return "MAIN";
    if (strcmp(s, "printfFn") == 0) return "PRINT";
    if (strstr(s, "Fn"))   return "FUNCTION";
    if (strstr(s, "loop")) return "LOOP";
    return "IDENTIFIER";
}

// Runs lexer on input.txt, writes token stream into out_tokens[]
// Returns number of tokens written
int run_lexer(char out_tokens[][MAXSYM]) {
    int next_state[8][12] = {
        /* 0:START */ { 1, 3, 2, 4, 5, 7, 0, 0, 0, 0, 0, 0 },
        /* 1:ID    */ { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 2:VAR   */ { 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 3:NUM   */ { 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 4:DOT   */ { 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 5:SLASH */ { 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0 },
        /* 6:COMM  */ { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 },
        /* 7:HEAD  */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 }
    };

    FILE *fptr = fopen("input.txt", "r");
    if (!fptr) { printf("ERROR: cannot open input.txt\n"); return 0; }

    char c, buffer[100];
    int  state = 0, b_idx = 0;
    int  n = 0;   // token count

    printf("\n========================================\n");
    printf("          LEXER OUTPUT\n");
    printf("========================================\n");
    printf("%-30s | %s\n", "Lexeme", "Token");
    printf("----------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col     = get_col(c);
        int n_state = next_state[state][col];

        if (n_state != 0 && !(state == 6 && c == '\n')) {
            if (state == 1 && c == ':') {
                // LOOP label like loop_main01:
                buffer[b_idx++] = c;
                buffer[b_idx]   = '\0';
                printf("%-30s | LOOP\n", buffer);
                strcpy(out_tokens[n++], "LOOP");
                b_idx = 0; state = 0;
            } else {
                state = n_state;
                buffer[b_idx++] = c;
                if (state == 7 && c == '>') {
                    // HEADER like #include<stdio.h>
                    buffer[b_idx] = '\0';
                    printf("%-30s | HEADER\n", buffer);
                    strcpy(out_tokens[n++], "HEADER");
                    b_idx = 0; state = 0;
                }
            }
        } else {
            // Flush buffered token
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char *tname;
                if      (state == 1 || state == 2) tname = refine_id(buffer, state);
                else if (state == 3)               tname = "NUMBER";
                else if (state == 4)               tname = "END";
                else if (state == 6)               tname = "COMMENT";
                else                               tname = "UNKNOWN";

                printf("%-30s | %s\n", buffer, tname);
                // Skip COMMENT tokens — parser doesn't need them
                if (strcmp(tname, "COMMENT") != 0)
                    strcpy(out_tokens[n++], tname);

                b_idx = 0; state = 0;
            }

            if (!isspace(c)) {
                // Single/compound operators and brackets
                if (c == '[') {
                    printf("%-30c | LEFT PARENTHESES\n", c);
                    strcpy(out_tokens[n++], "LEFT_PARENTHESES");
                } else if (c == ']') {
                    printf("%-30c | RIGHT PARENTHESES\n", c);
                    strcpy(out_tokens[n++], "RIGHT_PARENTHESES");
                } else if (c == '(') {
                    printf("%-30c | FIRST BRACKET\n", c);
                    strcpy(out_tokens[n++], "FIRST_BRACKET");
                } else if (c == ')') {
                    printf("%-30c | LAST BRACKET\n", c);
                    strcpy(out_tokens[n++], "LAST_BRACKET");
                } else if (strchr("<>!=+-*", c)) {
                    char lexeme[3] = {c, '\0', '\0'};
                    char next = fgetc(fptr);
                    if (next == '=' && strchr("<>!=", c)) {
                        lexeme[1] = next;   // compound: <= >= == !=
                    } else {
                        if (next != EOF) ungetc(next, fptr);
                    }
                    printf("%-30s | OPERATOR\n", lexeme);
                    strcpy(out_tokens[n++], "OPERATOR");
                }

                if (get_col(c) < 6)
                    ungetc(c, fptr);
            }
        }
    }

    fclose(fptr);

    // Append $ sentinel
    strcpy(out_tokens[n++], "$");
    return n;
}

// ============================================================
//  PARSER
// ============================================================

char *NT[] = {
    "PROGRAM","FUNC_DEF","FUNCNAME","PARAMLIST","MOREPARAMS",
    "STMTLIST","STMT","DECL_STMT","ASSIGN_STMT","RETURN_STMT",
    "LOOP_STMT","LOOP_COND","PRINT_STMT","EXPR","EXPRP",
    "TERM","FUNCALL","ARGLIST","MOREARGS","RETBODY"
};
#define NNT 20

char *TERMINALS[] = {
    "HEADER","DATATYPE","VARIABLE","OPERATOR","NUMBER","END",
    "FUNCTION","MAIN","PRINT","KEYWORD","LOOP",
    "LEFT_PARENTHESES","RIGHT_PARENTHESES","FIRST_BRACKET","LAST_BRACKET","$"
};
#define NTER 16

char *RHS[] = {
    "",
    /* 1  */ "HEADER PROGRAM",
    /* 2  */ "FUNC_DEF PROGRAM",
    /* 3  */ "",
    /* 4  */ "DATATYPE FUNCNAME FIRST_BRACKET PARAMLIST LAST_BRACKET LEFT_PARENTHESES STMTLIST RIGHT_PARENTHESES",
    /* 5  */ "FUNCTION",
    /* 6  */ "MAIN",
    /* 7  */ "DATATYPE VARIABLE MOREPARAMS",
    /* 8  */ "",
    /* 9  */ "",
    /* 10 */ "STMT STMTLIST",
    /* 11 */ "",
    /* 12 */ "DECL_STMT",
    /* 13 */ "ASSIGN_STMT",
    /* 14 */ "RETURN_STMT",
    /* 15 */ "LOOP_STMT",
    /* 16 */ "PRINT_STMT",
    /* 17 */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 18 */ "VARIABLE OPERATOR EXPR END",
    /* 19 */ "KEYWORD RETBODY END",
    /* 20 */ "LOOP KEYWORD FIRST_BRACKET LOOP_COND LAST_BRACKET LEFT_PARENTHESES STMTLIST RIGHT_PARENTHESES",
    /* 21 */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 22 */ "PRINT FIRST_BRACKET VARIABLE LAST_BRACKET END",
    /* 23 */ "TERM EXPRP",
    /* 24 */ "OPERATOR TERM EXPRP",
    /* 25 */ "",
    /* 26 */ "VARIABLE",
    /* 27 */ "NUMBER",
    /* 28 */ "FUNCALL",
    /* 29 */ "FUNCTION FIRST_BRACKET ARGLIST LAST_BRACKET",
    /* 30 */ "VARIABLE MOREARGS",
    /* 31 */ "NUMBER MOREARGS",
    /* 32 */ "",
    /* 33 */ "",
    /* 34 */ "VARIABLE",
    /* 35 */ "NUMBER",
    /* 36 */ ""
};

int TABLE[NNT][NTER] = {
/* PROGRAM    */ { 1,  2,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  3 },
/* FUNC_DEF   */ { 0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* FUNCNAME   */ { 0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0 },
/* PARAMLIST  */ { 0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0 },
/* MOREPARAMS */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0 },
/* STMTLIST   */ { 0, 10, 10,  0,  0,  0,  0,  0, 10, 10, 10,  0, 11,  0,  0, 11 },
/* STMT       */ { 0, 12, 13,  0,  0,  0,  0,  0, 16, 14, 15,  0,  0,  0,  0,  0 },
/* DECL_STMT  */ { 0, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* ASSIGN_STMT*/ { 0,  0, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* RETURN_STMT*/ { 0,  0,  0,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,  0,  0 },
/* LOOP_STMT  */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0 },
/* LOOP_COND  */ { 0, 21,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* PRINT_STMT */ { 0,  0,  0,  0,  0,  0,  0,  0, 22,  0,  0,  0,  0,  0,  0,  0 },
/* EXPR       */ { 0,  0, 23,  0, 23,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* EXPRP      */ { 0,  0,  0, 24,  0, 25,  0,  0,  0,  0,  0,  0,  0,  0, 25, 25 },
/* TERM       */ { 0,  0, 26,  0, 27,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* FUNCALL    */ { 0,  0,  0,  0,  0,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* ARGLIST    */ { 0,  0, 30,  0, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32,  0 },
/* MOREARGS   */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  0 },
/* RETBODY    */ { 0,  0, 34,  0, 35, 36,  0,  0,  0,  0,  0,  0,  0,  0,  0, 36 }
};

// Stack
char stk[MAXSTACK][MAXSYM];
int  top = -1;
void push(char *s) { strcpy(stk[++top], s); }
void pop_s()       { if (top >= 0) top--; }

int find_nt(char *x) {
    for (int i = 0; i < NNT;  i++) if (strcmp(NT[i],        x) == 0) return i;
    return -1;
}
int find_t(char *x) {
    for (int i = 0; i < NTER; i++) if (strcmp(TERMINALS[i], x) == 0) return i;
    return -1;
}

void run_parser(char tokens[][MAXSYM], int n) {
    printf("\n========================================\n");
    printf("          PARSER OUTPUT\n");
    printf("========================================\n");

    // Print token stream
    printf("Token stream: ");
    for (int i = 0; i < n; i++) printf("%s ", tokens[i]);
    printf("\n\n");

    push("$");
    push("PROGRAM");

    int ip       = 0;
    int rejected = 0;
    int step     = 1;

    printf("%-4s  %-48s %-20s %-20s %s\n",
           "Step","Stack (top->bottom)","Lookahead","Top","Production/Action");
    printf("%s\n",
           "----  ------------------------------------------------ "
           "-------------------- -------------------- "
           "-----------------------------------");

    while (top >= 0) {
        char X[MAXSYM];
        strcpy(X, stk[top]);
        char *a = tokens[ip];

        // Build stack string on heap
        char *sb = (char *)malloc(MAXSTACK * MAXSYM + 4);
        strcpy(sb, "[");
        for (int i = top; i >= 0; i--) {
            strcat(sb, stk[i]);
            if (i > 0) strcat(sb, " ");
        }
        strcat(sb, "]");

        printf("%-4d  %-48s %-20s %-20s ", step, sb, a, X);
        free(sb);

        int tindex = find_t(X);

        if (tindex != -1) {
            pop_s();
            if (strcmp(X, a) == 0) {
                printf("MATCH\n");
                ip++;
            } else {
                printf("ERROR: expected %s got %s\n", X, a);
                rejected = 1; break;
            }
        } else {
            int ntindex = find_nt(X);
            int aindex  = find_t(a);

            if (ntindex == -1 || aindex == -1) {
                printf("ERROR: unknown symbol '%s'\n", ntindex == -1 ? X : a);
                rejected = 1; break;
            }

            int prod = TABLE[ntindex][aindex];
            if (prod == 0) {
                printf("ERROR: no rule for %s on %s\n", X, a);
                rejected = 1; break;
            }

            pop_s();

            // Print production on heap buffer
            char *pl = (char *)malloc(512);
            if (strlen(RHS[prod]) == 0)
                snprintf(pl, 512, "%s -> epsilon", X);
            else
                snprintf(pl, 512, "%s -> %s", X, RHS[prod]);
            printf("%s\n", pl);
            free(pl);

            // Push RHS in reverse
            if (strlen(RHS[prod]) > 0) {
                char *tmp = strdup(RHS[prod]);
                char *p   = strtok(tmp, " ");
                char syms[20][MAXSYM];
                int  k = 0;
                while (p) { strcpy(syms[k++], p); p = strtok(NULL, " "); }
                for (int i = k - 1; i >= 0; i--) push(syms[i]);
                free(tmp);
            }
        }
        step++;
    }

    printf("\n");
    if (rejected || !(top == -1 && ip >= n)) {
        printf("========================================\n");
        printf("        RESULT:  *** REJECTED ***\n");
        printf("========================================\n");
    } else {
        printf("========================================\n");
        printf("        RESULT:  *** ACCEPTED ***\n");
        printf("========================================\n");
    }
}

// ============================================================
//  MAIN  —  runs lexer then parser in one shot
// ============================================================
int main() {
    char tokens[MAXTOK][MAXSYM];

    // Step 1: Lexer — tokenise input.txt → tokens[]
    int n = run_lexer(tokens);

    // Step 2: Parser — parse tokens[]
    run_parser(tokens, n);

    return 0;
}
