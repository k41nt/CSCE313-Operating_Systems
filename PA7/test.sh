#!/bin/sh
#  test.sh
#  Created by Joshua Higginbotham on 11/8/15.

echo "Cleaning..."
make clean
echo "Compiling..."
make
echo "Running tests..."

./client -n 10000 -b 50 -w 140
echo "Test 29, w = 145:" >> output.txt
./client -n 10000 -b 50 -w 145
echo "Test 30, w = 150:" >> output.txt
./client -n 10000 -b 50 -w 150
echo "Test 31, w = 155:" >> output.txt
./client -n 10000 -b 50 -w 155
echo "Test 32, w = 160:" >> output.txt
./client -n 10000 -b 50 -w 160
echo "Test 33, w = 165:" >> output.txt
./client -n 10000 -b 50 -w 165
echo "Test 34, w = 170:" >> output.txt
./client -n 10000 -b 50 -w 170
echo "Test 35, w = 175:" >> output.txt
./client -n 10000 -b 50 -w 175
echo "Test 36, w = 180:" >> output.txt
./client -n 10000 -b 50 -w 180
echo "Test 37, w = 185:" >> output.txt
./client -n 10000 -b 50 -w 185
echo "Test 38, w = 190:" >> output.txt
./client -n 10000 -b 50 -w 190
echo "Test 39, w = 195:" >> output.txt
./client -n 10000 -b 50 -w 195
echo "Test 40, w = 200:" >> output.txt
./client -n 10000 -b 50 -w 200
echo "Test 41, w = 205:" >> output.txt
./client -n 10000 -b 50 -w 205
echo "Test 42, w = 210:" >> output.txt
./client -n 10000 -b 50 -w 210
echo "Test 43, w = 215:" >> output.txt
./client -n 10000 -b 50 -w 215
echo "Test 44, w = 220:" >> output.txt
./client -n 10000 -b 50 -w 220
echo "Test 45, w = 225:" >> output.txt
./client -n 10000 -b 50 -w 225
echo "Test 46, w = 230:" >> output.txt
./client -n 10000 -b 50 -w 230
echo "Test 47, w = 235:" >> output.txt
./client -n 10000 -b 50 -w 235
echo "Test 48, w = 240:" >> output.txt
./client -n 10000 -b 50 -w 240
echo "Test 49, w = 245:" >> output.txt
./client -n 10000 -b 50 -w 245
echo "Test 50, w = 250:" >> output.txt
./client -n 10000 -b 50 -w 250
echo "Finished!"
