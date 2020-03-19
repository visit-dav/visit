import string, sys, subprocess

# Returns (key,value) for a version
def increment_for_version(ver):
    key = ""
    incr = 0
    p = subprocess.Popen(["svn", "log", "-r", str(ver), "."], stdout=subprocess.PIPE)
    output = p.communicate()[0]
    eoln = string.find(output, "\n")
    if eoln != -1:
        line = output[eoln+1:]
    else:
        line = output
    if len(line) > 0 and line[0] == 'r':
        p1 = string.find(line, "|")
        if p1 != -1:
            p2 = string.find(line, "|", p1+1)
            if p2 != -1:
                key = line[p2+2:p2+2+7]
                incr = 1
    return (key, incr)

def main(startver, endver):
    hits_per_month = {}
    count = 0
    for v in range(startver, endver):
        k,incr = increment_for_version(v)
        if incr > 0:
            if k in list(hits_per_month.keys()):
                hits_per_month[k] = hits_per_month[k] + incr
            else:
                hits_per_month[k] = incr
        count = count + 1
        if count == 10:
            print(v - startver + 1, "/", endver - startver)
            count = 0

    print("\n\nMonth  ", "Number of commits")
    keys = list(hits_per_month.keys())
    keys.sort()
    for k in keys:
        print(k, hits_per_month[k])

if len(sys.argv) < 3:
    print(sys.stderr << "Usage: python yearly_commits.py startver endver\n")
    sys.exit(-1)

startver = int(sys.argv[1])
endver = int(sys.argv[2])
if endver < startver:
    tmp = startver 
    startver = endver
    endver = tmp
#print "incr(17525) = ", increment_for_version(17526)
#main(17525, 17600)
main(startver, endver)

sys.exit(0)
