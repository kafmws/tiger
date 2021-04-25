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
while                   {yylval.pos=EM_tokPos; adjust(); return WHILE;}
for                     {yylval.pos=EM_tokPos; adjust(); return FOR;}
to                      {yylval.pos=EM_tokPos; adjust(); return TO;}
break                   {yylval.pos=EM_tokPos; adjust(); return BREAK;}
let                     {yylval.pos=EM_tokPos; adjust(); return LET;}
in                      {yylval.pos=EM_tokPos; adjust(); return IN;}
end                     {yylval.pos=EM_tokPos; adjust(); return END;}
function                {yylval.pos=EM_tokPos; adjust(); return FUNCTION;}
var                     {yylval.pos=EM_tokPos; adjust(); return VAR;}
type                    {yylval.pos=EM_tokPos; adjust(); return TYPE;}
array                   {yylval.pos=EM_tokPos; adjust(); return ARRAY;}
if                      {yylval.pos=EM_tokPos; adjust(); return IF;}
then                    {yylval.pos=EM_tokPos; adjust(); return THEN;}
else                    {yylval.pos=EM_tokPos; adjust(); return ELSE;}
do                      {yylval.pos=EM_tokPos; adjust(); return DO;}
of                      {yylval.pos=EM_tokPos; adjust(); return OF;}
nil                     {yylval.pos=EM_tokPos; adjust(); return NIL;}

[a-zA-Z]+([0-9]|[a-zA-Z]|"_")*       {adjust(); yylval.sval=newString(yytext, yyleng);
                                        return ID;}
[0-9]+                  {adjust(); yylval.ival=atoi(yytext); return INT;}
[ \t]+                  {adjust(); continue;}

","                     {yylval.pos=EM_tokPos; adjust(); return COMMA;}
":"                     {yylval.pos=EM_tokPos; adjust(); return COLON;}
";"                     {yylval.pos=EM_tokPos; adjust(); return SEMICOLON;}
"("                     {yylval.pos=EM_tokPos; adjust(); return LPAREN;}
")"                     {yylval.pos=EM_tokPos; adjust(); return RPAREN;}
"["                     {yylval.pos=EM_tokPos; adjust(); return LBRACK;}
"]"                     {yylval.pos=EM_tokPos; adjust(); return RBRACK;}
"{"                     {yylval.pos=EM_tokPos; adjust(); return LBRACE;}
"}"                     {yylval.pos=EM_tokPos; adjust(); return RBRACE;}
"."                     {yylval.pos=EM_tokPos; adjust(); return DOT;}
"+"                     {yylval.pos=EM_tokPos; adjust(); return PLUS;}
"-"                     {yylval.pos=EM_tokPos; adjust(); return MINUS;}
"*"                     {yylval.pos=EM_tokPos; adjust(); return TIMES;}
"/"                     {yylval.pos=EM_tokPos; adjust(); return DIVIDE;}
"="                     {yylval.pos=EM_tokPos; adjust(); return EQ;}
"<>"                    {yylval.pos=EM_tokPos; adjust(); return NEQ;}
"<"                     {yylval.pos=EM_tokPos; adjust(); return LT;}
"<="                    {yylval.pos=EM_tokPos; adjust(); return LE;}
">"                     {yylval.pos=EM_tokPos; adjust(); return GT;}
">="                    {yylval.pos=EM_tokPos; adjust(); return GE;}
"&"                     {yylval.pos=EM_tokPos; adjust(); return AND;}
"|"                     {yylval.pos=EM_tokPos; adjust(); return OR;}
":="                    {yylval.pos=EM_tokPos; adjust(); return ASSIGN;}

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

