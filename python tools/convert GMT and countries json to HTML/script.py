import json
import os

# begin conf
inputFileGmtName = "zones.json"
inputFileCountriesName = "ISO3166-alpha2.json"
outputFileName = "output.txt"
# end conf

outputFile = open(outputFileName, "w")

inputFileGmt = open(inputFileGmtName, "r")
inputFileCountries = open(inputFileCountriesName, "r")

gmtJson = json.load(inputFileGmt)
countriesJson = json.load(inputFileCountries)

outputFile.write("<label for=\"country-code\"><b>COUNTRY CODE</b></label><br>\n")
outputFile.write("<select id=\"country-code\" name=\"country-code\">\n")

for i in countriesJson:
    outputFile.write("\t<option value=\"" + str(i) + "\">" + str(countriesJson[i]) + "</option>\n")

outputFile.write("</select><br><br>\n\n")

outputFile.write("<label for=\"timezones\"><b>TIME ZONE</b></label><br>\n")
outputFile.write("<select id=\"timezones\" name=\"timezones\">\n")

for i in gmtJson:
    outputFile.write("\t<option value=\"" + str(gmtJson[i]) + "\">" + str(i) + "</option>\n")

outputFile.write("</select><br><br>\n\n")

print("Exported to \"" + os.path.join(os.path.dirname(__file__), outputFileName) + "\"")