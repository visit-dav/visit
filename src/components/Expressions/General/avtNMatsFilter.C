/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

// ************************************************************************* //
//                               avtNMatsFilter.C                            //
// ************************************************************************* //

#include <avtNMatsFilter.h>

#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>

#include <avtMaterial.h>
#include <avtMetaData.h>

#include <DebugStream.h>


// ****************************************************************************
//  Method: avtNMatsFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNMatsFilter::avtNMatsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNMatsFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtNMatsFilter::~avtNMatsFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtNMatsFilter::DeriveVariable
//
//  Purpose:
//      Determines the number of materials in a cell.
//
//  Arguments:
//      inDS      The input dataset.
//
//  Returns:      The derived variable.  The calling class must free this
//                memory.
//
//  Programmer:   Hank Childs
//  Creation:     August 21, 2003
//
//  Modifications:
//
//    Sean Ahern, Thu Aug 21 12:17:27 PDT 2003
//    Fixed up a small error and a warning.
//
//    Hank Childs, Wed Feb 11 11:23:28 PST 2004
//    Fix bug with calculation of mixed zones.  Also operate on zones where
//    the connectivity has changed.
//
//    Kathleen Bonnell, Mon Jun 28 07:48:55 PDT 2004
//    Send currentTimeState to GetMaterial. 
//
// ****************************************************************************

vtkDataArray *
avtNMatsFilter::DeriveVariable(vtkDataSet *in_ds)
{
    int    i;

    int ncells = in_ds->GetNumberOfCells();

    //
    // The 'currentDomainsIndex' is a data member of the base class that is
    // set to be the id of the current domain right before DeriveVariable is
    // called.  We need that index to make sure we are getting the right mat.
    //
    // The 'currentTimeState' is a data member of the base class that is
    // set to be the current timestep during ExamineSpecification. 
    // We need that timestep to make sure we are getting the right mat.
    //
 
    avtMaterial *mat = GetMetaData()->GetMaterial(currentDomainsIndex,
                                                  currentTimeState);
    vtkFloatArray *rv = vtkFloatArray::New();
    rv->SetNumberOfTuples(ncells);

    vtkDataArray *zn =in_ds->GetCellData()->GetArray("avtOriginalCellNumbers");
    if (zn == NULL)
    {
        debug1 << "Unable to find original cell numbers (needed for nmats)."
               << endl;
        return NULL;
    }
    int *ptr = (int *) zn->GetVoidPointer(0);
    int entry_size = zn->GetNumberOfComponents();
    int offset = entry_size-1;

    //
    // Walk through the material data structure and determine the number of
    // materials for each cell.
    //
    const int *matlist = mat->GetMatlist();
    const int *mix_next = mat->GetMixNext();
    for (i = 0 ; i < ncells ; i++)
    {
        int nmats = 0;
        bool shouldSkip = false;
        if (entry_size == 2)
        {
            if (ptr[entry_size*i + 0] != currentDomainsIndex)
            {
                nmats = 1;
                shouldSkip = true;
            }
        }
        if (!shouldSkip)
        {
            int zone = ptr[entry_size*i + offset];
            if (matlist[zone] >= 0)
            {
                nmats = 1;
            }
            else
            {
                int current = -matlist[zone]-1;
                nmats = 1;
                // nmats < 1000 just to prevent infinite loops if someone
                // set this structure up wrong.
                while ((mix_next[current] != 0) && (nmats < 1000))
                {
                    current = mix_next[current]-1;
                    nmats++;
                }
            }
        }
        rv->SetTuple1(i, (float) nmats);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtNMatsFilter::PerformRestriction
//
//  Purpose:
//      State that we need the zone numbers.
//
//  Programmer: Hank Childs
//  Creation:   February 11, 2004
//
// ****************************************************************************

avtPipelineSpecification_p
avtNMatsFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    avtPipelineSpecification_p rv = 
                      avtSingleInputExpressionFilter::PerformRestriction(spec);

    avtDataSpecification_p ds = spec->GetDataSpecification();
    ds->TurnZoneNumbersOn();

    return rv;
}


