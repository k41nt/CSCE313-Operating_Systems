#!/bin/sh
#  test2.sh
#  Created by Joshua Higginbotham on 11/8/15.

echo "Compiling..."
make
echo "Running tests..."
echo "Test 1, b = 50:" >> output2.txt
./client -n 10000 -b 50 -w 50 -m 2
echo "Test 2, b = 100:" >> output2.txt
./client -n 10000 -b 2 -w 50 -m 2
echo "Test 3, b = 150:" >> output2.txt
./client -n 10000 -b 150 -w 50 -m 2
echo "Test 4, b = 200:" >> output2.txt
./client -n 10000 -b 250 -w 50 -m 2
echo "Test 5, b = 300:" >> output2.txt
./client -n 10000 -b 300 -w 50 -m 2
echo "Test 6, b = 350:" >> output2.txt
./client -n 10000 -b 350 -w 50 -m 2
echo "Test 7, b = 400:" >> output2.txt
./client -n 10000 -b 400 -w 50 -m 2
echo "Test 8, b = 450:" >> output2.txt
./client -n 10000 -b 450 -w 50 -m 2
echo "Test 9, b = 500:" >> output2.txt
./client -n 10000 -b 500 -w 50 -m 2
echo "Finished!"
