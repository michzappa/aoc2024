VALGRIND_FLAGS = -s --track-origins=yes --leak-check=full
CC=gcc
CFLAGS = -Werror=all #-g
INCLUDED_OBJS = include/data.o include/dyn_array.o include/handler.o include/hash_table.o

#### Compile shared code in 'include', dependencies of solution.
include/%.o: include/%.c
	$(CC) $(CFLAGS) -c -o $@ $<

include/%.s: include/%.c
	$(CC) $(CFLAGS) -S -c -o $@ $<

#### Create and Run the Solutions.
solution-%: $(INCLUDED_OBJS) day%/solution.c
	$(CC) $(CFLAGS) -o $@ $^

solution-%.s: day%/solution.c
	$(CC) $(CFLAGS) -S -o $@ $^

practice-%: solution-% day%/practice_input.txt
	./$^

run-%: solution-% day%/input.txt
	./$^

valgrind-practice-%: solution-% day%/practice_input.txt
	valgrind $(VALGRIND_FLAGS) ./$^

valgrind-run-%: solution-% day%/input.txt
	valgrind $(VALGRIND_FLAGS) ./$^

#### Scratch 'test' file. (which may not exist)
test: $(INCLUDED_OBJS) test.c
	$(CC) $(CFLAGS) -o $@ $^

#### Utilities.
format:
	clang-format -i **/*.c **/*.h

clean:
	rm -f include/*.o
	rm -f solution-*
