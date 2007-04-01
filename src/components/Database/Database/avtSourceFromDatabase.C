// ************************************************************************* //
//                          avtSourceFromDatabase.C                          //
// ************************************************************************* //

#include <avtSourceFromDatabase.h>

#include <vtkDataSet.h>

#include <string>
#include <vector>

#include <avtDatasetDatabase.h>
#include <avtDatabaseMetaData.h>
#include <avtDataset.h>
#include <avtSILRestriction.h>
#include <avtSILRestrictionTraverser.h>

#include <BadIndexException.h>
#include <DebugStream.h>


using     std::string;
using     std::vector;


// ****************************************************************************
//  Method: avtSourceFromDatabase constructor
//
//  Arguments:
//      db      The database this source is connected to.
//      vn      The name of the variable this source corresponds to.
//      ts      The timestep this variable corresponds to.
//
//  Programmer: Hank Childs
//  Creation:   August 9, 2000
//
//  Modifications:
//
//    Hank Childs, Fri May 11 08:44:23 PDT 2001
//    Hooked load balancer into hint mechanism for I/O optimization.
//
//    Hank Childs, Wed Jun  6 13:40:28 PDT 2001
//    Removed argument for the domains.
//
// ****************************************************************************

avtSourceFromDatabase::avtSourceFromDatabase(avtDatasetDatabase *db, 
                                             const char *vn, int ts)
{
    database = db;

    variable = new char[strlen(vn)+1];
    strcpy(variable, vn);
    timestep = ts;

    lastSpec = NULL;
}


// ****************************************************************************
//  Method: avtSourceFromDatabase destructor
//
//  Programmer: Hank Childs
//  Creation:   March 2, 2001
//
// ****************************************************************************

