gcc -Wall -Werror -o build/test.o -c test.c
gcc -o build/test build/test.o
cd build 
test.exe 
cd ..