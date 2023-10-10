%{
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<ctype.h>
#include <stdbool.h>
#include <stddef.h>
#define YYSTYPE char*
#define MAX_SYMBOLS 100
struct SymbolEntry {
    char *name;
    char *value;
};
struct SymbolEntry symbolTable[MAX_SYMBOLS];
int symbolCount = 0;
char idstr[50];
char numstr[50];
int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const char*s);
%}
%token NUMBER
%token ID
%token ADD SUB MUL DIV LKO RKO
%left LKO
%left ADD SUB
%left MUL DIV
%right UMINUS
%right RKO
%%

lines : lines expr '\n'{printf("%s\n",$2);}
      | lines '\n'
      |
      ;

expr : expr ADD expr {$$=(char*)malloc(strlen($1)+strlen($3)+2); strcpy($$,$1);
                      strcat($$,$3);strcat($$,"+");free($1);free($3);}
     | expr SUB expr {$$=(char*)malloc(strlen($1)+strlen($3)+2); strcpy($$,$1);
                      strcat($$,$3);strcat($$,"-");free($1);free($3);}
     | expr MUL expr {$$=(char*)malloc(strlen($1)+strlen($3)+2); strcpy($$,$1);
                      strcat($$,$3);strcat($$,"*");free($1);free($3); }
     | expr DIV expr {$$=(char*)malloc(strlen($1)+strlen($3)+2); strcpy($$,$1);
                      strcat($$,$3);strcat($$,"/");free($1);free($3);}
     | LKO expr RKO {$$=(char*)malloc(strlen($2)+1);strcpy($$,$2);free($2);}
     | NUMBER   {$$ = (char*)malloc(strlen($1)+1); strcpy($$,$1);}
     | ID   {$$=(char*)malloc(strlen($1)+1);strcpy($$,$1);}


%%


int yylex()
{
      int t;
      while(1){
          t=getchar();     
          if(t==' '||t=='\t');
          else if((t>='0'&& t<='9')){
                   int ti=0;
                  while((t>='0'&& t<='9')){
                  numstr[ti]=t;
                  t=getchar();
                  ti++;
                }
           numstr[ti]='\0';
           yylval=numstr;
           ungetc(t,stdin);
           return NUMBER;
         }
      else if((t>='a'&&t<='z')||(t>='A'&&t<='Z')||(t=='_')){
              int ti=0;
              while((t>='a'&&t<='z')||(t>='A'&&t<='Z')
                     ||(t=='_')||(t>='0'&& t<='9')){
                      idstr[ti]=t;
                      ti++;
                      t=getchar();
                  }
               idstr[ti]='\0';
               yylval=idstr;
               ungetc(t,stdin);
               return ID;
             }
       else 
          {
            switch(t)
           {
              case '+':
                   return ADD;
              case '-':
                   return SUB;
              case '*':
                   return MUL;
              case '/':
                   return DIV;
              case'(':
                   return LKO;
              case')':
                   return RKO;
              default: 
                   return t;
            }
          }
      return t;
   }
}

int main(void)
{
   yyin = stdin;
  do{
      yyparse();
    }while(!feof(yyin));
  return 0;
}
void yyerror(const char* s)
{
   fprintf(stderr,"Parse error:%s\n",s);
   exit(1);
}

