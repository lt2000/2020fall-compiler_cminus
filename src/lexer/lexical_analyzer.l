%option noyywrap
%{
/*****************声明和选项设置  begin*****************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "lexical_analyzer.h"

int lines;
int pos_start;
int pos_end;

/*****************声明和选项设置  end*****************/
void Change_pos()
{
    pos_start = pos_end;
    pos_end = pos_start + strlen(yytext);
}
%}


%%
    /*运算符*/
\+   {Change_pos();return ADD;}
\-   {Change_pos();return SUB;}
\*   {Change_pos();return MUL;}
\/   {Change_pos();return DIV;}
\<   {Change_pos();return LT;}
\<=  {Change_pos();return LTE;}
\>   {Change_pos();return GT;}
\>=  {Change_pos();return GTE;}
\==  {Change_pos();return EQ;}
\!=  {Change_pos();return NEQ;}
\=   {Change_pos();return ASSIN;}
    /*符号*/
;   {Change_pos();return SEMICOLON;}
,   {Change_pos();return COMMA;}
\(   {Change_pos();return LPARENTHESE;}
\)   {Change_pos();return RPARENTHESE;}
\[   {Change_pos();return  LBRACKET;}
\]   {Change_pos();return  RBRACKET;}
\{   {Change_pos();return  LBRACE ;}
\}   {Change_pos();return  RBRACE ;}
     /*关键字*/
else {Change_pos();return  ELSE;}
if   {Change_pos();return  IF;}
int  {Change_pos();return  INT;}
float {Change_pos();return FLOAT;}
return {Change_pos();return RETURN;}
void {Change_pos();return VOID;}
while {Change_pos();return WHILE;}
    /*ID和NUM，字母在标识符之后*/
[a-zA-Z]+  {Change_pos();return IDENTIFIER;}
[0-9]+     {Change_pos();return  INTEGER;}
[0-9]+\.|[0-9]*\.[0-9]+  {Change_pos();return  FLOATPOINT;}
[a-zA-Z]   {Change_pos();return LETTER;}
\[\] {Change_pos();return  ARRAY;}
    /*其他*/
\/\*([^\*]|(\*)*[^\*/])*(\*)*\*\/  {return COMMENT;}
[ \t]  {return BLANK;}    
\n {return EOL;}
. {return ERROR;}




 /****请在此补全所有flex的模式与动作  end******/
%%
/****************C代码 start*************/

/// \brief analysize a *.cminus file
///
/// \param input_file, 需要分析的文件路径
/// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(char* input_file, Token_Node* token_stream){
    lines = 1;
    pos_start = 1;
    pos_end = 1;
    int i;
    if(!(yyin = fopen(input_file,"r"))){
        printf("[ERR] No input file\n");
        exit(1);
    }
    printf("[START]: Read from: %s\n", input_file);

    int token;
    int index = 0;

    while(token = yylex()){
        switch(token){
            case COMMENT:
                {
                    for(i = 0;i<strlen(yytext);i++) //注释占据几行
                    {
                        if(yytext[i] == '\n')
                          lines++;
                    }
                    for(i = strlen(yytext);i>=0;i--) //注释有多行，最后一行注释占据的位置
                    {
                        if(yytext[i] == '\n')
                          {
                              pos_start = strlen(yytext) - i;
                              pos_end = strlen(yytext) - i;
                              break;
                          }
                    }
                    if(i<0) //注释只有一行且不在一句的最后，所占据的位置
                    {
                        Change_pos();
                    }
                }
                break;
            case BLANK:
                {
                    pos_start = pos_end;     //空格和\t位置只加一
                    pos_end = pos_start + 1;
                    
                }
                break;
            case EOL:
                    {
                        lines++;
                        pos_end = 1;
                        pos_start = 1;
                    }
                break;
            case ERROR:
                printf("[ERR]: unable to analysize %s at %d line, from %d to %d\n", yytext, lines, pos_start, pos_end);
            default :
                if (token == ERROR){
                    sprintf(token_stream[index].text, "[ERR]: unable to analysize %s at %d line, from %d to %d", yytext, lines, pos_start, pos_end);
                } else {
                    strcpy(token_stream[index].text, yytext);
                }
                token_stream[index].token = token;
                token_stream[index].lines = lines;
                token_stream[index].pos_start = pos_start;
                token_stream[index].pos_end = pos_end;
                index++;
                if (index >= MAX_NUM_TOKEN_NODE){
                    printf("%s has too many tokens (> %d)", input_file, MAX_NUM_TOKEN_NODE);
                    exit(1);
                }
        }
    }
    printf("[END]: Analysis completed.\n");
    return;
}



/****************C代码 end*************/
