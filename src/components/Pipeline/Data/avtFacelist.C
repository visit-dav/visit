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

// ************************************************************************* //
//                                avtFacelist.C                              //
// ************************************************************************* //

#ifdef PARALLEL
#include <mpi.h>
#endif

#include <vtkCellData.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkUnstructuredGrid.h>
#include <vtkUnsignedIntArray.h>

#include <avtFacelist.h>
#include <avtParallel.h>

#include <BadDomainException.h>
#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>


using     std::vector;


// ****************************************************************************
//  Method: avtFacelist constructor
//
//  Arguments:
//      nl       A nodelist.
//      lnl      The length of nl.
//      ns       The number of shapes.
//      sc       The amount of each shape (array of size ns).
//      ss       The number of nodes for each shape (array of size ss).
//      zo       The zone numbers of each face.
//      origin   An origin for the nodes.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Thu Aug  9 17:55:51 PDT 2001
//    Change nodelist to type vtkIdType from int to match VTK 4.0 API for
//    insertion of cells.
//
//    Hank Childs, Fri May 17 13:25:05 PDT 2002
//    Added some debugging statements when an exception is thrown.
//
//    Hank Childs, Mon Jun 17 09:55:29 PDT 2002
//    Added an argument for the origin.
//
//    Hank Childs (for Matthew Wheeler and Paul Selby),
//                                                 Mon Apr  7 14:10:23 PDT 2003
//    Reflected origin in the zoneno array.
//
// ****************************************************************************

avtFacelist::avtFacelist(int *nl, int lnl, int ns, int *sc, int *ss, int *zo,
                         int origin)
{
    int   i;

    if (nl == NULL || lnl <= 0 || ns <= 0 || sc == NULL || ss == NULL)
    {
        debug1 << "Not enough information to make a facelist:" << endl;
        debug1 << "Nodelist = " << nl << endl;
        debug1 << "Length = " << lnl << endl;
        debug1 << "# of Shapes = " << ns << endl;
        debug1 << "Shape count = " << sc << endl;
        debug1 << "Shape size = " << ss << endl;
        EXCEPTION0(ImproperUseException);
    }

    lnodelist = lnl;
    nodelist  = new vtkIdType[lnodelist];
    for (i = 0 ; i < lnodelist ; i++)
    {
        nodelist[i] = nl[i] - origin;
    }

    nshapes   = ns;
    nfaces    = 0;
    shapecnt  = new int[nshapes];
    shapesize = new int[nshapes];
    for (i = 0 ; i < nshapes ; i++)
    {
        shapecnt[i]  = sc[i];
        shapesize[i] = ss[i];
        nfaces += shapecnt[i];
    }
    
    //
    // The zoneno array for Silo facelists are not always populated.
    //
    if (zo != NULL)
    {
        zones = new int[nfaces];
        for (i = 0 ; i < nfaces ; i++)
        {
            zones[i] = zo[i] - origin;
        }
    }
    else
    {
        zones = NULL;
    }
}


// ****************************************************************************
//  Method: avtFacelist destructor
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

avtFacelist::~avtFacelist()
{
    if (nodelist != NULL)
    {
        delete [] nodelist;
        nodelist = NULL;
    }
    if (shapecnt != NULL)
    {
        delete [] shapecnt;
        shapecnt = NULL;
    }
    if (shapesize != NULL)
    {
        delete [] shapesize;
        shapesize = NULL;
    }
    if (zones != NULL)
    {
        delete [] zones;
        zones = NULL;
    }
}


// ****************************************************************************
//  Method: avtFacelist::Destruct
//
//  Purpose:
//      A routine that a void_ref_ptr can call to cleanly destruct a facelist.
//
//  Programmer: Hank Childs
//  Creation:   September 25, 2002
//
// ****************************************************************************

void
avtFacelist::Destruct(void *p)
{
    avtFacelist *fl = (avtFacelist *) p;
    delete fl;
}


