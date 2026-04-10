// ===================================
/*  Team Members:
    Md.Hasibur Rahman	241-15-806
    Md. Wahidur Rahman 	241-15-865
    Md. Arman Hossen Ripon	241-15-883
    Md. Rahul Hossain	241-15-766
    Md. Sabbir Hossine	241-15-673  */
// ===================================


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
    if (state == 2) {
        
        int len = strlen(s);
        if (len < 4) return "unk";          // minimum valid: _a1b
        if (s[0] != '_') return "unk";
        int i = 1;
        // one or more alphabets after underscore
        if (!isalpha(s[i])) return "unk";
        while (i < len && isalpha(s[i])) i++;
        // exactly one digit
        if (i >= len || !isdigit(s[i])) return "unk";
        i++;
        // exactly one alphabet, must be the last char
        if (i >= len || !isalpha(s[i])) return "unk";
        i++;
        if (i != len) return "unk";         // extra chars = invalid
        return "var";
    }

    if (strcmp(s, "dec") == 0 || strcmp(s, "int") == 0) return "type";
    if (strcmp(s, "return") == 0 || strcmp(s, "while") == 0 || strcmp(s, "break") == 0) return "key";
    if (strcmp(s, "mainFn") == 0) return "main";
    if (strcmp(s, "printfFn") == 0) return "prnt";

    
    int len = strlen(s);
    if (len > 2 && s[len-2] == 'F' && s[len-1] == 'n') {
        int ok = 1;
        for (int i = 0; i < len; i++) if (!isalpha(s[i])) { ok = 0; break; }
        if (ok) return "func";
    }

    
    if (strstr(s, "loop")) return "loop";

    return "id";
}


