a.out: parsetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o parse.o prabsyn.o symbol.o table.o
	cc -g parsetest.o y.tab.o lex.yy.o errormsg.o util.o absyn.o parse.o prabsyn.o symbol.o table.o

parsetest.o: parsetest.c absyn.h errormsg.h parse.h symbol.h util.h prabsyn.h
	cc -g -c parsetest.c 

y.tab.o: y.tab.c absyn.h
	cc -g -c y.tab.c

y.tab.c: tiger.grm
	yacc -dv tiger.grm

y.tab.h: y.tab.c
	echo "y.tab.h was created at the same time as y.tab.c"

absyn.o: absyn.c absyn.h symbol.h util.h
	cc -g -c absyn.c

parse.o: parse.c absyn.h errormsg.h parse.h symbol.h util.h
	cc -g -c parse.c

prabsyn.o: prabsyn.c prabsyn.h absyn.h util.h symbol.h
	cc -g -c prabsyn.c

symbol.o: symbol.c util.h symbol.h table.h
	cc -g -c symbol.c

table.o: table.c util.h table.h
	cc -g -c table.c

errormsg.o: errormsg.c errormsg.h util.h
	cc -g -c errormsg.c

lex.yy.o: lex.yy.c y.tab.h errormsg.h util.h
	cc -g -c lex.yy.c

lex.yy.c: tiger.lex
	lex tiger.lex

util.o: util.c util.h
	cc -g -c util.c

clean: 
	rm -f a.out util.o parsetest.o lex.yy.o errormsg.o y.tab.c y.tab.h y.tab.o
