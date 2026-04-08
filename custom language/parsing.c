#include <stdio.h>
#include <string.h>
#define MAXSTACK 200
#define MAXTOK 200
#define MAXSYM 50

// ----------------------
// Grammar Symbols
// ----------------------
char *NT[] = {"S","A","E","Eprime","T","Tprime","F"};
#define NNT 7

char *TERMINALS[] = {"id","[","]","+","*","$"};
#define NTER 6

char *RHS[] = {
    "id A",       //1 S -> id A
    "[ E ] A",    //2 A -> [ E ] A
    "",           //3 A -> epsilon
    "T Eprime",   //4 E -> T E'
    "+ T Eprime", //5 E' -> + T E'
    "",           //6 E' -> epsilon
    "F Tprime",   //7 T -> F T'
    "* F Tprime", //8 T' -> * F T'
    "",           //9 T' -> epsilon
    "id"          //10 F -> id
};

// ----------------------
// LL(1 Parsing Table (NT x T)
// ----------------------
int TABLE[NNT][NTER] = {
    {1,0,0,0,0,0},  // S
    {0,2,3,0,0,3},  // A
    {4,0,0,0,0,4},  // E
    {0,0,6,5,0,6},  // Eprime
    {7,0,0,0,0,7},  // T
    {0,0,9,9,8,9},  // Tprime
    {10,0,0,0,0,10} // F
};

// ----------------------
// Stack
// ----------------------
char stack[MAXSTACK][MAXSYM];
int top = -1;
void push(char *s){ strcpy(stack[++top], s); }
char* pop(){ return (top>=0)?stack[top--]:NULL; }

void print_stack(){
    printf("[");
    for(int i=top;i>=0;i--){
        printf("%s",stack[i]);
        if(i>0) printf(", ");
    }
    printf("]");
}

// ----------------------
// Helpers
// ----------------------
int find_nt(char *x){
    for(int i=0;i<NNT;i++)
        if(strcmp(NT[i],x)==0) return i;
    return -1;
}

int find_t(char *x){
    for(int i=0;i<NTER;i++)
        if(strcmp(TERMINALS[i],x)==0) return i;
    return -1;
}

int tokenize(char *line, char tokens[][MAXSYM]){
    int n=0;
    char *p=strtok(line," \t\n");
    while(p){
        strcpy(tokens[n++],p);
        p=strtok(NULL," \t\n");
    }
    if(n==0 || strcmp(tokens[n-1],"$")!=0)
        strcpy(tokens[n++],"$");
    return n;
}

// ----------------------
// Parser
// ----------------------
int main(){
    char line[500];
    printf("Enter input : ");
    fgets(line,sizeof(line),stdin);

    char input[MAXTOK][MAXSYM];
    int n = tokenize(line,input);

    push("$");
    push("S"); // start symbol

    int ip=0;
    int rejected = 0;  // flag

    printf("%-35s %-12s %-12s %-25s\n","Stack","Lookahead","Top","Production Applied");
    printf("----------------------------------------------------------------------------------\n");

    while(top>=0){
        char X[MAXSYM];
        strcpy(X,pop());
        char *a = input[ip];

        printf("%-35s %-12s %-12s ","",a,X);

        int tindex = find_t(X);

        // Terminal
        if(tindex!=-1){
            if(strcmp(X,a)==0){
                printf("%-25s\n","match");
                ip++;
            } else {
                printf("%-25s\n","not match");
                rejected=1;
                break;
            }
            print_stack(); printf("\n");
            continue;
        }

        int ntindex = find_nt(X);
        int aindex = find_t(a);

        if(ntindex==-1 || aindex==-1){
            printf("%-25s\n","not match");
            rejected=1;
            break;
        }

        int prod = TABLE[ntindex][aindex];
        if(prod==0){
            printf("%-25s\n","not match");
            rejected=1;
            break;
        }

        if(strlen(RHS[prod-1])==0)
            printf("%-25s\n","epsilon");
        else
            printf("%-25s\n",RHS[prod-1]);

        if(strlen(RHS[prod-1])>0){
            char temp[200];
            strcpy(temp,RHS[prod-1]);
            char *p=strtok(temp," ");
            char symbols[10][MAXSYM];
            int k=0;
            while(p){ strcpy(symbols[k++],p); p=strtok(NULL," "); }
            for(int i=k-1;i>=0;i--) push(symbols[i]);
        }

        print_stack(); printf("\n");
    }

    if(rejected) printf("\nREJECTED\n");
    else if(top==-1 && ip==n)
        printf("\nACCEPTED\n");
    else
        printf("\nREJECTED\n");

    return 0;
}