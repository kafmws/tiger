a.out: translatetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o parse.o prabsyn.o\
symbol.o table.o semant.o types.o env.o translate.o temp.o x86frame.o tree.o
	cc -g translatetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o parse.o prabsyn.o\
    symbol.o table.o semant.o types.o env.o translate.o temp.o x86frame.o tree.o

translatetest.o: translatetest.c absyn.h errormsg.h parse.h symbol.h util.h prabsyn.h\
semant.h prabsyn.h
	cc -g -c translatetest.c

translate.o: translate.c translate.h frame.h
	cc -g -c translate.c

x86frame.o: x86frame.c frame.h tree.h
	cc -g -c x86frame.c

tree.o: tree.c tree.h util.h symbol.h temp.h
	cc -g -c tree.c

temp.o: temp.c temp.h util.h symbol.h table.h
	cc -g -c temp.c

table.o: table.c util.h table.h
	cc -g -c table.c

types.o: types.c types.h symbol.h
	cc -g -c types.c

env.o: env.c env.h
	cc -g -c env.c

semant.o:semant.c semant.h absyn.h env.h errormsg.h table.h translate.h frame.h
	cc -g -c semant.c

absyn.o: absyn.c absyn.h symbol.h util.h
	cc -g -c absyn.c

symbol.o: symbol.c util.h symbol.h table.h
	cc -g -c symbol.c

parse.o: parse.c absyn.h errormsg.h parse.h symbol.h util.h
	cc -g -c parse.c

prabsyn.o: prabsyn.c prabsyn.h absyn.h util.h symbol.h
	cc -g -c prabsyn.c

y.tab.o: y.tab.c absyn.h
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

errormsg.o: errormsg.c errormsg.h util.h
	cc -g -c errormsg.c

util.o: util.c util.h
	cc -g -c util.c

clean: 
	rm -f util.o parsetest.o lex.yy.o errormsg.o y.tab.c y.tab.h y.tab.o lex.yy.c parse.o absyn.o prabsyn.o symbol.o table.o
