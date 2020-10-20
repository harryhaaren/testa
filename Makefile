
# Build and run
all: uint atm
	./atm_bin

atm:
	clang -g -Wall -Wextra main.c -o atm_bin

uint:
	clang -g -Wall -Wextra uint16_add.c -o uint16_bin
