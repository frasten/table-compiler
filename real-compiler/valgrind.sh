make clean && make &&
valgrind --tool=memcheck --leak-check=full --track-origins=yes ./table < ../test/templategrande.table
