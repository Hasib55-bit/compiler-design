#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTACK 300
#define MAXTOK   300
#define MAXSYM   50

// ---- Non-terminals (Shortened) ----
char *NT[] = {
    "pgm",  "fdef", "fnam", "plst", "mpar", 
    "slst", "stmt", "dstm", "astm", "rstm", 
    "lstm", "lcnd", "pstm", "expr", "expp", 
    "term", "fcal", "alst", "marg", "rbod"
};
#define NNT 20

// ---- Terminals (Shortened) ----
char *TERMINALS[] = {
    "head", "type", "var", "op", "num", "end", "func", "main", 
    "prnt", "key", "loop", "lpar", "rpar", "lbrk", "rbrk", "$"
};
#define NTER 16

char *RHS[] = {
    /* 0 */ "",
    /* 1 */ "head pgm",
    /* 2 */ "fdef pgm",
    /* 3 */ "",
    /* 4 */ "type fnam lbrk plst rbrk lpar slst rpar",
    /* 5 */ "func",
    /* 6 */ "main",
    /* 7 */ "type var mpar",
    /* 8 */ "",
    /* 9 */ "",
    /* 10*/ "stmt slst",
    /* 11*/ "",
    /* 12*/ "dstm",
    /* 13*/ "astm",
    /* 14*/ "rstm",
    /* 15*/ "lstm",
    /* 16*/ "pstm",
    /* 17*/ "type var op expr end",
    /* 18*/ "var op expr end",
    /* 19*/ "key rbod end",
    /* 20*/ "loop key lbrk lcnd rbrk lpar slst rpar",
    /* 21*/ "type var op expr end",
    /* 22*/ "prnt lbrk var rbrk end",
    /* 23*/ "term expp",
    /* 24*/ "op term expp",
    /* 25*/ "",
    /* 26*/ "var",
    /* 27*/ "num",
    /* 28*/ "func",
    /* 29*/ "func lbrk alst rbrk",
    /* 30*/ "var marg",
    /* 31*/ "num marg",
    /* 32*/ "",
    /* 33*/ "",
    /* 34*/ "var",
    /* 35*/ "num",
    /* 36*/ ""
};

