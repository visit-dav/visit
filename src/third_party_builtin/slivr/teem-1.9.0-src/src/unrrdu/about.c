/*
  Teem: Tools to process and visualize scientific data and images
  Copyright (C) 2005  Gordon Kindlmann
  Copyright (C) 2004, 2003, 2002, 2001, 2000, 1999, 1998  University of Utah

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public License
  (LGPL) as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.
  The terms of redistributing and/or modifying this software also
  include exceptions to the LGPL that facilitate static linking.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
*/

#include "unrrdu.h"
#include "privateUnrrdu.h"

#define INFO "Information about this program and its use"

int
unrrdu_aboutMain(int argc, char **argv, char *me, hestParm *hparm) {
  char buff[AIR_STRLEN_MED], fmt[AIR_STRLEN_MED];
  char par1[] = "\t\t\t\t"
    "\"unu\" is a command-line interface to much of the functionality "
    "in \"nrrd\", a C library for raster data processing. Nrrd is one "
    "library in the \"Teem\" collection of libraries.  More information "
    "about Teem is at <http://teem.sourceforge.net>.\n";
  char par7[] = "\t\t\t\t"
    "Users are strongly encouraged to join the teem-users mailing list: "
    "<http://lists.sourceforge.net/lists/listinfo/teem-users>. This is "
    "the primary forum for feedback, questions, and feature requests.\n";
  char par2[] = "\t\t\t\t"
    "The utility of unu is mainly as a pre-processing tool for getting "
    "data into a type, encoding, format, or dimensions best suited for some "
    "visualization or rendering task.  Also, slices and projections are "
    "effective ways to visually inspect the contents of a dataset. "
    "Especially useful commands include make, resample, crop, slice, "
    "project, histo, dhisto, quantize, and save.  Full "
    "documentation for each command is shown by typing the command alone, "
    "e.g., \"unu make\".  Unu can process CT and MRI volume "
    "datasets, grayscale and color images, time-varying volumes of "
    "vector fields (5-D arrays), and more.  Currently supported formats are "
    "plain text files (2-D float arrays), "
    "NRRD, VTK structured points, and PNG and PNM images.  "
    "\"unu make -bs -1\" can read from DICOM files.  "
    "\"unu save\" can generate EPS files. "
    "Supported encodings are raw, ascii, hex, gzip, and bzip2.\n";
  char par3[] = "\t\t\t\t"
    "Much of the functionality of unu derives from chaining multiple "
    "invocations together with pipes (\"|\"), minimizing the "
    "need to save out intermediate files. For example, if "
    "\"data.raw.gz\" is a gzip'ed 256\tx\t256\tx\t80 volume of raw floats "
    "written from a PC, "
    "then the following will save to \"zsum.png\" a histogram "
    "equalized summation projection along the slowest axis:\n";
  char par4[] = "\tunu make -i data.raw.gz -t float -s 256 256 80 "
    "-e gzip -en little \\\n "
    "  | unu project -a 2 -m sum \\\n "
    "  | unu heq -b 2000 -s 1 \\\n "
    "  | unu quantize -b 8 -o zsum.png"
    "\n";
  char par5[] = "\t\t\t\t"
    "Nrrd and unu fill a frustrating gap in existing software for "
    "handling image and volume datasets, offering generality of type "
    "and dimension, minimal overhead for getting data in and out, "
    "with a file format and data structure that represents important "
    "meta-information: sample spacing, cell vs. node centering, " 
    "measurement units, key/value pairs, comments, etc.\n";
  char par6[] = "\t\t\t\t"
    "If unu or nrrd repeatedly proves itself useful for your research, an "
    "acknowledgment to that effect in your publication would be greatly "
    "appreciated, such as (for LaTeX): "
    "\"Dataset processing performed with the {\\tt unu} tool "
    "(or the {\\tt nrrd} library), "
    "part of the {\\tt Teem} toolkit available at "
    "{\\tt\t$<$http://teem.sourceforge.net$>$}\"\n ";
  /*
  char par8[] = "\t\t\t\t"
    "Feedback, questions, requests welcome: gk@bwh.harvard.edu.\n";
  */

  AIR_UNUSED(argc);
  AIR_UNUSED(argv);
  AIR_UNUSED(me);

  fprintf(stderr, "\n");
  sprintf(buff, "--- unu: Utah Nrrd Utilities command-line interface ---");
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  sprintf(buff, "(Teem version %s, %s)",
          airTeemVersion, airTeemReleaseDate);
  sprintf(fmt, "%%%ds\n",
          (int)((hparm->columns-strlen(buff))/2 + strlen(buff) - 1));
  fprintf(stderr, fmt, buff);
  fprintf(stderr, "\n");

  _hestPrintStr(stderr, 1, 0, 78, par1, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par7, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par2, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par3, AIR_FALSE);
  _hestPrintStr(stderr, 2, 0, 78, par4, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par5, AIR_FALSE);
  _hestPrintStr(stderr, 1, 0, 78, par6, AIR_FALSE);
  /*
  _hestPrintStr(stderr, 1, 0, 78, par8, AIR_FALSE);
  */

  return 0;
}

UNRRDU_CMD(about, INFO);
