// ************************************************************************* //
//                             avtOnionPeelFilter.C                          //
// ************************************************************************* //

#include <avtOnionPeelFilter.h>

#include <vtkCellData.h>
#include <vtkFieldData.h>
#include <vtkIntArray.h>
#include <vtkOnionPeelFilter.h>
#include <vtkDataSetRemoveGhostCells.h>
#include <vtkUnstructuredGrid.h>

#include <avtTerminatingSource.h>

#include <BadCellException.h>
#include <GhostCellException.h>
#include <InvalidVariableException.h>
#include <InvalidCategoryException.h>
#include <InvalidSetException.h>
#include <LogicalIndexException.h>
#include <DebugStream.h>
#include <CompactSILRestrictionAttributes.h>

#include <avtCallback.h>
#include <avtParallel.h>

using std::string;
using std::vector;


// ****************************************************************************
//  Method: avtOnionPeelFilter constructor
//
//  Arguments:
//      opf       The VTK OnionPeel filter this filter should use.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an OnionPeelAttributes class.
//
//    Hank Childs, Thu May 23 16:18:42 PDT 2002
//    Set up handling for bad seed cells.
//
//    Kathleen Bonnell, Thu Aug 15 18:30:44 PDT 2002 
//    Added another bad seed category (ghost), and flag specifying groups.
//
// ****************************************************************************

avtOnionPeelFilter::avtOnionPeelFilter()
{
    opf  = vtkOnionPeelFilter::New();
    opf->SetBadSeedCellCallback(avtOnionPeelFilter::BadSeedCellCallback, this);

    encounteredBadSeed = false;
    encounteredGhostSeed = false;
    groupCategory = false;
}


// ****************************************************************************
//  Method: avtOnionPeelFilter destructor
//
//  Programmer: Jeremy Meredith
//  Creation:   September 10, 2000
//
//  Modifications:
//
// ****************************************************************************

avtOnionPeelFilter::~avtOnionPeelFilter()
{
    opf->Delete();
    opf = NULL;
}


// ****************************************************************************
//  Method:  avtOnionPeelFilter::Create
//
//  Purpose:
//    Call the constructor.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March  4, 2001
//
// ****************************************************************************

avtFilter *
avtOnionPeelFilter::Create()
{
    return new avtOnionPeelFilter();
}


// ****************************************************************************
//  Method:  avtOnionPeelFilter::SetAtts
//
//  Purpose:
//      Sets the attributes for this filter.
//
//  Arguments:
//      a        The onion peel attributes.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2001
//
// ****************************************************************************

