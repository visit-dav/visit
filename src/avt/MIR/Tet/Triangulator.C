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

#include "Triangulator.h"
#include <vector>
#include <vtkCell.h>
#include <VisItException.h>
using std::vector;


// ****************************************************************************
//  Constructor:  Triangulator::Triangulator
//
//  Programmer:  Jeremy Meredith
//  Creation:    August 30, 2002
//
// ****************************************************************************
Triangulator::Triangulator(int nmat_)
{
    nmat = nmat_;
    for (int i=0; i<MAX_TRIS_PER_CELL; i++)
        tri[i] = MaterialTriangle(nmat);
}

// ****************************************************************************
//  Method:  Triangulator::calc_poly_low
//
//  Purpose:
//    Calculate the low-subdivision triangulation for a polygon.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattri's get re-initialized
//    every time.  This lets us re-use the Triangulator across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output triangles.
//
// ****************************************************************************
void
Triangulator::calc_poly_low()
{
    ntri = npts-2;

    for (int t=0; t<ntri; t++)
    {
        tri[t].reset();

        int p0 = t+1;
        int p1 = t+2;

        if (celltype == VTK_PIXEL)
        {
            static int clockwise_pixel_numbering[4] = {0,1,3,2};
            p0 = clockwise_pixel_numbering[p0];
            p1 = clockwise_pixel_numbering[p1];
        }

        tri[t].node[0].origindex = c_ptr[0];
        tri[t].node[1].origindex = c_ptr[p0];
        tri[t].node[2].origindex = c_ptr[p1];

        tri[t].node[0].weight[0]  = 1.0;
        tri[t].node[1].weight[p0] = 1.0;
        tri[t].node[2].weight[p1] = 1.0;

        for (int m=0; m<nmat; m++)
        {
            tri[t].node[0].matvf[m] = vf_node[0][m];
            tri[t].node[1].matvf[m] = vf_node[p0][m];
            tri[t].node[2].matvf[m] = vf_node[p1][m];
        }
    }
}

// ****************************************************************************
//  Method:  Triangulator::calc_poly_med
//
//  Purpose:
//    Calculate the med-subdivision triangulation for a polygon.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattri's get re-initialized
//    every time.  This lets us re-use the Triangulator across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output triangles.
//
// ****************************************************************************
void
Triangulator::calc_poly_med()
{
    ntri = npts;

    // for every triangle, the first node is the zone center
    int t;
    for (t=0; t<ntri; t++)
    {
        tri[t].reset();

        for (int w=0; w<ntri; w++)
            tri[t].node[0].weight[w] = 1.0/float(ntri);
        for (int m=0; m<nmat; m++)
            tri[t].node[0].matvf[m]  = vf_zone[m];
    }

    // do the 2nd and 3rd nodes for each triangle
    for (t=0; t<ntri; t++)
    {
        int p0 = (t+0);
        int p1 = (t+1)%ntri;

        if (celltype == VTK_PIXEL)
        {
            static int clockwise_pixel_numbering[4] = {0,1,3,2};
            p0 = clockwise_pixel_numbering[p0];
            p1 = clockwise_pixel_numbering[p1];
        }

        tri[t].node[1].origindex = c_ptr[p0];
        tri[t].node[2].origindex = c_ptr[p1];

        tri[t].node[1].weight[p0] = 1.0;
        tri[t].node[2].weight[p1] = 1.0;

        for (int m=0; m<nmat; m++)
        {
            tri[t].node[1].matvf[m] = vf_node[p0][m];
            tri[t].node[2].matvf[m] = vf_node[p1][m];
        }
    }
}

