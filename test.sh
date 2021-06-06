cmake . &&
make &&
./Phemia test/8.txt &&
llc -filetype=obj test/output.bc &&
gcc test/output.o -oOutput &&
./Output