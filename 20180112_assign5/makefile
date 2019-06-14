CC = gcc209
CCFLAGS = -O3 -D NDEBUG -std=gnu99

# Pattern rule
%.o: %.c
	$(CC) $(CCFLAGS) -c $<

# Dependency rules for non-file targets
all: testheapmgrgnu testheapmgrkr testheapmgrbase testheapmgr1 testheapmgr2
clobber: clean
	rm -f *~ \#*\# core
clean:
	rm -f testheapmgrgnu testheapmgrkr testheapmgrbase testheapmgr1 testheapmgr2 *.o

# Dependency rules for file targets
testheapmgrgnu: testheapmgr.o heapmgrgnu.o
	$(CC) $(CCFLAGS) $? -o $@

heapmgrgnu.o: heapmgr.h

testheapmgrkr: testheapmgr.o heapmgrkr.o
	$(CC) $(CCFLAGS) $? -o $@

heapmgrkr.o: heapmgr.h

testheapmgrbase: testheapmgr.o heapmgrbase.o chunkbase.o
	$(CC) $(CCFLAGS) $? -o $@

heapmgrbase.o: chunkbase.h

chunkbase.o: chunkbase.h

testheapmgr1: testheapmgr.o heapmgr1.o chunk.o
	$(CC) $(CCFLAGS) $? -o $@

heapmgr1.o: chunk.h

testheapmgr2: testheapmgr.o heapmgr2.o chunk.o
	$(CC) $(CCFLAGS) $? -o $@

heapmgr2.o: chunk.h

chunk.o: chunk.h

testheapmgr.o: heapmgr.h
