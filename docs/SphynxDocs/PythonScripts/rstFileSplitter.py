import sys

if len(sys.argv) != 3:
  print("Need file name and heading string, example: Operators.rst --")
  raise SystemExit

heading = str(sys.argv[2])
f = open(str(sys.argv[1]))
lines_in_file = f.readlines()
section_line_numbers = []
section_titles = []
counter = 0;
for line in lines_in_file:  
  if line[0:len(line)-2] == (len(line) -2) * heading[0] and line[0:len(line)-2] != "":
#    print("*****************************")
#    print(line[0:len(line)-2])
#    print((len(line) -2) * heading[0])
    section_line_numbers.append(counter-1)
    section_titles.append(lines_in_file[counter - 1])
  counter = counter + 1
print(section_line_numbers)
index_names = []
for idx, not_used in enumerate(section_titles):
  section_titles[idx] = section_titles[idx].replace(" ","_")
  section_titles[idx] = section_titles[idx].replace("\r","")
  index_names.append(section_titles[idx])
  section_titles[idx] = section_titles[idx].replace("\n","")
  section_titles[idx] = section_titles[idx] + ".rst"
num_sections = len(section_line_numbers)
last_line = len(lines_in_file)
for idx, section in enumerate(section_line_numbers):
   text = ""
   print(idx) 
   if idx != num_sections -1:
    text = lines_in_file[section_line_numbers[idx]:section_line_numbers[idx+1]]
   else:
    text = lines_in_file[section_line_numbers[idx]:last_line]
   text = ''.join(text)
   out_file = open(section_titles[idx],"w")
   out_file.write(text)
   out_file.close()

before_split = lines_in_file[0:section_line_numbers[0]]
before_split = ''.join(before_split)

out_file = open(sys.argv[1],"w")
out_file.write(before_split)
out_file.close()

index_file = open("index.txt", "w")
index_file.write(''.join(index_names)) 
index_file.close()
print(before_split)
print(section_titles)
