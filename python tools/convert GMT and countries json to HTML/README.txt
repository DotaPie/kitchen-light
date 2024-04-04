Directories and files explained:
root
 |- output.txt (generated options for <select></select> HTML tag)
 |- README.txt (readme)
 |- script.py (main python script)
 |- zones.json (downloaded from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json)
 |- ISO3166-alpha2.json (downloaded from https://gist.github.com/ssskip/5a94bfcd2835bf1dea52)

Functionality of script.py:
	1) Loads json from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json (manual download needed, put it to the same folder as script.py)
	2) Loads json from https://gist.github.com/ssskip/5a94bfcd2835bf1dea52 (manual download needed, put it to the same folder as script.py)
	3) Converts content of downloaded json files to HTML compatible <option></option> tags to an output.txt file to the same folder as script.py with proper <label></label> and <select></select>

What to do next after script.py run:
	1) Copy content of generated file to proper spot to "const char *htmlWebPageForm..." in "html.cpp"