void
avtOnionPeelFilter::SetAtts(const AttributeGroup *a)
{
    atts = *(const OnionPeelAttributes*)a;
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::Equivalent
//
//  Purpose:
//      Returns true if creating a new avtOnionPeelFilter with the given
//      parameters would result in an equivalent avtOnionPeelFilter.
//
//  Arguments:
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//    Jeremy Meredith, Thu Mar  1 13:29:27 PST 2001
//    Made attributes be stored as an OnionPeelAttributes class.
//
// ****************************************************************************

bool
avtOnionPeelFilter::Equivalent(const AttributeGroup *a)
{
    return (atts == *(OnionPeelAttributes*)a);
}


// ****************************************************************************
//  Method: avtOnionPeelFilter::ExecuteData
//
//  Purpose:
//      Sends the specified input and output through the OnionPeel filter.
//
//  Arguments:
//      in_ds      The input dataset.
//      <unused>   The domain number.
//
//  Returns:       The output unstructured grid.
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 09, 2000
//
//  Modifications:
//
//    Hank Childs, Fri Oct 27 10:23:52 PDT 2000
//    Added argument for domain number to match inherited interface.
//
//    Jeremy Meredith, Thu Mar  1 13:49:38 PST 2001
//    Made arguments be stored as an AttributeGroup.
//
//    Kathleen Bonnell, Tue Apr 10 10:49:10 PDT 2001 
//    Renamed method from ExecuteDomain to ExecuteData. 
//
//    Hank Childs, Sun Jun 16 20:42:14 PDT 2002
//    Account for non 0-origin meshes.
//
//    Kathleen Bonnell, Thu Aug 15 18:30:44 PDT 2002 
//    Modified to support logical indices, and groups. 
//
//    Hank Childs, Wed Sep 11 08:39:56 PDT 2002
//    Fixed memory leak.
//
//    Kathleen Bonnell, Wed May  7 11:09:05 PDT 2003 
//    Remove ghost cells, if present. 
//
//    Kathleen Bonnell, Fri Dec 19 09:07:33 PST 2003 
//    Verify logical indices for groups are valid for this domain. 
//
// ****************************************************************************

vtkDataSet *
avtOnionPeelFilter::ExecuteData(vtkDataSet *in_ds, int DOM, std::string)
{
    encounteredBadSeed = false;
    encounteredGhostSeed = false;

    vector<int> id = atts.GetIndex();
    if (atts.GetLogical())
    {
        if ((in_ds->GetDataObjectType() != VTK_STRUCTURED_GRID) && 
            (in_ds->GetDataObjectType() != VTK_RECTILINEAR_GRID))
        {
            char msg[1024];
            sprintf(msg, "A Logical Index can only be used with structured data.");
            EXCEPTION1(LogicalIndexException, msg);
        }
        if (groupCategory)
        {
            vtkDataArray *bi_arr = in_ds->GetFieldData()->GetArray("base_index");
            int minIJK[3] = { 0, 0, 0};
            int maxIJK[3] = { 0, 0, 0};
            vtkDataArray *rd_arr = in_ds->GetFieldData()->GetArray("avtRealDims");
            if (bi_arr != NULL && rd_arr != NULL)
            {
                int *bi = ((vtkIntArray *)bi_arr)->GetPointer(0);
                int *rd = ((vtkIntArray *)rd_arr)->GetPointer(0);

                minIJK[0] = bi[0];
                minIJK[1] = bi[1];
                minIJK[2] = bi[2];
                maxIJK[0] = rd[1] - rd[0] + minIJK[0] -1;
                maxIJK[1] = rd[3] - rd[2] + minIJK[1] -1;
                maxIJK[2] = rd[5] - rd[4] + minIJK[2] -1;

                //
                // Verify that the logical index is valid for this domain
                // If not, set encounteredBadSeed to true, return NULL.
                //
                for (int i = 0; i < id.size(); i++)
                {
                    if (id[i] < minIJK[i] || id[i] > maxIJK[i])
                    {
                        encounteredBadSeed = true;
                        return NULL;
                    }
                }

                id[0] = (id[0]-minIJK[0] > 0 ? id[0] - minIJK[0] : 0);
                id[1] = (id[1]-minIJK[1] > 0 ? id[1] - minIJK[1] : 0);
                if (id.size() == 3)
                {
                    id[2] = (id[2]-minIJK[2] > 0 ? id[2] - minIJK[2] : 0);
                }
            }
        }
    }
    vtkDataSet *ds = in_ds;
    vtkDataSetRemoveGhostCells *removeGhostCells = NULL;
    if (in_ds->GetCellData()->GetArray("vtkGhostLevels"))
    {
        removeGhostCells = vtkDataSetRemoveGhostCells::New();
        removeGhostCells->SetInput(ds);
        ds = removeGhostCells->GetOutput();
        ds->Update();
        ds->SetSource(NULL);
    }

    opf->SetInput(ds);
    avtDataAttributes &da = GetInput()->GetInfo().GetAttributes();
    if (!atts.GetLogical())
    {
        if (da.GetCellOrigin() != 0)
        {
            debug5 << "Offsetting seed cell by origin = " << da.GetCellOrigin()
                   << endl;
            if (da.GetCellOrigin() > 1)
            {
                debug1 << "WARNING: mesh origin to offset seed cell by is "
                        << da.GetCellOrigin() << endl;
            }
        }
        int seedCell = id[0] - da.GetCellOrigin();

        //
        // This often comes up when someone has their default saved to cell 0 and
        // the origin is 1.  Then the cell they're asking for is '-1'.
        //
        if (seedCell < 0)
        {
            seedCell = 0;
        }
        opf->SetSeedCellId(seedCell);
    }
    else
    {
        if (id.size() == 3)
           opf->SetLogicalIndex(id[0], id[1], id[2]);
        else  
           opf->SetLogicalIndex(id[0], id[1]);
    }
    opf->SetRequestedLayer(atts.GetRequestedLayer());
    opf->SetAdjacencyType(atts.GetAdjacencyType());

    vtkUnstructuredGrid *out_ug = vtkUnstructuredGrid::New();
    opf->SetOutput(out_ug);
    out_ug->Delete();

    //
    // Update will check the modifed time and call Execute.  We have no direct
    // hooks into the Execute method.
    //
    opf->Update();
    
    if (removeGhostCells != NULL)
    { 
        removeGhostCells->Delete(); 
    }
    successfullyExecuted |= (!encounteredBadSeed && !encounteredGhostSeed);
    return out_ug;
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::BadSeedCellCallback
//
//  Purpose:
//      This is the static function that is called when a bad seed cell is
//      encountered.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2002
//
// ****************************************************************************

void
avtOnionPeelFilter::BadSeedCellCallback(void *ptr, int seedCell, int numCells, 
                                        bool ghost)
{
    avtOnionPeelFilter *opf = (avtOnionPeelFilter *) ptr;
    opf->BadSeedCell(seedCell, numCells, ghost);
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::BadSeedCell
//
//  Purpose:
//      Called when a bad seed cell is encountered.
//
//  Programmer: Hank Childs
//  Creation:   May 23, 2002
//
//  Modifications:
//    Kathleen Bonnell, Thu Aug 15 18:30:44 PDT 2002
//    Added new category of bad seed cell:  ghost.
// ****************************************************************************

void
avtOnionPeelFilter::BadSeedCell(int seedCell, int numCells, bool ghost)
{
    //
    // At this point I would like to throw the actual exception.  
    // Unfortunately, the catch code does not seem to operate correctly on the
    // Suns.  I re-compiled with fake exceptions and everything worked
    // perfectly, so I am going to designate this compiler error.  However,
    // I want to continue using real exceptions on the Sun, so I am going to
    // use a work-around, which is to wait until we are back in the AVT code
    // (more likely this being a shared library is the culprit) and issue the
    // exception then.
    //
    badSeedCell = seedCell;
    maximumCells = numCells;
    if (ghost)
        encounteredGhostSeed = true;
    else
        encounteredBadSeed = true;
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::RefashionDataObjectInfo
//
//  Purpose:
//      Indicates that the zone numberings are no longer valid after this
//      operation.
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
// ****************************************************************************

void
avtOnionPeelFilter::RefashionDataObjectInfo(void)
{
    GetOutput()->GetInfo().GetValidity().InvalidateZones();
}


// ****************************************************************************
//  Method: avtOnionPeelFilter::PerformRestriction
//
//  Purpose:
//    Restricts the SIL to only the domain requested by user. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//
//    Hank Childs, Sun Jun 16 20:50:53 PDT 2002
//    Account for non 0-origins for blocks.
//
//    Kathleen Bonnell, Thu Aug 15 08:55:15 PDT 2002  
//    Set ZoneNumbers Flag when appropriate. 
//
//    Kathleen Bonnell, Thu Aug 15 18:30:44 PDT 2002 
//    Reflect changes that allow user to specify groups and logical indices. 
//
//    Kathleen Bonnell, Thu Feb 26 12:50:38 PST 2004 
//    Added code to test that the currently chosen set is turned on. 
//
// ****************************************************************************

avtPipelineSpecification_p
avtOnionPeelFilter::PerformRestriction(avtPipelineSpecification_p spec)
{
    if (atts.GetSubsetName() == "Whole")
    {
        //
        // No restriction necessary. 
        //
        return spec;
    }

    avtPipelineSpecification_p rv = new avtPipelineSpecification(spec);

    string subset = atts.GetSubsetName();
    int setID;
    avtSILRestriction_p silr = spec->GetDataSpecification()->GetRestriction();
    CompactSILRestrictionAttributes *silAtts = silr->MakeCompactAttributes();
    const unsignedCharVector &useSet =  silAtts->GetUseSet();
    setID = silr->GetSetIndex(subset);
    if (useSet[setID] == 0) 
    {
        EXCEPTION1(InvalidSetException, subset.c_str());
    }
    TRY
    {
        silr = rv->GetDataSpecification()->GetRestriction();
        silr->TurnOffAll();
        silr->TurnOnSet(setID);
    }
    CATCH(InvalidVariableException)
    {
        // If for some reason the GetSetIndex fails.
        RETHROW;
    }
    ENDTRY
    
    if (rv->GetDataSpecification()->MayRequireZones())
    {
        rv->GetDataSpecification()->TurnZoneNumbersOn();
    }
    return rv;
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::VerifyInput
//
//  Purpose:
//    Throw an exception if user-selected domain is out of range. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   October 16, 2001 
//
//  Modifications:
//    Hank Childs, Sun Jun 16 20:50:53 PDT 2002
//    Account for non 0-origins for blocks.
//
//    Kathleen Bonnell, Thu Aug 15 18:37:59 PDT 2002  
//    Reflect changes that allow user to specify groups and logical indices. 
//
//    Hank Childs, Wed Jan 22 18:17:42 PST 2003
//    Do not allow onion peel to execute if the zone connectivity has been
//    modified.
//
//    Hank Childs, Fri Apr  4 10:10:22 PST 2003
//    Move connectivity check to PreExecute, since whether the connectivity
//    has changed may be more accurate there than it is here.
//
// ****************************************************************************

void
avtOnionPeelFilter::VerifyInput()
{
    if (atts.GetSubsetName() == "Whole")
    {
        return;
    }

    std::string category = atts.GetCategoryName();
    std::string subset = atts.GetSubsetName();
    avtSILRestriction_p silr = GetTerminatingSource()->
        GetFullDataSpecification()->GetRestriction();

    int setID, collectionID;
    TRY
    {
        collectionID = silr->GetCollectionIndex(category);
        setID = silr->GetSetIndex(subset);
        avtSILCollection_p coll = silr->GetSILCollection(collectionID);

        if (coll->GetRole() != SIL_DOMAIN && coll->GetRole() != SIL_BLOCK)
        {
            //
            //  May occur if user types in a category name.
            //
            EXCEPTION1(InvalidCategoryException, category.c_str()); 
        }

        const vector<int> &els = coll->GetSubsetList();
        bool validSet = false;
        for (int i = 0; i < els.size() && !validSet; i++)
        {
            validSet = (setID == els[i]);
        }

        if (!validSet)
        {
            //
            //  May occur if user types in a set name.
            //
            EXCEPTION2(InvalidSetException, category.c_str(), subset.c_str());
        }

        if (coll->GetRole() == SIL_BLOCK)
        {
            //
            //  Perform some more checking
            //
            if (!atts.GetLogical())
            {
                EXCEPTION0(LogicalIndexException);
            }
            groupCategory = true;
        }
    }
    CATCH(InvalidVariableException)
    {
        //
        //  SIL could not match category name or subset name to an id.
        //
        RETHROW; 
    }
    ENDTRY
}

// ****************************************************************************
//  Method: avtOnionPeelFilter::PostExecute
//
//  Purpose:
//    Called after Execute (which calls ExecuteData). 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 15, 2002 
//
//  Modificiations:
//    Kathleen Bonnell, Thu Jan 15 09:15:10 PST 2004
//    Grab the return value of UnifyMaximumValue so that this code
//    will work correctly in parallel.
//
// ****************************************************************************

void
avtOnionPeelFilter::PostExecute()
{
    int success = UnifyMaximumValue((int) successfullyExecuted);
    //
    //  Throw exceptions if necessary.  Done here instead of execute,
    //  because Groups may have invalid execution in some domains, but valid
    //  executions in others.  Only throw the exceptions if all the
    //  domains had exceptions.
    //
    if (success == 0)
    {
        if (encounteredBadSeed)
        {
            encounteredBadSeed = false;
            if (atts.GetLogical())
            {
                EXCEPTION1(BadCellException, atts.GetIndex());
            }
            else 
            {
                EXCEPTION2(BadCellException, badSeedCell, maximumCells);
            }
        }
        if (encounteredGhostSeed)
        {
            encounteredGhostSeed = false;
            char msg [512];
            sprintf(msg, "Please choose a different seed cell.");
            if (atts.GetLogical())
            {
                EXCEPTION2(GhostCellException, atts.GetIndex(), msg);
            }
            else
            {
                EXCEPTION2(GhostCellException, badSeedCell, msg);
            }
        }
    }

}


// ****************************************************************************
//  Method: avtOnionPeelFilter::PreExecute
//
//  Purpose:
//    Called before Execute. 
//
//  Programmer: Kathleen Bonnell 
//  Creation:   August 15, 2002 
//
//  Modifications:
//
//    Hank Childs, Fri Apr  4 10:10:22 PST 2003 
//    Moved connectivity check from VerifyInput to here.
//
// ****************************************************************************

void 
avtOnionPeelFilter::PreExecute()
{
    successfullyExecuted = false;

    if (!GetInput()->GetInfo().GetValidity().GetZonesPreserved())
    {
        avtCallback::IssueWarning("The onion peel operator will not perform "
                   "as expected, because the connectivity of the underlying "
                   "mesh was modified before the onion peel operator got a "
                   "chance to execute.  The VisIt team plans to make the onion"
                   " peel operator work in these conditions in the future.  If"
                   " you need this capability in the short term, please "
                   "contact a VisIt developer.");
    }
}


// ****************************************************************************
//  Method: avtOnionPeelFilter::ReleaseData
//
//  Purpose:
//      Releases the problem size data associated with this filter.
//
//  Programmer: Hank Childs
//  Creation:   September 10, 2002
//
// ****************************************************************************

void
avtOnionPeelFilter::ReleaseData(void)
{
    avtPluginStreamer::ReleaseData();
    opf->SetInput(NULL);
    opf->SetOutput(NULL);
}


