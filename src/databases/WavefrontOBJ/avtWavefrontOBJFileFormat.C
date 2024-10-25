// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                        avtWavefrontOBJFileFormat.C                        //
// ************************************************************************* //

#include <avtWavefrontOBJFileFormat.h>

#include <vtkBitArray.h>
#include <vtkCellData.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPolyData.h>
#include <vtkStringArray.h>
#include <vtkVisItOBJReader.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterialMetaData.h>
#include <avtScalarMetaData.h>
#include <avtMaterial.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>

#include <set>
#include <sstream>
#include <string>
#include <vector>

//
// Define the static const's
//

const char   *avtWavefrontOBJFileFormat::MESHNAME = "OBJMesh";
const char   *avtWavefrontOBJFileFormat::VARNAME1 = "OBJVar1";
const char   *avtWavefrontOBJFileFormat::VARNAME2 = "OBJVar2";


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat constructor
//
//  Arguments:
//      fname    The file name.
//
//  Programmer:  Hank Childs
//  Creation:    February 14, 2002
//
// ****************************************************************************

avtWavefrontOBJFileFormat::avtWavefrontOBJFileFormat(const char *fname) 
    : avtSTSDFileFormat(fname)
{
    dataset = NULL;
    readInDataset = false;
    nVars = 0;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
// ****************************************************************************

avtWavefrontOBJFileFormat::~avtWavefrontOBJFileFormat()
{
    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::ReadInDataset
//
//  Purpose:
//      Reads in the dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords has been deprecated in VTK 4.0, use vtkDataArray instead. 
//
//    Hank Childs, Tue May 28 08:50:58 PDT 2002
//    Don't assume that there are texture coordinates.
//
//    Kathleen Bonnell, Wed May 17 10:58:54 PDT 2006
//    Remove call to SetSource(NULL) as it now removes information necessary
//    for the dataset. 
//
//    Eric Brugger, Wed Dec 24 08:00:23 PST 2008
//    I replaced vtkOBJReader with vtkVisItOBJReader so that the engine
//    would load the one defined in this plugin, rather than the one in the
//    vtk library.
//
//    Jeremy Meredith, Thu Jan  7 12:35:04 EST 2010
//    The VTK reader will parse just about anything, but at least it returns
//    an empty dataset for some erroneous cases.  Error on an empty dataset.
//
//    Brad Whitlock, Web Feb 3 15:51:34 PST 2010
//    I moved the empty check until after the data has been read.
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::ReadInDataset(void)
{
    debug4 << "Reading in dataset from WavefrontOBJ file " << filename << endl;

    //
    // This shouldn't ever happen (since we would already have the dataset
    // we are trying to read from the file sitting in memory), but anything
    // to prevent leaks.
    //
    if (dataset != NULL)
    {
        dataset->Delete();
    }

    //
    // Create a file reader and set our dataset to be its output.
    //
    vtkVisItOBJReader *reader = vtkVisItOBJReader::New();
    reader->SetFileName(filename);
    reader->Update();
    dataset = reader->GetOutput();
    dataset->Register(NULL);

    //dataset->SetSource(NULL);
    reader->Delete();

    // Now that the dataset has been read, check to see if it is empty.
    if (dataset->GetNumberOfPoints() == 0 && dataset->GetNumberOfCells() == 0)
        EXCEPTION2(InvalidFilesException, filename, "No OBJ data in the file.");

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords != NULL)
    {
        nVars = tcoords->GetNumberOfComponents();
    }
    else
    {
        nVars = 0;
    }

    readInDataset = true;

    hasGroups = false;
    vtkStringArray *groupNames = vtkStringArray::SafeDownCast(
        dataset->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_AggregateGroupNames"));
    if (!groupNames) return;
    hasGroups = true;

    /* If the dataset was returned with group info, process that in preparation 
       for definining an enum scalar. The entries returned in the AggregateGroupNames
       array potentially list multiple names. That happens if the groups statements
       in the .obj file have spaces indicating the element's memberships in multiple
       groups. We unravel that here into two lists. One is a std::set of unique group
       names used to define an enumerated scalar. The other is a std::set where we
       aggregate the space separated names into a single string and the resulting
       aggregatedGroupNames is used to define a material object. */
    for (int i = 0; i < groupNames->GetNumberOfTuples(); i++)
    {
        std::string str1 = groupNames->GetValue(i);
        std::string str2 = str1;

        std::replace(str2.begin(), str2.end(), ' ', '_');
        aggregatedGroupNames.insert(str2);

        std::istringstream iss(str1);
        int n = 0;
        do
        {
            std::string sub;
            iss >> sub;
            if (!sub.size()) continue; // skip blank strings
            uniqueGroupNames.insert(sub);
            n++;
        } while (iss);
    }
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh.  The mesh is actually just the dataset, so return that.
//
//  Arguments:
//      mesh     The desired meshname, this should be MESHNAME.
//
//  Returns:     The mesh as a WavefrontOBJ dataset.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Jul 23 10:55:03 PDT 2002
//    Account for the way that the data is handled when it is returned.
//
// ****************************************************************************

vtkDataSet *
avtWavefrontOBJFileFormat::GetMesh(const char *mesh)
{
    debug5 << "Getting mesh from WavefrontOBJ file " << filename << endl;

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    if (!readInDataset)
    {
        ReadInDataset();
    }

    //
    // The generic database will assume it owns this piece of memory, so add
    // a reference so our dataset won't be deleted out from under us.
    //
    dataset->Register(NULL);

    return dataset;
}


// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::GetVar
//
//  Purpose:
//      Gets the variable.
//
//  Arguments:
//      var      The desired varname, this should be VARNAME.
//
//  Returns:     The varialbe as WavefrontOBJ scalars.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Mar 20 09:23:26 PST 2001
//    Account for vector variable in error checking.
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords and vtkScalars have been deprecated in VTK 4.0, 
//    use vtkDataArray and vtkFloatArray instead. 
//
// ****************************************************************************

vtkDataArray *
avtWavefrontOBJFileFormat::GetVar(const char *var)
{
    debug5 << "Getting var from WavefrontOBJ file " << filename << endl;

    if (!readInDataset)
    {
        ReadInDataset();
    }

    if (hasGroups && std::string(var) == "Groups")
    {
        vtkStringArray *strArray = vtkStringArray::SafeDownCast(
            dataset->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_AggregateGroupNames"));
        vtkIntArray *intArray = vtkIntArray::SafeDownCast(
            dataset->GetCellData()->GetArray("_vtkVisItOBJReader_AggregateGroupIDs"));

        // Initialize the return variable array
        const int bpuc = sizeof(unsigned char)*8;
        int nCells = dataset->GetNumberOfCells();
        vtkBitArray *groups = vtkBitArray::New();
        groups->SetNumberOfComponents(((uniqueGroupNames.size()+bpuc-1)/bpuc)*bpuc);
        groups->SetNumberOfTuples(nCells);
        memset(groups->GetVoidPointer(0), 0, groups->GetSize()/bpuc);

        for (int i = 0 ; i < nCells; i++)
        {
            int aggGroupId = intArray->GetValue(i);

            /* Given the group ID, get the name(s) of the group(s) it is in */
            std::string aggGroupList = strArray->GetValue(aggGroupId);

            /* Since the group string may contain space separated list of
               names, process it as such and turn on associated bit for
               each group */
            std::istringstream iss(aggGroupList);
            do
            {
                std::string sub;
                iss >> sub;
                if (!sub.size()) continue; // skip blank strings

                std::set<std::string>::const_iterator cit = uniqueGroupNames.find(sub);
                if (cit != uniqueGroupNames.end())
                {
                    int k = (int) std::distance(uniqueGroupNames.begin(),cit);
                    groups->SetComponent(i, k, 1);
                }
            } while (iss);
        }

        return groups;
    }

    if (nVars <= 0 || 
        (strcmp(var, VARNAME1) != 0 && strcmp(var, VARNAME2) != 0))
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    vtkFloatArray *s1 = vtkFloatArray::New();
    int varId = (strcmp(var, VARNAME1) == 0 ? 0 : 1);
    int nPts = dataset->GetNumberOfPoints();
    s1->SetNumberOfTuples(nPts);
    for (int i = 0 ; i < nPts ; i++)
    {
        double *tuple = tcoords->GetTuple(i);
        s1->SetTuple1(i, tuple[varId]);
    }

    return s1;
}

void *
avtWavefrontOBJFileFormat::GetAuxiliaryData(const char *var,
    const char *type, void *, DestructorFunction &df)
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        vtkIntArray *intArray = vtkIntArray::SafeDownCast(
            dataset->GetCellData()->GetArray("_vtkVisItOBJReader_AggregateGroupIDs"));

        int nCells = dataset->GetNumberOfCells();
        int *matlist = new int[nCells];
       
        for (int i = 0 ; i < nCells; i++)
        {
            int aggGroupId = intArray->GetValue(i);
            matlist[i] = aggGroupId;
        }

        vtkStringArray *groupNames = vtkStringArray::SafeDownCast(
        dataset->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_AggregateGroupNames"));
        std::vector<std::string> matnames;
        for (int i = 0; i < groupNames->GetNumberOfTuples(); i++)
        {
            std::string str = groupNames->GetValue(i);
            std::replace(str.begin(), str.end(), ' ', '_');
            matnames.push_back(str);
        }

        avtMaterial *mat = new avtMaterial((int) matnames.size(), matnames, nCells,
            matlist, 0, 0, 0, 0, 0);

       delete [] matlist;
       df = avtMaterial::Destruct;
       return (void*) mat;
   }

   return 0;
}
    
// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up resources.  Since this module does not keep an open file, that
//      only means deleting the dataset.  Since this is all reference counted,
//      there is no worry that we will be deleting something that is being
//      used.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::FreeUpResources(void)
{
    debug4 << "WavefrontOBJ file " << filename 
           << " forced to free up resources." << endl;

    if (dataset != NULL)
    {
        dataset->Delete();
        dataset = NULL;
    }

    readInDataset = false;
}


static void
AddGroupings(avtDatabaseMetaData *md, vtkDataSet *ds, char const *meshname,
    std::set<std::string> const &uniqueGroupNames,
    std::set<std::string> const &aggregatedGroupNames)
{
    /* Always add groupings using an enum scalar */
    avtScalarMetaData *smd = new avtScalarMetaData("Groups", meshname, AVT_ZONECENT);

    for (std::set<std::string>::const_iterator cit = uniqueGroupNames.begin();
        cit != uniqueGroupNames.end(); cit++)
    {
        int k = (int) std::distance(uniqueGroupNames.begin(), cit);
        smd->AddEnumNameValue(*cit, k);
    }
    smd->SetEnumerationType(avtScalarMetaData::ByBitMask);
    smd->SetEnumPartialCellMode(avtScalarMetaData::Dissect);
    md->Add(smd);

    vtkStringArray *groupColors = vtkStringArray::SafeDownCast(
        ds->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_AggregateGroupColors"));
    vtkStringArray *groupNamesTmp = vtkStringArray::SafeDownCast(
        ds->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_AggregateGroupNames"));
    vtkStringArray *groupNames = vtkStringArray::New();
    for (int n = 0; n < (int) groupNamesTmp->GetNumberOfTuples(); n++)
    {
        std::string str = groupNamesTmp->GetValue(n);
        std::replace(str.begin(), str.end(), ' ', '_');
        groupNames->InsertNextValue(str);
    }

    if (!groupColors || !groupNames) return;
    if (groupColors->GetNumberOfTuples() != groupNames->GetNumberOfTuples()) return;

    vtkStringArray *colorNames = vtkStringArray::SafeDownCast(
        ds->GetCellData()->GetAbstractArray("_vtkVisItOBJReader_ColorNames"));
    vtkFloatArray *colorValues = vtkFloatArray::SafeDownCast(
        ds->GetCellData()->GetArray("_vtkVisItOBJReader_RGBValues"));
    std::vector<std::string> matnames;
    std::vector<std::string> matcolors;
    bool valid = true;
    for (std::set<std::string>::const_iterator cit = aggregatedGroupNames.begin();
        cit != aggregatedGroupNames.end(); cit++)
    {
        char const *hchars = "0123456789ABCDEFabcdef#";

        /* specify material names in order of their existence in aggregatedGroupNames */
        matnames.push_back(*cit);
        int groupIndex = groupNames->LookupValue(*cit);
        if (groupIndex < 0)
        {
            debug5 << "Invalidating material object due to negative "
                   << "index for group name \"" << *cit << "\"" << endl;
            valid = false;
            continue;
        }

        /* Special non-OBJ format convenience. If the associated color for this group
           is of the form '#HHHHHH' where 'H' is a hex digit, then treat it as the
           rgb value itself */
        std::string groupColor = groupColors->GetValue(groupIndex);
        if (groupColor[0] == '#' && groupColor.find_first_not_of(hchars) == std::string::npos)
        {
            matcolors.push_back(groupColor);
            continue;
        }

        /* Given the name of the color, lookit up in the colorNames array and get its index */
        int colorIndex = (int) colorNames->LookupValue(groupColors->GetValue(groupIndex));
        if (colorIndex < 0)
        {
            debug5 << "Invalidating material object due to negative "
                   << "index for color name \"" << groupColors->GetValue(groupIndex)<< "\"" << endl;
            valid = false;
            continue;
        }

        /* Get float rgb values from colorValues */
        float rgb[3];
        colorValues->GetTypedTuple(colorIndex, rgb);

        /* Define X windows #RRGGBB color string format */
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "#%02X%02X%02X", (int) (rgb[0]*255), (int) (rgb[1]*255),
            (int) (rgb[2]*255));
        matcolors.push_back(tmp);
    }

    avtMaterialMetaData *mmd = new avtMaterialMetaData("GroupsAsMaterials", meshname,
        groupNames->GetNumberOfTuples(), matnames, matcolors);
    mmd->validVariable = valid;
    groupNames->Delete();

    md->Add(mmd);
}

// ****************************************************************************
//  Method: avtWavefrontOBJFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta data.
//
//  Programmer: Hank Childs
//  Creation:   February 14, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Kathleen Bonnell, Tue Mar 19 08:27:40 PST 2002
//    vtkTCoords has been deprecated in VTK 4.0, use vtkDataArray instead. 
//
// ****************************************************************************

void
avtWavefrontOBJFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (! readInDataset)
    {
        ReadInDataset();
    }

    int spat = 3;
    int topo = 2;

    avtMeshType type = AVT_SURFACE_MESH;;

    double bounds[6];
    dataset->GetBounds(bounds);

    if (bounds[4] == bounds[5])
    {
        spat = 2;
        topo = 2;
    }

    AddMeshToMetaData(md, MESHNAME, type, bounds, 1, 0, spat, topo);

    vtkDataArray *tcoords = dataset->GetPointData()->GetTCoords();
    if (tcoords != NULL)
    {
        nVars = tcoords->GetNumberOfComponents();
        if (nVars != 2)
        {
            EXCEPTION0(ImproperUseException);
        }
        AddScalarVarToMetaData(md, VARNAME1, MESHNAME, AVT_NODECENT);
        AddScalarVarToMetaData(md, VARNAME2, MESHNAME, AVT_NODECENT);
    }

    if (hasGroups)
        AddGroupings(md, dataset, MESHNAME, uniqueGroupNames,
            aggregatedGroupNames);
}
