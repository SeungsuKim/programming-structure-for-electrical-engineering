all: ish
clobber: clean
	rm -f *~ \#*\# core
clean:
	rm -f ish synAnalyzer lexAnalyzer dynarray *.o

ish: ish.o synAnalyzer.o lexAnalyzer.o dynarray.o
	gcc209 -D_BSD_SOURCE -D_GNU_SOURCE ish.o synAnalyzer.o lexAnalyzer.o dynarray.o -o ish

ish.o: ish.c
	gcc209 -D_BSD_SOURCE -D_GNU_SOURCE -c ish.c

synAnalyzer.o: synAnalyzer.c
	gcc209 -D_BSD_SOURCE -D_GNU_SOURCE -c synAnalyzer.c

lexAnalyzer.o: lexAnalyzer.c
	gcc209 -D_BSD_SOURCE -D_GNU_SOURCE -c lexAnalyzer.c

dynarray.o: dynarray.c
	gcc209 -D_BSD_SOURCE -D_GNU_SOURCE -c dynarray.c
