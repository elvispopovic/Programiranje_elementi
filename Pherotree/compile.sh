clang++ -Wall -std=c++17 -stdlib=libc++ -g  -c "pherotree.cpp" -o pherotree.o
clang++ -fsanitize=address -fno-omit-frame-pointer -o pherotree pherotree.o 