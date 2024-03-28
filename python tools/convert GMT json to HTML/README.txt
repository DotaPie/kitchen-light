Directories and files explained:
root
 |- output.txt (generated options for <select></select> HTML tag)
 |- README.txt (readme)
 |- script.py (main python script)
 |- zones.json (downloaded from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json)

Functionality of script.py:
	1) Take json from https://github.com/nayarsystems/posix_tz_db/blob/master/zones.json (manual download needed, put it to the same folder as script.py)
	2) Script converts content of downloaded json file to HTML compatible <option></option> tag to an output.txt file to the same folder as script.py

What to do next after script.py run:
	1) Copy content of this file to "html.cpp" -> "const char *htmlWebPageForm..." -> "<select id="timezones" name="timezones">[output.txt]</select>" (replace [output.txt])