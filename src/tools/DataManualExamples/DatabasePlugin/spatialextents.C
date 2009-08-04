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

// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <avtIntervalTree.h>

// STMD version of GetAuxiliaryData.
void *
avtXXXXFileFormat::GetAuxiliaryData(const char *var,
    int domain, const char *type, void *,
    DestructorFunction &df)
{
    void *retval = 0;

    if(strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        // Read the number of domains for the mesh.
        int ndoms = READ NUMBER OF DOMAINS FROM FILE;

        // Read the spatial extents for each domain of the
        // mesh. This information should be in a single
        // and should be available without having to 
        // read the real data. The expected format for
        // the data in the spatialextents array is to
        // repeat the following pattern for each domain:
        // xmin, xmax, ymin, ymax, zmin, zmax.
        double *spatialextents = new double[ndoms * 6];
        READ ndoms*6 DOUBLE VALUES INTO spatialextents ARRAY.

        // Create an interval tree
        avtIntervalTree *itree = new avtIntervalTree(ndoms, 3);
        double *extents = spatialextents;
        for(int dom = 0; dom < ndoms; ++dom)
        {
            itree->AddElement(dom, extents);
            extents += 6;
        }
        itree->Calculate(true);

        // Delete temporary array.
        delete [] spatialextents;

        // Set return values
        retval = (void *)itree;
        df = avtIntervalTree::Destruct;
    }

    return retval;
}