// ****************************************************************************
//  Method: avtFacelist::CalcFacelist
//
//  Purpose:
//      Calculates the facelist from the input unstructured grid and places
//      it in the output polydata.
//
//  Arguments:
//      ugrid   The input unstructured grid (that the fields correspond to).
//      pdata   The output poly data (the facelist).
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//    Kathleen Bonnell, Tue Mar 27 08:59:23 PST 2001
//    Added extra check for presence of cell data, because Mesh plots
//    may not pass around data.
//
//    Kathleen Bonnell, Tue Mar 27 08:59:23 PST 2001
//    Added code to copy over original zones array if present in input. 
//
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray 
//    and vtkFloatArray instead.
//
//    Kathleen Bonnell, Mon May 20 13:40:17 PDT 2002 
//    avtOriginalCellNumbers now has 2 components. 
//
//    Hank Childs, Thu Jun 20 10:57:09 PDT 2002
//    Made use of new routine CalcFacelistFromPoints to generate the actual
//    cells.
//
//    Jeremy Meredith, Tue Jul  9 12:40:29 PDT 2002
//    Corrected a copy and paste typo when copying original cells.
//    Added code to copy the avt3DCellNumbers array as well.
//
//    Brad Whitlock, Mon Jul 15 16:22:19 PST 2002
//    I made it work on Windows again.
//
//    Jeremy Meredith, Sat Aug  3 13:50:08 PDT 2002
//    There is a possibility now that avtOriginalCellNumbers could be set
//    without avt3DCellNumbers being set, so I allowed that case.
//
//    Hank Chlids, Sun Aug  4 18:03:00 PDT 2002
//    Copy over all of the cell variables.
//
//    Hank Childs, Tue Jun  3 15:07:11 PDT 2003
//    Check to make sure that the zones are valid before copying over the data.
//
// ****************************************************************************

void
avtFacelist::CalcFacelist(vtkUnstructuredGrid *ugrid, vtkPolyData *pdata)
{
    //
    // A subroutine will calculate the polygons that actually go with this
    // facelist.
    //
    CalcFacelistFromPoints(ugrid->GetPoints(), pdata);
 
    //
    // The point data is easy -- we have the same points, so we have the same
    // point data.
    //
    pdata->GetPointData()->PassData(ugrid->GetPointData());
 
    //
    // We need to have the zone list to be able to copy over the cell data
    // properly.  Presumably, this routine is never called if there is cell
    // data and the zone list is NULL (because the caller can check with a
    // member function and decide not to use it).
    //
    vtkCellData *inCD = ugrid->GetCellData();
    vtkCellData *outCD = pdata->GetCellData();
    bool shouldCopyCellData = (inCD->GetNumberOfArrays() > 0);
    if (zones != NULL && shouldCopyCellData)
    {
        outCD->CopyAllocate(inCD);
        int nInZones = ugrid->GetNumberOfCells();
        for (int i = 0 ; i < nfaces ; i++)
        {
            if (zones[i] >= nInZones)
            {
                EXCEPTION2(BadIndexException, zones[i], nInZones);
            }
            outCD->CopyData(inCD, zones[i], i);
        }
        outCD->Squeeze();
    }
}


// ****************************************************************************
//  Method: avtFacelist::CalcFacelistFromPoints
//
//  Purpose:
//      Calculates a facelist based strictly on points (no variables are copied
//      over).
//
//  Arguments:
//      pts     The points that the facelist corresponds to.
//      pdata   The poly data object that will be returned.  This should be
//              already allocated.
//
//  Programmer: Hank Childs
//  Creation:   June 20, 2002
//
// ****************************************************************************

