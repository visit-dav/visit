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

// NOTE - This code incomplete and requires underlined portions
// to be replaced with code to read values from your file format.

#include <avtGhostData.h>
#include <vtkUnsignedCharArray.h>

vtkDataSet *
avtXXXXFileFormat::GetMesh(const char *meshname)
{
    // Code to create your mesh goes here.
    vtkDataSet *retval = CODE TO CREATE YOUR MESH;

    // Now that you have your mesh, figure out which cells need
    // to be removed.
    int nCells = retval->GetNumberOfCells();
    int *blanks = new int[nCells];
    READ nCells INTEGER VALUES INTO blanks ARRAY.

    // Now that we have the blanks array, create avtGhostZones.
    unsigned char realVal = 0, ghost = 0;
    avtGhostData::AddGhostZoneType(ghost, ZONE_NOT_APPLICABLE_TO_PROBLEM);
    vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
    ghostCells->SetName("avtGhostZones");
    ghostCells->Allocate(nCells);
    for(int i = 0; i < nCells; ++i)
    {
        if(blanks[i])
            ghostCells->InsertNextValue(realVal);
        else
            ghostCells->InsertNextValue(ghost);
    }
    retval->GetCellData()->AddArray(ghostCells);
    retval->SetUpdateGhostLevel(0);
    ghostCells->Delete();

    // Clean up
    delete [] blanks;

    return retval;
}
