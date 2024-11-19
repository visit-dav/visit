import os
import hashlib
import shutil
import fnmatch

out_dir = "/usr/workspace/wsa/visit/dashboard/dashboard"

base_dirs = []

imode = 0
mode_dirs = ["poodle_trunk_serial", "poodle_trunk_parallel", "poodle_trunk_scalable_parallel_icet"]

##############################################################################
#
# Calculate the list of baseline directories.
#
##############################################################################
def calc_base_dirs():
    #
    # Get the list of baseline directories
    #
    global out_dir
    global base_dirs
    base_dirs = []
    for dir in os.listdir(os.path.join(out_dir, "baselines", mode_dirs[imode])):
        base_dirs.append(dir)

    base_dirs.sort(reverse = True)
    print("        The baseline directories are:")
    for dir in base_dirs:
        print("           %s" % dir)

##############################################################################
#
# Get the directory of the baseline for this particular file.
#
##############################################################################
def get_base_dir(file):
    for dir in base_dirs:
        base_file = os.path.join(out_dir, "baselines", mode_dirs[imode], dir, file)
        if (os.path.exists(base_file)):
            return dir

    return ""

##############################################################################
#
# Determine if the file is a baseline, either a baseline image or an html
# file of a text file.
#
##############################################################################
def is_baseline(cur_dir, file):
    baseline = False
    if (file.endswith(".png") and file.startswith("b_") and not file.endswith("thumb.png")):
        #
        # This is an image baseline.
        #
        baseline = True
    elif (file.endswith(".html")):
        #
        # This may be a text baseline. Unfortunately, we can't tell by
        # the name and have to look at the contents to find out if it
        # is or is not.
        #
        f = open(os.path.join(cur_dir, mode_dirs[imode], file), "r")
        line = f.readline()
        line = f.readline()
        line = f.readline()
        line = f.readline()
        line = f.readline()
        if (line[29:35] == "Legend"):
            line1 = f.readline()
            line2 = f.readline()
            line3 = f.readline()
            if (line1[26:28] == "0 " and line2[26:28] == "0 " and line3[26:28] == "0 "):
                baseline = True

        f.close()

    return baseline

##############################################################################
#
# Copy any new baselines to the baseline directory in the dashboard. The
# baselines may either be images or html files of text baselines. In the
# case of text baselines, the html file is of the difference, which of
# course in this case doesn't show any differences. This seems a little
# strange, but that's what the test suite outputs.
#
##############################################################################
def copy_new_baselines(cur_dir):
    #
    # Get the list of baseline directories
    #
    calc_base_dirs()

    nbase= len(base_dirs)

    #
    # Process the serial results.
    #
    mode_dir = os.path.join(cur_dir, mode_dirs[imode])
    
    newbase = []
    for file in os.listdir(mode_dir):
        if (is_baseline(cur_dir, file)):
            found = False
            ibase = 0
            while (ibase < nbase and not found):
                base_file = os.path.join(out_dir, "baselines", mode_dirs[imode], base_dirs[ibase], file)
                if (os.path.exists(base_file)):
                    found = True
                    #
                    # File exists, check if md5sums match.
                    #
                    cur_file = os.path.join(mode_dir, file)
                    hasher1 = hashlib.md5()
                    file1 = open(cur_file, 'rb')
                    buf1 = file1.read()
                    hasher1.update(buf1)
                    hash1 = hasher1.hexdigest()

                    hasher2 = hashlib.md5()
                    file2 = open(base_file, 'rb')
                    buf2 = file2.read()
                    hasher2.update(buf2)
                    hash2 = hasher2.hexdigest()
                    if (hash1 != hash2):
                        newbase.append(file)

                ibase += 1

            if (not found):
                newbase.append(file)

    #
    # If we have any new baselines, add a new baseline directory and populate it.
    #
    if (len(newbase) > 0):
        print("        We have new baselines")
        new_dir = os.path.join(out_dir, "baselines", mode_dirs[imode], cur_dir)
        os.mkdir(new_dir)
        for base in newbase:
            shutil.copy(os.path.join(mode_dir, base), os.path.join(new_dir, base))
            if (base.endswith(".png")):
                base_thumb = base[0:-4] + "_thumb.png"
                shutil.copy(os.path.join(mode_dir, base_thumb), os.path.join(new_dir, base_thumb))

        calc_base_dirs()