avtSourceFromDatabase::~avtSourceFromDatabase()
{
    if (variable != NULL)
    {
        delete [] variable;
        variable = NULL;
    }
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::FetchDataset
//
//  Purpose:
//      Fetches the specified dataset.  This means going to the database and
//      requesting it.
//
//  Arguments:
//      spec    A specification of the dataset.
//      tree    A place to put the dataset.
//
//  Returns:    Whether or not the data has changed upstream (this always
//              returns false).
//
//  Programmer: Hank Childs
//  Creation:   June 6, 2001
//
//  Modifications:
//
//    Hank Childs, Wed Jul 25 15:40:54 PDT 2001
//    Made the database always return true so that dynamic load balancing will
//    be happier.  This is a temporary thing.
//
//    Hank Childs, Thu Jul 26 19:23:15 PDT 2001
//    Put in the hooks to definitively say if the request is different from
//    the previous one by comparing SILs.
//
//    Kathleen Bonnell, Wed Sep 19 13:45:33 PDT 2001 
//    Prune the tree if the request is different. 
//    
//    Hank Childs, Mon Mar 11 13:39:35 PST 2002
//    Add cycle and time.
//
//    Brad Whitlock, Wed Jul 3 13:41:18 PST 2002
//    Replaced the "/" string with SLASH_STRING so it will work on Windows.
//
//    Jeremy Meredith, Thu Aug 29 15:21:10 PDT 2002
//    I replaced the strstr call with a strpbrk so that I could remove
//    visit-config from the included headers.
//
//    Hank Childs, Fri Nov 22 16:52:14 PST 2002
//    Use a SIL restriction traverser to get the domain list.
//
//    Hank Childs, Fri May 16 10:14:40 PDT 2003
//    Catch exceptions thrown by the database.
//
//    Hank Childs, Thu Sep 25 16:30:09 PDT 2003
//    Make sure the variable being sent up to the database is a variable
//    contained within that database.
//
//    Hank Childs, Tue Sep 30 15:37:44 PDT 2003
//    Made sure input downstream would be digestible by all filters.
//
//    Hank Childs, Fri Oct 17 21:56:36 PDT 2003
//    Don't crash when NULL filenames are encountered.
//
// ****************************************************************************

bool
avtSourceFromDatabase::FetchDataset(avtDataSpecification_p spec,
                                    avtDataTree_p &tree)
{
    TRY
    {
        avtDataSpecification_p tmp_spec = new avtDataSpecification(spec,
                                                        spec->GetDBVariable());
        tree = database->GetOutput(tmp_spec, this);
    }
    CATCH2(VisItException, e)
    {
        debug1 << "Catching the exception at the database level." << endl;
        avtDataValidity &v = GetOutput()->GetInfo().GetValidity();
        v.ErrorOccurred();
        string tmp = e.GetMessage(); // Otherwise there is a const problem.
        v.SetErrorMessage(tmp);
        tree = new avtDataTree();
    }
    ENDTRY

    bool rv;
    if (*lastSpec == NULL)
    {
        rv = true;
    }
    else if (**lastSpec == **spec)
    {
        rv = false;
    }
    else
    {
        rv = true;
    }

    if (rv)
    {
        vector<int> list;
        avtSILRestrictionTraverser trav(spec->GetRestriction());
        trav.GetDomainList(list);
        tree = tree->PruneTree(list);
    }
    else
    {
        //
        // PruneTree does an additional service that is a bit hidden.  If the
        // output from the database is NULL, many of our filters will choke
        // on that.  PruneTree puts it in a more digestible form.  The "right"
        // thing to do in this case is to not call GetOutput on the database
        // when the last data specification is the same as the current one.
        // However, I'm a bit scared to do that because it may break other
        // things.  In addition, if the database threw an error, we are not
        // catching that.  So, if an error was thrown, and we did re-work the
        // logic to not call "GetOutput" unnecessarily, we would have to
        // remember the error and re-call it in that case.
        //
        if ((*tree == NULL) ||
            (!tree->HasData() && (tree->GetNChildren() == 0)))
        {
            tree = new avtDataTree();
        }
    }

    int timestep = spec->GetTimestep();
    avtDatabaseMetaData *md = database->GetMetaData(timestep);
    avtDataAttributes &atts = GetOutput()->GetInfo().GetAttributes();
    if (md->IsCycleAccurate(timestep))
    {
        int cycle = md->GetCycles()[timestep];
        atts.SetCycle(cycle);
    }
    if (md->IsTimeAccurate(timestep))
    {
        double dtime = md->GetTimes()[timestep];
        atts.SetTime(dtime);
    }

    const char *filename = database->GetFilename(timestep);
    if (filename != NULL)
    {
        const char *latest, *greatest;
        greatest = filename;
        latest   = filename;
        while (latest != NULL)
        {
            latest = strpbrk(greatest, "\\/");
            if (latest != NULL)
            {
                greatest = latest+1;
            }
        }
        atts.SetFilename(string(greatest));
    }

    //
    // We can't just share a reference to that specification, because it might
    // change.  No good copy constructor for data specification, so make use
    // a two-step process.
    //
    lastSpec = new avtDataSpecification(spec, 0);
    lastSpec = spec;

    return rv;
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::FetchVariableAuxiliaryData
//
//  Purpose:
//      Goes to the database to get auxiliary data.
//
//  Arguments:
//      type    The type of auxiliary data.
//      args    Additional arguments.
//      spec    The data specification.
//      output  A place to put the output void * reference pointers.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
// ****************************************************************************

void
avtSourceFromDatabase::FetchVariableAuxiliaryData(const char *type, void *args,
                              avtDataSpecification_p spec, VoidRefList &output)
{
    database->GetAuxiliaryData(spec, output, type, args);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::FetchMeshAuxiliaryData
//
//  Purpose:
//      Goes to the database to get auxiliary data.
//
//  Arguments:
//      type    The type of auxiliary data.
//      args    Additional arguments.
//      spec    The data specification.
//      output  A place to put the output void * reference pointers.
//
//  Programmer: Hank Childs
//  Creation:   March 1, 2001
//
// ****************************************************************************

void
avtSourceFromDatabase::FetchMeshAuxiliaryData(const char *type, void *args,
                              avtDataSpecification_p spec, VoidRefList &output)
{
    //
    // We only have hooks to the variable defined on the mesh.  We can get
    // the mesh name by accessing the database object.
    //
    string mn = database->GetMetaData(timestep)->MeshForVar(variable);
    avtDataSpecification_p newspec =new avtDataSpecification(spec, mn.c_str());
    database->GetAuxiliaryData(newspec, output, type, args);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::FetchMaterialAuxiliaryData
//
//  Purpose:
//      Goes to the database to get auxiliary data.
//
//  Arguments:
//      type    The type of auxiliary data.
//      args    Additional arguments.
//      dl      The domains to get the data for.
//
//  Returns:    The data as void *.
//
//  Programmer: Hank Childs
//  Creation:   April 10, 2001
//
// ****************************************************************************

void 
avtSourceFromDatabase::FetchMaterialAuxiliaryData(const char *type, void *args,
                              avtDataSpecification_p spec, VoidRefList &output)
{
    //
    // We only have hooks to the variable defined on the material.  We can get
    // the material name by accessing the database object.
    //
    string mn   = database->GetMetaData(timestep)->MeshForVar(variable);
    string mat  = database->GetMetaData(timestep)->MaterialOnMesh(mn);
    avtDataSpecification_p newspec =new avtDataSpecification(spec, mat.c_str());
    database->GetAuxiliaryData(newspec, output, type, args);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::GetFullDataSpecification
//
//  Purpose:
//      Gets the full database specification for this variable on this
//      database.
//
//  Programmer: Hank Childs
//  Creation:   June 13, 2001
//
//  Modifications:
//
//    Hank Childs, Thu Jul 26 12:59:02 PDT 2001
//    Added logic to set the top set.
//
//    Hank Childs, Thu Sep 25 16:27:38 PDT 2003
//    If a 'last spec' is available, use its variable.
//
// ****************************************************************************

avtDataSpecification_p
avtSourceFromDatabase::GetFullDataSpecification(void)
{
    const char *acting_var = variable;
    if (*lastSpec != NULL)
        acting_var = lastSpec->GetVariable();

    avtSIL *sil = database->GetSIL(timestep);
    avtSILRestriction_p silr = new avtSILRestriction(sil);

    //
    // Indicate which mesh is the mesh we are interested in.
    //
    avtDatabaseMetaData *md = database->GetMetaData(timestep);
    string mesh = md->MeshForVar(variable);
    silr->SetTopSet(mesh.c_str());

    return new avtDataSpecification(acting_var, timestep, silr);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::InvalidateZones
//
//  Purpose:
//      Goes to its output and indicates that the zones are now invalid.
//
//  Programmer: Hank Childs
//  Creation:   August 13, 2001
//
// ****************************************************************************

void
avtSourceFromDatabase::InvalidateZones()
{
    avtDataObject_p output = GetOutput();
    avtDataValidity &validity = output->GetInfo().GetValidity();
    validity.InvalidateZones();
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::DatabaseProgress
//
//  Purpose:
//      Makes a callback indicating the progress.
//
//  Arguments:
//      cur       The current domain.
//      tot       The total number of domains.
//      desc      The description of the database progress.
//
//  Programmer:   Hank Childs
//  Creation:     August 13, 2001
//
// ****************************************************************************

void
avtSourceFromDatabase::DatabaseProgress(int cur, int tot, const char *desc)
{
    UpdateProgress(cur, tot, "Reading from database", desc);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::CanDoDynamicLoadBalancing
//
//  Purpose:
//      Returns whether or not this source can do dynamic load balancing.
//
//  Programmer: Hank Childs
//  Creation:   October 25, 2001
//
// ****************************************************************************

bool
avtSourceFromDatabase::CanDoDynamicLoadBalancing(void)
{
    return database->CanDoDynamicLoadBalancing();
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::NumStagesForFetch
//
//  Purpose:
//      Determines how many stages the database wants to do a fetch in.
//
//  Arguments:
//      spec    A data specification (sometimes needed to determine if
//                                    matsel is done, etc).
//
//  Returns:    The number of stages the database will do during the fetch.
//
//  Programmer: Hank Childs
//  Creation:   October 26, 2001
//
// ****************************************************************************

int
avtSourceFromDatabase::NumStagesForFetch(avtDataSpecification_p spec)
{
    return database->NumStagesForFetch(spec);
}

// ****************************************************************************
//  Method: avtSourceFromDatabase::Query
//
//  Purpose:
//      Allows the database to return information about a particular cell and 
//      variables as specified by PickAttributes. 
//
//  Arguments:
//      pa      The PickAttributes that hold query information.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 15, 2002 
//
// ****************************************************************************

void
avtSourceFromDatabase::Query(PickAttributes *pa)
{
    database->Query(pa);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::FindElementForPoint
//
//  Purpose:
//      Allows the database to return the cell containing a point, or the 
//      closest node to the point (which one specified by elType).
//
//  Returns:
//      True for successful search, false otherwise.
// 
//  Arguments:
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    dom       The domain to use in searching the database.
//    elType    Specified which type of element (node, zone) to search for.
//    pt        The point to use in searching the database.
//    elNum     A place to store the zone or node number associated with the
//              point pt.
//
//  Programmer: Kathleen Bonnell
//  Creation:   November 13, 2003 
//
// ****************************************************************************

bool
avtSourceFromDatabase::FindElementForPoint(const char *var, const int ts,
    const int dom, const char *elType, float pt[3], int &elNum)
{
    return database->FindElementForPoint(var, ts, dom, elType, pt, elNum);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::GetDomainName
//
//  Purpose:
//      Allows the database to return a string representing the name
//      of the given domain. 
//
//  Arguments:
//    var       The variable to use in searching the database.
//    ts        The timestep to use in searching the database.
//    dom       The domain to use in searching the database.
//    domName   A place to store the domain name. 
//
//  Programmer: Kathleen Bonnell
//  Creation:   December 22, 2003 
//
// ****************************************************************************
void
avtSourceFromDatabase::GetDomainName(const std::string &var, const int ts,
    const int dom, std::string &domName)
{
    database->GetDomainName(var, ts, dom, domName);
}


// ****************************************************************************
//  Method: avtSourceFromDatabase::QueryZoneCenter
//
//  Purpose:    A query to find the geometric center of a zone.
//
//  Arguments:
//    var       The variable to use in searching the database.
//    dom       The domain to use in searching the database.
//    zone      The zone to use in searching the database.
//    ts        The timestep to use in searching the database.
//    coord     A place to store the zone center. 
//
//  Returns:    True if the query was successful, false otherwise.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 25, 2004 
//
// ****************************************************************************

bool
avtSourceFromDatabase::QueryZoneCenter(const std::string &var, const int dom,
    const int zone, const int ts, float coord[3])
{
    return database->QueryZoneCenter(var, dom, zone, ts, coord);
}