// ****************************************************************************
//  Method:  Triangulator::calc_poly_high
//
//  Purpose:
//    Calculate the high-subdivision triangulation for a polygon.
//
//  Arguments:
//    
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 12:01:19 PDT 2001
//    Added 'origindex' so we can avoid using the weights when possible.
//
//    Jeremy Meredith, Tue Oct 16 09:05:20 PDT 2001
//    Added a clear() before the resize so the mattri's get re-initialized
//    every time.  This lets us re-use the Triangulator across cells.
//
//    Jeremy Meredith, Fri Aug 30 17:03:45 PDT 2002
//    Removed resizing of vector, and instead reset the output triangles.
//
// ****************************************************************************
void
Triangulator::calc_poly_high()
{
    ntri = npts*2;

    // for every triangle, the first node is the zone center
    for (int t=0; t<ntri; t++)
    {
        tri[t].reset();

        for (int w=0; w<npts; w++)
            tri[t].node[0].weight[w] = 1.0/float(npts);
        for (int m=0; m<nmat; m++)
            tri[t].node[0].matvf[m]  = vf_zone[m];
    }

    // do the 2nd and 3rd nodes for each triangle
    for (int p=0; p<npts; p++)
    {
        int t1 = p*2;
        int t2 = p*2+1;

        int p0 = p;
        int p1 = (p+1)%npts;

        if (celltype == VTK_PIXEL)
        {
            static int clockwise_pixel_numbering[4] = {0,1,3,2};
            p0 = clockwise_pixel_numbering[p0];
            p1 = clockwise_pixel_numbering[p1];
        }

        tri[t1].node[1].origindex = c_ptr[p0];

        tri[t1].node[1].weight[p0] = 1.0;
        tri[t1].node[2].weight[p0] = 0.5;
        tri[t1].node[2].weight[p1] = 0.5;
        int m;
        for (m=0; m<nmat; m++)
        {
            tri[t1].node[1].matvf[m] = vf_node[p0][m];
            tri[t1].node[2].matvf[m] = vf_edge[p][m];
        }

        tri[t2].node[2].origindex = c_ptr[p1];

        tri[t2].node[1].weight[p0] = 0.5;
        tri[t2].node[1].weight[p1] = 0.5;
        tri[t2].node[2].weight[p1] = 1.0;

        for (m=0; m<nmat; m++)
        {
            tri[t2].node[1].matvf[m] = vf_edge[p][m];
            tri[t2].node[2].matvf[m] = vf_node[p1][m];
        }
    }
}

// ****************************************************************************
//  Method:  Triangulator::Triangulate
//
//  Purpose:
//    Create the triangultion using the given parameters.
//
//  Arguments:
//    s          the subdivision level
//    c          the cell
//    vf_xxxx_   locally indexed VF arrays for zones/nodes/edges
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 11, 2000
//
//  Modifications::
//    Jeremy Meredith, Fri Aug 30 17:03:24 PDT 2002
//    Moved nmat into constructor.
//
//    Hank Childs, Fri Jan 28 15:43:04 PST 2005
//    Use expression macros.
//
// ****************************************************************************
void
Triangulator::Triangulate(MIROptions::SubdivisionLevel s, int ct, int np,
                          const int *cptr,
                          vector<float>   vf_zone_,
                          vector<float>  *vf_node_,
                          vector<float>  *vf_edge_)
{
    subdiv = s;
    celltype = ct;
    npts     = np;
    c_ptr    = cptr;

    vf_zone = vf_zone_;
    vf_node = vf_node_;
    vf_edge = vf_edge_;

    switch (subdiv)
    {
      case MIROptions::Low:
        calc_poly_low();
        break;
      case MIROptions::Med:
        calc_poly_med();
        break;
      case MIROptions::High:
        calc_poly_high();
        break;
      default:
        EXCEPTION0(VisItException);
    }

    // these are the basic types of 2d zones in case we need to split them:
    /*
    switch (subdiv)
    {
      case MIR::Low:
        switch (celltype)
        {
          case VTK_QUAD:       calc_poly_low;     break;
          case VTK_POLYGON:    calc_poly_low;     break;
          case VTK_PIXEL:      calc_pixel_low;    break;
          default:             EXCEPTION0(VisItException);
        }
        break;
      case MIR::Med:
        switch (celltype)
        {
          case VTK_QUAD:       calc_poly_med;     break;
          case VTK_POLYGON:    calc_poly_med;     break;
          case VTK_PIXEL:      calc_pixel_med;    break;
          default:             EXCEPTION0(VisItException);
        }
        break;
      case MIR::High:
        switch (celltype)
        {
          case VTK_QUAD:       calc_poly_high;    break;
          case VTK_POLYGON:    calc_poly_high;    break;
          case VTK_PIXEL:      calc_pixel_high;   break;
          default:             EXCEPTION0(VisItException);
        }
        break;
      default:
        EXCEPTION0(VisItException);
    }
    */
}
