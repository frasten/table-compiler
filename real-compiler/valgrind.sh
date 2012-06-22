make clean && make &&
valgrind --tool=memcheck --leak-check=full ./table < ../test/prova3
