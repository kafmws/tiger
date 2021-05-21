%{
#include <ctype.h>
#include <string.h>
#include "util.h"
#include "absyn.h"
#include "y.tab.h"
#include "errormsg.h"

#if 0
#define dbgprintf(tag, yytext, yyleng) \
do{printf(tag": "); char *p = yytext; int i; for(i = 0;i<yyleng;i++) printf("%c", *p++);printf("\n");}while(0)
#else 
#define dbgprintf(tag, yytext, yyleng) ;
#endif

int charPos=1;

static string buf;
static size_t buf_cnt;
static int commentLevel = 0;

int yywrap(void)
{
 charPos=1;
 return 1;
}

void adjust(void)
{
 EM_tokPos=charPos;
 charPos+=yyleng;
}

#define PUSHSTR(str) push_str(str, str+strlen(str));

void push_str(string s, string end){//[,)

    static size_t BUFSIZE = 1 << 10;

    if(BUFSIZE - buf_cnt < end - s || !buf) {
        while(BUFSIZE - buf_cnt < end - s) BUFSIZE = BUFSIZE << 1;
        buf = realloc(buf, sizeof(*buf)*BUFSIZE);
        if(buf == NULL){
            fprintf(stderr, "Out of memory.\n");
            exit(-1);
        }
    }
    while(s!=end) buf[buf_cnt++] = *s++;
}

string newString(string buf, size_t cnt){
    if(cnt == 0) return "(null)";
    string str = checked_malloc(sizeof(*str) * (buf_cnt + 1));
    memcpy(str, buf, cnt);
    str[cnt] = 0;
    return str;
}

%}

%x string comment
%%
while                   {adjust(); return WHILE;}
for                     {adjust(); return FOR;}
to                      {adjust(); return TO;}
break                   {adjust(); return BREAK;}
let                     {adjust(); return LET;}
in                      {adjust(); return IN;}
end                     {adjust(); return END;}
function                {adjust(); return FUNCTION;}
var                     {adjust(); return VAR;}
type                    {adjust(); return TYPE;}
array                   {adjust(); return ARRAY;}
if                      {adjust(); return IF;}
then                    {adjust(); return THEN;}
else                    {adjust(); return ELSE;}
do                      {adjust(); return DO;}
of                      {adjust(); return OF;}
nil                     {adjust(); return NIL;}

[a-zA-Z]+([0-9]|[a-zA-Z]|"_")*       {adjust(); yylval.sval=newString(yytext, yyleng);
                                        return ID;}
[0-9]+                  {adjust(); yylval.ival=atoi(yytext); return INT;}
[ \t]+                  {adjust(); continue;}

","                     {adjust(); return COMMA;}
":"                     {adjust(); return COLON;}
";"                     {adjust(); return SEMICOLON;}
"("                     {adjust(); return LPAREN;}
")"                     {adjust(); return RPAREN;}
"["                     {adjust(); return LBRACK;}
"]"                     {adjust(); return RBRACK;}
"{"                     {adjust(); return LBRACE;}
"}"                     {adjust(); return RBRACE;}
"."                     {adjust(); return DOT;}
"+"                     {adjust(); return PLUS;}
"-"                     {adjust(); return MINUS;}
"*"                     {adjust(); return TIMES;}
"/"                     {adjust(); return DIVIDE;}
"="                     {adjust(); return EQ;}
"<>"                    {adjust(); return NEQ;}
"<"                     {adjust(); return LT;}
"<="                    {adjust(); return LE;}
">"                     {adjust(); return GT;}
">="                    {adjust(); return GE;}
"&"                     {adjust(); return AND;}
"|"                     {adjust(); return OR;}
":="                    {adjust(); return ASSIGN;}

\"                                      {adjust(); buf_cnt = 0; BEGIN(string); continue;}
<string>"\\\\"                          {charPos+=yyleng; push_str("\\", "\\" + 1); continue;}
<string>"\\n"                           {charPos+=yyleng; push_str("\n", "\n" + 1); continue;}
<string>"\\r"                           {charPos+=yyleng; push_str("\r", "\r" + 1); continue;}
<string>"\\t"                           {charPos+=yyleng; push_str("\t", "\t" + 1); continue;}
<string>"\\f"                           {charPos+=yyleng; push_str("\f", "\f" + 1); continue;}
<string>"\\\""                          {charPos+=yyleng; push_str("\"", "\"" + 1); continue;}
<string>\\\^([A-Z]|\[|\\|\]|\^|_)       {charPos+=yyleng; char c = (*(yytext+2)-64);
                                        push_str(&c, &c+1); continue;}
<string>\\((0[0-9][0-9])|(1[0-1][0-9])|(12[0-7]))       {charPos+=yyleng; char c =
                                                        (*(yytext + 1) - '0') * 100
                                                        + (*(yytext + 2) - '0') * 10
                                                        + (*(yytext + 3) - '0');
                                                        push_str(&c, &c+1); continue;}
<string>\( |\t|\n|\r|\f)+\              {charPos+=yyleng; continue;}
<string>[^\n\r\f\t\"\\]+|(\\\")         {charPos+=yyleng; push_str(yytext, yytext+yyleng); continue;}
<string>\n                              {charPos+=yyleng; EM_newline(); continue;}
<string>\"                              {charPos+=yyleng; yylval.sval=newString(buf, buf_cnt);
                                        BEGIN(INITIAL); return STRING;}

<INITIAL,comment>"/*"       {adjust(); commentLevel++; 
                            if(commentLevel == 1) BEGIN(comment); continue;}
<comment>[^\r\n\*/]+        {charPos+=yyleng; continue;}
<comment>\n                 {charPos+=yyleng; EM_newline(); continue;}
<comment>\r                 {continue;}
<comment>"*"                {charPos+=yyleng; continue;}
<comment>"*/"               {charPos+=yyleng; commentLevel--;
                            if(commentLevel == 0) BEGIN(INITIAL); continue;}

\n                          {adjust(); EM_newline(); continue;}
\r                          {adjust(); continue;}
.                           {adjust(); EM_error(EM_tokPos,"illegal token");}

