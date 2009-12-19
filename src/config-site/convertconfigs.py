import os, string

#
# This program converts an old style config-site into a cmake config-site
#
# Programmer: Brad Whitlock
# Date: Thu Nov 12 12:06:54 PST 2009
#
################################################################################

def translate_vars(s):
    if s[-4:] == "/lib":
        s = s[:-4]
    if string.find(s, "$") != -1:
        s0 = string.replace(s, "$VISITHOME", "${VISITHOME}")
        s1 = string.replace(s0, "$VISITARCH2", "${VISITARCH2}")
        s2 = string.replace(s1, "$VISITARCH", "${VISITARCH}")
        s3 = string.replace(s2, "$ARCH", "${VISITARCH}")
        s4 = string.replace(s3, "$CXXFLAGS", "${CXXFLAGS}")
        return s4
    return s

def remove_quote(s):
    if s[0] == '"':
        s = s[1:]
    if s[-1] == '"':
        s = s[:-1]
    return s

def filter_libs(libs):
    s = remove_quote(string.replace(libs, "-l", ""))
    return s

def get_value(key, allsections):
    for section in allsections:
        if key in section[0].keys():
            return section[0][key]
    return None

def translate_section(f, section):
    keys = section.keys()
    if "VISITHOME" in keys:
        f.write("SET(VISITHOME %s)\n" % translate_vars(section["VISITHOME"]))
        if "VISITARCH" in keys:
            f.write("SET(VISITARCH %s)\n" % translate_vars(section["VISITARCH"]))
        f.write("SET(CMAKE_VERBOSE_MAKEFILE TRUE)\n")
    elif "CC" in keys or "CXX" in keys or "CFLAGS" in keys or "CXXFLAGS" in keys:
        if "CC" in keys:
            f.write("SET(CMAKE_C_COMPILER %s)\n" % remove_quote(section["CC"]))
        if "CFLAGS" in keys:
            toks = string.split(string.replace(remove_quote(section["CFLAGS"]), "$CFLAGS", ""), " ")
            f.write("SET(CMAKE_C_FLAGS ")
            mpiflags = {}
            for t in toks:
                if string.find(t, "mpi") != -1 or string.find(t, "MPI") != -1:
                    mpiflags[t] = 0
                else:
                    f.write("%s " % t)
            f.write(")\n")
            if len(mpiflags.keys()) > 0:
                f.write("SET(MPI_CFLAGS ")
                for k in mpiflags.keys():
                     f.write("%s " % k)
                f.write(")\n")
        if "CXX" in keys:
            f.write("SET(CMAKE_CXX_COMPILER %s)\n" % remove_quote(section["CXX"]))
        if "CXXFLAGS" in keys:
            toks = string.split(string.replace(remove_quote(section["CXXFLAGS"]), "$CXXFLAGS", ""), " ")
            f.write("SET(CMAKE_CXX_FLAGS ")
            mpiflags = {}
            for t in toks:
                if string.find(t, "mpi") != -1 or string.find(t, "MPI") != -1:
                    mpiflags[t] = 0
                else:
                    f.write("%s " % t)
            f.write(")\n")
            if len(mpiflags.keys()) > 0:
                f.write("SET(MPI_CXXFLAGS ")
                for k in mpiflags.keys():
                     f.write("%s " % k)
                f.write(")\n")
    elif "MPI_LIBS" in keys:
        mpilibs = filter_libs(section["MPI_LIBS"])
        mpildflags = []
        if "LDFLAGS" in keys:
            s = string.replace(remove_quote(section["LDFLAGS"]), "$LDFLAGS", "")
            toks = string.split(s, " ")
            for t in toks:
                if t[:2] == "-L" or t[:3] == "-Wl":
                    mpildflags = mpildflags + [t]
        toks = string.split(mpilibs, " ")
        mpilibs = []
        for t in toks:
            if t[:2] == "-L" or t[:3] == "-Wl":
                mpildflags = mpildflags + [t]
            else:
                mpilibs = mpilibs + [t]
        if "CPPFLAGS" in keys:
            s = string.replace(remove_quote(section["CPPFLAGS"]), "$CPPFLAGS", "")
            f.write("SET(MPI_CXXFLAGS %s)\n" % s)
        if "CXXFLAGS" in keys:
            s = string.replace(remove_quote(section["CXXFLAGS"]), "$CXXFLAGS", "")
            f.write("SET(MPI_CXXFLAGS %s)\n" % s)
        if len(mpildflags) > 0:
            f.write("SET(MPI_LDFLAGS ")
            for flag in mpildflags:
                f.write("%s " % flag)
            f.write(")\n")
        f.write("SET(MPI_LIBS ")
        for t in mpilibs:
            f.write("%s " % t)
        f.write(")\n")
    elif "LDFLAGS" in keys:
        ldflags = string.replace(filter_libs(section["LDFLAGS"]), "$LDFLAGS", "")
        f.write("SET(CMAKE_EXE_LINKER_FLAGS %s)\n" % ldflags)
    elif "MESA" in keys:
        # Mesa section
        f.write("SET(MESA_DIR %s CACHE PATH \"Mesa root directory\")\n" % translate_vars(section["MESA"]))
    elif "VTK" in keys:
        # VTK section
        f.write("SET(VTK_DIR %s/lib/vtk-5.0 CACHE PATH \"VTK root directory\")\n" % translate_vars(section["VTK"]))
    elif "QT_BIN" in keys:
        # Qt section
        f.write("SET(QT_BIN %s)\n" % translate_vars(section["QT_BIN"]))
        f.write("SET(ENV{PATH} \"${QT_BIN}:$ENV{PATH}\")\n")
    elif "PYDIR" in keys:
        # Python section
        f.write("SET(PYTHON_DIR %s)\n" % translate_vars(section["PYDIR"]))
    elif "DEFAULT_CFITSIO_LIB" in keys:
        f.write("SET(CFITSIO_DIR %s)\n" % translate_vars(section["DEFAULT_CFITSIO_LIB"]))
    elif "DEFAULT_CCMIO_LIB" in keys:
        f.write("SET(CCMIO_DIR %s)\n" % translate_vars(section["DEFAULT_CCMIO_LIB"]))
    elif "DEFAULT_CGNS_LIB" in keys:
        f.write("SET(CGNS_DIR %s)\n" % translate_vars(section["DEFAULT_CGNS_LIB"]))
    elif "DEFAULT_EXODUS_LIBRARY" in keys:
        f.write("SET(EXODUSII_DIR %s)\n" % translate_vars(section["DEFAULT_EXODUS_LIBRARY"]))
    elif "DEFAULT_FASTBIT_LIB" in keys:
        f.write("SET(FASTBIT_DIR %s)\n" % translate_vars(section["DEFAULT_FASTBIT_LIB"]))
    elif "DEFAULT_GDAL_LIB" in keys:
        f.write("SET(GDAL_DIR %s)\n" % translate_vars(section["DEFAULT_GDAL_LIB"]))
    elif "DEFAULT_H5PART_LIB" in keys:
        f.write("SET(H5PART_DIR %s)\n" % translate_vars(section["DEFAULT_H5PART_LIB"]))
    elif "DEFAULT_H5PART_LIBS" in keys:
        f.write("SET(H5PART_DIR %s)\n" % translate_vars(section["DEFAULT_H5PART_LIBS"]))
    elif "DEFAULT_HDF4_LIBS" in keys:
        f.write("SET(HDF4_DIR %s)\n" % translate_vars(section["DEFAULT_HDF4_LIBS"]))
        f.write("SET(HDF4_LIBDEP /usr/lib jpeg)\n")
    elif "DEFAULT_HDF5_LIBLOC" in keys:
        f.write("SET(HDF5_DIR %s)\n" % translate_vars(section["DEFAULT_HDF5_LIBLOC"]))
        f.write("SET(HDF5_LIBDEP ${VISITHOME}/szip/2.1/${VISITARCH}/lib sz)\n")
    elif "MILI_DIR" in keys:
        f.write("SET(MILI_DIR %s)\n" % translate_vars(section["MILI_DIR"]))
    elif "BV_MILI_DIR" in keys:
        f.write("SET(MILI_DIR %s)\n" % translate_vars(section["BV_MILI_DIR"]))
    elif "DEFAULT_NETCDF_LIB" in keys:
        f.write("SET(NETCDF_DIR %s)\n" % translate_vars(section["DEFAULT_NETCDF_LIB"]))
    elif "DEFAULT_SILO_LIBLOC" in keys:
        f.write("SET(SILO_DIR %s)\n" % translate_vars(section["DEFAULT_SILO_LIBLOC"]))
        f.write("SET(SILO_LIBDEP HDF5_LIBRARY_DIR hdf5 ${HDF5_LIBDEP})\n")
    elif "DEFAULT_VISUS_LIB" in keys:
        f.write("SET(VISUS_DIR %s)\n" % translate_vars(section["DEFAULT_VISUS_LIB"]))
    elif "DEFAULT_ADVIO_LIB" in keys:
        f.write("SET(ADVIO_DIR %s)\n" % translate_vars(section["DEFAULT_ADVIO_LIB"]))
    elif "DEFAULT_SZIP_LIB" in keys:
        f.write("SET(SZIP_DIR %s)\n" % translate_vars(section["DEFAULT_SZIP_LIB"]))
    elif "DEFAULT_BOXLIB2D_LIBS" in keys:
        f.write("SET(BOXLIB2D_DIR %s)\n" % translate_vars(section["DEFAULT_BOXLIB2D_LIBS"]))
        f.write("SET(BOXLIB3D_DIR %s)\n" % translate_vars(section["DEFAULT_BOXLIB3D_LIBS"]))
    elif "DEFAULT_CMAKE" in keys:
        f.write("\n") # It's up at the top of the file now.
    else:
        for k in keys:
            f.write("#not done -- %s = %s\n" % (k, section[k]))

