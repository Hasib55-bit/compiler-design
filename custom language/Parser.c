#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTACK 300
#define MAXTOK   300
#define MAXSYM   60

// ---- Non-terminals ----
char *NT[] = {
    "PROGRAM",    // 0
    "FUNC_DEF",   // 1
    "FUNCNAME",   // 2
    "PARAMLIST",  // 3
    "MOREPARAMS", // 4
    "STMTLIST",   // 5
    "STMT",       // 6
    "DECL_STMT",  // 7
    "ASSIGN_STMT",// 8
    "RETURN_STMT",// 9
    "LOOP_STMT",  // 10
    "LOOP_COND",  // 11
    "PRINT_STMT", // 12
    "EXPR",       // 13
    "EXPRP",      // 14
    "TERM",       // 15
    "FUNCALL",    // 16
    "ARGLIST",    // 17
    "MOREARGS",   // 18
    "RETBODY"     // 19
};
#define NNT 20

// ---- Terminals ----
char *TERMINALS[] = {
    "HEADER",            // 0
    "DATATYPE",          // 1
    "VARIABLE",          // 2
    "OPERATOR",          // 3
    "NUMBER",            // 4
    "END",               // 5
    "FUNCTION",          // 6
    "MAIN",              // 7
    "PRINT",             // 8
    "KEYWORD",           // 9
    "LOOP",              // 10
    "LEFT_PARENTHESES",  // 11
    "RIGHT_PARENTHESES", // 12
    "FIRST_BRACKET",     // 13
    "LAST_BRACKET",      // 14
    "$"                  // 15
};
#define NTER 16

char *RHS[] = {
    /* 0  unused    */ "",
    /* 1  PROGRAM   */ "HEADER PROGRAM",
    /* 2  PROGRAM   */ "FUNC_DEF PROGRAM",
    /* 3  PROGRAM   */ "",
    /* 4  FUNC_DEF  */ "DATATYPE FUNCNAME FIRST_BRACKET PARAMLIST LAST_BRACKET LEFT_PARENTHESES STMTLIST RIGHT_PARENTHESES",
    /* 5  FUNCNAME  */ "FUNCTION",
    /* 6  FUNCNAME  */ "MAIN",
    /* 7  PARAMLIST */ "DATATYPE VARIABLE MOREPARAMS",
    /* 8  PARAMLIST */ "",
    /* 9  MOREPARAMS*/ "",
    /* 10 STMTLIST  */ "STMT STMTLIST",
    /* 11 STMTLIST  */ "",
    /* 12 STMT      */ "DECL_STMT",
    /* 13 STMT      */ "ASSIGN_STMT",
    /* 14 STMT      */ "RETURN_STMT",
    /* 15 STMT      */ "LOOP_STMT",
    /* 16 STMT      */ "PRINT_STMT",
    /* 17 DECL_STMT */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 18 ASSIGN    */ "VARIABLE OPERATOR EXPR END",
    /* 19 RETURN    */ "KEYWORD RETBODY END",
    /* 20 LOOP_STMT */ "LOOP KEYWORD FIRST_BRACKET LOOP_COND LAST_BRACKET LEFT_PARENTHESES STMTLIST RIGHT_PARENTHESES",
    /* 21 LOOP_COND */ "DATATYPE VARIABLE OPERATOR EXPR END",
    /* 22 PRINT     */ "PRINT FIRST_BRACKET VARIABLE LAST_BRACKET END",
    /* 23 EXPR      */ "TERM EXPRP",
    /* 24 EXPRP     */ "OPERATOR TERM EXPRP",
    /* 25 EXPRP     */ "",
    /* 26 TERM      */ "VARIABLE",
    /* 27 TERM      */ "NUMBER",
    /* 28 TERM      */ "FUNCALL",
    /* 29 FUNCALL   */ "FUNCTION FIRST_BRACKET ARGLIST LAST_BRACKET",
    /* 30 ARGLIST   */ "VARIABLE MOREARGS",
    /* 31 ARGLIST   */ "NUMBER MOREARGS",
    /* 32 ARGLIST   */ "",
    /* 33 MOREARGS  */ "",
    /* 34 RETBODY   */ "VARIABLE",
    /* 35 RETBODY   */ "NUMBER",
    /* 36 RETBODY   */ ""
};

/*
 * LL(1) Parse Table [NNT][NTER]
 * Cols: HDR(0) DT(1) VAR(2) OP(3) NUM(4) END(5) FN(6) MAIN(7)
 *       PRT(8) KWD(9) LOOP(10) L_P(11) R_P(12) F_B(13) L_B(14) $(15)
 */
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

// ---- Stack ----
char stack[MAXSTACK][MAXSYM];
int top = -1;
void push(char *s) { strcpy(stack[++top], s); }
char *pop()        { return (top >= 0) ? stack[top--] : NULL; }

// ---- Helpers ----
int find_nt(char *x) {
    for (int i = 0; i < NNT; i++)
        if (strcmp(NT[i], x) == 0) return i;
    return -1;
}

int find_t(char *x) {
    for (int i = 0; i < NTER; i++)
        if (strcmp(TERMINALS[i], x) == 0) return i;
    return -1;
}

int tokenize(char *line, char tokens[][MAXSYM]) {
    int n = 0;
    char *p = strtok(line, " \t\n\r");
    while (p && n < MAXTOK - 1) {
        if (strcmp(p, "COMMENT") != 0)   // skip COMMENT tokens
            strcpy(tokens[n++], p);
        p = strtok(NULL, " \t\n\r");
    }
    if (n == 0 || strcmp(tokens[n - 1], "$") != 0)
        strcpy(tokens[n++], "$");
    return n;
}

