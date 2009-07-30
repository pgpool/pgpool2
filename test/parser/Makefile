# $Header$

PROGRAM=parser-test
PGPOOL_SRC=../../parser
PARSER_OBJS=gram.o parser.o pool_string.o list.o makefuncs.o value.o nodes.o pool_memory.o main.o keywords.o outfuncs.o copyfuncs.o kwlookup.o

all: $(PROGRAM)

.SUFFIXES:
.SUFFIXES: .o .c

.c.o:
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

$(PROGRAM): $(PARSER_OBJS)
	gcc $(PARSER_OBJS) -o $(PROGRAM)

main.o: main.c

keywords.o: $(PGPOOL_SRC)/keywords.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

outfuncs.o: $(PGPOOL_SRC)/outfuncs.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

gram.o: gram.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

gram.c: $(PGPOOL_SRC)/gram.y $(PGPOOL_SRC)/scan.c
	bison -d $(PGPOOL_SRC)/gram.y
	mv -f gram.tab.c gram.c
	mv -f gram.tab.h parse.h

pool_string.o : $(PGPOOL_SRC)/pool_string.c $(PGPOOL_SRC)/pool_string.h
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

parser.o : $(PGPOOL_SRC)/parser.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

list.o : $(PGPOOL_SRC)/list.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

makefuncs.o : $(PGPOOL_SRC)/makefuncs.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

value.o : $(PGPOOL_SRC)/value.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

nodes.o : $(PGPOOL_SRC)/nodes.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

scanner : scan.c $(PGPOOL_SRC)/keywords.h
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

scan.o : scan.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

scan.c : $(PGPOOL_SRC)/scan.l
	flex -o$@ $<

pool_memory.o: $(PGPOOL_SRC)/pool_memory.c $(PGPOOL_SRC)/pool_memory.h
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

copyfuncs.o: $(PGPOOL_SRC)/copyfuncs.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

kwlookup.o: $(PGPOOL_SRC)/kwlookup.c
	gcc -Wall -c -g -include pool.h -I. -I $(PGPOOL_SRC) $<

test: $(PROGRAM)
	./run-test parse_schedule

clean:
	rm -f $(PROGRAM)
	rm -f *.o
	rm -f gram.c parse.h
