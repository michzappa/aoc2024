VALGRIND_FLAGS = -s --track-origins=yes --leak-check=full --show-leak-kinds=all
CC=gcc
CFLAGS = -Werror=all -g
INCLUDED_OBJS = include/data.o include/dyn_array.o include/handler.o include/hash_table.o

#### Compile code.
%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

%.s: %.c
	$(CC) $(CFLAGS) -S -c -o $@ $<

#### Create and Run the Solutions.
solution-%: $(INCLUDED_OBJS) day%/solution.c
	$(CC) $(CFLAGS) -o $@ $^

practice-%: solution-% day%/practice_input.txt
	./$^

run-%: solution-% day%/input.txt
	./$^

valgrind-practice-%: solution-% day%/practice_input.txt
	valgrind $(VALGRIND_FLAGS) ./$^

valgrind-run-%: solution-% day%/input.txt
	valgrind $(VALGRIND_FLAGS) ./$^

#### Tests.
test-%: $(INCLUDED_OBJS) test/%.c
	$(CC) $(CFLAGS) -o $@ $^

run-test-%: test-%
	./$^

valgrind-test-%: test-%
	valgrind $(VALGRIND_FLAGS) ./$^

#### Utilities.
format:
	clang-format -i **/*.c **/*.h

clean:
	rm -f **/*.o
	rm -f solution-*
	rm -f **/*.s
