build:
	rm -f ./a.out
	rm -f *.gcda
	rm -f *.gcno
	g++ -std=c++17 -g -Wall main.c ram.c tests.c gtest.o -I. -lm -lpthread --coverage -Wno-unused-variable -Wno-unused-function -Wno-write-strings


run:
	rm -f *.gcda
	./a.out


valgrind:
	rm -f ./a.out
	rm -f *.gcda
	rm -f *.gcno
	g++ -std=c++17 -g -Wall main.c ram.c tests.c gtest.o -I. -lm -lpthread --coverage -Wno-unused-variable -Wno-unused-function -Wno-write-strings
	valgrind --tool=memcheck --leak-check=full ./a.out


clean:
	rm -f ./a.out
	rm -f *.gcda
	rm -f *.gcno
 

framework:
	rm -f *.o
	gcc -g -c ./googletest/src/gtest-all.cc -I. -I./googletest/src
	ld -relocatable gtest-all.o -o gtest.o
	rm -f gtest-all.o

	