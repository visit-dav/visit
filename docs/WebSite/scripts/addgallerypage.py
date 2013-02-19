# Program: addgallery.py
#
# Purpose: This script adds a new page to the VisIt gallery.
#
# Programmer: Brad Whitlock
# Date: Thu Apr 5 09:19:51 PDT 2007
#
# Modifications:
#   Brad Whitlock, Fri Jul 31 10:56:00 PDT 2009
#   I updated it to use a newer page style and I made it use SVN.
#
###############################################################################
import os, string

def UpdateLinksOnPage(filename, currentPageIndex, lastPageIndex):
    # Read the gallery HTML page.
    f = open(filename)
    lines = f.readlines()
    f.close()

    # Open a new file
    tmpnam = filename + ".tmp"
    out = open(tmpnam, "wt")

    # Look for certain lines that we're going to change.
    for line in lines:
        # Update the links line for the first gallery page
        first = '<td align="right" width="40%">First'
        idx = string.find(line, first)
        if idx != -1:
            line = line[:idx + len(first) - 5]
            line = line + 'First | Previous | <a href="gallery_01.html">Next</a> | <a href="gallery_%02d.html">Last</a></td>\n' % lastPageIndex
            out.write(line)
            continue

        # Update the links line.
        first = '<a href="gallery_00.html">First</a>'
        idx = string.find(line, first)
        if idx != -1:
            line = line[:idx]
            line = line + '<a href="gallery_00.html">First</a> | <a href="gallery_%02d.html">Previous</a> | <a href="gallery_%02d.html">Next</a> | <a href="gallery_%02d.html">Last</a></td>\n' % (currentPageIndex-1,currentPageIndex+1,lastPageIndex)
            out.write(line)
            continue

        # The line was not the links line. Let's see if it was the "N of M" line.
        nofm = "<h2>%d of %d</h2>" % (currentPageIndex+1, lastPageIndex)
        if string.find(line, nofm) != -1:
            newnofm = "<h2>%d of %d</h2>" % (currentPageIndex+1, lastPageIndex+1)
            line = string.replace(line, nofm, newnofm)
            out.write(line)
            continue
        nofm = "<h2>%d of %d </h2>" % (currentPageIndex+1, lastPageIndex)
        if string.find(line, nofm) != -1:
            newnofm = "<h2>%d of %d</h2>" % (currentPageIndex+1, lastPageIndex+1)
            line = string.replace(line, nofm, newnofm)
            out.write(line)
            continue

        out.write(line)
    out.close()

    os.rename(tmpnam, filename)

