/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                       avtDatasetToDataObjectFilter.C                      //
// ************************************************************************* //

#include <avtDatasetToDataObjectFilter.h>

#include <avtCommonDataFunctions.h>
#include <avtDatasetExaminer.h>
#include <avtOriginatingSource.h>

#include <DebugStream.h>


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
//  Method: avtDatasetToDataObjectFilter::SearchDataForSpatialExtents
//
//  Purpose:
//      Searches the dataset for the spatial extents.
//
//  Programmer: Hank Childs
//  Creation:   November 30, 2010
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::SearchDataForSpatialExtents(double *extents)
{
    avtDataset_p input = GetTypedInput();
    avtDatasetExaminer::GetSpatialExtents(input, extents);
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
//  Modifications:
//    Kathleen Biagas, Thu Apr 26 13:56:47 PDT 2012
//    Added debug statement to aid in debugging new filters that inadvertently
//    trigger the transformation.
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
        debug3 << "avtDatasetToDataObjectFilter transforming rectilinear grid"
               << " to curvilinear grid." << endl;
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


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::FetchArbitraryVTKObject
//
//  Purpose:
//      Checks to see if a vtkObject was cached in the pipeline and fetches
//      it if so.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2010
//
// ****************************************************************************

vtkObject *
avtDatasetToDataObjectFilter::FetchArbitraryVTKObject(int dependencies,
                                             const char *name, int dom, int ts,
                                             const char *type)
{
    // If we can't cache it, then there's something in the pipeline that
    // subset the cells, transforms the data, etc.
    // So, even if there is something in the cache, we can't use it ... it
    // might be appropriate for a different pipeline, but not ours.
    bool canUse = CheckDependencies(dependencies);
    if (! canUse)
        return NULL;

    avtOriginatingSource *source = GetInput()->GetOriginatingSource();
    return source->FetchArbitraryVTKObject(name, dom, ts, type);
}


// ****************************************************************************
//  Method: avtDatasetToDataObjectFilter::StoreArbitraryVTKObject
//
//  Purpose:
//      Stores a ref_ptr in the database cache.
//
//  Programmer: Hank Childs
//  Creation:   November 28, 2010
//
//  Modifications:
//
//    David Camp, Mon Jan 24 11:53:02 PST 2011
//    Remove cerr debug message and add if (DebugStream::Level5()).
//
// ****************************************************************************

void
avtDatasetToDataObjectFilter::StoreArbitraryVTKObject(int dependencies,
                                           const char *name, int dom, int ts, 
                                           const char *type, vtkObject *obj)
{
    bool canUse = CheckDependencies(dependencies);
    if (! canUse)
    {
        if (DebugStream::Level5())
        {
            debug5 << "Cannot cache " << type << " for " << name << " because "
               << "dependencies were not satisfied.\n"
               << "This is an informational message, not an error." << endl;
        }
        return;
    }

    avtOriginatingSource *source = GetInput()->GetOriginatingSource();
    source->StoreArbitraryVTKObject(name, dom, ts, type, obj);
}

