# This script enables the user to cut off the edges of the specified point cloud up to the specified thresholds.

import sys

if len(sys.argv) < 7:
    print("Usage: [program] inPath outPath minX maxX minY maxY")
    exit()

inPath = sys.argv[1]
outPath = sys.argv[2]
minX = float(sys.argv[3])
maxX = float(sys.argv[4])
minY = float(sys.argv[5])
maxY = float(sys.argv[6])

inputFile = open(inPath, 'r')
outputFile = open(outPath, 'w')

line = inputFile.readline()
outputFile.write(line)
line = inputFile.readline()
while line != "":
    splits = line.split(',')

    x = float(splits[0])
    y = float(splits[1])

    if minX <= x <= maxX and minY <= y <= maxY:
        outputFile.write(line)

    line = inputFile.readline()
