import os

# begin conf 
folderNameWithBmpImages = "images-bmp" # folder must be in the same folder as this script
imageWidth = 64
imageHeight = 32
colorBits = 16 # using RGB565, use only values 8, 16 or 32
outputFileName = "output.txt" # output file will be in the same folder as this script
# end conf

# paths
path = os.path.join(os.path.dirname(__file__), folderNameWithBmpImages)
dir_list = os.listdir(path)

# check invalid conf
if not colorBits == 8 and not colorBits == 16 and not colorBits == 32:
    print("Export failed. Please change colorBits configuration to 8, 16 or 32.")
    exit()
    
# open output file
outputFile = open(os.path.join(os.path.dirname(__file__), outputFileName), "w")

# print .h part
outputFile.write("Put to images.h:\n")

for i in dir_list:
    outputFile.write("extern const uint" + str(colorBits) + "_t image_" + i.replace(".bmp", "") + "[" + str(imageWidth * imageHeight) + "];\n")

# print .cpp part
outputFile.write("\nPut to images.cpp:\n")

for i in dir_list:
    # load image to byte array
    image = open(os.path.join(path, i), "rb")
    b = bytearray(image.read())

    # strip array of bmp header
    headerSize = len(b) - (imageWidth * imageHeight * int(colorBits/8))
    b = b[headerSize:]

    C_string = "const uint" + str(colorBits) + "_t image_" + i.replace(".bmp", "") + "[" + str(imageWidth * imageHeight) + "] = {"

    nByte = 0
    colorBytes = bytearray(b'')

    for j in b:
        colorBytes.append(j)    

        if nByte % int(colorBits/8) == int(colorBits/8) - 1:
            # dont put comma before first value
            if nByte != int(colorBits/8) - 1:
                C_string = C_string + ","  

            C_string = C_string + "0x"

            colorBytes.reverse()
            for k in colorBytes:
                C_string = C_string + "{:02x}".format(k)
            colorBytes.clear()

        nByte += 1

    C_string = C_string + "};\n\n"

    outputFile.write(C_string)

print("Exported to \"" + os.path.join(os.path.dirname(__file__), outputFileName) + "\"")