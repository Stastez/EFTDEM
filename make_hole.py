import sys
import math

inPath = sys.argv[1]
outPath = sys.argv[2]
centerX = float(sys.argv[3])
centerY = float(sys.argv[4])
threshold = float(sys.argv[5])

inputFile = open(inPath, 'r')
outputFile = open(outPath, 'w')

line = inputFile.readline()
outputFile.write(line)
line = inputFile.readline()
while (line != ""):
    splits = line.split(',')

    x = float(splits[0])
    y = float(splits[1])

    if (math.sqrt(math.pow(x - centerX, 2) + math.pow(y - centerY, 2)) > threshold):
        outputFile.write(line)

    line = inputFile.readline()