build:
	rm -f ./a.out
	gcc -std=c11 -g -Wall -lm main.c scanner.c -Wno-unused-variable -Wno-unused-function

run:
	./a.out

valgrind:
	rm -f ./a.out
	gcc -std=c11 -g -Wall -lm main.c scanner.c -Wno-unused-variable -Wno-unused-function
	valgrind --tool=memcheck --leak-check=no ./a.out

submit:
	/home/cs211/w2024/tools/project01  submit  scanner.c