##############################################################################
#
# Copy an html file with the results from a python test file, translating
# portions of the file as appropriate.
#
##############################################################################
def copy_test_result_file(cur_file, out_file):
    cur = open(cur_file, "r")
    out = open(out_file, "wt")

    prev_line = cur.readline()
    while prev_line:
        cur_line = cur.readline()
        if (not cur_line):
            out.write(prev_line)
            prev_line = cur_line
            continue
        if ("modifications" in cur_line):
            #
            # This is the result of a text comparison.
            #
            if (prev_line[0:24] == "  <td bgcolor=\"#00ff00\">"):
                #
                # This is a pass, change the link in the previous line to
                # point to a baseline.
                #
                result_name = prev_line.split(">")[1]
                result_name = result_name.split("<")[0]
                base_dir = get_base_dir(result_name + ".html")
                prev_line = "  <td bgcolor=\"#00ff00\"><a href=\"" + \
                    os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, result_name) + ".html\">" + \
                    result_name + "</a></td>\n"
            out.write(prev_line)
            prev_line = cur_line
        elif (cur_line[0:33] == "  <td bgcolor=\"#00ff00\"><a href=\"" or \
              cur_line[0:33] == "  <td bgcolor=\"#0000ff\"><a href=\"" or \
              cur_line[0:33] == "  <td bgcolor=\"#ffff00\"><a href=\""):
            #
            # This is a passed test, either a clean pass or one within tolerance,
            # or a skipped test. Remove the link.
            #
            result_name = cur_line.split(">")[2]
            result_name = result_name.split("<")[0]
            out.write(prev_line)
            prev_line = cur_line[0:24] + result_name + "</td>\n"
        elif (cur_line[0:38] == "  <td colspan=3 align=center><a href=\""):
            #
            # This is baseline image of a passed test. Change the link to point
            # to a baseline.
            #
            image_name = cur_line.split("\"")[1]
            image_thumb_name = cur_line.split("\"")[7]
            base_dir = get_base_dir(image_name)
            out.write(prev_line)
            prev_line = "  <td colspan=3 align=center><a href=\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "\" onclick='return popup(\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "\",\"image\");'><img src=\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_thumb_name) + \
                "\"></a></td>\n"
        elif (cur_line[0:30] == "  <td align=center><a href=\"b_"):
            #
            # This is a baseline image of a failed test. Change the link to point
            # to a baseline.
            #
            image_name = cur_line.split("\"")[1]
            image_thumb_name = cur_line.split("\"")[7]
            base_dir = get_base_dir(image_name)
            out.write(prev_line)
            prev_line = "  <td align=center><a href=\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "\" onclick='return popup(\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "\",\"image\");'><img src=\"" + os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_thumb_name) + \
                "\"></a></td>\n"
        else:
            #
            # This is a non-special line. Don't do anything other than output
            # the previous line.
            #
            out.write(prev_line)
            prev_line = cur_line

    cur.close()
    out.close()

##############################################################################
#
# Copy an html file with the baseline, current and difference map of a
# failed test, translating links to baseline images to point the appropriate
# location.
#
##############################################################################
def copy_failed_file(cur_file, out_file):
    cur = open(cur_file, "r")
    out = open(out_file, "wt")

    for line in cur:
        if (line[0:46] == "    <td><a href=\"\" onMouseOver=\"document.b.src"):
            #
            # This is the baseline image. Change the link to point to
            # a baseline.
            # 
            image_name = line.split("'")[3]
            base_dir = get_base_dir(image_name)
            new_line = "    <td><a href=\"\" onMouseOver=\"document.b.src='c" + \
                image_name[1:] + "'\" onMouseOut=\"document.b.src='" + \
                os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "'\"><img name=\"b\" border=0 src=\"" + \
                os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "\"></img></a></td>\n"
            out.write(new_line)
        elif (line[0:46] == "    <td><a href=\"\" onMouseOver=\"document.d.src"):
            #
            # This is the difference image. Change the link to point to
            # a baseline.
            # 
            image_name = line.split("'")[1]
            base_dir = get_base_dir(image_name)
            new_line = "    <td><a href=\"\" onMouseOver=\"document.d.src='" + \
                os.path.join("..", "..", "baselines", mode_dirs[imode], base_dir, image_name) + \
                "'\" onMouseOut=\"document.d.src='d" + image_name[1:] + \
                "'\"><img name=\"d\" border=0 src=\"d" + image_name[1:] + \
                "\"></img></a></td>\n"
            out.write(new_line)
        else:
            out.write(line)

    cur.close()
    out.close()

