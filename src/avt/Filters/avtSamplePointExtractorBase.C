/*****************************************************************************
*
* Copyright (c) 2000 - 2018, Lawrence Livermore National Security, LLC
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
//                      avtSamplePointExtractorBase.C                        //
// ************************************************************************* //

#include <avtSamplePointExtractorBase.h>

#include <float.h>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkPointData.h>

#include <avtMemory.h>
#include <avtParallel.h>
#include <avtRelativeValueSamplePointArbitrator.h>
#include <avtSamplePoints.h>

#include <DebugStream.h>
#include <TimingsManager.h>
#include <StackTimer.h>

#include <stack>


// ****************************************************************************
//  Method: avtSamplePointExtractorBase constructor
//
//  Arguments:
//      w       The width.
//      h       The height.
//      d       The depth.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//     
//  Modifications:
//
//    Hank Childs, Thu Nov 15 15:39:48 PST 2001
//    Moved construction of cell list to Execute to account new limitations of
//    sample points involving multiple variables.
//
//    Hank Childs, Tue Jan  1 10:01:20 PST 2002
//    Initialized sendCells.
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Initialized massVoxelExtractor.
//
//    Hank Childs, Fri Nov 19 13:57:02 PST 2004
//    Initialized rectilinearGridsAreInWorldSpace.
//
//    Hank Childs, Fri Dec 10 09:59:57 PST 2004
//    Initialized shouldDoTiling.
//
//    Hank Childs, Wed Feb  2 08:56:00 PST 2005
//    Initialize modeIs3D.
//
//    Hank Childs, Sun Dec  4 19:12:42 PST 2005
//    Initialize kernelBasedSampling.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Added point extractor.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Added hex20 extractor.
//
//    Hank Childs, Tue Jan 15 14:26:06 PST 2008
//    Initialize members for sample point arbitration.
//
//    Hank Childs, Fri Jan  9 14:10:25 PST 2009
//    Initialize jitter.
//
//    Mark C. Miller, Thu Oct  2 09:41:37 PDT 2014
//    Initialize lightDirection.
// ****************************************************************************

avtSamplePointExtractorBase::avtSamplePointExtractorBase(int w, int h, int d)
{
    width  = w;
    height = h;
    depth  = d;

    currentNode = 0;
    totalNodes  = 0;

    jitter           = false;

    rectilinearGridsAreInWorldSpace = false;
    aspect = 1.;

    shouldDoTiling = false;

    shouldSetUpArbitrator    = false;
    arbitratorPrefersMinimum = false;
    arbitrator               = NULL;

    transferFn1D = NULL;
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase destructor
//
//  Programmer: Hank Childs
//  Creation:   December 8, 2000
//      
//  Modifications:
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Deleted massVoxelExtractor.
//
//    Hank Childs, Tue Jan 24 16:42:40 PST 2006
//    Deleted pointExtractor.
//
//    Timo Bremer, Thu Sep 13 14:02:40 PDT 2007
//    Deleted hex20Extractor.
//
//    Hank Childs, Tue Jan 15 21:25:01 PST 2008
//    Delete arbitrator.
//
// ****************************************************************************

avtSamplePointExtractorBase::~avtSamplePointExtractorBase()
{
    if (arbitrator != NULL)
    {
        delete arbitrator;
        arbitrator = NULL;
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::SetRectilinearGridsAreInWorldSpace
//
//  Purpose:
//      Tells this object that any input rectilinear grids are in world space,
//      not image space.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
//  Modifications:
//    Jeremy Meredith, Thu Feb 15 12:14:04 EST 2007
//    Set rectilinearGridsAreInWorldSpace based on the passed in value
//    (which might be false), not to true.
//
// ****************************************************************************

void
avtSamplePointExtractorBase::SetRectilinearGridsAreInWorldSpace(bool val,
                 const avtViewInfo &v, double a)
{
    rectilinearGridsAreInWorldSpace = val;
    view = v;
    aspect = a;
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::RestrictToTile
//
//  Purpose:
//      Tells the extractor whether or not it should only sample within a tile.
//
//  Programmer: Hank Childs
//  Creation:   November 19, 2004
//
// ****************************************************************************

void
avtSamplePointExtractorBase::RestrictToTile(int wmin, int wmax,
                                                                                        int hmin, int hmax)
{
    shouldDoTiling = true;
    width_min  = wmin;
    width_max  = wmax;
    height_min = hmin;
    height_max = hmax;
    modified = true;
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::Execute
//
//  Purpose:
//      This is the real execute method that gets the sample points out of a
//      dataset.
//
//  Programmer: Hank Childs
//  Creation:   December 5, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Sat Apr 21 13:06:27 PDT 2001 
//    Moved major portion of code to recursive Execute method that walks 
//    down the data tree.
//
//    Hank Childs, Wed Jun  6 10:31:04 PDT 2001
//    Removed domain list argument.  Blew away outdated comments.
//
//    Eric Brugger, Mon Nov  5 13:46:04 PST 2001
//    Modified to always compile the timing code.
//
//    Hank Childs, Thu Nov 15 15:39:48 PST 2001
//    Set up the extractors here (instead of constructor), since they must
//    know how many variables they are working on.
//
//    Hank Childs, Mon Nov 19 14:56:40 PST 2001
//    Gave progress while resampling.
//
// ****************************************************************************

void
avtSamplePointExtractorBase::Execute(void)
{
    int timingsIndex = visitTimer->StartTimer();

    SetUpExtractors();

    avtDataTree_p tree = GetInputDataTree();
    totalNodes = tree->GetNumberOfLeaves();
    currentNode = 0;
    ExecuteTree(tree);

    visitTimer->StopTimer(timingsIndex, "Sample point extraction");
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::ExecuteTree
//
//  Purpose:
//      This is the recursive execute method that gets the sample points 
//      out of a dataset.  
//
//  Arguments:
//      dt      The data tree that should be processed.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   April 21, 2001. 
//
//  Modifications:
//
//    Hank Childs, Wed Jun  6 10:22:48 PDT 2001
//    Renamed ExecuteTree.
//
//    Hank Childs, Tue Jun 19 19:24:39 PDT 2001
//    Put in logic to handle bad data trees.
//
//    Hank Childs, Tue Nov 13 15:22:07 PST 2001
//    Add support for multiple variables.
//
//    Hank Childs, Mon Nov 19 14:56:40 PST 2001
//    Gave progress while resampling.
//
//    Hank Childs, Mon Apr 15 15:34:43 PDT 2002
//    Give clearer error messages.
//
//    Hank Childs, Fri Jul 18 11:42:10 PDT 2003
//    Do not sample ghost zones.  This gives slightly better performance.
//    And ghost zones occasionally have the wrong value (due to problems with
//    the code that produced it).
//
//    Hank Childs, Sun Dec 14 11:07:56 PST 2003
//    Make use of massVoxelExtractor.
//
//    Hank Childs, Fri Aug 27 16:47:45 PDT 2004
//    Rename ghost data arrays.
//
//    Hank Childs, Fri Nov 19 13:57:02 PST 2004
//    If the rectilinear grids are in world space, then let the mass voxel
//    extractor know about it.
//
//    Hank Childs, Sat Jan 29 13:32:54 PST 2005
//    Added 2D extractors.
//
//    Hank Childs, Sun Jan  1 10:53:09 PST 2006
//    Moved raster based sampling into its own method.  Added support for
//    kernel based sampling.
//
//    Manasa Prasad, 
//    Converted the recursive function to iteration
//
//    Kathleen Biagas, Wed Apr 18 16:53:21 PDT 2018
//    Added call to DoSampling, for derived classes.
//
// ****************************************************************************

struct datatree_childindex
{
    avtDataTree_p dt;
    int idx;
    bool visited;
    datatree_childindex(avtDataTree_p dt_, int idx_) :
        dt(dt_),idx(idx_),visited(false) {}
};


void
avtSamplePointExtractorBase::ExecuteTree(avtDataTree_p dt)
{
    /////////////////////////
    // Check memory before //
    /////////////////////////
    if (DebugStream::Level5())
    { 
        unsigned long m_size, m_rss;
        avtMemory::GetMemorySize(m_size, m_rss);
        debug5 << "RAR_Rank: " << PAR_Rank() << " "
               << GetType() << "::ExecuteTree " 
               << " Initial Memory Usage: "
               << m_size << " rss (MB): "
               << m_rss/(1024*1024) << endl;
    }

    //
    // Start timing
    //
    StackTimer t0(std::string(GetType()) + std::string("::ExecuteTree"));

    //
    // Call this function for initialization if necessary by children classes
    //
    InitSampling(dt);   

    if (*dt == NULL || (dt->GetNChildren() <= 0 && (!(dt->HasData()))))
        return;

    //
    // Process tree
    //
    std::stack<datatree_childindex*> nodes;    
    nodes.push(new datatree_childindex(dt,0)); //iterative depth-first sampling
    while (!nodes.empty())
    {
        datatree_childindex *ci=nodes.top();
        avtDataTree_p ch=ci->dt;

        if (ch->GetNChildren() != 0)
        {
            nodes.pop();  // if it has children, it never gets processed below
            for (int i = 0; i < ch->GetNChildren(); i++)
            {
                if (ch->ChildIsPresent(i))
                {
                    if (*ch == NULL || (ch->GetNChildren() <= 0 &&
                                        (!(ch->HasData()))))
                        continue;
                    nodes.push(new datatree_childindex(ch->GetChild(i),i));
                }
            }

            continue;
        }

        nodes.pop();

        if (*ch == NULL || (ch->GetNChildren() <= 0 && (!(ch->HasData()))))
            continue;

        // Get the dataset for this leaf in the tree.
        vtkDataSet *ds = ch->GetDataRepresentation().GetDataVTK();

        // Performed by derived classes
        DoSampling(ds, ci->idx);

        UpdateProgress(10*currentNode+9, 10*totalNodes);
        currentNode++;
    }

    /////////////////////////
    // Check memory before //
    /////////////////////////
    if (DebugStream::Level5())
    { 
        unsigned long m_size, m_rss;
        avtMemory::GetMemorySize(m_size, m_rss);
        debug5 << "RAR_Rank: " << PAR_Rank() << " "
               << GetType() << "::ExecuteTree " 
               << " Final Memory Usage: "
               << m_size << " rss (MB): "
               << m_rss/(1024*1024) << endl;
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::SetUpArbitrator
//
//  Purpose:
//      Tells this module that it should set up an arbitrator.
//
//  Programmer: Hank Childs
//  Creation:   January 15, 2008
//
// ****************************************************************************

void
avtSamplePointExtractorBase::SetUpArbitrator(std::string &name, bool pm)
{
    arbitratorVarName        = name;
    arbitratorPrefersMinimum = pm;
    shouldSetUpArbitrator    = true;
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::PreExecute
//
//  Purpose:
//      Determines how many points we have if we have a point mesh.  This will
//      allow us to choose an appropriate radius.
//
//  Programmer: Hank Childs
//  Creation:   February 28, 2006
//
//  Modifications:
//
//    Hank Childs, Tue Jan 15 21:23:49 PST 2008
//    Set up the sample point arbitrator.
//
//    Hank Childs, Sat Nov 21 13:29:21 PST 2009
//    Add support for long longs.
//
// ****************************************************************************

void
avtSamplePointExtractorBase::PreExecute(void)
{
    avtDatasetToSamplePointsFilter::PreExecute();

    if (shouldSetUpArbitrator)
    {
        avtSamplePoints_p samples = GetTypedOutput();
        int nvars = samples->GetNumberOfRealVariables();
        int theMatch = -1;
        int tmpIndex = 0;
        for (int i = 0 ; i < nvars ; i++)
        {
            bool foundMatch = false;
            if (samples->GetVariableName(i) == arbitratorVarName)
                foundMatch = true;

            if (foundMatch)
            {
                theMatch = tmpIndex;
                break;
            }
            else
                tmpIndex += samples->GetVariableSize(i);
        }

        if (theMatch != -1)
        {
            arbitrator = new avtRelativeValueSamplePointArbitrator(
                                      arbitratorPrefersMinimum, tmpIndex);
            avtRay::SetArbitrator(arbitrator);
        }
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::SetJittering
//
//  Purpose:
//      Tell the individual cell extractors whether or not to jitter.
//
//  Arguments:
//      j     true if the cell extractors should jitter
//
//  Programmer: Hank Childs
//  Creation:   January 9, 2009
//
// ****************************************************************************

void
avtSamplePointExtractorBase::SetJittering(bool j)
{
    jitter = j;

    SendJittering();
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::PostExecute
//
//  Purpose:
//      Unregisters the sample point arbitrator
//
//  Programmer: Hank Childs
//  Creation:   January 15, 2008
//
// ****************************************************************************

void
avtSamplePointExtractorBase::PostExecute(void)
{
    avtDatasetToSamplePointsFilter::PostExecute();

    if (shouldSetUpArbitrator)
    {
        avtRay::SetArbitrator(NULL);
        if (arbitrator != NULL)
        {
            delete arbitrator;
            arbitrator = NULL;
        }
    }
}


// ****************************************************************************
//  Method: avtSamplePointExtractorBase::GetLoadingInfoForArrays
//
//  Purpose:
//      Gets the "loading info" the data arrays.  The loading info pertains
//      to how to put the variable information into a cell so that it can
//      be handed off to a derived type of avtExtractor.
//
//  Programmer: Hank Childs
//  Creation:   June 1, 2007
//
// ****************************************************************************

void
avtSamplePointExtractorBase::GetLoadingInfoForArrays(vtkDataSet *ds,
                                                 LoadingInfo &li)
{
    int  i, j, k;

    avtSamplePoints_p samples = GetTypedOutput();
    int numVars = samples->GetNumberOfRealVariables(); // Counts vector as 1
    li.nVars = samples->GetNumberOfVariables();        // Counts vector as 3

    int ncd = ds->GetCellData()->GetNumberOfArrays();
    li.cellDataIndex.resize(ncd);
    li.cellDataSize.resize(ncd);
    li.cellArrays.resize(ncd);
    for (i = 0 ; i < ncd ; i++)
    {
        vtkDataArray *arr = ds->GetCellData()->GetArray(i);
        li.cellArrays[i] = arr;
        const char *name = arr->GetName();
        li.cellDataSize[i]  = arr->GetNumberOfComponents();
        li.cellDataIndex[i] = -1;
        for (j = 0 ; j < numVars ; j++)
        {
            if (samples->GetVariableName(j) == name)
            {
                int idx = 0;
                for (k = 0 ; k < j ; k++)
                    idx += samples->GetVariableSize(k);
                li.cellDataIndex[i] = idx;
                break;
            }
        }
    }

    int npd = ds->GetPointData()->GetNumberOfArrays();
    li.pointDataIndex.resize(npd);
    li.pointDataSize.resize(npd);
    li.pointArrays.resize(npd);
    for (i = 0 ; i < npd ; i++)
    {
        vtkDataArray *arr = ds->GetPointData()->GetArray(i);
        li.pointArrays[i] = arr;
        const char *name = arr->GetName();
        li.pointDataSize[i]  = arr->GetNumberOfComponents();
        li.pointDataIndex[i] = -1;
        for (j = 0 ; j < numVars ; j++)
        {
            if (samples->GetVariableName(j) == name)
            {
                int idx = 0;
                for (k = 0 ; k < j ; k++)
                    idx += samples->GetVariableSize(k);
                li.pointDataIndex[i] = idx;
                break;
            }
        }
    }
}