def process_file(filename):
    f = open(filename)
    lines = f.readlines()
    f.close()

    newname = string.replace(filename, ".conf", ".cmake")
    f = open(newname, "wt")

    allsections = []
    section = {}
    lines_in_section = []
    in_section = 0

    # Pass 1
    for i in range(len(lines)):
        line = string.replace(lines[i], "\n", "")
        print line
        if line == '':
            continue
        elif line[0] == '#':
            if in_section:
                if len(section.keys()) > 0:
                    allsections = allsections + [(section, lines_in_section)]
                section = {}
                lines_in_section = []
                in_section = 0
        else:
            eq = string.find(line, '=')
            if eq == -1:
                continue
            in_section = 1
            key = string.replace(line[:eq], " ", "")
            val = line[eq+1:]
            section[key] = val
            lines_in_section = lines_in_section + [i]
    if in_section:
        if len(section.keys()) > 0:
            allsections = allsections + [(section, lines_in_section)]

    print allsections

    # Write the path to cmake on the 1st line of the new file.
    defaultcmake = get_value("DEFAULT_CMAKE", allsections)
    if defaultcmake != None:
        if string.find(defaultcmake, "$VISITHOME") != -1:
            visithome = get_value("VISITHOME", allsections)
            if visithome != None:
                defaultcmake = string.replace(defaultcmake, "$VISITHOME", visithome)
        if string.find(defaultcmake, "$VISITARCH") != -1:
            visitarch = get_value("VISITARCH", allsections)
            if visitarch != None:
                defaultcmake = string.replace(defaultcmake, "$VISITARCH", visitarch)
        if string.find(defaultcmake, "2.6.4") == -1:
            defaultcmake = string.replace(defaultcmake, "2.4.5", "2.6.4")
            f.write("#%s\n\n" % defaultcmake)
            f.write("# 1. YOU MUST UPGRADE TO cmake 2.6.4 !!!! This script assumes 2.6.4 but it is unknown whether it exists on your system.\n")
        else:
            f.write("#%s\n\n" % defaultcmake)
        if get_value("VISITARCH", allsections) == None:
            f.write("# 2. This script does not use ${VISITARCH}. You should rerun build_visit to produce more uniformly installed libraries and a better config file.\n")
            f.write("\n")
    else:
        f.write("#/what/is/the/path/to/bin/cmake\n\n")

    # Pass 2
    sections_handled = [0]*len(allsections)
    for i in range(len(lines)):
        line = string.replace(lines[i], "\n", "")
        if line == '#':
            f.write("%s\n" % line)
        else:
            section = -1
            for s  in range(len(allsections)):
                if i in allsections[s][1]:
                    section = s
                    break
            if section == -1:
                if line[:2] == "if" or line[:2] == "fi":
                    f.write("#%s\n" % line)
                else:
                    f.write("%s\n" % line)
            elif sections_handled[section] == 0:
                translate_section(f, allsections[section][0])
                sections_handled[section] = 1
      
    f.close()


skips = ("Linux.conf", "AIX.conf", "Darwin.conf", "IRIX.conf", "IRIX64.conf", "AutobuildTemplate.conf", "SunOS.conf", "Template.conf")

def do_all_files():
    files = os.listdir(".")
    for file in files:
        if file[-4:] == "conf" and \
           not os.path.islink(file) and \
           not os.path.exists(file[:-4]+"cmake") and \
           file not in skips:
               process_file(file)

do_all_files()
     
#process_file("pion.conf")
#process_file("franklin.nersc.gov.conf")
#process_file("wig.ornl.gov.conf")
#process_file("sunspot.llnl.gov.conf")
#process_file("purple1441.conf")