##############################################################################
#
# Copy the results to the output location.
#
##############################################################################
def copy_results(cur_dir):
    global imode
    mode_dir = os.path.join(cur_dir, mode_dirs[imode])
    
    #
    # Copy the index.html, the py files, and relevant html files.
    #
    out_mode_dir = os.path.join(out_dir, cur_dir)
    if (not os.path.exists(out_mode_dir)):
        os.mkdir(out_mode_dir)

    out_mode_dir = os.path.join(out_mode_dir, mode_dirs[imode])
    os.mkdir(out_mode_dir)
    shutil.copy(os.path.join(mode_dir, "index.html"), out_mode_dir)
    css_dir = os.path.join(out_mode_dir, "css")
    os.mkdir(css_dir)
    shutil.copy(os.path.join(mode_dir, "css", "styles.css"), css_dir)
    js_dir = os.path.join(out_mode_dir, "js")
    os.mkdir(js_dir)
    shutil.copy(os.path.join(mode_dir, "js", "jquery-3.6.1.min.js"), js_dir)
    shutil.copy(os.path.join(mode_dir, "js", "jquery.tablesorter.js"), js_dir)
    shutil.copy(os.path.join(mode_dir, "js", "purl.js"), js_dir)
    for file in os.listdir(mode_dir):
        if (file.endswith("_py.html")):
            #
            # This is an html file of the python script.
            #
            shutil.copy(os.path.join(mode_dir, file), out_mode_dir)
            copy_test_result_file(os.path.join(mode_dir, file[0:-8] + ".html"), os.path.join(out_mode_dir, file[0:-8] + ".html"))
        elif ((file.startswith("c_") or file.startswith("d_")) and file.endswith(".png")):
            #
            # This is a current or difference image.
            #
            shutil.copy(os.path.join(mode_dir, file), out_mode_dir)
        elif (file.startswith("c_") and file.endswith(".txt")):
            #
            # This is a current text file. This isn't needed for the
            # web pages, but is there for use with the rebaseline script.
            #
            shutil.copy(os.path.join(mode_dir, file), out_mode_dir)
        elif (file.endswith(".html")):
            #
            # This is a generic html file. Unfortunately, we have to open it
            # to determine what kind it is to determine if we copy it or not.
            #
            f = open(os.path.join(mode_dir, file), "r")
            line = f.readline()
            line = f.readline()
            line = f.readline()
            line = f.readline()
            line = f.readline()
            #
            # This is an html file related to a text file difference. 
            #
            if (line[29:35] == "Legend"):
                line1 = f.readline()
                line2 = f.readline()
                line3 = f.readline()
                if (line1[18:24] == "00ff88" and line2[18:24] == "ff8888" and line3[18:24] == "ffff00" \
                    and (line1[26:28] != "0 " or line2[26:28] != "0 " or line3[26:28] != "0 ")):
                    print("        Copying text diff %s" % file)
                    shutil.copy(os.path.join(mode_dir, file), out_mode_dir)

            line = f.readline()
            line = f.readline()
            #
            # This is an html file related to a failed test (skipped or not skipped). 
            #
            if (line[18:24] == "Failed"):
                print("        Copying failure %s" % file)
                copy_failed_file(os.path.join(mode_dir, file), os.path.join(out_mode_dir, file))

            f.close()

##############################################################################
#
# Copy any new output directories to the dashboard.
#
##############################################################################
def main():
    dirs = []
    for dir in os.listdir("."):
        if (fnmatch.fnmatch(dir, "????-??-??-??:??")):
            dirs.append(dir)

    dirs.sort()

    global imode
    for dir in dirs:
        print("Processing %s" % dir)
        for imode in range(3):
            print("    Doing mode %d" % imode)
            copy_new_baselines(dir)
            copy_results(dir)

if __name__ == "__main__":
    main()
