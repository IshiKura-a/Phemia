cmake . &&
make &&

echo "---------QuickSort---------"
./Phemia test/QuickSort/ans.txt &&
llc -filetype=obj test/output.ll &&
gcc test/output.o -o test/QuickSort/QuickSort
./test/QuickSort/darwin-amd64 ./test/QuickSort/QuickSort

echo "---------MatrixMul---------"
./Phemia test/MatrixMul/ans.txt &&
llc -filetype=obj test/output.ll &&
gcc test/output.o -o test/MatrixMul/MatrixMul
./test/MatrixMul/darwin-amd64 ./test/MatrixMul/MatrixMul

echo "---------Course---------"
./Phemia test/Course/ans.txt &&
llc -filetype=obj test/output.ll &&
gcc test/output.o -o test/Course/Course
./test/Course/darwin-amd64 ./test/Course/Course