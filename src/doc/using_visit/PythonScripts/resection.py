import sys

header_levels = []
# =======
# Chapter
# =======
header_levels.append("=") 
# Section
# =======
header_levels.append("=")
# Sub-section
# -----------
header_levels.append("-")
# sub-sub-section
# ~~~~~~~~~~~~~~~
header_levels.append("~")
# paragraph
# """""""""
header_levels.append("\"")

blank_line ="\n"
if len(sys.argv) != 2:
  print("Usage: resection.py file_name.rst")
  raise SystemExit 
f = open(str(sys.argv[1]))
file_lines = f.readlines()
f.close()
# Indentify section locations and levels
section_header_lines = []
section_header_levels = []
for idx,line in enumerate(file_lines):
  #strip away the \r\n
  stripped = line[0:len(line) - 2]
  if stripped == "":
    continue
  for idx2,header in enumerate(header_levels):
    if stripped == len(stripped) * header[0]:
      #check to see if this is a chapter heading
      if idx2 == 0:
        possible_match_idx = idx - 2
        if possible_match_idx >= 0:
          if file_lines[idx] == file_lines[possible_match_idx]:
            section_header_lines.append(idx)
            section_header_levels.append(idx2)
            break
      # make sure this is not a chapter 
      elif idx2 == 1:
        possible_title_idx = idx - 1
        if possible_title_idx >= 0:
          if len(file_lines[idx]) == len(file_lines[possible_title_idx]):
            section_header_lines.append(idx)
            section_header_levels.append(idx2)
            break
      else:
        section_header_lines.append(idx)
        section_header_levels.append(idx2)
print(section_header_lines)
print(section_header_levels)

for level in section_header_levels:
  if level >= len(header_levels) - 1:
    print("Error: pushing levels down one would exceed currently supported sectioning depth")
    print("Either add an additional level to this file or we could handle this another way")
    raise  SystemExit
# Now replace them with one level down

for idx,line_number in enumerate(section_header_lines):
  current_level = section_header_levels[idx]
  new_level = current_level + 1
  if current_level == 0:
    file_lines[line_number - 2] = blank_line
  file_lines[line_number] = file_lines[line_number].replace(header_levels[current_level], header_levels[new_level])

f = open(str(sys.argv[1]), "w")
#f = open("test_file.rst", "w")
f.write(''.join(file_lines))
f.close


