a.out: main.o prog1.o slp.o util.o interpret.o
	cc -g main.o prog1.o slp.o util.o interpret.o

main.o: main.c slp.h util.h interpret.h
	cc -g -c main.c

prog1.o: prog1.c slp.h util.h
	cc -g -c prog1.c

slp.o: slp.c slp.h util.h
	cc -g -c slp.c

util.o: util.c util.h
	cc -g -c util.c

interpret.o : interpret.c interpret.h slp.h util.h
	cc -g -c interpret.c

clean: 
	rm -f util.o prog1.o slp.o main.o interpret.o
