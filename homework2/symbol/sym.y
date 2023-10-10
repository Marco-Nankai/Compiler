%{
/*********************************************
YACC file
基础程序
Date:2023/9/19
forked SherryXiye
**********************************************/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#define bool int
#define true 1
#define false 0
#define MAXENTRY 100
#ifndef YYSTYPE
#define YYSTYPE double
#endif
int yylex();
extern int yyparse();
FILE* yyin;
void yyerror(const char* s);
bool LookUp(char* name);
int GetIndex(char* name);
char idstr[50];
struct symbolEntry
{ 
    char* name;
    double value;
    bool initialized;
};
struct symbolEntry symbolTable[MAXENTRY];
int CountEntry = 0;
%}
//注意先后定义的优先级区别
%token ID 
%token NUMBER
%token ADD SUB 
%token MUL DIV 
%token LKO RKO
%left ADD SUB
%left MUL DIV
%right UMINUS      
%%


lines   :       lines stat '\n' { printf("%f\n", $2); }
        |       lines '\n'
        |
        ;
stat    :       ID  '=' expr    {
                                   int index = $1;
                                   symbolTable[index].value=$3;
                                   symbolTable[index].initialized=true;
                                   printf("this is the name:%s \n",symbolTable[index].name);
                                   printf("this is the index:%d \n",index);
                                   printf("this is the value:%f \n",symbolTable[index].value);
                                   printf("this is the initialized:%d \n",symbolTable[index].initialized);
                                   $$ = $3;
                                }

        |       expr            {$$=$1;}
        ;
expr    :       expr ADD expr   { $$=$1+$3; }
        |       expr SUB expr   { $$=$1-$3; }
        |       expr MUL expr   { $$=$1*$3; }
        |       expr DIV expr   { $$=$1/$3; }
        |       LKO expr RKO      { $$=$2;}
        |       SUB expr %prec UMINUS   {$$=-$2;}
        |       NUMBER          {$$=$1;}
        |       ID              {
                                 int index = $1;
                                 printf("this is the name:%s \n",symbolTable[index].name);
                                 printf("this is the index:%d \n",index);
                                 printf("this is the value:%f \n",symbolTable[index].value);
                                 printf("this is the initialized:%d \n",symbolTable[index].initialized);
                                 if(!symbolTable[index].initialized)
                                 {
                                   yyerror("can't find the name!");
                                 }
                                 else
                                 {
                                  $$ = symbolTable[index].value;
                                 }
                                }
        ;

%%

// programs section

int yylex()
{
      int t;
      while(1){
          t=getchar();     
         if(t==' '||t=='\t'||t=='\n'){
            //do noting
        }
          else if((t>='0'&& t<='9')){
                  double temp = 0;
                  while((t>='0'&& t<='9')){
                  temp = temp * 10 + t - '0';
                  t=getchar();
                }
           yylval = temp;
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
               bool existed = LookUp(idstr);
               if(!existed){                    
               symbolTable[CountEntry].name=(char*)malloc(strlen(idstr)+2);
               strcpy(symbolTable[CountEntry].name,idstr);
               yylval = CountEntry;
               CountEntry++;
               }
               else
               {
                    int index = GetIndex(idstr);
                    yylval = index;
               }
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
    yyin=stdin;
    do{
        yyparse();
    }while(!feof(yyin));
    return 0;
}
void yyerror(const char* s){
    fprintf(stderr,"Parse error: %s\n",s);
    exit(1);
}
bool LookUp(char* name)
{
     bool found = false;
     for(int i = 0;i < CountEntry;i++)
     {
          if(!strcmp(name,symbolTable[i].name))
          {
               found = true;
               return found;
          }
     }
     return found;
}
int GetIndex(char* name)
{    
     for(int i = 0;i < CountEntry;i++)
     {
          if(!strcmp(name,symbolTable[i].name))
          {
               return i;
          }
     }
}
