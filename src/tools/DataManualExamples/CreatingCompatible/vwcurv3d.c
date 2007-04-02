/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <visit_writer.h>

#define NX 4
#define NY 3
#define NZ 2

int
main(int argc, char *argv[])
{
    /* Curvilinear mesh points stored x0,y0,z0,x1,y1,z1,...*/
    float pts[] = {0, 0.5, 0, 1, 0, 0, 2, 0, 0,
        3, 0.5, 0, 0, 1, 0, 1, 1, 0,
        2, 1, 0, 3, 1, 0, 0, 1.5, 0,
        1, 2, 0, 2, 2, 0, 3, 1.5, 0, 
        0, 0.5, 1, 1, 0, 1, 2, 0, 1,
        3, 0.5, 1, 0, 1, 1, 1, 1, 1,
        2, 1, 1, 3, 1, 1, 0, 1.5, 1,
        1, 2, 1, 2, 2, 1, 3, 1.5, 1
    };
    int dims[] = {NX, NY, NZ};
    /* Zonal and nodal variable data. */
    float zonal[NZ-1][NY-1][NX-1], nodal[NZ][NY][NX];
    /* Info about the variables to pass to visit_writer. */
    int nvars = 2;
    int vardims[] = {1, 1};
    int centering[] = {0, 1};
    const char *varnames[] = {"zonal", "nodal"};
    float *vars[] = {(float *)zonal, (float *)nodal};
    int i,j,k, index = 0;

    /* Create zonal variable */
    for(k = 0; k < NZ-1; ++k)
        for(j = 0; j < NY-1; ++j)
            for(i = 0; i < NX-1; ++i, ++index)
                zonal[k][j][i] = (float)index;

    /* Create nodal variable. */
    index = 0;
    for(k = 0; k < NZ; ++k)
        for(j = 0; j < NY; ++j)
            for(i = 0; i < NX; ++i, ++index)
                nodal[k][j][i] = index;

    /* Pass the data to visit_writer to write a binary VTK file. */
    write_curvilinear_mesh("vwcurv3d.vtk", 1, dims, pts, nvars,
        vardims, centering, varnames, vars);

    return 0;
}
