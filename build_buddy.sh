#/bin/sh

clang++ -g -c memory/buddy.cpp -o obj/memory/buddy.o -I./
clang++ -g -c test/memory/buddy.cpp -o obj/test/memory/buddy.o -I./
clang++ -g -o buddy obj/memory/buddy.o obj/test/memory/buddy.o
