
PROGRAM=timestamp-test
topsrc_dir=../..
CPPFLAGS=-I$(topsrc_dir) -I$(shell pg_config --includedir)
CFLAGS=-Wall -O0 -g

OBJS=main.o \
	 $(topsrc_dir)/strlcpy.o \
	 $(topsrc_dir)/pool_timestamp.o \
	 $(topsrc_dir)/parser/libsql-parser.a

all: all-pre $(PROGRAM)

all-pre:
	$(MAKE) -C $(topsrc_dir)/parser
	$(MAKE) -C $(topsrc_dir) strlcpy.o
	$(MAKE) -C $(topsrc_dir) pool_timestamp.o

$(PROGRAM): $(OBJS)
	$(CC) $(OBJS) -o $(PROGRAM)

main.o: main.c

test: $(PROGRAM)
	./run-test parse_schedule

clean:
	-rm *.o
	-rm $(PROGRAM)
	-rm result/*.out
	-rm test.diff

.PHONY: all all-pre test clean
