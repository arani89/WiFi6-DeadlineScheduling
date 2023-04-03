#!/bin/bash

# Loop 100 times
for i in {1..100}
do
  # Run your program here
#   push a newline to the output file
    echo "" >> ./outputUC4100.txt
    ./main2.o < ../testcases/usecase4.txt >> ./outputUC4100.txt
done