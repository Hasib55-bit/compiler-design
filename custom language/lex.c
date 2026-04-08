#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define NUM_STATES 15
#define NUM_INPUTS 6

// Column mapping: _=0, alpha=1, digit=2, .=3, :=4, others=5
int get_col(char c) {
    if (c == '_') return 0;
    if (isalpha(c)) return 1;
    if (isdigit(c)) return 2;
    if (c == '.') return 3;
    if (c == ':') return 4;
    return 5;
}

// Token labels for final states
const char *token_labels[NUM_STATES] = {
    [4]  = "VAR",
    [6]  = "END",
    [12] = "NUMBER",
    [13] = "SYMBOL",
    [14] = "LOOP"
};

int main() {
    // DFA table: next_state[state][input]
    // Columns: _(0)  alpha(1)  digit(2)  .(3)  :(4)  oth(5)
    int next_state[NUM_STATES][NUM_INPUTS] = {
        /*D0: Start*/   {  1,  7, 12,  5, -1, 13 },
        /*D1: Var _*/   { -1,  2, -1, -1, -1, -1 },
        /*D2: Var al*/  { -1,  2,  3, -1, -1, -1 },
        /*D3: Var dig*/ { -1,  4, -1, -1, -1, -1 },
        /*D4: VAR ACC*/ { -1, -1, -1, -1, -1, -1 },
        /*D5: Dot 1*/   { -1, -1, -1,  6, -1, -1 },
        /*D6: END ACC*/ { -1, -1, -1, -1, -1, -1 },
        /*D7: ID/TYPE*/ {  8,  7, -1, -1, -1, -1 },
        /*D8: Loop_*/   { -1,  9, -1, -1, -1, -1 },
        /*D9: Loop al*/ { -1,  9, 10, -1, -1, -1 },
        /*D10: Loop d1*/{ -1, -1, 11, -1, -1, -1 },
        /*D11: Loop d2*/{ -1, -1, -1, -1, 14, -1 },
        /*D12: NUM ACC*/{ -1, -1, 12, -1, -1, -1 },
        /*D13: SYM ACC*/{ -1, -1, -1, -1, -1, -1 },
        /*D14: LOOP ACC*/{ -1, -1, -1, -1, -1, -1 }
    };

    FILE *fptr = fopen("input.txt", "r");
    if (!fptr) { printf("File error\n"); return 1; }

    char c, buffer[100];
    int state = 0, b_idx = 0;

    printf("%-20s | %s\n", "Lexeme", "Token");
    printf("---------------------|----------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        // Special manual check for the fixed Header line
        if (c == '#' && state == 0) {
            buffer[b_idx++] = c;
            while ((c = fgetc(fptr)) != EOF && !isspace(c)) buffer[b_idx++] = c;
            buffer[b_idx] = '\0';
            printf("%-20s | HEADER\n", buffer);
            b_idx = 0; state = 0; continue;
        }

        // Special manual check for Comments
        if (c == '/' && state == 0) {
            char next = fgetc(fptr);
            if (next == '/') {
                buffer[b_idx++] = '/'; buffer[b_idx++] = '/';
                while ((c = fgetc(fptr)) != EOF && c != '\n') buffer[b_idx++] = c;
                buffer[b_idx] = '\0';
                printf("%-20s | COMMENT\n", buffer);
                b_idx = 0; state = 0; continue;
            } else ungetc(next, fptr);
        }

        int col = get_col(c);
        int n_state = (state != -1) ? next_state[state][col] : -1;

        // Transition logic: if current char breaks current token or is space
        if (isspace(c) || n_state == -1) {
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char* label = token_labels[state];
                
                // Refinement for keywords/functions ending in State 7
                if (state == 7) {
                    if (strcmp(buffer, "int") == 0 || strcmp(buffer, "dec") == 0) label = "TYPE";
                    else if (strcmp(buffer, "mainFn") == 0) label = "MAIN";
                    else if (strcmp(buffer, "printfFn") == 0) label = "PRINT";
                    else if (strstr(buffer, "Fn")) label = "FUNC";
                    else label = "KEYWORD";
                }
                
                printf("%-20s | %s\n", buffer, label ? label : "SYMBOL");
                b_idx = 0; state = 0;
                if (!isspace(c)) ungetc(c, fptr); // Reprocess char
                continue;
            }
            if (isspace(c)) continue;
        }

        state = n_state;
        buffer[b_idx++] = c;

        // Instant Acceptance for terminal symbols (.. or :)
        if (state == 6 || state == 14 || state == 4) {
            buffer[b_idx] = '\0';
            printf("%-20s | %s\n", buffer, token_labels[state]);
            b_idx = 0; state = 0;
        }
    }

    fclose(fptr);
    return 0;
}