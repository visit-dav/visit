// ************************************************************************* //
//                            avtEnSightFileFormat.C                          //
// ************************************************************************* //

#include <avtEnSightFileFormat.h>

#include <vector>
#include <string>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDataArraySelection.h>
#include <vtkPointData.h>
#include <vtkStructuredGrid.h>
#include <vtkVisItEnSightReader.h>
#include <vtkVisItEnSightGoldBinaryReader.h>
#include <vtkVisItEnSightGoldReader.h>
#include <vtkVisItEnSight6BinaryReader.h>
#include <vtkVisItEnSight6Reader.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtEnSightFileFormat constructor
//
//  Arguments:
//      fname    The name of the EnSight file.
//
//  Programmer:  Hank Childs
//  Creation:    April 22, 2003
//
// ****************************************************************************

avtEnSightFileFormat::avtEnSightFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    InstantiateReader(fname);
    doneUpdate = false;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::InstantiateReader
//
//  Purpose:
//      Creates an EnSight reader of the appropriate type (6/Gold,
//      Binary/ASCII).
//
//  Programmer: Hank Childs
//  Creation:   April 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 12 16:06:21 PST 2004
//    Use vtkVisIt EnSight readers, until we update to the VTK version that
//    has the ByteOrder fix (Dated January 30, 2004 or later).
//
//    Hank Childs, Fri Jul  9 07:16:59 PDT 2004
//    Account for timestep and file set numbers in case file.  Also allow for
//    wildcards in geometry file names.
//
// ****************************************************************************

