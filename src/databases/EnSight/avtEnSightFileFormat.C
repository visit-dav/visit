// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtEnSightFileFormat.C                         //
// ************************************************************************* //

#include <avtEnSightFileFormat.h>

#include <vector>
#include <string>

#include <vtkCellData.h>
#include <vtkCompositeDataSet.h>
#include <vtkDataArray.h>
#include <vtkDataArrayCollection.h>
#include <vtkDataArraySelection.h>
#include <vtkDataSet.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkEnSight6BinaryReader.h>
#include <vtkEnSight6Reader.h>
#include <vtkEnSightGoldBinaryReader.h>
#include <vtkEnSightGoldReader.h>
#include <vtkEnSightReader.h>
#include <vtkGenericEnSightReader.h>
#include <vtkImageData.h>
#include <vtkMultiBlockDataSet.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>

#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <StringHelpers.h>
using StringHelpers::vstrtonum;


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
//  Modifications:
//    Kathleen Biagas, Thu Aug 10 14:51:53 MST 2017
//    Add support for particles.
//
// ****************************************************************************

avtEnSightFileFormat::avtEnSightFileFormat(const char *fname)
    : avtMTMDFileFormat(fname)
{
    hasParticles = false;
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
//    Jeremy Meredith, Thu Jan  7 12:34:08 EST 2010
//    File reading can generated errors instead of eofs.  Changed the test.
//
//    Cyrus Harrison, Mon Mar 21 13:19:32 PDT 2011
//    Extended the binary file header check to include the strings
//     "binary" && "BINARY"
//
//    Cyrus Harrison, Tue Mar 12 12:43:31 PDT 2013
//    Support FORTAN output files (lines padded to end with spaces).
//    Use std strings to avoid indexing bugs.
//
//    Burlen Loring, Mon Jul 14 14:44:40 PDT 2014
//    fix out of bounds index into string during wildcard substitution.
//
//    Kathleen Biagas, Tue Apr 26 08:05:53 PDT 2016
//    Utilize 'filename  start number' (if present) when substituting
//    wildcards for geometry name.
//
//    Kathleen Biagas, Thu Aug 10 14:51:53 MST 2017
//    Add support for particles.
//
// ****************************************************************************

void
avtEnSightFileFormat::InstantiateReader(const char *fname_c)
{
    //
    // Later on we will need separate path and filename components, so separate
    // them out now.
    //

    string fname(fname_c);
    string path;
    string case_name;

    size_t last_slash_pos = fname.rfind('/');
    if (last_slash_pos == std::string::npos)
        last_slash_pos = fname.rfind('\\');

    if(last_slash_pos == std::string::npos)
    {
        case_name = fname;
    }
    else
    {
        case_name = fname.substr(last_slash_pos+1);
        path      = fname.substr(0,last_slash_pos+1);
    }


    //
    // Now read in the case file ourselves and determine if it is EnSight
    // gold or not.
    //

    ifstream case_file(fname.c_str());
    if (case_file.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }


    std::string line;
    std::string type_line;
    std::string model_line;
    int geomFileNameStart = -1;

    while (case_file.good())
    {
        // use std::string 'getline'
        getline(case_file,line);
        StringHelpers::rtrim(line);
        if(line.find("type:") != std::string::npos)
            type_line = line;
        if(line.find("model:") != std::string::npos)
            model_line = line;
        if(line.find("measured:") != std::string::npos)
            hasParticles = true;
        if(line.find("filename start number:") != std::string::npos)
        {
            std::string fileStart = line.substr(line.find(":")+1);
            geomFileNameStart = vstrtonum<int>(fileStart.c_str());
        }
    }

    if(type_line.empty())
    {
        // Could not even find the line with 'type'.
        EXCEPTION1(InvalidFilesException, fname);
    }

    bool isGold = false;

    if (type_line.find("gold") != std::string::npos)
        isGold = true;

    if(model_line.empty())
    {
        // Could not even find the line with 'model'.
        EXCEPTION1(InvalidFilesException, fname);
    }

    //
    // The model line has syntax: model: [ts] [fs] geo_name
    // We want geo_name.  So just find the start of the last word in the
    // line.
    //

    bool lastWasSpace = false;
    int lastword = -1;
    for (size_t i = 0 ; i < model_line.size() ; i++)
    {
        if (lastWasSpace && !(isspace(model_line[i])))
            lastword = (int)i;
        lastWasSpace = (isspace(model_line[i]) != 0);
    }
    if (lastword <= 0)
        EXCEPTION1(InvalidFilesException, fname);

    //
    // There may be wildcards in the case name.  If so, and we have a valid
    // 'filename start number' then substitute in that number, otherwise,
    // substitute 001 for any *** (or 0001 for ****, etc).  This way we can
    // get the name of a valid geometry file to open.
    //

    string model_name = model_line.substr(lastword);
    if(geomFileNameStart != -1)
    {
        int numStars = 0;
        std::string model_begin, model_end;

        for (size_t i = 0 ; i < model_name.size(); ++i)
        {
            if (model_name[i] == '*')
            {
                numStars++;
            }
            else if (numStars > 0)
            {
                model_end.append(&model_name[i], 1);
            }
            else
            {
                model_begin.append(&model_name[i], 1);
            }
        }
        if (numStars > 0)
        {
            char format[12];
            snprintf(format, 12, "%s%d%s", "%0", numStars, "d");
            char model_mid[12];
            snprintf(model_mid, 12, format, geomFileNameStart);
            model_name = model_begin + std::string(model_mid) + model_end;
        }
    }
    else
    {
        debug3 << "avtEnSightFileFormat::InstantiateReader, did not read 'file"
               << " start number', guesstimating first geometry file name." << endl;
        for (size_t i = 0 ; i < model_name.size(); ++i)
        {
            if (model_name[i] == '*')
            {
                if (i+1 < model_name.size() && model_name[i+1] == '*')
                    model_name[i] = '0';
                else
                    model_name[i] = '1';
            }
        }
    }

    string geo_filename = path + model_name;
    debug5 << "geometry file name: \"" << geo_filename << "\"" <<endl;
    ifstream geo_file(geo_filename.c_str());
    if (geo_file.fail())
    {
        EXCEPTION1(InvalidFilesException, fname);
    }

    char buff[257] =  { '\0' };
    geo_file.read(buff, 256);
    string buff_str(buff);

    bool isBinary = false;
    if( buff_str.find("Binary") != std::string::npos ||
        buff_str.find("BINARY") != std::string::npos ||
        buff_str.find("binary") != std::string::npos)
    {
        isBinary = true;
    }

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
            reader = vtkEnSightGoldBinaryReader::New();
        else
            reader = vtkEnSight6BinaryReader::New();
    else
        if (isGold)
            reader = vtkEnSightGoldReader::New();
        else
            reader = vtkEnSight6Reader::New();

    debug5 << "case file name: \"" << case_name << "\"" <<endl;
    debug5 << "file path:      \"" << path << "\"" <<endl;

    reader->SetCaseFileName(case_name.c_str());

    if (!path.empty())
        reader->SetFilePath(path.c_str());
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
//    Hank Childs, Thu Feb 23 09:54:50 PST 2012
//    Automatically add arrays for materials if they are available.
//    This may lead to unnecessary reads (when materials are present, but not
//    being used), but doing better takes big effort.
//
//    Kathleen Biagas, Thu Aug 10 14:51:53 MST 2017
//    Add support for particles.
//
// ****************************************************************************

void
avtEnSightFileFormat::RegisterVariableList(const char *primVar,
                                           const vector<CharStrRef> &vars2nd)
{
    reader->SetReadAllVariables(0);
    reader->GetPointDataArraySelection()->RemoveAllArrays();
    reader->GetCellDataArraySelection()->RemoveAllArrays();

    vector<const char *> vars;
    vars.push_back(primVar);
    for (size_t i = 0 ; i < vars2nd.size() ; i++)
        vars.push_back(*(vars2nd[i]));

    if (matnames.size() > 0)
    {
        size_t numRealMats = matnames.size()-1;
        for (size_t i = 0 ; i < numRealMats ; i++)
        {
            vars.push_back(matnames[i].c_str());
        }
    }

    //
    // Loop through all of the variables and add the ones we are interested in.
    //
    for (size_t j = 0 ; j < vars.size() ; j++)
    {
        if (strcmp(vars[j], "mesh") == 0)
            continue;
        if (strcmp(vars[j], "particles") == 0)
            continue;
        if (strcmp(vars[j], "materials") == 0)
            continue;
        if (strcmp(vars[j], "parts") == 0)
            continue;
        if (strcmp(vars[j], "parts(mesh)") == 0)
            continue;
        if (strcmp(vars[j], "parts(particles)") == 0)
            continue;

        string name(vars[j]);
        if (hasParticles)
        {
            // vars are split up by mesh name, eg mesh/var1 particles/pvar1
            size_t pos = name.rfind('/');
            if (pos != string::npos)
                name = name.substr(pos+1);
        }
        bool isNodal = false;
        bool foundVar = false;
        if (!foundVar)
        {
            int nsn = reader->GetNumberOfScalarsPerNode();
            for (int i = 0 ; i < nsn ; i++)
            {
                const char *desc = reader->GetDescription(i,
                                       vtkEnSightReader::SCALAR_PER_NODE);
                if (name == desc)
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
            for (int i = 0 ; i < nsz ; i++)
            {
                const char *desc = reader->GetDescription(i,
                                    vtkEnSightReader::SCALAR_PER_ELEMENT);
                if (name == desc)
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
            for (int i = 0 ; i < nsn ; i++)
            {
                const char *desc = reader->GetDescription(i,
                                       vtkEnSightReader::VECTOR_PER_NODE);
                if (name == desc)
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
            for (int i = 0 ; i < nsz ; i++)
            {
                const char *desc = reader->GetDescription(i,
                                    vtkEnSightReader::VECTOR_PER_ELEMENT);
                if (name == desc)
                {
                    isNodal = false;
                    foundVar = true;
                    break;
                }
            }
        }
        if (hasParticles)
        {
            if (!foundVar)
            {
                int nsz = reader->GetNumberOfScalarsPerMeasuredNode();
                for (int i = 0 ; i < nsz ; i++)
                {
                    const char *desc = reader->GetDescription(i,
                           vtkEnSightReader::SCALAR_PER_MEASURED_NODE);
                    if (name == desc)
                    {
                        isNodal = true;
                        foundVar = true;
                        break;
                    }
                }
            }
            if (!foundVar)
            {
                int nsz = reader->GetNumberOfVectorsPerMeasuredNode();
                for (int i = 0 ; i < nsz ; i++)
                {
                    const char *desc = reader->GetDescription(i,
                           vtkEnSightReader::VECTOR_PER_MEASURED_NODE);
                    if (name == desc)
                    {
                        isNodal = true;
                        foundVar = true;
                        break;
                    }
                }
            }
        }

        if (!foundVar)
            EXCEPTION1(InvalidVariableException, name);

        if (isNodal)
        {
            reader->GetPointDataArraySelection()->EnableArray(name.c_str());
        }
        else
        {
            reader->GetCellDataArraySelection()->EnableArray(name.c_str());
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
//  Modifications:
//    Brad Whitlock, Tue Jun 27 10:04:51 PDT 2006
//    I made the method set the time state that we want to look at.
//
//    Brad Whitlock, Tue Sep 26 09:37:44 PDT 2006
//    Set the doneUpdate flag to false so the reader gets the right values
//    after setting the time value.
//
// ****************************************************************************

void
avtEnSightFileFormat::PrepReader(int ts)
{
    // Get the times.
    const char *mName = "avtEnSightFileFormat::PrepReader: ";
    debug4 << mName << "start" << endl;
    std::vector<double> times;
    GetTimes(times);

    // Tell the reader which time we want to look at .
    debug4 << mName << "Setting time state to " << ts << " time="
           << times[ts] << endl;
    reader->SetTimeValue(times[ts]);
    doneUpdate = false;
    debug4 << mName << "end" << endl;
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
//  Modifications:
//    Brad Whitlock, Tue Jun 27 10:04:11 PDT 2006
//    I made it return the real number of time states.
//
// ****************************************************************************

int
avtEnSightFileFormat::GetNTimesteps(void)
{
    const char *mName = "avtEnSightFileFormat::GetNTimesteps: ";

    debug4 << mName << "start" << endl;
    std::vector<double> times;
    GetTimes(times);

    debug4 << mName << "end. returning " << times.size() << endl;
    return (int)times.size();
}

// ****************************************************************************
// Method: avtEnSightFileFormat::GetTimes
//
// Purpose:
//   Returns the list of times from the file.
//
// Arguments:
//   times : The list of times that we want to return.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jun 27 10:05:13 PDT 2006
//
// Modifications:
//
//   Hank Childs, Mon Jan 29 09:24:16 PST 2007
//   If there are no times, then spoof one time slice.
//
// ****************************************************************************

void
avtEnSightFileFormat::GetTimes(std::vector<double> &times)
{
    times.clear();

    // Update the reader so it gets the times. This is really the first
    // plugin method called so we need to do the update.
    if (!doneUpdate)
    {
        reader->Update();
        doneUpdate = true;
    }

    // Iterate through the time arrays for the reader and package them
    // up into a vector.
    debug4 << "avtEnSightFileFormat::GetTimes {";
    vtkDataArrayCollection *tsc = reader->GetTimeSets();
    for(int i = 0; i < tsc->GetNumberOfItems(); ++i)
    {
        vtkDataArray *t = tsc->GetItem(i);
        for(int j = 0; j < t->GetNumberOfTuples(); ++j)
        {
            times.push_back(t->GetComponent(j,0));
            debug4 << ", " << t->GetComponent(j,0);
        }
    }
    if (times.size() == 0)
    {
        times.push_back(0.);
        debug4 << " <Spoofed one time slice ... no time info in file>" << endl;
    }
    debug4 << "}" << endl;
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::ConvertDomainToBlock
//
//  Purpose:
//      Converts domain id to the correct block Id for the mesh.
//
//  Arguments:
//      dom         The domain.
//      meshName    The mesh name.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 10, 2017
//
//  Modifications:
//
// ****************************************************************************

int
avtEnSightFileFormat::ConvertDomainToBlock(int dom, const string &meshName)
{
    int blockId = dom;
    if (hasParticles)
    {
        if (meshName == "mesh")
           blockId = stdBlockIds[dom];
        else if (meshName == "particles")
           blockId = particleBlockIds[dom];
    }
    return blockId;
}

// ****************************************************************************
//  Method: ConvertImageToRGrid
//
//  Purpose:
//      Converts vtkImageData to a vtkRectilinearGrid
//
//  Arguments:
//      image       the image data to convert
//
//  Programmer: Kathleen Biagas
//  Creation:   August 10, 2017
//
//  Modifications:
//
// ****************************************************************************

vtkDataSet *
ConvertImageToRGrid(vtkImageData *image)
{
    int wholeDims[3];
    double spacing[3];
    double wholeOrigin[3];
    image->GetDimensions(wholeDims);
    image->GetSpacing(spacing);
    image->GetOrigin(wholeOrigin);

    int pieceDims[3];
    double pieceOrigin[3];
    pieceDims[0] = wholeDims[0];
    pieceDims[1] = wholeDims[1];
    pieceDims[2] = wholeDims[2];
    pieceOrigin[0] = wholeOrigin[0];
    pieceOrigin[1] = wholeOrigin[1];
    pieceOrigin[2] = wholeOrigin[2];

    vtkFloatArray *x = vtkFloatArray::New();
    x->SetNumberOfComponents(1);
    x->SetNumberOfTuples(pieceDims[0]);
    vtkFloatArray *y = vtkFloatArray::New();
    y->SetNumberOfComponents(1);
    y->SetNumberOfTuples(pieceDims[1]);
    vtkFloatArray *z = vtkFloatArray::New();
    z->SetNumberOfComponents(1);
    z->SetNumberOfTuples(pieceDims[2]);

    vtkRectilinearGrid *outRG = vtkRectilinearGrid::New();
    outRG->SetDimensions(pieceDims);
    outRG->SetXCoordinates(x);
    outRG->SetYCoordinates(y);
    outRG->SetZCoordinates(z);
    x->Delete();
    y->Delete();
    z->Delete();

    int i;
    float *ptr = x->GetPointer(0);
    for (i = 0; i < pieceDims[0]; i++, ptr++)
        *ptr = pieceOrigin[0] + i * spacing[0];

    ptr = y->GetPointer(0);
    for (i = 0; i < pieceDims[1]; i++, ptr++)
        *ptr = pieceOrigin[1] + i * spacing[1];

    ptr = z->GetPointer(0);
    for (i = 0; i < pieceDims[2]; i++, ptr++)
        *ptr = pieceOrigin[2] + i * spacing[2];

    return outRG;
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
//    Kathleen Biagas, Thu Aug 10 17:44:31 PDT 2017
//    Convert image data to rectilinear grids.
//
// ****************************************************************************

vtkDataSet *
avtEnSightFileFormat::GetMesh(int ts, int dom, const char *name)
{
    if (ts < 0 || ts >= GetNTimesteps())
    {
        EXCEPTION2(BadIndexException, ts, GetNTimesteps());
    }

    if (strcmp(name, "mesh") != 0  && strcmp(name, "particles") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    PrepReader(ts);

    if (!doneUpdate)
    {
        reader->Update();
        doneUpdate = true;
    }

    vtkDataSet *block = vtkDataSet::SafeDownCast(
        reader->GetOutput()->GetBlock(ConvertDomainToBlock(dom, name)));

    vtkDataSet *rv = NULL;
    if(block != NULL)
    {
        if (block->GetDataObjectType() == VTK_IMAGE_DATA)
        {
            rv = ConvertImageToRGrid((vtkImageData*) block);
        }
        else
        {
           rv = block->NewInstance();
           rv->CopyStructure(block);
        }
    }
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
//    Kathleen Biagas, Thu Aug 10 15:18:51 MST 2017
//    Add support for particles, and vars not defined on all parts.
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

    int blockId = dom;
    string vname(name);
    if (hasParticles)
    {
        // vars are split up by mesh name, eg mesh/var1 particles/pvar1
        size_t pos = vname.rfind('/');
        if (pos != string::npos)
        {
            string mesh(vname.substr(0, pos));
            vname = vname.substr(pos+1);
            blockId = ConvertDomainToBlock(dom, mesh);
        }
    }
    vtkDataArray *rv = NULL;
    vtkDataSet *block = (vtkDataSet *) reader->GetOutput()->GetBlock(blockId);
    if (block->GetPointData()->GetArray(vname.c_str()) != NULL)
    {
        vtkDataArray *dat = block->GetPointData()->GetArray(vname.c_str());
        rv = dat;
        rv->Register(NULL);
    }
    else if (block->GetCellData()->GetArray(vname.c_str()) != NULL)
    {
        vtkDataArray *dat = block->GetCellData()->GetArray(vname.c_str());
        rv = dat;
        rv->Register(NULL);
    }
    else
    {
        // We are looking for valid var that isn't defined on this
        // part, so returning NULL is acceptable.
        debug3 << "avtEnSightFileFormat could not find scalar var " << vname
               << " for part " << blockId << endl;
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
//    Kathleen Biagas, Thu Aug 10 15:18:51 MST 2017
//    Add support for particles, and vars not defined on all parts.
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

    int blockId = dom;
    string vname(name);
    if (hasParticles)
    {
        // vars are split up by mesh name, eg mesh/var1 particles/pvar1
        size_t pos = vname.rfind('/');
        if (pos != string::npos)
        {
            string mesh(vname.substr(0, pos));
            vname = vname.substr(pos+1);
            blockId = ConvertDomainToBlock(dom, mesh);
        }
    }

    vtkDataArray *rv = NULL;
    vtkDataSet *block = (vtkDataSet *) reader->GetOutput()->GetBlock(blockId);
    if (block->GetPointData()->GetArray(vname.c_str()) != NULL)
    {
        vtkDataArray *dat = block->GetPointData()->GetArray(vname.c_str());
        rv = dat;
        rv->Register(NULL);
    }
    else if (block->GetCellData()->GetArray(vname.c_str()) != NULL)
    {
        vtkDataArray *dat = block->GetCellData()->GetArray(vname.c_str());
        rv = dat;
        rv->Register(NULL);
    }
    else
    {
        // We are looking for valid var that isn't defined on this
        // part, so returning NULL is acceptable.
        debug3 << "avtEnSightFileFormat could not find vector var " << vname
               << " for part " << blockId << endl;
    }

    return rv;
}


// ****************************************************************************
//  Method: MeshTypeFromObjectType 
//
//  Purpose:
//      Helper function to convert a VTK dataset type to avtMeshType. 
//      An in-exhaustive conversion, as only type supported by EnSight
//      format are considered.
//
//  Arguments:
//      vtk_object_type   The vtk object type.
//
//  Programmer: Kathleen Biagas 
//  Creation:   August 10, 2017 
//
//  Modifications:
//
// ****************************************************************************

avtMeshType
MeshTypeFromObjectType(int vtk_object_type)
{
    avtMeshType mt = AVT_UNKNOWN_MESH;
    switch (vtk_object_type)
    {
        case VTK_POLY_DATA:
             // This may need to be changed to POINT_MESH
             mt = AVT_SURFACE_MESH;
             break;
        case VTK_UNSTRUCTURED_GRID:
             mt = AVT_UNSTRUCTURED_MESH;
             break;
        case VTK_STRUCTURED_GRID:
             mt = AVT_CURVILINEAR_MESH;
             break;
        // we convert image data to rectilinear grid
        case VTK_IMAGE_DATA:
        case VTK_RECTILINEAR_GRID:
             mt= AVT_RECTILINEAR_MESH;
             break;
        default:
             mt = AVT_UNKNOWN_MESH;
             break;
    }
    return mt;
}


// ****************************************************************************
//  Method: GetMeshType
//
//  Purpose:
//      Helper function to retrieve the mesh type from a multiblock dataset. 
//
//  Notes:  Looks only at block 0. Assumes the rest conform (particles have
//          already been considered).
//
//  Arguments:
//      mbds    The multiblock dataset.
//  
//  Programmer: Kathleen Biagas
//  Creation:   August 10, 2017 
//
//  Modifications:
//
// ****************************************************************************

avtMeshType
GetMeshType(vtkMultiBlockDataSet *mbds)
{
    if (mbds == NULL)
        return AVT_UNKNOWN_MESH;

    vtkDataObject *block = mbds->GetBlock(0);
    if (block != NULL)
    {
        if (block->GetDataObjectType() == VTK_MULTIBLOCK_DATA_SET)
        {
            return GetMeshType(vtkMultiBlockDataSet::SafeDownCast(block));
        }
        else
        {
            return MeshTypeFromObjectType(block->GetDataObjectType()); 
        }
    }
    return AVT_UNKNOWN_MESH;
}


// ****************************************************************************
//  Method: DistinguishBlockTypes
//
//  Purpose:
//      Helper function to distinguish which blocks are standard, and
//      which are particle.  Stores ids in appropriate vector for later use.
//
//  Arguments:
//     mbds     The multibblock dataset.
//     stdIds   Storage for 'standard' (non particle) block ids.
//     particleIds   Storage for particle block ids.
//     index    For recursive calls.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 10, 2017
//
//  Modifications:
//
// ****************************************************************************

void
DistinguishBlockTypes(vtkMultiBlockDataSet *mbds,
    vector<int> &stdIds, vector<int> &particleIds, int index)
{
    if (mbds == NULL)
        return;

    int nb = mbds->GetNumberOfBlocks();
    for (int i = 0; i < nb; ++i)
    {
        vtkDataObject *block = mbds->GetBlock(i);
        if (block != NULL)
        {
            if (block->GetDataObjectType() == VTK_MULTIBLOCK_DATA_SET)
            {
                DistinguishBlockTypes(
                     vtkMultiBlockDataSet::SafeDownCast(block),
                     stdIds, particleIds, i);
            }
            else
            {
                if (block->GetDataObjectType() == VTK_POLY_DATA)
                {
                    particleIds.push_back(index + i);
                }
                else
                {
                    stdIds.push_back(index + i);
                } 
            }
        }
    }
}


// ****************************************************************************
//  Method: GetBlockNames
//
//  Purpose:
//      Helper function to retrieve part names from the dataset if available.
//
//  Arguments:
//    mbds      The multiblock dataset.
//    ids       The block ids for the datset.
//    names     Storage for the block names.
//
//  Programmer: Kathleen Biagas
//  Creation:   August 10, 2017
//
//  Modifications:
//
// ****************************************************************************

void
GetBlockNames(vtkMultiBlockDataSet *mbds, vector<int> ids,
              vector<string> &names)
{
    for (size_t i = 0; i < ids.size(); ++i)
    {
        string name;
        if (mbds->HasMetaData(ids[i]))
        {
            if (mbds->GetMetaData(ids[i])->Get(vtkCompositeDataSet::NAME()) != NULL)
                name = mbds->GetMetaData(ids[i])->Get(vtkCompositeDataSet::NAME());
        }
        if (name.empty())
        {
            std::ostringstream ss;
            ss << "part" << ids[i];
            name = ss.str();
        }
        names.push_back(name);
    }
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
//    Hank Childs, Thu Feb 23 09:54:50 PST 2012
//    Add support for materials when "volume_fraction" scalars are present.
//
//    Kathleen Biagas, Thu Aug 10 14:51:53 MST 2017
//    Add support for particle blocks, retrieving block names and actual
//    mesh type.
//
// ****************************************************************************

void
avtEnSightFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md,
                                               int timeState)
{
    // Need this to get number of outputs.
    reader->SetReadAllVariables(0);
    reader->GetPointDataArraySelection()->RemoveAllArrays();
    reader->GetCellDataArraySelection()->RemoveAllArrays();
    reader->Update();

    vtkMultiBlockDataSet *mbds = reader->GetOutput();

    particleBlockIds.clear();
    stdBlockIds.clear();

    DistinguishBlockTypes(mbds, stdBlockIds, particleBlockIds, 0);

    vector< string > blockNames;
    GetBlockNames(mbds, stdBlockIds, blockNames);

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = GetMeshType(mbds);
    mesh->numBlocks = (int) stdBlockIds.size();
    mesh->blockTitle = "parts";
    mesh->blockNames = blockNames;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 3;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    if (hasParticles)
    {
        avtMeshMetaData *pmesh = new avtMeshMetaData;
        pmesh->name = "particles";
        pmesh->meshType = AVT_POINT_MESH;
        pmesh->numBlocks = (int) particleBlockIds.size();
        blockNames.clear();
        GetBlockNames(mbds, particleBlockIds, blockNames);
        pmesh->blockTitle = "parts";
        pmesh->blockNames = blockNames;
        pmesh->spatialDimension = 3;
        pmesh->topologicalDimension = 1;
        pmesh->hasSpatialExtents = false;
        md->Add(pmesh);
    }

    for (int i = 0 ; i < reader->GetNumberOfScalarsPerNode() ; i++)
    {
        const char *name = reader->GetDescription(i,
                                       vtkEnSightReader::SCALAR_PER_NODE);
        string vname(name);
        if (hasParticles)
            vname = string("mesh/") + vname;

        AddScalarVarToMetaData(md, vname, "mesh", AVT_NODECENT);
    }

    if (hasParticles)
    {
        for (int i = 0 ; i < reader->GetNumberOfScalarsPerMeasuredNode() ; i++)
        {
            string vname("particles/");
            vname += reader->GetDescription(i,
                           vtkEnSightReader::SCALAR_PER_MEASURED_NODE);
            AddScalarVarToMetaData(md, vname, "particles", AVT_NODECENT);
        }
    }

    matnames.clear();
    for (int i = 0 ; i < reader->GetNumberOfScalarsPerElement() ; i++)
    {
        const char *name = reader->GetDescription(i,
                                    vtkEnSightReader::SCALAR_PER_ELEMENT);
        string vname(name);
        if (hasParticles)
            vname = string("mesh/") + vname;
        AddScalarVarToMetaData(md, vname, "mesh", AVT_ZONECENT);
        if (strncmp(name, "volume_fraction", strlen("volume_fraction")) == 0)
            matnames.push_back(name);
    }

    for (int i = 0 ; i < reader->GetNumberOfVectorsPerNode() ; i++)
    {
        const char *name = reader->GetDescription(i,
                vtkEnSightReader::VECTOR_PER_NODE);
        string vname(name);
        if (hasParticles)
            vname = string("mesh/") + vname;
        AddVectorVarToMetaData(md, vname, "mesh", AVT_NODECENT);
    }

    if (hasParticles)
    {
        for (int i = 0 ; i < reader->GetNumberOfVectorsPerMeasuredNode() ; i++)
        {
            string vname("particles/");
            vname += reader->GetDescription(i,
                vtkEnSightReader::VECTOR_PER_MEASURED_NODE);
            AddVectorVarToMetaData(md, vname, "particles", AVT_NODECENT);
        }
    }

    for (int i = 0 ; i < reader->GetNumberOfVectorsPerElement() ; i++)
    {
        const char *name = reader->GetDescription(i,
                                    vtkEnSightReader::VECTOR_PER_ELEMENT);
        string vname(name);
        if (hasParticles)
            vname = string("mesh/") + vname;
        AddVectorVarToMetaData(md, vname, "mesh", AVT_ZONECENT);
    }

    if (matnames.size() > 0)
    {
        char str[128];
        snprintf(str, 128, "volume_fraction%ld", matnames.size()+1);
        matnames.push_back(str);
        avtMaterialMetaData *mmd;
        mmd = new avtMaterialMetaData("materials", "mesh",
                                      (int)matnames.size(), matnames);
        md->Add(mmd);
    }
}


// ****************************************************************************
//  Method: avtEnSightFileFormat::GetAuxiliaryData
//
//  Purpose:
//      Reads in material information.
//
//  Programmer: Hank Childs
//  Creation:   February 23, 2012
//
// ****************************************************************************

void *
avtEnSightFileFormat::GetAuxiliaryData(const char *var, int ts, int domain,
                              const char *type, void *, DestructorFunction &df)
{
    if (strcmp(type, AUXILIARY_DATA_MATERIAL) != 0)
        return NULL;

    int nMaterials = (int)matnames.size();

    // Get the material fractions
    std::vector<float *> mats(nMaterials);
    std::vector<vtkFloatArray *> deleteList;
    vtkIdType nCells = 0;
    for (int i = 0; i < nMaterials-1; i++)
    {
        vtkDataArray *arr = GetVar(ts, domain, matnames[i].c_str());
        if (arr == NULL)
        {
            EXCEPTION1(InvalidVariableException, var);
        }
        nCells = arr->GetNumberOfTuples();
        mats[i] = (float *) arr->GetVoidPointer(0);
        deleteList.push_back((vtkFloatArray *) arr);
    }


    // Calculate fractions for additional "missing" material
    float *addMatPtr =  new float[nCells];
    for(vtkIdType cellNo = 0; cellNo < nCells; ++cellNo)
    {
        double frac = 1.0;
        for (int matNo = 0; matNo < nMaterials - 1; ++matNo)
            frac -= mats[matNo][cellNo];
        addMatPtr[cellNo] = frac;
    }
    mats[nMaterials - 1] = addMatPtr;

    // Build the appropriate data structures
    std::vector<int> material_list(nCells);
    std::vector<int> mix_mat;
    std::vector<int> mix_next;
    std::vector<int> mix_zone;
    std::vector<float> mix_vf;

    for (vtkIdType i = 0; i < nCells; ++i)
    {
         // First look for pure materials
        int nmats = 0;
        int lastMat = -1;
        for (int j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] > 0)
            {
                nmats++;
                lastMat = (int)j;
            }
        }

        if (nmats == 1)
        {
            material_list[i] = lastMat;
            continue;
        }

        // For unpure materials, we need to add entries to the tables.
        material_list[i] = -1 * (1 + (int)mix_zone.size());
        for (int j = 0; j < nMaterials; ++j)
        {
            if (mats[j][i] <= 0)
                continue;
            // For each material that's present, add to the tables
            mix_zone.push_back(i);
            mix_mat.push_back(j);
            mix_vf.push_back(mats[j][i]);
            mix_next.push_back((int)mix_zone.size() + 1);
        }

        // When we're done, the last entry is a '0' in the mix_next
        mix_next[mix_next.size() - 1] = 0;
    }

    int mixed_size = (int)mix_zone.size();
    // get pointers to pass to avtMaterial.  Windows will except if
    // an empty std::vector's zeroth item is dereferenced.
    int *ml = NULL, *mixm = NULL, *mixn = NULL, *mixz = NULL;
    float *mixv = NULL;
    if (material_list.size() > 0)
        ml = &(material_list[0]);
    if (mix_mat.size() > 0)
        mixm = &(mix_mat[0]);
    if (mix_next.size() > 0)
        mixn = &(mix_next[0]);
    if (mix_zone.size() > 0)
        mixz = &(mix_zone[0]);
    if (mix_vf.size() > 0)
        mixv = &(mix_vf[0]);

    avtMaterial * mat = new avtMaterial(nMaterials, matnames, nCells, ml,
                                        mixed_size, mixm, mixn, mixz, mixv);

    df = avtMaterial::Destruct;

    delete [] addMatPtr;
    for (size_t i = 0 ; i < deleteList.size() ; i++)
        deleteList[i]->Delete();

    return (void*) mat;
}