void
avtFacelist::CalcFacelistFromPoints(vtkPoints *pts, vtkPolyData *pdata)
{
    int   i, j;

    pdata->SetPoints(pts);
    int  size = 0;
    for (i = 0 ; i < nshapes ; i++)
    {
        size += shapecnt[i];
    }
    pdata->Allocate(size);

    int  inodelist = 0;

    for (i = 0 ; i < nshapes ; i++)
    {
        int  numnodes = shapesize[i];
        int  vtktype = -1;
        switch (numnodes)
        {
          case 2:
            vtktype = VTK_LINE;
            break;
          case 3:
            vtktype = VTK_TRIANGLE;
            break;
          case 4:
            vtktype = VTK_QUAD;
            break;
          default:
            debug1 << "Cannot handle type " << numnodes << endl;
            break;
        }
        if (vtktype == -1)
        {
            continue;
        }
        for (j = 0 ; j < shapecnt[i] ; j++)
        {
            pdata->InsertNextCell(vtktype, numnodes, nodelist + inodelist);
            inodelist += numnodes;
            if (inodelist > lnodelist)
            {
                EXCEPTION0(ImproperUseException);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtMultiFacelist constructor
//
//  Arguments:
//      nd      The number of domains the multi-facelist will have.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

avtMultiFacelist::avtMultiFacelist(int nd)
{
    int  i;

    numDomains = nd;
    fl = new avtFacelist*[numDomains];
    for (i = 0 ; i < numDomains ; i++)
    {
        fl[i] = NULL;
    }
    validFL = new bool[numDomains];
    for (i = 0 ; i < numDomains ; i++)
    {
        validFL[i] = false;
    }
}


// ****************************************************************************
//  Method: avtMultiFacelist destructor
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

avtMultiFacelist::~avtMultiFacelist()
{
    //
    // We don't own the facelists, just the array that holds the pointers.
    //
    if (fl != NULL)
    {
        delete [] fl;
    }
    if (validFL != NULL)
    {
        delete [] validFL;
    }
}


// ****************************************************************************
//  Method: avtMultiFacelist::Valid
//
//  Purpose:
//      Determines if the facelists for its domains are valid.  It is not
//      always straight forward to determine this when reading in the domains
//      since interior domains will not have facelists.  Many facelists will
//      be invalid, though, because they did not come from Silo files or
//      because they come from rectilinear or curvilinear meshes.
//
//  Returns:    true if the multi-facelist is valid, false otherwise.
//      
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

bool
avtMultiFacelist::Valid(void)
{
    for (int i = 0 ; i < numDomains ; i++)
    {
        if (validFL[i])
        {
            return true;
        }
    }

    return false;
}


// ****************************************************************************
//  Method: avtMultiFacelist::GetDomainsList
//
//  Purpose:
//      Gets the list of domains that have faces in the face list.
//
//  Programmer:  Hank Childs
//  Creation:    October 27, 2000
//
// ****************************************************************************

void
avtMultiFacelist::GetDomainsList(vector<int> &domains)
{
    domains.clear();
    for (int i = 0 ; i < numDomains ; i++)
    {
        if (validFL[i])
        {
            domains.push_back(i);
        }
    }
}


// ****************************************************************************
//  Method: avtMultiFacelist::SetDomain
//
//  Purpose:
//      Sets a domain with a facelist.
//
//  Arguments:
//      f       The new facelist.
//      dom     The domain of f.
//
//  Note:       The multi-facelist assumes it owns the facelist being added and
//              will destruct it when being deleted.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

void
avtMultiFacelist::SetDomain(avtFacelist *f, int dom)
{
    if (dom < 0 || dom >= numDomains)
    {
        EXCEPTION2(BadDomainException, dom, numDomains);
    }

    fl[dom] = f;
    if (fl[dom] != NULL)
    {
        validFL[dom] = true;
    }
}


// ****************************************************************************
//  Method: avtMultiFacelist::CalcFacelist
//
//  Purpose:
//      Finds the proper facelist and has it calculate the facelist.
//
//  Arguments:
//      ug         The unstructured grid that our facelist corresponds to.
//      d          The domain for in_ug.
//      pd         The poly data output (the facelist).
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
// ****************************************************************************

void
avtMultiFacelist::CalcFacelist(vtkUnstructuredGrid *ug, int d, vtkPolyData *pd)
{
    if (d < 0 || d >= numDomains)
    {
        EXCEPTION2(BadDomainException, d, numDomains);
    }

    fl[d]->CalcFacelist(ug, pd);
}


// ****************************************************************************
//  Method: avtMultiFacelist::Calculate
//
//  Purpose:
//      Calculates the valid domains across all processors.  This should do
//      nothing in serial.
//
//  Programmer: Hank Childs
//  Creation:   October 27, 2000
//
//  Modifications:
//
//    Mark C. Miller, Mon Jan 22 22:09:01 PST 2007
//    Changed MPI_COMM_WORLD to VISIT_MPI_COMM
// ****************************************************************************

void
avtMultiFacelist::Calculate(void)
{
#ifdef PARALLEL
    //
    // Get all of the domains that are valid on each processor.
    //
    int  *validFLInt    = new int[numDomains];
    int  *validFLIntOut = new int[numDomains];
    int   i;

    for (i = 0 ; i < numDomains ; i++)
    {
        validFLInt[i] = (validFL[i] ? 1 : 0);
    }

    MPI_Allreduce(validFLInt, validFLIntOut, numDomains, MPI_INT, MPI_MAX,
                  VISIT_MPI_COMM);

    for (i = 0 ; i < numDomains ; i++)
    {
        validFL[i] = (validFLIntOut[i] == 1 ? true : false);
    }

    delete [] validFLInt;
    delete [] validFLIntOut;
#endif
}


