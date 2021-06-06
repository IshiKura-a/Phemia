cmake . &&
make &&
./Phemia test/1.txt &&
llc -filetype=obj test/output.bc &&
gcc test/output.o -oOutput &&
./output > out1.txt