// ---- Main ----
int main() {
    FILE *fptr = fopen("output.txt", "r");
    if (!fptr) { printf("Cannot open output.txt\n"); return 1; }

    // ── FIX 1: heap-allocate large buffers to avoid stack overflow ──
    char *alltext = (char *)calloc(10000, 1);
    char line[1024];
    while (fgets(line, sizeof(line), fptr))
        strcat(alltext, line);
    fclose(fptr);

    char input[MAXTOK][MAXSYM];
    int n = tokenize(alltext, input);
    free(alltext);

    FILE *outptr = fopen("parser_output.txt", "w");
    if (!outptr) { printf("Cannot create parser_output.txt\n"); return 1; }

    // Print token stream
    printf("\n===== TOKEN STREAM =====\n");
    fprintf(outptr, "===== TOKEN STREAM =====\n");
    for (int i = 0; i < n; i++) {
        printf("%s ", input[i]);
        fprintf(outptr, "%s ", input[i]);
    }
    printf("\n\n");
    fprintf(outptr, "\n\n");

    push("$");
    push("PROGRAM");

    int ip       = 0;
    int rejected = 0;
    int step     = 1;

    // column widths
    printf("%-4s %-50s %-20s %-20s %-s\n",
           "Step","Stack (top->bottom)","Lookahead","Top","Production/Action");
    printf("%-4s %-50s %-20s %-20s %-s\n",
           "----","--------------------------------------------------",
           "--------------------","--------------------",
           "-----------------------------------");
    fprintf(outptr,"%-4s %-50s %-20s %-20s %-s\n",
            "Step","Stack (top->bottom)","Lookahead","Top","Production/Action");
    fprintf(outptr,"%-4s %-50s %-20s %-20s %-s\n",
            "----","--------------------------------------------------",
            "--------------------","--------------------",
            "-----------------------------------");

    while (top >= 0) {
        char X[MAXSYM];
        strcpy(X, stack[top]);   // peek without popping
        char *a = input[ip];

        // ── FIX 2: heap-allocate stackbuf so it can't overflow ──
        char *stackbuf = (char *)malloc(MAXSTACK * MAXSYM + 4);
        strcpy(stackbuf, "[");
        for (int i = top; i >= 0; i--) {
            strcat(stackbuf, stack[i]);
            if (i > 0) strcat(stackbuf, " ");
        }
        strcat(stackbuf, "]");

        printf("%-4d %-50s %-20s %-20s ", step, stackbuf, a, X);
        fprintf(outptr, "%-4d %-50s %-20s %-20s ", step, stackbuf, a, X);
        free(stackbuf);

        int tindex = find_t(X);

        if (tindex != -1) {
            // Terminal on stack
            pop();
            if (strcmp(X, a) == 0) {
                printf("MATCH\n");
                fprintf(outptr, "MATCH\n");
                ip++;
            } else {
                printf("ERROR: terminal mismatch (expected %s got %s)\n", X, a);
                fprintf(outptr, "ERROR: terminal mismatch (expected %s got %s)\n", X, a);
                rejected = 1;
                break;
            }
        } else {
            int ntindex = find_nt(X);
            int aindex  = find_t(a);

            if (ntindex == -1 || aindex == -1) {
                printf("ERROR: unknown symbol '%s'\n", (ntindex == -1 ? X : a));
                fprintf(outptr, "ERROR: unknown symbol '%s'\n", (ntindex == -1 ? X : a));
                rejected = 1;
                break;
            }

            int prod = TABLE[ntindex][aindex];
            if (prod == 0) {
                printf("ERROR: no rule for %s on lookahead %s\n", X, a);
                fprintf(outptr, "ERROR: no rule for %s on lookahead %s\n", X, a);
                rejected = 1;
                break;
            }

            pop();

            // ── FIX 3: heap-allocate prodlabel so long productions don't overflow ──
            char *prodlabel = (char *)malloc(512);
            if (strlen(RHS[prod]) == 0) {
                snprintf(prodlabel, 512, "%s -> epsilon", X);
            } else {
                snprintf(prodlabel, 512, "%s -> %s", X, RHS[prod]);
            }
            printf("%s\n", prodlabel);
            fprintf(outptr, "%s\n", prodlabel);
            free(prodlabel);

            // Push RHS symbols in reverse order
            if (strlen(RHS[prod]) > 0) {
                char *temp = strdup(RHS[prod]);
                char *p    = strtok(temp, " ");
                char syms[20][MAXSYM];
                int  k = 0;
                while (p) { strcpy(syms[k++], p); p = strtok(NULL, " "); }
                for (int i = k - 1; i >= 0; i--) push(syms[i]);
                free(temp);
            }
        }
        step++;
    }

    printf("\n");
    fprintf(outptr, "\n");

    if (rejected) {
        printf("===== RESULT: REJECTED =====\n");
        fprintf(outptr, "===== RESULT: REJECTED =====\n");
    } else if (top == -1 && ip >= n) {
        printf("===== RESULT: ACCEPTED =====\n");
        fprintf(outptr, "===== RESULT: ACCEPTED =====\n");
    } else {
        printf("===== RESULT: REJECTED =====\n");
        fprintf(outptr, "===== RESULT: REJECTED =====\n");
    }

    fclose(outptr);
    printf("\nParser output saved to parser_output.txt\n");
    return 0;
}