int valid_loop_label(char *s) {
    int len = strlen(s);
    // must start with "loop_"
    if (len < 9) return 0;                  // loop_ + 1alpha + 2digit + : = min 9
    if (strncmp(s, "loop_", 5) != 0) return 0;
    // last char must be ':'
    if (s[len-1] != ':') return 0;
    int i = 5;
    // one or more alphabets
    if (!isalpha(s[i])) return 0;
    while (i < len-1 && isalpha(s[i])) i++;
    // exactly two digits
    if (i + 2 > len - 1) return 0;
    if (!isdigit(s[i]) || !isdigit(s[i+1])) return 0;
    i += 2;
    // must now be at ':'
    if (i != len - 1) return 0;
    return 1;
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

    printf("\n========================================\n");
    printf("           LEXER OUTPUT\n");
    printf("========================================\n");
    printf("%-30s | %s\n", "Lexeme", "Token");
    printf("----------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col     = get_col(c);
        int n_state = next_state[state][col];

        if (n_state != 0 && !(state == 6 && c == '\n')) {

            
            if (state == 6 && !isalpha(c) && c != ' ') {
                printf("LEXER ERR: invalid char '%c' inside comment\n", c);
            }

            if (state == 1 && c == ':') {
                buffer[b_idx++] = c;
                buffer[b_idx]   = '\0';
                
                if (valid_loop_label(buffer)) {
                    printf("%-30s | loop\n", buffer);
                    strcpy(out_tokens[n++], "loop");
                } else {
                    printf("%-30s | unk (invalid loop label)\n", buffer);
                    strcpy(out_tokens[n++], "unk");
                }
                b_idx = 0; state = 0;
            } else {
                state = n_state;
                buffer[b_idx++] = c;
                if (state == 7 && c == '>') {
                    buffer[b_idx] = '\0';
                    printf("%-30s | head\n", buffer);
                    strcpy(out_tokens[n++], "head");
                    b_idx = 0; state = 0;
                }
            }
        } else {
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char *tname;
                if      (state == 1 || state == 2) tname = refine_id(buffer, state);
                else if (state == 3)               tname = "num";
                else if (state == 4)               tname = "end";
                else if (state == 6)               tname = "cmt";
                else                               tname = "unk";

                printf("%-30s | %s\n", buffer, tname);
                if (strcmp(tname, "cmt") != 0)
                    strcpy(out_tokens[n++], tname);

                b_idx = 0; state = 0;
            }

            if (!isspace(c)) {
                if (c == '[') {
                    printf("%-30c | lpar\n", c);
                    strcpy(out_tokens[n++], "lpar");
                } else if (c == ']') {
                    printf("%-30c | rpar\n", c);
                    strcpy(out_tokens[n++], "rpar");
                } else if (c == '(') {
                    printf("%-30c | lbrk\n", c);
                    strcpy(out_tokens[n++], "lbrk");
                } else if (c == ')') {
                    printf("%-30c | rbrk\n", c);
                    strcpy(out_tokens[n++], "rbrk");
                } else if (strchr("<>!=+-*", c)) {
                    char lexeme[3] = {c, '\0', '\0'};
                    char next = fgetc(fptr);
                    if (next == '=' && strchr("<>!=", c)) {
                        lexeme[1] = next;
                    } else {
                        if (next != EOF) ungetc(next, fptr);
                    }
                    printf("%-30s | op\n", lexeme);
                    strcpy(out_tokens[n++], "op");
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

char *NT[] = {
    "pgm",  "fdef", "fnam", "plst", "mpar",
    "slst", "stmt", "dstm", "astm", "rstm",
    "lstm", "lcnd", "pstm", "expr", "expp",
    "term", "fcal", "alst", "marg", "rbod"
};
#define NNT 20

char *TERMINALS[] = {
    "head", "type", "var", "op", "num", "end",
    "func", "main", "prnt", "key", "loop",
    "lpar", "rpar", "lbrk", "rbrk", "$"
};
#define NTER 16

char *RHS[] = {
    /* 0  */ "",
    /* 1  */ "head pgm",
    /* 2  */ "fdef pgm",
    /* 3  */ "",
    /* 4  */ "type fnam lbrk plst rbrk lpar slst rpar",
    /* 5  */ "func",
    /* 6  */ "main",
    /* 7  */ "type var mpar",
    /* 8  */ "",
    /* 9  */ "",
    /* 10 */ "stmt slst",
    /* 11 */ "",
    /* 12 */ "dstm",
    /* 13 */ "astm",
    /* 14 */ "rstm",
    /* 15 */ "lstm",
    /* 16 */ "pstm",
    /* 17 */ "type var op expr end",
    /* 18 */ "var op expr end",
    /* 19 */ "key rbod end",
    /* 20 */ "loop key lbrk lcnd rbrk lpar slst rpar",
    /* 21 */ "type var op expr end",
    /* 22 */ "prnt lbrk var rbrk end",
    /* 23 */ "term expp",
    /* 24 */ "op term expp",
    /* 25 */ "",
    /* 26 */ "var",
    /* 27 */ "num",
    /* 28 */ "fcal",
    /* 29 */ "func lbrk alst rbrk",
    /* 30 */ "var marg",
    /* 31 */ "num marg",
    /* 32 */ "",
    /* 33 */ "",
    /* 34 */ "var",
    /* 35 */ "num",
    /* 36 */ ""
};

int TABLE[NNT][NTER] = {
/* pgm  */ {  1,  2,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  3 },
/* fdef */ {  0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* fnam */ {  0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0 },
/* plst */ {  0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0 },
/* mpar */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0 },
/* slst */ {  0, 10, 10,  0,  0,  0,  0,  0, 10, 10, 10,  0, 11,  0,  0, 11 },
/* stmt */ {  0, 12, 13,  0,  0,  0,  0,  0, 16, 14, 15,  0,  0,  0,  0,  0 },
/* dstm */ {  0, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* astm */ {  0,  0, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* rstm */ {  0,  0,  0,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,  0,  0 },
/* lstm */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0 },
/* lcnd */ {  0, 21,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* pstm */ {  0,  0,  0,  0,  0,  0,  0,  0, 22,  0,  0,  0,  0,  0,  0,  0 },
/* expr */ {  0,  0, 23,  0, 23,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* expp */ {  0,  0,  0, 24,  0, 25,  0,  0,  0,  0,  0,  0,  0,  0, 25, 25 },
/* term */ {  0,  0, 26,  0, 27,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* fcal */ {  0,  0,  0,  0,  0,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* alst */ {  0,  0, 30,  0, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32,  0 },
/* marg */ {  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  0 },
/* rbod */ {  0,  0, 34,  0, 35, 36,  0,  0,  0,  0,  0,  0,  0,  0,  0, 36 }
};

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
    printf("\n\n======================================================= PARSER OUTPUT ========================================================\n\n");

    printf("Token stream: ");
    for (int i = 0; i < n; i++) printf("%s ", tokens[i]);
    printf("\n\n");

    push("$");
    push("pgm");

    int ip       = 0;
    int rejected = 0;

    printf("%-60s %-15s %-10s %s\n", "Stack", "Lookahead", "Top", "Rule/Action");
    printf("------------------------------------------------------------------------------------------------------------------------------\n");

    FILE *outptr = fopen("parser_output.txt", "w");
    if (!outptr) { printf("Cannot create parser_output.txt\n"); return; }

    fprintf(outptr, "Token stream: ");
    for (int i = 0; i < n; i++) fprintf(outptr, "%s ", tokens[i]);
    fprintf(outptr, "\n\n");
    fprintf(outptr, "%-60s %-15s %-10s %s\n", "Stack", "Lookahead", "Top", "Rule/Action");
    fprintf(outptr, "----------------------------------------------------------------------------------------------------------------------------\n");

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

        printf("%-60s %-15s %-10s ", sb, a, X);
        fprintf(outptr, "%-60s %-15s %-10s ", sb, a, X);
        free(sb);

        int tindex = find_t(X);

        if (tindex != -1) {
            pop_s();
            if (strcmp(X, a) == 0) {
                printf("MATCH\n");
                fprintf(outptr, "MATCH\n");
                ip++;
            } else {
                printf("ERR: mismatch (expected %s got %s)\n", X, a);
                fprintf(outptr, "ERR: mismatch (expected %s got %s)\n", X, a);
                rejected = 1; break;
            }
        } else {
            int ntindex = find_nt(X);
            int aindex  = find_t(a);

            if (ntindex == -1 || aindex == -1) {
                printf("ERR: unknown symbol '%s'\n", ntindex == -1 ? X : a);
                fprintf(outptr, "ERR: unknown symbol '%s'\n", ntindex == -1 ? X : a);
                rejected = 1; break;
            }

            int prod = TABLE[ntindex][aindex];
            if (prod == 0) {
                printf("ERR: no rule for %s on %s\n", X, a);
                fprintf(outptr, "ERR: no rule for %s on %s\n", X, a);
                rejected = 1; break;
            }

            pop_s();

            if (strlen(RHS[prod]) == 0) {
                printf("%s -> eps\n", X);
                fprintf(outptr, "%s -> eps\n", X);
            } else {
                printf("%s -> %s\n", X, RHS[prod]);
                fprintf(outptr, "%s -> %s\n", X, RHS[prod]);
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

    int found_main = 0;
    for (int i = 0; i < n; i++) {
        if (strcmp(tokens[i], "main") == 0) { found_main = 1; break; }
    }
    if (!found_main) {
        printf("ERR: mainFn is missing — program must have a mainFn function\n");
        fprintf(outptr, "ERR: mainFn is missing — program must have a mainFn function\n");
        rejected = 1;
    }

    if (rejected || ip < n - 1) {
        printf("\n======================================================= REJECTED ===============================================================\n");
        fprintf(outptr, "\n======================================================= REJECTED ===============================================================\n");
    } else {
        printf("\n======================================================= ACCEPTED ===============================================================\n");
        fprintf(outptr, "\n======================================================= ACCEPTED ===============================================================\n");
    }

    fclose(outptr);
    printf("\nParser output saved to parser_output.txt\n");
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

// AI chat history : https://github.com/armanhossen-dev/compiler-design/tree/main/perfectCode2/history