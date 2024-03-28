import json
import os

# begin conf
inputFileName = "zones.json"
outputFileName = "output.txt"
# end conf

inputFile = open(inputFileName, "r")
outputFile = open(outputFileName, "w")
x = json.load(inputFile)

for i in x:
    outputFile.write("<option value=\"" + x[i] + "\">" + i + "</option>\n")

print("Exported to \"" + os.path.join(os.path.dirname(__file__), outputFileName) + "\"")