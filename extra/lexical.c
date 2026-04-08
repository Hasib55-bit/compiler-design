#include <stdio.h>
#include <string.h>
#include <ctype.h>

#define NUM_STATES 18
#define NUM_INPUTS 13

// Column mapping optimized for your custom rules
int get_col(char c) {
    if (c == '_') return 0;
    if (c == '#') return 1;
    if (isdigit(c)) return 2;
    if (c == '.') return 3;
    if (c == ':') return 4;
    if (c == '(' || c == ')') return 5;
    if (c == '[' || c == ']') return 6;
    if (c == '/') return 7;
    if (isalpha(c)) return 8; // Generic alphabets
    return 12; // Others
}

// Token labels for acceptance states
const char *token_labels[NUM_STATES] = {
    [2]  = "HEADER",
    [4]  = "VAR",
    [6]  = "END",
    [9]  = "LOOP",
    [10] = "NUMBER",
    [11] = "SYMBOL",
    [15] = "COMMENT",
    [17] = "IDENTIFIER" // Temporary state for keywords/functions
};

int main() {
    // DFA table: next_state[state][input]
    // Columns: _  #  0-9  .  :  ()  []  /  a-z  oth
    int next_state[NUM_STATES][NUM_INPUTS] = {
        /*D0: Start*/    { 1, 12, 10,  5, 11, 11, 11, 13, 16, 11, 11, 11, 11 },
        /*D1: Var _*/    { -1, -1, -1, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1 },
        /*D2: Var alph*/ { -1, -1,  3, -1, -1, -1, -1, -1,  2, -1, -1, -1, -1 },
        /*D3: Var dig*/  { -1, -1, -1, -1, -1, -1, -1, -1,  4, -1, -1, -1, -1 },
        /*D4: ACC VAR*/  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D5: Dot 1*/    { -1, -1, -1,  6, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D6: ACC END*/  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D7: Loop mid*/ {  8, -1, -1, -1, -1, -1, -1, -1,  7, -1, -1, -1, -1 },
        /*D8: Loop dig*/ { -1, -1,  9, -1, -1, -1, -1, -1,  8, -1, -1, -1, -1 },
        /*D9: ACC LOOP*/ { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D10: ACC NUM*/ { -1, -1, 10, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D11: ACC SYM*/ { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D12: Header*/  { 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12, 12 }, // Simplified
        /*D13: Div/Com*/ { -1, -1, -1, -1, -1, -1, -1, 14, -1, -1, -1, -1, -1 },
        /*D14: ComBody*/ { 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14, 14 },
        /*D15: ACC COM*/ { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 },
        /*D16: Alpha*/   {  7, -1, -1, -1, -1, -1, -1, -1, 16, -1, -1, -1, -1 },
        /*D17: ACC ID*/  { -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1 }
    };

    FILE *fptr = fopen("input.txt", "r");
    if (!fptr) { printf("File error\n"); return 1; }

    char c, buffer[100];
    int state = 0, b_idx = 0;

    printf("%-20s | %s\n", "Lexeme", "Token");
    printf("------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col = get_col(c);

        // Word breaking logic
        if (isspace(c) || (state != 0 && (col == 5 || col == 6 || col == 4 || col == 3))) {
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char* label = token_labels[state];
                
                // Refine identification for Keywords/Functions
                if (state == 16 || state == 7) {
                    if (strcmp(buffer, "int") == 0 || strcmp(buffer, "dec") == 0) label = "TYPE";
                    else if (strcmp(buffer, "mainFn") == 0) label = "MAIN";
                    else if (strcmp(buffer, "printfFn") == 0) label = "PRINT";
                    else if (strstr(buffer, "Fn")) label = "FUNC";
                    else label = "KEYWORD";
                }
                if (buffer[0] == '#') label = "HEADER";
                
                printf("%-20s | %s\n", buffer, label ? label : "SYMBOL");
                b_idx = 0; state = 0;
            }
            if (isspace(c)) continue;
        }

        // Handle Comments
        if (c == '/' && state == 0) {
            char next = fgetc(fptr);
            if (next == '/') {
                buffer[b_idx++] = '/'; buffer[b_idx++] = '/';
                char comm[100]; fgets(comm, 100, fptr);
                strtok(comm, "\n");
                printf("%-20s | COMMENT\n", strcat(buffer, comm));
                b_idx = 0; state = 0; continue;
            } else ungetc(next, fptr);
        }

        state = (state != -1) ? next_state[state][col] : -1;
        buffer[b_idx++] = c;

        // Instant symbols
        if (col == 5 || col == 6 || (col == 4 && state == 14) || (state == 6)) {
            buffer[b_idx] = '\0';
            const char* label = token_labels[state];
            if (state == 6) label = "END";
            else if (state == 9) label = "LOOP";
            else label = "SYMBOL";
            
            printf("%-20s | %s\n", buffer, label);
            b_idx = 0; state = 0;
        }
    }
    fclose(fptr);
    return 0;
}