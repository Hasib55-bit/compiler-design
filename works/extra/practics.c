#include <stdio.h>
#include <string.h>
#include <ctype.h>

// Column Mapping
int get_col(char c) {
    if (c == '_') return 0;
    if (isalpha(c)) return 1;
    if (isdigit(c)) return 2;
    if (c == '.') return 3;
    if (c == ':') return 4;
    if (strchr("()[]", c)) return 5;
    return 6;
}

// Fixed 2D DFA Matrix
int dfa[20][7] = {
    {1, 8, 15, 5, -1, 16, -1},   // D0: Start
    {-1, 2, -1, -1, -1, -1, -1}, // D1: Var _
    {-1, 2, 3, -1, -1, -1, -1},  // D2: Var alpha
    {-1, 4, -1, -1, -1, -1, -1}, // D3: Var digit
    {-1, -1, -1, -1, -1, -1, -1},// D4: ACCEPT VAR
    {-1, -1, -1, 6, -1, -1, -1}, // D5: Dot 1
    {-1, -1, -1, -1, -1, -1, -1},// D6: ACCEPT END
    {-1, -1, -1, -1, -1, -1, -1},// D7: Null
    {10, 8, -1, -1, -1, -1, -1}, // D8: Alpha/Func
    {-1, -1, -1, -1, -1, -1, -1},// D9: Null
    {-1, 11, -1, -1, -1, -1, -1},// D10: loop_
    {-1, 11, 12, -1, -1, -1, -1},// D11: loop alpha
    {-1, -1, 13, -1, -1, -1, -1},// D12: loop dig1
    {-1, -1, -1, -1, 14, -1, -1},// D13: loop dig2
    {-1, -1, -1, -1, -1, -1, -1},// D14: ACCEPT LOOP
    {-1, -1, 15, -1, -1, -1, -1},// D15: ACCEPT NUMBER
    {-1, -1, -1, -1, -1, -1, -1} // D16: ACCEPT SYMBOL
};

void identify(char *lexeme) {
    if (strlen(lexeme) == 0) return;
    int state = 0;
    for (int i = 0; lexeme[i]; i++) {
        int col = get_col(lexeme[i]);
        state = (state != -1 && col < 6) ? dfa[state][col] : -1;
    }

    if (state == 4)  printf("%-20s | VAR\n", lexeme);
    else if (state == 6)  printf("%-20s | END\n", lexeme);
    else if (state == 14) printf("%-20s | LOOP\n", lexeme);
    else if (state == 15) printf("%-20s | NUMBER\n", lexeme);
    else if (state == 16) printf("%-20s | SYMBOL\n", lexeme);
    else if (state == 8) {
        if (strcmp(lexeme, "int") == 0 || strcmp(lexeme, "dec") == 0) printf("%-20s | TYPE\n", lexeme);
        else if (strcmp(lexeme, "mainFn") == 0) printf("%-20s | MAIN\n", lexeme);
        else if (strcmp(lexeme, "printfFn") == 0) printf("%-20s | PRINT\n", lexeme);
        else if (strstr(lexeme, "Fn")) printf("%-20s | FUNC\n", lexeme);
    }
}

int main() {
    FILE *f = fopen("input.txt", "r");
    if (!f) return 1;

    char raw[100];
    printf("%-20s | %s\n", "Lexeme", "Token");
    printf("------------------------------------\n");

    while (fscanf(f, "%s", raw) != EOF) {
        // Handle Header & Comments
        if (strcmp(raw, "#include<stdio.h>") == 0) { printf("%-20s | HEADER\n", raw); continue; }
        if (strncmp(raw, "//", 2) == 0) {
            char comment[100]; fgets(comment, 100, f);
            printf("//%-18s | COMMENT\n", strtok(comment, "\n")); continue;
        }

        // Split ".." if it's attached to the end of a word
        int len = strlen(raw);
        if (len > 2 && raw[len-1] == '.' && raw[len-2] == '.') {
            raw[len-2] = '\0';
            identify(raw);
            identify("..");
        } else {
            identify(raw);
        }
    }
    fclose(f);
    return 0;
}