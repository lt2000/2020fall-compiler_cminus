%option noyywrap
%{
#include <stdio.h>
#include <stdlib.h>

#include "syntax_tree.h"
#include "syntax_analyzer.h"


int files_count;
int lines;
int pos_start;
int pos_end;

void pass_node(char *text){
     yylval.node = new_syntax_tree_node(text);
}

void Change_pos()
{
    pos_start = pos_end;
    pos_end = pos_start + strlen(yytext);
}


%}


%%
    /*运算符*/
\+   {Change_pos();pass_node(yytext); return ADD;}
\-   {Change_pos();pass_node(yytext); return SUB;}
\*   {Change_pos();pass_node(yytext); return MUL;}
\/   {Change_pos();pass_node(yytext); return DIV;}
\<   {Change_pos();pass_node(yytext); return LT;}
\<=  {Change_pos();pass_node(yytext); return LTE;}
\>   {Change_pos();pass_node(yytext); return GT;}
\>=  {Change_pos();pass_node(yytext); return GTE;}
\==  {Change_pos();pass_node(yytext); return EQ;}
\!=  {Change_pos();pass_node(yytext); return NEQ;}
\=   {Change_pos();pass_node(yytext); return ASSIN;}
    /*符号*/
;   {Change_pos();pass_node(yytext); return SEMICOLON;}
,   {Change_pos();pass_node(yytext); return COMMA;}
\(   {Change_pos();pass_node(yytext); return LPARENTHESE;}
\)   {Change_pos();pass_node(yytext); return RPARENTHESE;}
\[   {Change_pos();pass_node(yytext); return  LBRACKET;}
\]   {Change_pos();pass_node(yytext); return  RBRACKET;}
\{   {Change_pos();pass_node(yytext); return  LBRACE ;}
\}   {Change_pos();pass_node(yytext); return  RBRACE ;}
     /*关键字*/
else {Change_pos();pass_node(yytext); return  ELSE;}
if   {Change_pos();pass_node(yytext); return  IF;}
int  {Change_pos();pass_node(yytext); return  INT;}
float {Change_pos();pass_node(yytext); return FLOAT;}
return {Change_pos();pass_node(yytext); return RETURN;}
void {Change_pos();pass_node(yytext); return VOID;}
while {Change_pos();pass_node(yytext); return WHILE;}
    /*ID和NUM，字母在标识符之后*/
[a-zA-Z]+  {Change_pos();pass_node(yytext); return IDENTIFIER;}
[0-9]+     {Change_pos();pass_node(yytext); return  INTEGER;}
[0-9]+\.|[0-9]*\.[0-9]+  {Change_pos();pass_node(yytext); return  FLOATPOINT;}
\[\] {Change_pos();pass_node(yytext); return  ARRAY;}
    /*其他*/
\/\*([^\*]|(\*)*[^\*/])*(\*)*\*\/  {analyzer(COMMENT);}
[ \t]  {analyzer(BLANK);}    
\n {analyzer(EOL);}
. {analyzer(ERROR);}

%%
 /****************C代码 start*************/

 /// \brief analysize a *.cminus file
 ///
 /// \param input_file, 需要分析的文件路径
 /// \param token stream, Token_Node结构体数组，用于存储分析结果，具体定义参考lexical_analyer.h

void analyzer(int token){
   
    int i;   
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
                break; 
        }
     return;
}
 
