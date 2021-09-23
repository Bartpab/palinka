gcc -Wall -Werror -o build/test.o -c test.c -O1 -fno-strict-aliasing -Wno-uninitialized
gcc -o build/test build/test.o
cd build 
test.exe 
cd ..
