

all : PrintTest
	./PrintTest

PrintTest : PrintTest.cpp ../src/Print64.cpp ../src/Print64.h libwiringgcc
	gcc PrintTest.cpp ../src/Print64.cpp gcclib/libwiringgcc.a -std=c++11 -lc++ -Igcclib -I../src -o PrintTest

check : PrintTest.cpp ../src/Print64.cpp ../src/Print64.h libwiringgcc
	gcc PrintTest.cpp ../src/Print64.cpp gcclib/libwiringgcc.a -g -O0 -std=c++11 -lc++ -Igcclib -I ../src -o PrintTest && valgrind --leak-check=yes ./PrintTest 

libwiringgcc :
	cd gcclib && make libwiringgcc.a 	
	
.PHONY: libwiringgcc