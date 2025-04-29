# Compiler and flags
CC = mpicc
CFLAGS = -Wall -O2

# Targets
all: modified_odd_even_sort standard_odd_even_sort sequential_quicksort

# Rule for the Modified Odd-Even Sort
modified_odd_even_sort: modified_odd_even_sort.c
	$(CC) $(CFLAGS) -o modified_odd_even_sort modified_odd_even_sort.c

# Rule for the Standard Odd-Even Sort
standard_odd_even_sort: standard_odd_even_sort.c
	$(CC) $(CFLAGS) -o standard_odd_even_sort standard_odd_even_sort.c

# Rule for the Sequential Quicksort
sequential_quicksort: sequential_quicksort.c
	gcc -Wall -O2 -o sequential_quicksort sequential_quicksort.c

# Clean target to remove compiled files
clean:
	rm -f modified_odd_even_sort standard_odd_even_sort sequential_quicksort