void
avtEnSightFileFormat::InstantiateReader(const char *fname)
{
    int  i;

    //
    // Later on we will need separate path and filename components, so separate
    // them out now.
    //
    char tmp[1024];
    strcpy(tmp, fname);
    int lastSlash = -1;
    int len = strlen(tmp);
    for (i = 0 ; i < len ; i++)
        if (tmp[i] == '/' || tmp[i] == '\\')
            lastSlash = i;

    char case_name[1024];
    if (lastSlash == -1)
        strcpy(case_name, tmp);
    else
        strcpy(case_name, tmp + lastSlash + 1);

    char path[1024] = { '\0' };
    if (lastSlash != -1)
    {
        tmp[lastSlash+1] = '\0';
        strcpy(path, tmp);
    }

    //
    // Now read in the case file ourselves and determine if it is EnSight
    // gold or not.
    //
    ifstream case_file(fname);
    if (case_file.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    char type_line[1024] = { '\0' };
    char model_line[1024] = { '\0' };

    while (!case_file.eof())
    {
        char line[1024];
        case_file.getline(line, 1024);
        if (strstr(line, "type:") != NULL)
            strcpy(type_line, line);
        if (strstr(line, "model:") != NULL)
            strcpy(model_line, line);
    }

    if (type_line[0] == '\0')
    {
        // Could not even find the line with 'type'.
        EXCEPTION1(InvalidFilesException, fname);
    }
 
    bool isGold = false;
    if (strstr(type_line, "gold") != NULL)
        isGold = true;
       
    if (model_line[0] == '\0')
    {
        // Could not even find the line with 'model'.
        EXCEPTION1(InvalidFilesException, fname);
    }

    //
    // The model line has syntax: model: [ts] [fs] geo_name
    // We want geo_name.  So just find the start of the last word in the
    // line.
    //
    int len2 = strlen(model_line);
    bool lastWasSpace = false;
    int lastword = -1;
    for (i = 0 ; i < len2 ; i++)
    {
        if (lastWasSpace && !(isspace(model_line[i])))
            lastword = i;
        lastWasSpace = (isspace(model_line[i]) != 0);
    }
    if (lastword <= 0)
        EXCEPTION1(InvalidFilesException, fname);

    //
    // There may be wildcards in the case name.  If so, then substitute in
    // 001 for any *** (or 0001 for ****, etc).  This way we can get the
    // name of a valid geometry file to open.
    //
    char model_name[1024];
    strcpy(model_name, model_line+lastword);
    len2 = strlen(model_name);
    for (i = 0 ; i < len2 ; i++)
    {
        if (model_name[i] == '*')
        {
            if (i+1 < len && model_name[i+1] == '*')
                model_name[i] = '0';
            else
                model_name[i] = '1';
        }
    }
    char geo_filename[1024];
    sprintf(geo_filename, "%s%s", path, model_name);
 
    ifstream geo_file(geo_filename);
    if (geo_file.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    char buff[256];
    geo_file.read(buff, 256);
    
    bool isBinary = false;
    int bin_str_len = strlen("Binary");
    int end = 256 - bin_str_len;
    for (i = 0 ; i < end ; i++)
        if (strncmp(buff + i, "Binary", bin_str_len) == 0)
            isBinary = true;

    if (isBinary)
        debug3 << "Identified file as EnSight binary" << endl;
    else
        debug3 << "Identified file as EnSight ASCII" << endl;
    if (isGold)
        debug3 << "Identified file as EnSight Gold" << endl;
    else
        debug3 << "Identified file as EnSight 6" << endl;

    if (isBinary)
        if (isGold)
            reader = vtkVisItEnSightGoldBinaryReader::New();
        else
            reader = vtkVisItEnSight6BinaryReader::New();
    else
        if (isGold)
            reader = vtkVisItEnSightGoldReader::New();
        else
            reader = vtkVisItEnSight6Reader::New();
   
    reader->SetCaseFileName(case_name);
    if (path[0] != '\0')
        reader->SetFilePath(path);
}


// ****************************************************************************
//  Method: avtEnSightFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   April 22, 2003
//
// ****************************************************************************

avtEnSightFileFormat::~avtEnSightFileFormat()
{
    if (reader != NULL)
    {
        reader->Delete();
        reader = NULL;
    }
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::RegisterVariableList
//
//  Purpose:
//      Registers the variable list with the reader.  This will allow us to
//      read in all of the variables at the same time as the mesh.  (The
//      VTK EnSight reader must read in the mesh every time anything is
//      read in -- if we were to read in the variables one at a time, it
//      would force us to read in the mesh each time.)
//
//  Programmer: Hank Childs
//  Creation:   April 23, 2003
//
//  Modifications:
//    Kathleen Bonnell, Thu Feb 12 16:06:21 PST 2004
//    Reader's access to PointData and CellData has changed.
//
//    Hank Childs, Fri Jul  9 08:01:47 PDT 2004
//    Allow for "parts" to be specified as a variable.
//
// ****************************************************************************

void
avtEnSightFileFormat::RegisterVariableList(const char *primVar,
                                           const vector<CharStrRef> &vars2nd)
{
    int   i, j;

    reader->SetReadAllVariables(0);
    reader->GetPointDataArraySelection()->RemoveAllArrays();
    reader->GetCellDataArraySelection()->RemoveAllArrays();
    
    vector<const char *> vars;
    vars.push_back(primVar);
    for (i = 0 ; i < vars2nd.size() ; i++)
        vars.push_back(*(vars2nd[i]));
 
    //
    // Loop through all of the variables and add the ones we are interested in.
    //
    for (j = 0 ; j < vars.size() ; j++)
    {
        if (strcmp(vars[j], "mesh") == 0)
            continue;
        if (strcmp(vars[j], "parts") == 0)
            continue;

        const char *name = vars[j];

        bool isNodal = false;
        bool foundVar = false;
        if (!foundVar)
        {
            int nsn = reader->GetNumberOfScalarsPerNode();
            for (i = 0 ; i < nsn ; i++)
            {
                const char *desc = reader->GetDescription(i, 
                                       vtkVisItEnSightReader::SCALAR_PER_NODE);
                if (strcmp(name, desc) == 0)
                {
                    isNodal = true;
                    foundVar = true;
                    break;
                }
            }
        }
        if (!foundVar)
        {
            int nsz = reader->GetNumberOfScalarsPerElement();
            for (i = 0 ; i < nsz ; i++)
            {
                const char *desc = reader->GetDescription(i, 
                                    vtkVisItEnSightReader::SCALAR_PER_ELEMENT);
                if (strcmp(name, desc) == 0)
                {
                    isNodal = false;
                    foundVar = true;
                    break;
                }
            }
        }
        if (!foundVar)
        {
            int nsn = reader->GetNumberOfVectorsPerNode();
            for (i = 0 ; i < nsn ; i++)
            {
                const char *desc = reader->GetDescription(i, 
                                       vtkVisItEnSightReader::VECTOR_PER_NODE);
                if (strcmp(name, desc) == 0)
                {
                    isNodal = true;
                    foundVar = true;
                    break;
                }
            }
        }
        if (!foundVar)
        {
            int nsz = reader->GetNumberOfVectorsPerElement();
            for (i = 0 ; i < nsz ; i++)
            {
                const char *desc = reader->GetDescription(i, 
                                    vtkVisItEnSightReader::VECTOR_PER_ELEMENT);
                if (strcmp(name, desc) == 0)
                {
                    isNodal = false;
                    foundVar = true;
                    break;
                }
            }
        }

        if (!foundVar)
            EXCEPTION1(InvalidVariableException, name);

        char *vname = (char *) name; // remove const for VTK.
        
        if (isNodal)
        {
            reader->GetPointDataArraySelection()->EnableArray(vname);
        }
        else 
        { 
            reader->GetCellDataArraySelection()->EnableArray(vname);
        }
    }

    doneUpdate = false;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::PrepReader
//
//  Purpose:
//      Sets up the reader for the right timestep.  Also turns off all of the
//      variables.
//
//  Programmer: Hank Childs
//  Creation:   April 23, 2003
//
// ****************************************************************************

void
avtEnSightFileFormat::PrepReader(int ts)
{
    // We should be doing something about timesteps here.
    //vtkDataArrayCollection *coll = reader->GetTimeSets();
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::GetNTimesteps
//
//  Purpose:
//      Gets the number of timesteps.
//
//  Programmer: Hank Childs
//  Creation:   April 23, 2003
//
// ****************************************************************************

int
avtEnSightFileFormat::GetNTimesteps(void)
{
    return 1;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::GetMesh
//
//  Purpose:
//      Returns the grid associated with a timestep.
//
//  Arguments:
//      ts      The timestep.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   April 22, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Account for multiple parts.
//
// ****************************************************************************

vtkDataSet *
avtEnSightFileFormat::GetMesh(int ts, int dom, const char *name)
{
    if (ts < 0 || ts >= GetNTimesteps())
    {
        EXCEPTION2(BadIndexException, ts, GetNTimesteps());
    }

    if (strcmp(name, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    PrepReader(ts);

    if (!doneUpdate)
    {
        reader->Update();
        doneUpdate = true;
    }
    vtkDataSet *rv = (vtkDataSet *) reader->GetOutput(dom)->NewInstance();
    rv->CopyStructure(reader->GetOutput(dom));

    return rv;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      ts        The timestep.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     April 22, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Account for multiple parts.  Also, allow for multiple variables.
//
// ****************************************************************************

vtkDataArray *
avtEnSightFileFormat::GetVar(int ts, int dom, const char *name)
{
    if (ts < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    PrepReader(ts);

    if (!doneUpdate)
    {
        reader->Update();
        doneUpdate = true;
    }

    vtkDataArray *rv = NULL;
    if (reader->GetOutput(dom)->GetPointData()->GetArray(name) != NULL)
    {
        vtkDataArray *dat = 
                       reader->GetOutput(dom)->GetPointData()->GetArray(name);
        rv = dat;
        rv->Register(NULL);
    }
    else if (reader->GetOutput(dom)->GetCellData()->GetArray(name) != NULL)
    {
        vtkDataArray *dat =
                        reader->GetOutput(dom)->GetCellData()->GetArray(name);
        rv = dat;
        rv->Register(NULL);
    }
    else
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the vector variable associated with a domain number.
//
//  Arguments:
//      ts        The timestep.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     April 22, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Account for multiple parts.  Also allow for the reading of multiple
//    variables.
//
// ****************************************************************************

vtkDataArray *
avtEnSightFileFormat::GetVectorVar(int ts, int dom, const char *name)
{
    if (ts < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    PrepReader(ts);

    if (!doneUpdate)
    {
        reader->Update();
        doneUpdate = true;
    }

    vtkDataArray *rv = NULL;
    if (reader->GetOutput(dom)->GetPointData()->GetArray(name) != NULL)
    {
        vtkDataArray *dat = 
                       reader->GetOutput(dom)->GetPointData()->GetArray(name);
        rv = dat;
        rv->Register(NULL);
    }
    else if (reader->GetOutput(dom)->GetCellData()->GetArray(name) != NULL)
    {
        vtkDataArray *dat =
                        reader->GetOutput(dom)->GetCellData()->GetArray(name);
        rv = dat;
        rv->Register(NULL);
    }
    else
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    return rv;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this EnSight file.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Jul  9 07:37:46 PDT 2004
//    Add support for multiple blocks.
//
//    Mark C. Miller, Tue May 17 18:48:38 PDT 2005
//    Added timeState arg to satisfy new interface
//
// ****************************************************************************

void
avtEnSightFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
    int timeState)
{
    reader->ExecuteInformation();

    // Need this to get number of outputs.
    reader->SetReadAllVariables(0);
    reader->GetPointDataArraySelection()->RemoveAllArrays();
    reader->GetCellDataArraySelection()->RemoveAllArrays();
    reader->Update(); 

    int  i;

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = reader->GetNumberOfOutputs();
    mesh->blockOrigin = 1;
    mesh->blockTitle = "parts";
    mesh->blockPieceName = "part";
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    for (i = 0 ; i < reader->GetNumberOfScalarsPerNode() ; i++)
    {
        const char *name = reader->GetDescription(i, 
                                       vtkVisItEnSightReader::SCALAR_PER_NODE);
        AddScalarVarToMetaData(md, name, "mesh", AVT_NODECENT);
    }

    for (i = 0 ; i < reader->GetNumberOfScalarsPerElement() ; i++)
    {
        const char *name = reader->GetDescription(i, 
                                    vtkVisItEnSightReader::SCALAR_PER_ELEMENT);
        AddScalarVarToMetaData(md, name, "mesh", AVT_ZONECENT);
    }

    for (i = 0 ; i < reader->GetNumberOfVectorsPerNode() ; i++)
    {
        const char *name = reader->GetDescription(i, 
                                       vtkVisItEnSightReader::VECTOR_PER_NODE);
        AddVectorVarToMetaData(md, name, "mesh", AVT_NODECENT);
    }

    for (i = 0 ; i < reader->GetNumberOfVectorsPerElement() ; i++)
    {
        const char *name = reader->GetDescription(i, 
                                    vtkVisItEnSightReader::VECTOR_PER_ELEMENT);
        AddVectorVarToMetaData(md, name, "mesh", AVT_ZONECENT);
    }
}


