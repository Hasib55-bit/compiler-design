#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define NUM_STATES 10
#define NUM_INPUTS 12


// Column mapping using switch
int get_col(char c) {
    if (isalpha(c)) return 0; // Group all letters
    if (isdigit(c)) return 1; // Group all digits
    
    switch (c) {
        case '_': return 2;
        case '.': return 3;
        case '/': return 4;
        case '#': return 5;
        case '[': return 6;
        case ']': return 7;
        case '(': return 8;
        case ')': return 9;
        case '=': 
        case '+': 
        case '-': 
        case '*': 
        case '<': 
        case '>': return 10;
        default:  return 11; // Whitespace and others
    }
}



// Token labels for static states
const char *token_labels[NUM_STATES] = {
    [2] = "VARIABLE",
    [3] = "NUMBERS",
    [4] = "END",
    [6] = "COMMENT",
    [7] = "HEADER",
    [8] = "L_BRACKET", // Temporary labels for the switch logic
    [9] = "R_BRACKET"
};


// Function to refine ID tokens (Main, Function, Loop, Keyword, etc)
const char* refine_id(char *s, int state) {
    if (state == 2) return "VARIABLE";
    if (state == 3) return "NUMBERS";
    if (strcmp(s, "dec") == 0 || strcmp(s, "int") == 0) return "DATATYPE";
    if (strcmp(s, "return") == 0 || strcmp(s, "while") == 0 || strcmp(s, "break") == 0) return "KEYWORD";
    if (strcmp(s, "mainFn") == 0) return "MAIN";
    if (strcmp(s, "printfFn") == 0) return "PRINT";
    if (strstr(s, "Fn")) return "FUNCTION";
    if (s[strlen(s)-1] == ':') return "LOOP";
    return "IDENTIFIER";
}



int main() {
    // DFA Table: 0:START, 1:ID, 2:VAR(_), 3:NUM, 4:DOT, 5:SLASH, 6:COMM, 7:HEAD
    // Columns: Alpha, Digit, _, ., /, #, [, ], (, ), Op, Other
    int next_state[NUM_STATES][NUM_INPUTS] = {
        /* 0:START */ { 1, 3, 2, 4, 5, 7, 8, 9, 8, 9, 8, 0 },
        /* 1:ID    */ { 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 2:VAR   */ { 2, 2, 2, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 3:NUM   */ { 0, 3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 },
        /* 4:DOT   */ { 0, 0, 0, 4, 0, 0, 0, 0, 0, 0, 0, 0 }, // Wait for ..
        /* 5:SLASH */ { 0, 0, 0, 0, 6, 0, 0, 0, 0, 0, 0, 0 }, // Wait for //
        /* 6:COMM  */ { 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6, 6 }, // Comment state
        /* 7:HEAD  */ { 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7, 7 }  // Header state
    };

    FILE *fptr = fopen("input.txt", "r");
    if (!fptr) { printf("File error\n"); return 1; }

    char c, buffer[100];
    int state = 0, b_idx = 0;

    printf("\n\n%-30s | %s\n", "Lexeme", "Token");
    printf("---------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col = get_col(c);
        int n_state = next_state[state][col];

        // Transition logic
        if (n_state != 0 && !(state == 6 && c == '\n')) {
            // Check for Loop ':' which is special
            if (state == 1 && c == ':') {
                buffer[b_idx++] = c;
                buffer[b_idx] = '\0';
                printf("%-30s | LOOP\n", buffer);
                b_idx = 0; state = 0;
            } else {
                state = n_state;
                buffer[b_idx++] = c;

                // Instant Header Terminal
                if (state == 7 && c == '>') {
                    buffer[b_idx] = '\0';
                    printf("%-30s | HEADER\n", buffer);
                    b_idx = 0; state = 0;
                }
            }
        } else {
            // Process buffer if a token is complete
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                if (state == 1 || state == 2) printf("%-30s | %s\n", buffer, refine_id(buffer, state));
                else if (state == 3) printf("%-30s | NUMBER\n", buffer);
                else if (state == 4) printf("%-30s | END\n", "..");
                else if (state == 6) printf("%-30s | COMMENT\n", buffer);
                
                b_idx = 0;
                state = 0;
            }

            // Handle Single Symbols (Instant states)
            if (!isspace(c)) {
                if (c == '[') printf("%-30c | LEFT PARENTHESES\n", c);
                else if (c == ']') printf("%-30c | RIGHT PARENTHESES\n", c);
                else if (c == '(') printf("%-30c | FIRST BRACKET\n", c);
                else if (c == ')') printf("%-30c | LAST BRACKET\n", c);
                else if (strchr("=+-*<>", c)) printf("%-30c | OPERATOR\n", c);
                
                // If the character started a new state (like '.' or '/'), re-process it
                if (get_col(c) < 6) {
                    ungetc(c, fptr);
                }
            }
        }
    }

    fclose(fptr);
    return 0;
}