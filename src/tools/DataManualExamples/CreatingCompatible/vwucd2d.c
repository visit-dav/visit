/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#include <visit_writer.h>

/*   
  *---*---*--*   nodes (5,6,7,8)
  |   |\5/ 3 |
  | 1 |4*----*   nodes (3,4)
  |   |/  2  |
  *---*------*   nodes(0,1,2)

  cell 1 quad(0,1,6,5)
  cell 2 quad(1,2,4,3)
  cell 3 quad(3,4,8,7)
  cell 4 tri(1,3,6)
  cell 5 tri(3,7,6)

*/

int
main(int argc, char *argv[])
{
    /* Node coordinates */
    int nnodes = 9;
    int nzones = 5;
    float pts[] = {0., 0., 0., 2., 0., 0., 5., 0., 0.,
        3., 3., 0., 5., 3., 0., 0., 5., 0., 
        2., 5., 0., 4., 5., 0., 5., 5., 0.};

    /* Zone types */
    int zonetypes[] = {VISIT_TRIANGLE, VISIT_TRIANGLE,
        VISIT_QUAD, VISIT_QUAD, VISIT_QUAD};

    /* Connectivity */
    int connectivity[] = {
        1,3,6,    /* tri zone 1. */
        3,7,6,    /* tri zone 2. */
        0.,1,6,5, /* quad zone 3. */
        1,2,4,3,  /* quad zone 4. */
        3,4,8,7   /* quad zone 5. */
    };

    /* Data arrays */
    float nodal[] = {1,2,3,4,5,6,7,8,9};
    float zonal[] = {1,2,3,4,5};

    /* Info about the variables we're passing to visit_writer. */
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float *vars[] = {zonal, nodal};

    /* Pass the mesh and data to visit_writer. */
    write_unstructured_mesh("vwucd2d.vtk", 1, nnodes, pts, nzones,
        zonetypes, connectivity, nvars, vardims, centering,
        varnames, vars);

    return 0;
}
