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
//                         avtTerminatingDatasetSource.C                     //
// ************************************************************************* //

#include <avtTerminatingDatasetSource.h>

#include <float.h>

#include <vtkDataSet.h>
#include <vtkDataArray.h>
#include <vtkFieldData.h>

#include <avtCommonDataFunctions.h>
#include <avtDataAttributes.h>
#include <avtExtents.h>


// ****************************************************************************
//  Method: avtTerminatingDatasetSource constructor
//
//  Purpose:
//      Defines the constructor.  Note: this should not be inlined in the
//      header because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSource::avtTerminatingDatasetSource()
{
    ;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSource destructor
//
//  Purpose:
//      Defines the destructor.  Note: this should not be inlined in the header
//      because it causes problems for certain compilers.
//
//  Programmer: Hank Childs
//  Creation:   February 5, 2004
//
// ****************************************************************************

avtTerminatingDatasetSource::~avtTerminatingDatasetSource()
{
    ;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSource::FetchData
//
//  Purpose:
//      Defines FetchData, a method that is called when a terminating data
//      object does an Update.  This layer defines how the information obtained
//      (a data tree) should be associated with the data object.
//
//  Programmer: Hank Childs
//  Creation:   May 29, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Jul 17 10:10:25 PDT 2001
//    Better handling of an empty data specification.
//
//    Hank Childs, Tue Sep  4 15:25:38 PDT 2001
//    Swap out extent mechanism to formally handle the notion of cumulative
//    extents.
//
//    Hank Childs, Fri Sep  7 17:58:58 PDT 2001
//    Used doubles instead of floats when getting VTK extents.
//
//    Hank Childs, Thu Oct 18 13:32:04 PDT 2001
//    Since VTK assumes vectors are 3D, allocate enough memory to make sure
//    it does do an array bounds write.
//
//    Hank Childs, Thu Oct 18 16:17:22 PDT 2001
//    Perform a dataset verification phase.
//
//    Hank Childs, Tue Nov 13 12:05:57 PST 2001
//    Only perform dataset verification if this is a "real" terminating source.
//
//    Hank Childs, Thu Feb  7 11:51:46 PST 2002
//    Still execute even if we have an empty SIL.  This will allow databases
//    to do collective communication routines.
//
//    Hank Childs, Fri Jan  9 10:10:05 PST 2004
//    Change the arugments to the dataset verifier.
//
//    Hank Childs, Wed Feb 25 15:40:49 PST 2004
//    No longer determine extents, since that is done when the dataset is
//    populated (through MergeExtents).
//
// ****************************************************************************

bool
avtTerminatingDatasetSource::FetchData(avtDataSpecification_p spec)
{
    bool rv = false;
    rv = FetchDataset(spec, GetDataTree());
    avtDataTree_p tree = GetDataTree();

    if (!ArtificialPipeline())
    {
        int nleaves = 0;
        vtkDataSet **ds = tree->GetAllLeaves(nleaves);
        vector<int> domains;
        tree->GetAllDomainIds(domains);
        verifier.VerifyDatasets(nleaves, ds, domains);
        delete [] ds;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtTerminatingDatasetSource::MergeExtents
//
//  Purpose:
//      Merges extents into the output.  This is used by databases that
//      do subselection before handing back the data tree (like a material
//      select) and want to give extents.
//
//  Arguments:
//      ds      The VTK dataset to merge into the extents.
//
//  Programmer: Hank Childs
//  Creation:   August 14, 2001
//
//  Modifications:
//
//    Hank Childs, Fri Sep  7 17:58:58 PDT 2001
//    Better support for vector variables.
//
//    Hank Childs, Thu Oct 11 13:11:26 PDT 2001
//    Allow NULL input.
//
//    Hank Childs, Thu Oct 18 13:32:04 PDT 2001
//    Since VTK assumes vectors are 3D, allocate enough memory to make sure
//    it does do an array bounds write.
//
//    Hank Childs, Tue Jul 29 16:33:57 PDT 2003
//    Make use of cached bounds.
//
//    Hank Childs, Tue Feb 24 14:38:47 PST 2004
//    Account for multiple variables.
//
//    Kathleen Bonnell, Thu Mar 11 10:07:35 PST 2004 
//    DataExtents now always have only 2 components. 
//
//    Mark C. Miller, Tue Oct 19 14:08:56 PDT 2004
//    Added check for existence of true spatial extents before attempting to
//    merge cummulative
//
//    Hank Childs, Fri Mar 11 08:16:49 PST 2005
//    Initialize 6 values for vectors, even though we only use 2, since that
//    fixes a UMR in avtExtents, where it copies 6 values due to legacy reasons
//    (but still only uses 2).
//
//    Hank Childs, Wed Oct 10 16:12:11 PDT 2007
//    Only look at non-ghost data when calculating extents.
//
//    Jeremy Meredith, Thu Feb  7 17:57:33 EST 2008
//    Added support for initializing individual component data ranges for
//    array variables.
//
// ****************************************************************************

void
avtTerminatingDatasetSource::MergeExtents(vtkDataSet *ds)
{
    if (ds == NULL)
    {
        return;
    }

    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();

    if (atts.GetTrueSpatialExtents()->HasExtents() == false)
    {
        double bounds[6];
        if (ds->GetFieldData()->GetArray("avtOriginalBounds") != NULL)
        {
            vtkDataArray *arr = ds->GetFieldData()->GetArray("avtOriginalBounds");
            for (int i = 0 ; i < 6 ; i++)
                bounds[i] = arr->GetTuple1(i);
        }
        else
        {
            ds->GetBounds(bounds);
        }

        double dbounds[6];
        dbounds[0] = bounds[0];
        dbounds[1] = bounds[1];
        dbounds[2] = bounds[2];
        dbounds[3] = bounds[3];
        dbounds[4] = bounds[4];
        dbounds[5] = bounds[5];
        atts.GetCumulativeTrueSpatialExtents()->Merge(dbounds);
    }

    int nvars = atts.GetNumberOfVariables();
    // We will probably use only 2 of the 6.  But this avoids a UMR where
    // the avtExtents object copies 6 values for vectors (even though it only
    // uses the first 2 -- but that's a bigger battle.)
    double dextents[6] = 
                   { FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX, FLT_MAX, -FLT_MAX };
    for (int i = 0 ; i < nvars ; i++)
    {
        const char *vname = atts.GetVariableName(i).c_str();
        if (atts.GetTrueDataExtents(vname)->HasExtents())
        {
            //
            // There is no point in walking through the data and determining
            // what the cumulative extents are -- we know them already.
            //
            continue;
        }

        bool ignoreGhost = true;
        GetDataRange(ds, dextents, vname, true);
        atts.GetCumulativeTrueDataExtents(vname)->Merge(dextents);

        if (atts.GetVariableType(vname) == AVT_ARRAY_VAR)
        {
            double *compExt = new double[atts.GetVariableDimension(vname)*2];
            GetDataAllComponentsRange(ds, compExt, vname, true);
            atts.GetVariableComponentExtents(vname)->Merge(compExt);
            delete[] compExt;
        }   
    }
}


