import os, subprocess, string
searches = (
("Connecting to viewer", "Connecting to viewer", 0), 
("Reading data", "Calling avtTermSrc::FetchData ", 0), 
("Contouring", "avtContourFilter took", 0),
("Rendering+Compositing", "NM::Render", 1))

def getfiles(dirname):
    dfiles = []
    files = os.listdir(dirname)
    for f in files:
        if f[0] != ".":
            dfiles = dfiles + [os.path.join(dirname, f)]
    return dfiles

def process_one_timings_file(filename):
    result = {}
    for s in searches:
        cmd = "grep \"%s\" %s" % (s[1], filename)
        txt = subprocess.check_output(cmd, shell=True)
        lines = string.split(txt, "\n")
        took = string.find(lines[s[2]], "took")
        if took != -1:
            result[s[0]] = lines[s[2]][took+5:]
        else:
            print("Arrgh!")
    return result

def combine_results(results):
    combined = {}
    for s in searches:
        key = s[0]
        for r in results:
            val = r[key]
            if key in combined:
                combined[key] = combined[key] + [val]
            else:
                combined[key] = [val]
    return combined

def compute_stats(results):
    stats = {}
    for s in searches:
        key = s[0]
        minval = 0
        maxval = 0
        sum = 0
        for i in range(len(results[key])):
            val = float(results[key][i])
            if i == 0:
                minval = val
                maxval = val
            else:
                if val < minval:
                    minval = val
                if val > maxval:
                    maxval = val
            sum = sum + val
        stats[key] = (minval, sum / float(len(results[key])), maxval)
    return stats

def main():
    #casename = "timings_1_16"
    #casename = "timings_16_256"
    #casename = "timings_32_512"
    #casename = "timings_64_1024"
    casename = "timings_256_4096"
    files = getfiles(casename)
    results = []
    for f in files:
        result = process_one_timings_file(f)
        results = results + [result]
    combined = combine_results(results)
    stats = compute_stats(combined)
    print(stats)

main()
