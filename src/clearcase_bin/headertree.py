#!/usr/local/apps/bin/python
import os, sys, string

def GetHeaders(filename):
    f = open(filename, "r")
    text = f.readlines()
    f.close()
    headers = []
    for line in text:
        if(len(line) > 0 and line[:8] == "#include"):
            h = line[8:]
            ci = string.find(h, "/")
            if(ci > 0):
                h = h[:ci]
            h = string.replace(h, " ", "")
            h = string.replace(h, "<", "")
            h = string.replace(h, ">", "")
            h = string.replace(h, '"', "")
            h = string.replace(h, "\n", "")
            headers = headers + [h]
    return headers


def GetFullName(headerName, dirName, headers):
    fullName = ""
    for h in headers:
        if(h[0] == headerName):
            if(h[1] == 0):
                fullName = h[2]
            h[1] = 1
            break
    return fullName


def IndentedPrint(indent, str):
    indentStr = ""
    for i in range(indent):
        indentStr = indentStr + "    "
    indentStr = indentStr + str
    print indentStr


def PrintHeaderTree(indent, filename, visitHeaders, vtkHeaders, QtHeaders):
    # Get the headers that the file includes.
    headers = GetHeaders(filename)

    # Print the headers for each header.
    for h in headers:
        fullName = GetFullName(h, "visit", visitHeaders)
        if(fullName == ""):
            fullName = GetFullName(h, "vtk", vtkHeaders)
        if(fullName == ""):
            fullName = GetFullName(h, "qt", QtHeaders)
        IndentedPrint(indent+1, h)
        if(fullName != ""):
            PrintHeaderTree(indent+1, fullName, visitHeaders, vtkHeaders, QtHeaders)


def CreateIncludes(dirName):
    files = os.listdir(dirName)
    includes = []
    for f in files:
        fullName = "%s/%s" % (dirName, f)
        includes = includes + [[f, 0, fullName]]
    return includes


def ResetIncludes(headers):
    for h in headers:
        h[1] = 0


def main():
    visitDir = "/data_vobs/VisIt"
    visitHeaders = CreateIncludes("%s/include/visit" % visitDir)
    vtkHeaders   = CreateIncludes("%s/include/vtk" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Common" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Filtering" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Graphics" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Hybrid" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/IO" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Imaging" % visitDir)
    vtkHeaders   = vtkHeaders + CreateIncludes("%s/include/vtk/Rendering" % visitDir)
    QtHeaders    = CreateIncludes("%s/include/qt" % visitDir)

    i = 0
    for arg in sys.argv[1:]:
        print "arg[%d] = %s" % (i, arg)

        IndentedPrint(0, arg)
        PrintHeaderTree(0, arg, visitHeaders, vtkHeaders, QtHeaders)

        ResetIncludes(visitHeaders)
        ResetIncludes(vtkHeaders)
        ResetIncludes(QtHeaders)


# Call main
main()
