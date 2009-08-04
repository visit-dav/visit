/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

static char visItWriterInitDoc[] = 
"This module writes out files that are accessible to VisIt.\n"
"\n"
"It contains 5 methods:\n"
"\n"
"WritePointMesh(fileName, useBinary, points, vars )\n"
"WriteUnstructuredMesh(fileName, useBinary, points, connectivity, vars )\n"
"WriteRegularMesh(fileName, useBinary, dimensions, vars )\n"
"WriteRectilinearMesh(fileName, useBinary, x, y, z, vars )\n"
"WriteCurvilinearMesh(fileName, useBinary, dimensions, points, vars )\n"
"\n"
"In the following:\n"
"  1) an array can be either a python list or a python tuple.\n"
"  2) a number is either a float or an integer.\n"
"     For example [1, 3.14, 5, 6, 7.77] is an array of numbers.\n"
"\n"
"The arguments for the methods are defined as:\n"
"    fileName:     a string containing the name of the visIt file to create\n"
"\n"
"    useBinary:    an integer (0 produces ASCII output, all else produce\n"
"                              Binary output)\n"
"\n"
"    points:       a number array containing the points data of\n"
"                  length 3 * numberOfPoints\n"
"                    (i.e. [ x1, y1, z1, x2, y2, z2, x3, ..., xn, yn, zn ])\n"
"\n"
"    vars:         an array of var datum.\n"
"\n"
"    var datum: an array of [ name, dimension, centering, arrayOfValues ]\n"
"    name:          a string\n"
"    dimension:     an integer; either 1 for scalar or 3 for vector\n"
"    centering:     an integer; (0 is cell-wise, all else is point-wise)\n"
"    arrayOfValues: an array of numbers of length dimension * numberOfPoints\n"
"\n"
"    connectivity:  an array of connectivity datum.\n"
"    connectivity datum: array of [ ConnectivityObject, index1,index2,..indexn ]\n"
"    index:          an integer\n"
"\n"
"    ConnectivityObject: an integer or a string\n"
"\n"
"    Possible values for ConnectivityObject and number of indices required for\n"
"    each are:\n"
"         integer             | string        , number of indices\n"
"    -------------------------------------------------------------\n"
"    visit_writer.vertex      | 'vertex'      ,         1\n"
"    visit_writer.line        | 'line'        ,         2\n"
"    visit_writer.triangle    | 'triangle'    ,         3\n"
"    visit_writer.quad        | 'quad'        ,         4\n"
"    visit_writer.tetrahedron | 'tetrahedron' ,         4\n"
"    visit_writer.hexahedron  | 'hexahedron'  ,         8\n"
"    visit_writer.wedge       | 'wedge'       ,         6\n"
"    visit_writer.pyramid     | 'pyramid'     ,         5\n"
"\n"
"    dimensions:     an array of the 3 integers [nx, ny, nz];\n"
"                    numberOfPoints = nx * ny * nz\n"
"    x, y or z:      an array of numbers (i.e., [c1, c2, c3, c4, ... cn]\n"
"                                         where ci is either x, y or z)";

