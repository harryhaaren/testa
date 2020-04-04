all:
	clang -g -Wall -Wextra main.c && ./a.out

uint:
	clang -g -Wall -Wextra uint16_add.c && ./a.out
