#!/bin/bash

# Loop 100 times
# echo "" >> ./outputUC4100.txt
for i in {1..100}
do
  # Run your program here
#   push a newline to the output file
    ./main2.exe -bw 160 < ../testcases/usecase4.txt >> ../plotter/outputUC4100.txt
    # echo "" >> ../plotter/outputUC4100.txt
done