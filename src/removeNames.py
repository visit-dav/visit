import os, sys, string


def processFile(file):
    f = open(file, "rt")
    lines = f.readlines()
    f.close()
    print "Read lines from ", file  

    f = open(file, "wt")
    for line in lines:
        index = string.find(line, "new Q")
        if index == -1:
            index = string.find(line, "addMultiCellWidget")
            if index == -1:
                f.write(line)
            else:
                p0 = string.find(line[index+18:], "(")
                p1 = string.find(line[index+18:], ")")
                print p0, p1
                try:
                    args = string.split(line[index+18+p0+1:index+18+p1], ",")
                    if len(args) == 5:
                        w = args[0]
                        r0 = int(args[1])
                        r1 = int(args[2])
                        c0 = int(args[3])
                        c1 = int(args[4])
                        s = line[:index+18+p0+1] + w + ", "
                        s2 = "%d, %d, %d, %d" % (r0, c0, r1-r0+1, c1-c0+1)
                        s3 = line[index+18+p1:]
                        newline = string.replace(s + s2 + s3, "addMultiCellWidget", "addWidget")
                        print line, "--->", newline
                        f.write(newline)
                    else:
                        f.write(line)
                except:
                    f.write(line)
        else:
            # See if we can find a name in the line
            try:
                start = index + 5
                while line[start] != '(':
                    start = start + 1
                start = start + 1
                end = start + 1
                foundEnd = 0
                pLevel = 0
                while foundEnd == 0:
                    if line[end] == '(':
                        pLevel = pLevel + 1
                    elif line[end] == ')':
                        if pLevel == 0:
                            foundEnd = 1
                        else:
                            pLevel = pLevel - 1
                    end = end + 1
                args = line[start:end-1]
                comma = string.rfind(args, ",")
                if comma == -1:
                    modifiedLine = line
                else:
                    quote = string.find(args, "\"")
                    if quote == -1:
                        modifiedLine = line
                    else:
                        # Found a string as the last argument. Remove it.
                        modifiedLine = line[:start] + args[:comma] + line[end-1:]
                        print "%s ---> %s" % (line[:-1], modifiedLine[:-1])
                f.write(modifiedLine)
            except:
                # There was some dumb error, skip and just put in the old line.
                f.write(line)
    f.close()

for file in sys.argv[1:]:
    processFile(file)