def UpdateGalleryLinks(presentation):
    filedir = "."
    files = os.listdir(".")

    # Count all of the gallery pages.
    nGalleryPages = 0
    while 1:
        gallery = "gallery_%02d.html" % nGalleryPages
        if gallery in files:
            nGalleryPages = nGalleryPages + 1
        else:
            break

    # Now that we know how many pages there are, update the pages.
    for i in range(nGalleryPages):
        gallery = "%s/gallery_%02d.html" % (filedir, i)
        UpdateLinksOnPage(gallery, i, nGalleryPages)

    # Write the new last gallery page.
    outname = "%s/gallery_%02d.html" % (filedir, nGalleryPages)
    out = open(outname, "wt")
    out.write('<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.01 Transitional//EN"\n')
    out.write('"http://www.w3.org/TR/html4/loose.dtd">\n')
    out.write('<html>\n')
    out.write('<head>\n')
    out.write('<meta http-equiv="Content-Type" content="text/html; charset=iso-8859-1">\n')
    out.write('<meta name="LLNLRandR" content="UCRL-WEB-%s">\n' % presentation)
    out.write('<meta name="review" content="May 2012">\n')
    out.write('<meta name="copyright" content="This document is copyrighted U.S. Department of Energy under Contract DE-AC52-07NA27344">\n')
    out.write('<meta name="language" content="en">\n')
    out.write('<meta name="rating" content="general">\n')
    out.write('<meta name="distribution" content="global">\n')
    out.write('<meta name="keywords" content="visualization, qualitative, quantitative, analysis, distributed, parallel, tera-scale, rectilinear, curvilinear, structured, unstructured, mesh, grid, finite element, hydrodynamics, python, java, unix, irix, tru64, aix, linux, solaris, windows 98, windows xp, open source contour, vector, volume visualization, streamlines">\n')
    out.write('<meta name="description" content="Developed by the Lawrence Livermore National Laboratory, VisIt contains a rich set of visualization methods--such as contour plots, pesudocolor plots, volume plots, vector plots, and boundary plots--for visualizing scientific data. VisIt allows the ability to provide quantitative as well as qualitative information from a scientific data set.">\n')
    out.write('<meta http-equiv="Expires" content="never">\n')
    out.write('<meta name="date" content="2013-02-19">\n')
    out.write('<meta name="Author" content="Eric Brugger">\n')
    out.write('<!-- InstanceBeginEditable name="doctitle" -->\n')
    out.write('<title>VisIt Gallery</title>\n')
    out.write('<!-- InstanceEndEditable -->\n')
    out.write('<!-- InstanceBeginEditable name="head" -->\n')
    out.write('<!-- InstanceEndEditable -->\n')
    out.write('<link href="VisIt.css" rel="stylesheet" type="text/css">\n')
    out.write('</head>\n')
    out.write('\n')
    out.write('<body>\n')
    out.write('<table width="100%%"  border="0" cellspacing="5" cellpadding="0">\n')
    out.write('  <tr>\n')
    out.write('    <td colspan="3" background="media/gallerytop_bg.gif"><img src="media/gallerytop.gif" alt="Gallery top"></td>\n')
    out.write('  </tr>\n')
    out.write('  <tr>\n')
    out.write('    <td width="20%%"><a href="home.html">VisIt home page</a></td>\n')
    out.write('<!-- InstanceBeginEditable name="imagenavigate" -->\n')
    out.write('    <td align="right" width="40%%"><a href="gallery_00.html">First</a> | <a href="gallery_%02d.html">Previous</a> | Next | Last</td>\n' % (nGalleryPages-1))
    out.write('    <td align="right" width="30%%"><h2>%d of %d</h2></td>\n' % (nGalleryPages+1, nGalleryPages+1))
    out.write('<!-- InstanceEndEditable -->\n')
    out.write('  </tr>\n')
    out.write('</table>\n')
    out.write('<!-- InstanceBeginEditable name="imagearea" -->\n')
    out.write('<table width="90%%"  border="0" cellspacing="5" cellpadding="0">\n')
    out.write('  <tr>\n')
    out.write('    <td width="500" align="top"><img src="media/gallery/gallery_%d.jpg" alt="Add title here" width="500" height="500"></td>\n' % nGalleryPages)
    out.write('    <td valign="top"><h2>Add title here...</h2>\n')
    out.write('      <p>Add text here...</p>\n')
    out.write('    </td>\n')
    out.write('  </tr>\n')
    out.write('</table>\n')
    out.write('<!-- InstanceEndEditable -->\n')
    out.write('<div align="center"> \n')
    out.write('  <p><a href="mailto:brugger1@llnl.gov, whitlock2@llnl.gov" class="links">VisIt Webmaster</a><br>\n')
    out.write('    <span class="body_text">Updated: \n')
    out.write('    <!-- #BeginDate format:Am1 -->July 31, 2009<!-- #EndDate -->\n')
    out.write('    </span></p>\n')
    out.write('  <table width="100%%" border="0" cellspacing="0" cellpadding="0">\n')
    out.write('    <tr> \n')
    out.write('      <td background="media/bottom_band.jpg">&copy;<span class="body_text">UCRL-WEB-%s, \n' % presentation)
    out.write('        <a href="http://www.llnl.gov/disclaimer.html" target="_parent" class="links">privacy \n')
    out.write('        and legal notice</a></span></td>\n')
    out.write('      <td width="5">&nbsp;</td>\n')
    out.write('    </tr>\n')
    out.write('  </table>\n')
    out.write('</div>\n')
    out.write('</body>\n')
    out.write('</html>\n')
    out.close()

    # Enter the new file into SVN.
    command = 'svn add ' + outname
    os.system(command)

def main():
    UpdateGalleryLinks("555771")

main()