int TABLE[NNT][NTER] = {
/* pgm  */ { 1,  2,  0,  0,  0,  0,  0,  2,  0,  0,  0,  0,  0,  0,  0,  3 },
/* fdef */ { 0,  4,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* fnam */ { 0,  0,  0,  0,  0,  0,  5,  6,  0,  0,  0,  0,  0,  0,  0,  0 },
/* plst */ { 0,  7,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  8,  0 },
/* mpar */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  9,  0 },
/* slst */ { 0, 10, 10,  0,  0,  0,  0,  0, 10, 10, 10,  0, 11,  0,  0, 11 },
/* stmt */ { 0, 12, 13,  0,  0,  0,  0,  0, 16, 14, 15,  0,  0,  0,  0,  0 },
/* dstm */ { 0, 17,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* astm */ { 0,  0, 18,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* rstm */ { 0,  0,  0,  0,  0,  0,  0,  0,  0, 19,  0,  0,  0,  0,  0,  0 },
/* lstm */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 20,  0,  0,  0,  0,  0 },
/* lcnd */ { 0, 21,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* pstm */ { 0,  0,  0,  0,  0,  0,  0,  0, 22,  0,  0,  0,  0,  0,  0,  0 },
/* expr */ { 0,  0, 23,  0, 23,  0, 23,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* expp */ { 0,  0,  0, 24,  0, 25,  0,  0,  0,  0,  0,  0,  0,  0, 25, 25 },
/* term */ { 0,  0, 26,  0, 27,  0, 28,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* fcal */ { 0,  0,  0,  0,  0,  0, 29,  0,  0,  0,  0,  0,  0,  0,  0,  0 },
/* alst */ { 0,  0, 30,  0, 31,  0,  0,  0,  0,  0,  0,  0,  0,  0, 32,  0 },
/* marg */ { 0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0, 33,  0 },
/* rbod */ { 0,  0, 34,  0, 35, 36,  0,  0,  0,  0,  0,  0,  0,  0,  0, 36 }
};

char stack[MAXSTACK][MAXSYM];
int top = -1;
void push(char *s) { strcpy(stack[++top], s); }
char *pop()        { return (top >= 0) ? stack[top--] : NULL; }

int find_nt(char *x) {
    for (int i = 0; i < NNT; i++) if (strcmp(NT[i], x) == 0) return i;
    return -1;
}

int find_t(char *x) {
    for (int i = 0; i < NTER; i++) if (strcmp(TERMINALS[i], x) == 0) return i;
    return -1;
}

int tokenize(char *line, char tokens[][MAXSYM]) {
    int n = 0;
    char *p = strtok(line, " \t\n\r");
    while (p && n < MAXTOK - 1) {
        if (strcmp(p, "cmt") != 0) strcpy(tokens[n++], p);
        p = strtok(NULL, " \t\n\r");
    }
    if (n == 0 || strcmp(tokens[n - 1], "$") != 0) strcpy(tokens[n++], "$");
    return n;
}

int main() {
    FILE *fptr = fopen("output.txt", "r");
    if (!fptr) { printf("File error\n"); return 1; }

    char *alltext = (char *)calloc(10000, 1);
    char line[1024];
    while (fgets(line, sizeof(line), fptr)) strcat(alltext, line);
    fclose(fptr);

    char input[MAXTOK][MAXSYM];
    int n = tokenize(alltext, input);
    free(alltext);

    FILE *outptr = fopen("parser_output.txt", "w");
    if (!outptr) { printf("File error\n"); return 1; }

    push("$");
    push("pgm");

    int ip = 0, rejected = 0;

    printf("\n\n%-50s %-15s %-10s %-s\n", "Stack", "Lookahead", "Top", "Rule/Action");
    printf("----------------------------------------------------------------------------------------------------------------------------\n");
    fprintf(outptr, "%-50s %-15s %-10s %-s\n", "Stack", "Lookahead", "Top", "Rule/Action");

    while (top >= 0) {
        char X[MAXSYM];
        strcpy(X, stack[top]);
        char *a = input[ip];

        char *stackbuf = (char *)malloc(MAXSTACK * MAXSYM + 4);
        strcpy(stackbuf, "[");
        for (int i = top; i >= 0; i--) {
            strcat(stackbuf, stack[i]);
            if (i > 0) strcat(stackbuf, " ");
        }
        strcat(stackbuf, "]");

        printf("%-50s %-15s %-10s ", stackbuf, a, X);
        fprintf(outptr, "%-50s %-15s %-10s ", stackbuf, a, X);
        free(stackbuf);

        int tindex = find_t(X);

        if (tindex != -1) {
            pop();
            if (strcmp(X, a) == 0) {
                printf("MATCH\n");
                fprintf(outptr, "MATCH\n");
                ip++;
            } else {
                printf("ERR: Mismatch\n");
                fprintf(outptr, "ERR: Mismatch\n");
                rejected = 1; break;
            }
        } else {
            int ntindex = find_nt(X);
            int aindex  = find_t(a);
            if (ntindex == -1 || aindex == -1) {
                printf("ERR: Unknown\n");
                fprintf(outptr, "ERR: Unknown\n");
                rejected = 1; break;
            }
            int prod = TABLE[ntindex][aindex];
            if (prod == 0) {
                printf("ERR: No rule\n");
                fprintf(outptr, "ERR: No rule\n");
                rejected = 1; break;
            }
            pop();
            if (strlen(RHS[prod]) == 0) {
                printf("%s -> eps\n", X);
                fprintf(outptr, "%s -> eps\n", X);
            } else {
                printf("%s -> %s\n", X, RHS[prod]);
                fprintf(outptr, "%s -> %s\n", X, RHS[prod]);
                char *temp = strdup(RHS[prod]);
                char *p = strtok(temp, " ");
                char syms[20][MAXSYM];
                int k = 0;
                while (p) { strcpy(syms[k++], p); p = strtok(NULL, " "); }
                for (int i = k - 1; i >= 0; i--) push(syms[i]);
                free(temp);
            }
        }
    }

    if (rejected || ip < n - 1) {
        printf("\n======================================================= REJECTED ============================================================\n");
        fprintf(outptr, "\n======================================================= REJECTED ============================================================\n");
    } else {
        printf("\n======================================================= ACCEPTED ============================================================\n");
        fprintf(outptr, "\n======================================================= ACCEPTED ============================================================\n");
    }

    fclose(outptr);
    return 0;
}

// AI chat history : https://github.com/armanhossen-dev/compiler-design/tree/main/perfectCode2/history