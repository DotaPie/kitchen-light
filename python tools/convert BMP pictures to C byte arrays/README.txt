Directories and files explained:
root
 |- images-bmp (.bmp images converted with https://online-converting.com/image/convert2bmp/#)
 |- images-png (.png images generated by https://www.pixilart.com/draw)
 |- project-files-pixilart (some https://www.pixilart.com/draw projects/templates and info about colors I used, not all)
 |- output.txt (generated C headers and arrays)
 |- README.txt (readme)
 |- script.py (main python script)

Functionality of script.py:
	1) Load compatible .bmp images
		- from folder that is in the same directory as script.py
		- folder name is used from folderNameWithBmpImages variable
	2) Converts images to compatible uint16_t C byte arrays
		- uint16_t because I use 16-bit color display (RGB565), but script can be modified easily to any color space.
	3) Saves them to output.txt in the same folder as script.py

What to do next after script.py run:
	1) In output.txt there is content for images.h and images.cpp, copy them and paste them to appropriate project files.
	
How to actually obtain compatible .bmp images for the script?
	1) convert to .bmp format in this tool: https://online-converting.com/image/convert2bmp/#
	2) use these settings:
		- Color: 16 (5:6:5, RGB Hi color)
			- this is what my display and script supports
		- With row direction: Top - Bottom
		- Quantization equal to: 0
		- Using dithering: No
		- we can set resize if needed
			- I already prepared my images in correct resolution (64x32) in https://www.pixilart.com/draw
	3) download to the folder that is in the same directory as script.py
		- folder name is used from folderNameWithBmpImages variable