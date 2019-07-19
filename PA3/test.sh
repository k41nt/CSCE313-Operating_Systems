#!/bin/sh
#  test.sh
#  Created by Joshua Higginbotham on 11/8/15.

# This is only provided for your convenience.
# The tests used in this file may or may not
# match up with what is called for for the report,
# so be sure to read that part of the handout.
# But you're free to modify this script however
# you need to, it's not graded.

echo "Cleaning and compiling..."
make
echo "Running tests..."
echo "Test 1, w = 5:" >> output.txt
./client -n 10000 -w 5
echo "Test 2, w = 7:" >> output.txt
./client -n 10000 -w 7
echo "Test 3, w = 9:" >> output.txt
./client -n 10000 -w 9
echo "Test 4, w = 11:" >> output.txt
./client -n 10000 -w 11
echo "Test 5, w = 13:" >> output.txt
./client -n 10000 -w 13
echo "Test 6, w = 15:" >> output.txt
./client -n 10000 -w 15
echo "Test 7, w = 17:" >> output.txt
./client -n 10000 -w 17
echo "Test 8, w = 19:" >> output.txt
./client -n 10000 -w 19
echo "Test 9, w = 21:" >> output.txt
./client -n 10000 -w 21
echo "Test 10, w = 23:" >> output.txt
./client -n 10000 -w 23
echo "Test 11, w = 25:" >> output.txt
./client -n 10000 -w 25
echo "Test 12, w = 27:" >> output.txt
./client -n 10000 -w 27
echo "Test 13, w = 29:" >> output.txt
./client -n 10000 -w 29
echo "Test 14, w = 31:" >> output.txt
./client -n 10000 -w 31
echo "Test 15, w = 33:" >> output.txt
./client -n 10000 -w 33
echo "Test 16, w = 35:" >> output.txt
./client -n 10000 -w 35
echo "Test 17, w = 37:" >> output.txt
./client -n 10000 -w 37
echo "Test 18, w = 39:" >> output.txt
./client -n 10000 -w 39
echo "Test 19, w = 41:" >> output.txt
./client -n 10000 -w 41
echo "Test 20, w = 43:" >> output.txt
./client -n 10000 -w 43

echo "Finished!"
