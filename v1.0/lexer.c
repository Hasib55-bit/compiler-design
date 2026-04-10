#include <stdio.h>
#include <ctype.h>
#include <string.h>

#define NUM_STATES 10
#define NUM_INPUTS 12

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
        case '=': 
        case '+': 
        case '-': 
        case '*': 
        case '<': 
        case '!': 
        case '>': return 10;
        default:  return 11; 
    }
}

const char* refine_id(char *s, int state) {
    if (state == 2) return "var";
    if (state == 3) return "num";
    if (strcmp(s, "dec") == 0 || strcmp(s, "int") == 0) return "type";
    if (strcmp(s, "return") == 0 || strcmp(s, "while") == 0 || strcmp(s, "break") == 0) return "key";
    if (strcmp(s, "mainFn") == 0) return "main";
    if (strcmp(s, "printfFn") == 0) return "prnt";
    if (strstr(s, "Fn")) return "func";
    if (strstr(s, "loop") != NULL) return "loop";
    return "id";
}

int main() {
    int next_state[NUM_STATES][NUM_INPUTS] = {
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
    FILE *outptr = fopen("output.txt", "w");
    if (!fptr || !outptr) { printf("File error\n"); return 1; }

    char c, buffer[100];
    int state = 0, b_idx = 0;

    printf("\n\n%-45s | %s\n", "Lexeme", "Token");
    printf("------------------------------------------------------\n");

    while ((c = fgetc(fptr)) != EOF) {
        int col = get_col(c);
        int n_state = next_state[state][col];

        if (n_state != 0 && !(state == 6 && c == '\n')) {
            if (state == 1 && c == ':') {
                buffer[b_idx++] = c;
                buffer[b_idx] = '\0';
                printf("%-45s | loop\n", buffer);
                fprintf(outptr, "loop ");
                b_idx = 0; state = 0;
            } else {
                state = n_state;
                buffer[b_idx++] = c;
                if (state == 7 && c == '>') {
                    buffer[b_idx] = '\0';
                    printf("%-45s | head\n", buffer);
                    fprintf(outptr, "head ");
                    b_idx = 0; state = 0;
                }
            }
        } else {
            if (b_idx > 0) {
                buffer[b_idx] = '\0';
                const char* t_name;
                if (state == 1 || state == 2) t_name = refine_id(buffer, state);
                else if (state == 3) t_name = "num";
                else if (state == 4) t_name = "end";
                else if (state == 6) t_name = "cmt";
                else t_name = "unk";
                printf("%-45s | %s\n", buffer, t_name);
                fprintf(outptr, "%s ", t_name);
                b_idx = 0; state = 0;
            }

            if (!isspace(c)) {
                if (c == '[') {
                    printf("%-45c | lpar\n", c);
                    fprintf(outptr, "lpar ");
                }
                else if (c == ']') {
                    printf("%-45c | rpar\n", c);
                    fprintf(outptr, "rpar ");
                }
                else if (c == '(') {
                    printf("%-45c | lbrk\n", c);
                    fprintf(outptr, "lbrk ");
                }
                else if (c == ')') {
                    printf("%-45c | rbrk\n", c);
                    fprintf(outptr, "rbrk ");
                }
                else if (strchr("<>!=+-*", c)) {
                    char lexeme[3] = {c, '\0', '\0'};
                    char next = fgetc(fptr);
                    if (next == '=' && strchr("<>!=", c)) {
                        lexeme[1] = next;
                        printf("%-45s | op\n", lexeme);
                        fprintf(outptr, "op ");
                    } else {
                        if (next != EOF) ungetc(next, fptr);
                        printf("%-45s | op\n", lexeme);
                        fprintf(outptr, "op ");
                    }
                }

                if (get_col(c) < 6) {
                    ungetc(c, fptr);
                }
            }
        }
    }

    fclose(fptr);
    fclose(outptr);
    return 0;
}

// AI chat history : https://github.com/armanhossen-dev/compiler-design/tree/main/perfectCode2/history