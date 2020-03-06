CC=gcc
CFLAGS= -Wall -Wpedantic -Wextra -O3
SRCDIR=src
DISTDIR=dist
TESTDIR=tests


all: clean set_test test_hash_map test_hash_map_2 test_map_of_set_of_int test_map_of_bitset

set_test: set 
	$(CC) ./$(DISTDIR)/set.o $(CFLAGS) ./$(TESTDIR)/set_test.c -o ./$(DISTDIR)/test_set

test_hash_map: hash_map
	$(CC) ./$(DISTDIR)/hash_map.o $(CFLAGS) ./$(TESTDIR)/hash_map_test.c -o ./$(DISTDIR)/test_hash_map
	
test_hash_map_2: hash_map
	$(CC) ./$(DISTDIR)/hash_map.o $(CFLAGS) ./$(TESTDIR)/hash_map_test_2.c -o ./$(DISTDIR)/test_hash_map_2

test_map_of_set_of_int: map_of_set_of_int hash_map
	$(CC) ./$(DISTDIR)/hash_map.o ./$(DISTDIR)/map_of_set_of_int.o $(CFLAGS) ./$(TESTDIR)/map_of_set_of_int_test.c -o ./$(DISTDIR)/test_map_of_set_of_int

test_map_of_bitset: map_of_bitset hash_map
	$(CC) ./$(DISTDIR)/hash_map.o ./$(DISTDIR)/map_of_bitset.o $(CFLAGS) ./$(TESTDIR)/map_of_bitset_test.c -o ./$(DISTDIR)/test_map_of_bitset

set:
	$(CC) -c ./$(SRCDIR)/set.c -o ./$(DISTDIR)/set.o $(CFLAGS)
	
hash_map:
	$(CC) -c ./$(SRCDIR)/hash_map.c -o ./$(DISTDIR)/hash_map.o $(CFLAGS)

map_of_set_of_int:
	$(CC) -c ./$(SRCDIR)/map_of_set_of_int.c -o ./$(DISTDIR)/map_of_set_of_int.o $(CFLAGS)

map_of_bitset:
	$(CC) -c ./$(SRCDIR)/map_of_bitset.c -o ./$(DISTDIR)/map_of_bitset.o $(CFLAGS)

clean:
	rm -rf ./$(DISTDIR)/*
