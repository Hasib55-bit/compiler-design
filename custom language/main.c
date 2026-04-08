#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include <stdlib.h>

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
    int  n = 0;

    printf("\n===============================================\n");
    printf("          LEXER OUTPUT\n");
    printf("=================================================\n");
    printf("%-30s | %s\n", "Lexeme", "Token");
    printf("-------------------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col     = get_col(c);
        int n_state = next_state[state][col];

        if (n_state != 0 && !(state == 6 && c == '\n')) {
            if (state == 1 && c == ':') {
                buffer[b_idx++] = c;
                buffer[b_idx]   = '\0';
                printf("%-30s | LOOP\n", buffer);
                strcpy(out_tokens[n++], "LOOP");
                b_idx = 0; state = 0;
            } else {
                state = n_state;
                buffer[b_idx++] = c;
                if (state == 7 && c == '>') {
                    buffer[b_idx] = '\0';
                    printf("%-30s | HEADER\n", buffer);
                    strcpy(out_tokens[n++], "HEADER");
                    b_idx = 0; state = 0;
                }
            }
        } else {
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char *tname;
                if      (state == 1 || state == 2) tname = refine_id(buffer, state);
                else if (state == 3)               tname = "NUMBER";
                else if (state == 4)               tname = "END";
                else if (state == 6)               tname = "COMMENT";
                else                               tname = "UNKNOWN";

                printf("%-30s | %s\n", buffer, tname);
                if (strcmp(tname, "COMMENT") != 0)
                    strcpy(out_tokens[n++], tname);

                b_idx = 0; state = 0;
            }

            if (!isspace(c)) {
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
                        lexeme[1] = next;
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
    strcpy(out_tokens[n++], "$");
    return n;
}

// ============================================================
//  PARSER
// ============================================================

// Non-terminals (shortened)
char *NT[] = {
    "PROG",   // 0  PROGRAM
    "FDEF",   // 1  FUNC_DEF
    "FNAME",  // 2  FUNCNAME
    "PLIST",  // 3  PARAMLIST
    "MPAR",   // 4  MOREPARAMS
    "SLIST",  // 5  STMTLIST
    "STMT",   // 6  STMT
    "DECL",   // 7  DECL_STMT
    "ASGN",   // 8  ASSIGN_STMT
    "RET",    // 9  RETURN_STMT
    "LPST",   // 10 LOOP_STMT   (LPST avoids clash with terminal LOOP)
    "LCND",   // 11 LOOP_COND
    "PRNT",   // 12 PRINT_STMT
    "EXPR",   // 13 EXPR
    "EXPP",   // 14 EXPRP
    "TERM",   // 15 TERM
    "FCAL",   // 16 FUNCALL
    "ARGS",   // 17 ARGLIST
    "MARG",   // 18 MOREARGS
    "RETB"    // 19 RETBODY
};
#define NNT 20

char *TERMINALS[] = {
    "HEADER","DATATYPE","VARIABLE","OPERATOR","NUMBER","END",
    "FUNCTION","MAIN","PRINT","KEYWORD","LOOP",
    "LEFT_PARENTHESES","RIGHT_PARENTHESES","FIRST_BRACKET","LAST_BRACKET","$"
};
#define NTER 16

char *RHS[] = {
    /* 0  unused */ "",
    /* 1  PROG   */ "HEADER PROG",
    /* 2  PROG   */ "FDEF PROG",
    /* 3  PROG   */ "",
    /* 4  FDEF   */ "DATATYPE FNAME FIRST_BRACKET PLIST LAST_BRACKET LEFT_PARENTHESES SLIST RIGHT_PARENTHESES",
    /* 5  FNAME  */ "FUNCTION",
    /* 6  FNAME  */ "MAIN",
    /* 7  PLIST  */ "DATATYPE VARIABLE MPAR",
    /* 8  PLIST  */ "",
    /* 9  MPAR   */ "",
    /* 10 SLIST  */ "STMT SLIST",
    /* 11 SLIST  */ "",
    /* 12 STMT   */ "DECL",
    /* 13 STMT   */ "ASGN",
    /* 14 STMT   */ "RET",
    /* 15 STMT   */ "LPST",
    /* 16 STMT   */ "PRNT",
    /* 17 DECL   */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 18 ASGN   */ "VARIABLE OPERATOR EXPR END",
    /* 19 RET    */ "KEYWORD RETB END",
    /* 20 LPST   */ "LOOP KEYWORD FIRST_BRACKET LCND LAST_BRACKET LEFT_PARENTHESES SLIST RIGHT_PARENTHESES",
    /* 21 LCND   */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 22 PRNT   */ "PRINT FIRST_BRACKET VARIABLE LAST_BRACKET END",
    /* 23 EXPR   */ "TERM EXPP",
    /* 24 EXPP   */ "OPERATOR TERM EXPP",
    /* 25 EXPP   */ "",
    /* 26 TERM   */ "VARIABLE",
    /* 27 TERM   */ "NUMBER",
    /* 28 TERM   */ "FCAL",
    /* 29 FCAL   */ "FUNCTION FIRST_BRACKET ARGS LAST_BRACKET",
    /* 30 ARGS   */ "VARIABLE MARG",
    /* 31 ARGS   */ "NUMBER MARG",
    /* 32 ARGS   */ "",
    /* 33 MARG   */ "",
    /* 34 RETB   */ "VARIABLE",
    /* 35 RETB   */ "NUMBER",
    /* 36 RETB   */ ""
};

/*
 * LL(1) Parse Table [NNT][NTER]
 * Cols: HDR(0) DT(1) VAR(2) OP(3) NUM(4) END(5) FN(6) MAIN(7)
 *       PRT(8) KWD(9) LOOP(10) L_P(11) R_P(12) F_B(13) L_B(14) $(15)
 */
int TABLE[NNT][NTER] = {
/* PROG  */ {  1,  2,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  3 },
/* FDEF  */ {  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* FNAME */ {  0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0 },
/* PLIST */ {  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0 },
/* MPAR  */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0 },
/* SLIST */ {  0, 10, 10,  0,  0,  0,  0,  0, 10, 10, 10,  0, 11,  0,  0, 11 },
/* STMT  */ {  0, 12, 13,  0,  0,  0,  0,  0, 16, 14, 15,  0,  0,  0,  0,  0 },
/* DECL  */ {  0, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* ASGN  */ {  0,  0, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* RET   */ {  0,  0,  0,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,  0,  0 },
/* LPST  */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0 },
/* LCND  */ {  0, 21,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* PRNT  */ {  0,  0,  0,  0,  0,  0,  0,  0, 22,  0,  0,  0,  0,  0,  0,  0 },
/* EXPR  */ {  0,  0, 23,  0, 23,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* EXPP  */ {  0,  0,  0, 24,  0, 25,  0,  0,  0,  0,  0,  0,  0,  0, 25, 25 },
/* TERM  */ {  0,  0, 26,  0, 27,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* FCAL  */ {  0,  0,  0,  0,  0,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* ARGS  */ {  0,  0, 30,  0, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32,  0 },
/* MARG  */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  0 },
/* RETB  */ {  0,  0, 34,  0, 35, 36,  0,  0,  0,  0,  0,  0,  0,  0,  0, 36 }
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

    printf("Token stream: ");
    for (int i = 0; i < n; i++) printf("%s ", tokens[i]);
    printf("\n\n");

    push("$");
    push("PROG");

    int ip       = 0;
    int rejected = 0;
    
    printf("%-80s %-20s %-20s %s\n",
           "Stack (top->bottom)", "Lookahead", "Top", "Production/Action");
    printf("%s\n",
           "---------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------");

    while (top >= 0) {
        char X[MAXSYM];
        strcpy(X, stk[top]);
        char *a = tokens[ip];

        char *sb = (char *)malloc(MAXSTACK * MAXSYM + 4);
        strcpy(sb, "[");
        for (int i = top; i >= 0; i--) {
            strcat(sb, stk[i]);
            if (i > 0) strcat(sb, " ");
        }
        strcat(sb, "]");

        printf("%-80s %-20s %-20s ", sb, a, X);
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

            char *pl = (char *)malloc(512);
            if (strlen(RHS[prod]) == 0)
                snprintf(pl, 512, "%s -> epsilon", X);
            else
                snprintf(pl, 512, "%s -> %s", X, RHS[prod]);
            printf("%s\n", pl);
            free(pl);

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
    }

    printf("\n");
    if (rejected || !(top == -1 && ip >= n)) {
        printf("=====================================================================================================================================================================================================\n");
        printf("        RESULT:  *** REJECTED ***\n");
    } else {
        printf("=====================================================================================================================================================================================================\n");
        printf("        RESULT:  *** ACCEPTED ***\n");
    }
}

// ============================================================
//  MAIN
// ============================================================
int main() {
    char tokens[MAXTOK][MAXSYM];
    int n = run_lexer(tokens);
    run_parser(tokens, n);
    return 0;
}