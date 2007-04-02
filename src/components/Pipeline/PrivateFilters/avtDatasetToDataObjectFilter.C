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
//                       avtDatasetToDataObjectFilter.C                      //
// ************************************************************************* //

#include <avtDatasetToDataObjectFilter.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToDataObjectFilter::avtDatasetToDataObjectFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtDatasetToDataObjectFilter::~avtDatasetToDataObjectFilter()
{
    ;
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::InputSetActiveVariable
//
//  Purpose:
//      Sets the active variable in the input.
//
//  Arguments:
//      varname    The name of the new active variable.
//
//  Notes:     The method name contains "input" to prevent name collisions in
//             multiply inheriting derived types.  The compilers should be
//             able to handle this when given a scope resolution operator, but
//             I don't trust them.
//
//  Programmer: Hank Childs
//  Creation:   October 24, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Feb 20 15:14:32 PST 2004
//    Tell the data attributes what the new active variable is.
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::InputSetActiveVariable(const char *varname)
{
    SetActiveVariableArgs args;
    args.varname = varname;
    avtDataTree_p tree = GetInputDataTree();
    bool success;
    tree->Traverse(CSetActiveVariable, (void *) &args, success);

    //
    // Set up our data members for derived types that need this information.
    //
    activeVariableIsPointData = args.activeIsPoint;
    hasPointVars              = args.hasPointVars;
    hasCellVars               = args.hasCellVars;

    GetInput()->GetInfo().GetAttributes().SetActiveVariable(varname);
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::SearchDataForDataExtents
//
//  Purpose:
//      Searches the dataset for the current variable's extents.
//
//  Programmer: Hank Childs
//  Creation:   April 17, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Dec  3 14:28:02 PST 2004
//    Add support for variable names.
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::SearchDataForDataExtents(double *extents,
                                                       const char *varname)
{
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetDataExtents(input, extents, varname);
}


// ****************************************************************************
//  Method:  avtDatasetToDataObjectFilter::PreExecute
//
//  Purpose:
//    Called before main filter execution.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    February 15, 2007
//
// ****************************************************************************
void
avtDatasetToDataObjectFilter::PreExecute(void)
{
    avtFilter::PreExecute();

    // Here we check if the filter we are about to execute can
    // understand rectilinear grids with implied transforms set
    // in the data attributes.  If the filter can't, then we need
    // to apply the transform to the rectilinear grid, converting
    // it into a curvilinear grid.
    avtDataAttributes &inatts = GetInput()->GetInfo().GetAttributes();
    if (inatts.GetRectilinearGridHasTransform() &&
        !FilterUnderstandsTransformedRectMesh())
    {
        avtDataTree_p tree = GetInputDataTree();
        bool dummy;
        tree->Traverse(CApplyTransformToRectGrid,
                       (void*)inatts.GetRectilinearGridTransform(), dummy);

        // since we transformed the input, we need to change the input
        // data attributes
        inatts.SetRectilinearGridHasTransform(false);

        // ... and if we already copied the input data atts to the output,
        // we need to also update the output data attributes
        avtDataAttributes &outatts = GetOutput()->GetInfo().GetAttributes();
        outatts.SetRectilinearGridHasTransform(false);
    }
}
