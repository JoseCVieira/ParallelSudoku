import re,sys
import urllib.request
import math
from bs4 import BeautifulSoup

url = sys.argv[1]
file = urllib.request.urlopen(url)
count = 0
#the HTML code you've written above
parsed_html = BeautifulSoup(file, "html.parser")
table = parsed_html.findAll("tr", attrs={"class":"grid"})
text = ""
for row in table:
	td = row.findAll('td')
	count+=1
	for c in td:
		text+=c.text + "#"
	text+="\n"

new_text1 = str(int(math.sqrt(count))) + "\n"+text.replace(" ","0")
new_text_f = new_text1.replace("#", " ")

with open('input_file', 'w') as file:
    file.write(new_text_f)

file.close()
