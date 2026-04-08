#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define MAXSTACK 200
#define MAXTOK 500
#define MAXSYM 50

// ----------------------
// Grammar Symbols (Mapped to Lexer Output)
// ----------------------
char *NT[] = {"S","A","E","Eprime","T","Tprime","F"};
#define NNT 7

// Terminals must exactly match the strings in your output.txt
char *TERMINALS[] = {"VARIABLE", "LEFT_PARENTHESES", "RIGHT_PARENTHESES", "OPERATOR", "NUMBER", "$"};
#define NTER 6

char *RHS[] = {
    "VARIABLE A",                           //1 S -> VARIABLE A
    "LEFT_PARENTHESES E RIGHT_PARENTHESES A", //2 A -> [ E ] A
    "",                                     //3 A -> epsilon
    "T Eprime",                             //4 E -> T E'
    "OPERATOR T Eprime",                    //5 E' -> OPERATOR T E'
    "",                                     //6 E' -> epsilon
    "F Tprime",                             //7 T -> F T'
    "OPERATOR F Tprime",                    //8 T' -> OPERATOR F T'
    "",                                     //9 T' -> epsilon
    "VARIABLE"                              //10 F -> VARIABLE
};

// ----------------------
// LL(1) Parsing Table
// ----------------------
int TABLE[NNT][NTER] = {
    // VAR  [  ]  OP NUM  $
    { 1, 0, 0, 0, 0, 0},  // S
    { 0, 2, 3, 0, 0, 3},  // A
    { 4, 0, 0, 0, 4, 0},  // E
    { 0, 0, 6, 5, 0, 6},  // Eprime
    { 7, 0, 0, 0, 7, 0},  // T
    { 0, 0, 9, 9, 8, 9},  // Tprime
    {10, 0, 0, 0, 0, 0}   // F
};

// ----------------------
// Stack
// ----------------------
char stack[MAXSTACK][MAXSYM];
int top = -1;
void push(char *s){ strcpy(stack[++top], s); }
char* pop(){ return (top >= 0) ? stack[top--] : NULL; }

void print_stack(){
    printf("[");
    for(int i = top; i >= 0; i--){
        printf("%s", stack[i]);
        if(i > 0) printf(", ");
    }
    printf("]");
}

// ----------------------
// Helpers
// ----------------------
int find_nt(char *x){
    for(int i = 0; i < NNT; i++)
        if(strcmp(NT[i], x) == 0) return i;
    return -1;
}

int find_t(char *x){
    for(int i = 0; i < NTER; i++)
        if(strcmp(TERMINALS[i], x) == 0) return i;
    return -1;
}

// Loads tokens from output.txt and filters out non-grammar tokens (like HEADER/COMMENT)
int load_tokens_from_file(char tokens[][MAXSYM]){
    FILE *f = fopen("output.txt", "r");
    if(!f) { printf("Error: output.txt not found!\n"); return 0; }
    
    int n = 0;
    char word[MAXSYM];
    while(fscanf(f, "%s", word) != EOF){
        // Treat 'NUMBERS' from lexer as 'NUMBER' for grammar
        if(strcmp(word, "NUMBERS") == 0) strcpy(word, "NUMBER");
        
        // Only load tokens relevant to this grammar
        if(find_t(word) != -1) {
            strcpy(tokens[n++], word);
        }
    }
    if(n == 0 || strcmp(tokens[n-1], "$") != 0)
        strcpy(tokens[n++], "$");
    
    fclose(f);
    return n;
}

// ----------------------
// Main Parser
// ----------------------
int main(){
    char input[MAXTOK][MAXSYM];
    int n = load_tokens_from_file(input);

    if(n == 0) return 1;

    push("$");
    push("S"); 

    int ip = 0;
    int rejected = 0; 

    printf("\n%-40s %-20s %-12s %-25s\n", "Stack", "Lookahead", "Top", "Production Applied");
    printf("----------------------------------------------------------------------------------------------------\n");

    while(top >= 0){
        char X[MAXSYM];
        strcpy(X, pop());
        char *a = input[ip];

        printf("%-40s %-20s %-12s ", "", a, X);

        int tindex = find_t(X);

        if(tindex != -1){
            if(strcmp(X, a) == 0){
                printf("%-25s\n", "match");
                ip++;
            } else {
                printf("%-25s\n", "not match");
                rejected = 1; break;
            }
            print_stack(); printf("\n");
            continue;
        }

        int ntindex = find_nt(X);
        int aindex = find_t(a);

        if(ntindex == -1 || aindex == -1){
            printf("%-25s\n", "not match");
            rejected = 1; break;
        }

        int prod = TABLE[ntindex][aindex];
        if(prod == 0){
            printf("%-25s\n", "not match");
            rejected = 1; break;
        }

        if(strlen(RHS[prod-1]) == 0)
            printf("%-25s\n", "epsilon");
        else
            printf("%-25s\n", RHS[prod-1]);

        if(strlen(RHS[prod-1]) > 0){
            char temp[200];
            strcpy(temp, RHS[prod-1]);
            char *p = strtok(temp, " ");
            char symbols[20][MAXSYM];
            int k = 0;
            while(p){ strcpy(symbols[k++], p); p = strtok(NULL, " "); }
            for(int i = k - 1; i >= 0; i--) push(symbols[i]);
        }
        print_stack(); printf("\n");
    }

    if(rejected) printf("\nRESULT: REJECTED\n");
    else if(top == -1 && ip == n) printf("\nRESULT: ACCEPTED\n");
    else printf("\nRESULT: REJECTED\n");

    return 0;
}