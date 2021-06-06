cmake . &&
make &&
./Phemia test/7.txt &&
llc -filetype=obj test/output.ll &&
gcc test/output.o -oOutput &&
./Output