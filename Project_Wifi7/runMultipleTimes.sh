#!/bin/bash

# Loop 100 times
# echo "" >> ./outputUC4100.txt
for i in {1..100}
do
  # Run your program here
#   push a newline to the output file
    ./main.exe -bw 40 -nlp_ol 20 < ../testcases/usecase1.txt >> ../plotter/outputUC1_satisfactionRatio.txt
    # echo "" >> ../plotter/outputUC4100.txt
done