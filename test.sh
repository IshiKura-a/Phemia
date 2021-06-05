cmake . &&
make &&
./Phemia test/6.txt &&
llc -filetype=obj test/output.bc &&
gcc test/output.o -oOutput &&
./output