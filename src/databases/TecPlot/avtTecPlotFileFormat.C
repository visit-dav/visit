// ************************************************************************* //
//                            avtTecPlotFileFormat.C                           //
// ************************************************************************* //

#include <avtTecPlotFileFormat.h>

#include <string>

#include <vtkFloatArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkCellTypes.h>
#include <vtkIntArray.h>
#include <vtkIdTypeArray.h>
#include <vtkCellArray.h>
#include <avtDatabaseMetaData.h>
#include <vtkPolyData.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <visitstream.h>

using std::string;
using std::vector;
#ifndef MAX
#define MAX(a,b) ((a)>(b)?(a):(b))
#endif

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::PushBackToken
//
//  Purpose:
//    Needed for faking one-token lookahead.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::PushBackToken(const string &tok)
{
    saved_token = tok;
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::GetNextToken
//
//  Purpose:
//    The scanner.  It probably makes some serious assumptions about tecplot.
//
//  Arguments:
//    none
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
string
avtTecPlotFileFormat::GetNextToken()
{
    // this is where we fake a one-token lookahead
    if (!saved_token.empty())
    {
        string retval = saved_token;
        saved_token = "";
        return retval;
    }

    // oops!  we hit EOF and someone still wanted more.
    if (next_char_eof)
        return "";

    next_char_eol = false;
    token_was_string = false;

    string retval = "";
    if (!next_char_valid)
    {
        next_char = file.get();
        next_char_valid = true;
        if (!file)
        {
            next_char_eof = true;
        }
    }

    // skip inter-token whitespace
    while (!next_char_eof   &&
           (next_char == ' '  ||
            next_char == '\n' ||
            next_char == '\t' ||
            next_char == '='  ||
            next_char == '('  ||
            next_char == ')'  ||
            next_char == ','))
    {
        if (next_char == '\n')
        {
            next_char_eol = true;
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        

        // Ignore blank lines since they don't return a token
        if (next_char_eol)
            return GetNextToken();
    }
     
    if (next_char == '\"')
    {
        token_was_string = true;
        // do strings
        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        
        while (!next_char_eof && next_char != '\"')
        {
            retval += next_char;
            next_char = file.get();
            if (!file)
            {
                next_char_eof = true;
            }
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }
    }
    else
    {
        // do a normal token
        while (!next_char_eof   &&
               (next_char != ' '  &&
                next_char != '\n' &&
                next_char != '\t' &&
                next_char != '='  &&
                next_char != '('  &&
                next_char != ')'  &&
                next_char != ','))
        {
            if (next_char >= 'a' && next_char <= 'z')
                next_char += (int('A')-int('a'));
            retval += next_char;
            next_char = file.get();
            if (!file)
            {
                next_char_eof = true;
            }        
        }
    }

    // skip whitespace to EOL
    while (!next_char_eof   &&
           (next_char == ' '  ||
            next_char == '\n' ||
            next_char == '\t' ||
            next_char == '='  ||
            next_char == '('  ||
            next_char == ')'  ||
            next_char == ','))
    {
        if (next_char == '\n')
        {
            next_char_eol = true;
        }

        next_char = file.get();
        if (!file)
        {
            next_char_eof = true;
        }        

        if (next_char_eol)
            break;
    }
     
    return retval;
}

// ****************************************************************************
//  Method:  ParseNodesPoint
//
//  Purpose:
//    Reads the coordinates and/or nodal data for POINT and FEPOINT formats.
//
//  Arguments:
//    numNodes   the number of nodes
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
vtkPoints*
avtTecPlotFileFormat::ParseNodesPoint(int numNodes)
{
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *)points->GetVoidPointer(0);
    int i;

    for (i=0; i<numNodes*3; i++)
    {
        pts[i] = 0;
    }

    vector<vtkFloatArray*> allScalars;
    vector<float*> allptr;

    for (int v=0; v<numTotalVars; v++)
    {
        if (v==Xindex || v==Yindex || v==Zindex)
        {
            allScalars.push_back(NULL);
            allptr.push_back(NULL);
        }
        else
        {
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numNodes);
            float *ptr = (float *) scalars->GetVoidPointer(0);
            vars[allVariableNames[v]].push_back(scalars);

            allScalars.push_back(scalars);
            allptr.push_back(ptr);
        }
    }

    for (i=0; i<numNodes; i++)
    {
        for (int v = 0; v < numTotalVars; v++)
        {
            float val = atof(GetNextToken().c_str());
            if (v==Xindex)
            {
                pts[3*i + 0] = val;
            }
            else if (v==Yindex)
            {
                pts[3*i + 1] = val;
            }
            else if (v==Zindex)
            {
                pts[3*i + 2] = val;
            }
            else
            {
                allptr[v][i] = val;
            }
        }
    }

    return points;
}

// ****************************************************************************
//  Method:  ParseNodesBlock
//
//  Purpose:
//    Reads the coordinates and/or nodal data for BLOCK and FEBLOCK formats.
//
//  Arguments:
//    numNodes   the number of nodes
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
vtkPoints*
avtTecPlotFileFormat::ParseNodesBlock(int numNodes)
{
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(numNodes);
    float *pts = (float *)points->GetVoidPointer(0);

    for (int i=0; i<numNodes*3; i++)
    {
        pts[i] = 0;
    }

    for (int v = 0; v < numTotalVars; v++)
    {
        if (v==Xindex)
        {
            for (int i=0; i<numNodes; i++)
                pts[3*i + 0] = atof(GetNextToken().c_str());
        }
        else if (v==Yindex)
        {
            for (int i=0; i<numNodes; i++)
                pts[3*i + 1] = atof(GetNextToken().c_str());
        }
        else if (v==Zindex)
        {
            for (int i=0; i<numNodes; i++)
            {
                pts[3*i + 2] = atof(GetNextToken().c_str());
            }
        }
        else
        {
            vtkFloatArray *scalars = vtkFloatArray::New();
            scalars->SetNumberOfTuples(numNodes);
            float *ptr = (float *) scalars->GetVoidPointer(0);
            for (int i=0; i<numNodes; i++)
                ptr[i] = atof(GetNextToken().c_str());
            vars[allVariableNames[v]].push_back(scalars);
        }
    }

    return points;
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ParseElements
//
//  Purpose:
//    Parses the element connectivity arrays.
//
//  Arguments:
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
vtkUnstructuredGrid *
avtTecPlotFileFormat::ParseElements(int numElements, const string &elemType)
{
    vtkUnstructuredGrid *ugrid = vtkUnstructuredGrid::New();

    // construct the cell arrays
    int nelempts = -1;
    int idtype = -1;
    if (elemType == "BRICK")
    {
        nelempts = 8;
        idtype = VTK_HEXAHEDRON;
        topologicalDimension = MAX(topologicalDimension, 3);
    }
    else if (elemType == "TRIANGLE")
    {
        nelempts = 3;
        idtype = VTK_TRIANGLE;
        topologicalDimension = MAX(topologicalDimension, 2);
    }
    else if (elemType == "QUADRILATERAL")
    {
        nelempts = 4;
        idtype = VTK_QUAD;
        topologicalDimension = MAX(topologicalDimension, 2);
    }
    else if (elemType == "TETRAHEDRON")
    {
        nelempts = 4;
        idtype = VTK_TETRA;
        topologicalDimension = MAX(topologicalDimension, 3);
    }
    else
    {
        EXCEPTION2(InvalidFilesException, filename,
                   "Unknown element type for zone");
    }

    vtkIdTypeArray *nlist = vtkIdTypeArray::New();
    nlist->SetNumberOfValues((nelempts+1) * numElements);
    vtkIdType *nl = nlist->GetPointer(0);

    vtkUnsignedCharArray *cellTypes = vtkUnsignedCharArray::New();
    cellTypes->SetNumberOfValues(numElements);
    unsigned char *ct = cellTypes->GetPointer(0);

    vtkIntArray *cellLocations = vtkIntArray::New();
    cellLocations->SetNumberOfValues(numElements);
    int *cl = cellLocations->GetPointer(0);

    int offset = 0;
    for (int c=0; c<numElements; c++)
    {
        *ct++ = idtype;

        *nl++ = nelempts;
        // 1-origin connectivity array
        for (int j=0; j<nelempts; j++)
            *nl++ = atoi(GetNextToken().c_str())-1;
        
        *cl++ = offset;
        offset += nelempts+1;
    }

    vtkCellArray *cells = vtkCellArray::New();
    cells->SetCells(numElements, nlist);
    nlist->Delete();

    ugrid->SetCells(cellTypes, cellLocations, cells);
    cellTypes->Delete();
    cellLocations->Delete();
    cells->Delete();

    return ugrid;
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ParseFEBLOCK
//
//  Purpose:
//    Parses an FEBLOCK zone
//
//  Arguments:
//    numNodes      the number of nodes
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::ParseFEBLOCK(int numNodes, int numElements,
                                   const string &elemType)
{
    vtkPoints *points = ParseNodesBlock(numNodes);
    vtkUnstructuredGrid *ugrid = ParseElements(numElements, elemType);
    ugrid->SetPoints(points);
    points->Delete();

    if (topologicalDimension == 2 || topologicalDimension == 3)
    {
        meshes.push_back(ugrid);
    }
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ParseFEPOINT
//
//  Purpose:
//    Parses an FEPOINT zone
//
//  Arguments:
//    numNodes      the number of nodes
//    numElements   the number of elements
//    elemType      the element type
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::ParseFEPOINT(int numNodes, int numElements,
                                   const string &elemType)
{
    vtkPoints *points = ParseNodesPoint(numNodes);
    vtkUnstructuredGrid *ugrid = ParseElements(numElements, elemType);
    ugrid->SetPoints(points);
    points->Delete();

    if (topologicalDimension == 2 || topologicalDimension == 3)
    {
        meshes.push_back(ugrid);
    }
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ParseBLOCK
//
//  Purpose:
//    Parses a structured BLOCK zone
//
//  Arguments:
//    numI/J/K   the number of nodes in each dimension
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::ParseBLOCK(int numI, int numJ, int numK)
{
    int numNodes = numI * numJ * numK;

    if (numJ==1)
        topologicalDimension = MAX(topologicalDimension, 1);
    else if (numK==1)
        topologicalDimension = MAX(topologicalDimension, 2);
    else
        topologicalDimension = MAX(topologicalDimension, 3);

    vtkPoints *points = ParseNodesBlock(numNodes);

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();

    int dims[3] = {numI, numJ, numK};
    sgrid->SetDimensions(dims);

    if (topologicalDimension == 2 || topologicalDimension == 3)
    {
        meshes.push_back(sgrid);
    }
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ParsePOINT
//
//  Purpose:
//    Parses a structured POINT zone
//
//  Arguments:
//    numI/J/K   the number of nodes in each dimension
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::ParsePOINT(int numI, int numJ, int numK)
{
    int numNodes = numI * numJ * numK;

    if (numJ==1)
        topologicalDimension = MAX(topologicalDimension, 1);
    else if (numK==1)
        topologicalDimension = MAX(topologicalDimension, 2);
    else
        topologicalDimension = MAX(topologicalDimension, 3);

    vtkPoints *points = ParseNodesPoint(numNodes);

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();

    int dims[3] = {numI, numJ, numK};
    sgrid->SetDimensions(dims);

    if (topologicalDimension == 2 || topologicalDimension == 3)
    {
        meshes.push_back(sgrid);
    }
}

// ****************************************************************************
//  Method:  avtTecPlotFileFormat::ReadFile
//
//  Purpose:
//    Top-level method to re-read the whole file.
//
//  Arguments:
//    none       (filename is stored as a class member already)
//
//  Programmer:  Jeremy Meredith
//  Creation:    December 10, 2004
//
// ****************************************************************************
void
avtTecPlotFileFormat::ReadFile()
{
    file.open(filename.c_str());
    string tok = GetNextToken();
    int zoneIndex = 0;
    int numVars = 0;
    bool got_next_token_already = false;
    bool first_token = true;
    while (!next_char_eof)
    {
        got_next_token_already = false;
        if (tok == "")
        {
            // whitespace!  do nothing
        }
        else if (tok == "TITLE")
        {
            // it's a title
            title = GetNextToken();
        }
        else if (tok == "GEOMETRY")
        {
            // unsupported
            tok = GetNextToken();
            while (!next_char_eof     &&
                   tok != "TITLE"     &&
                   tok != "VARIABLES" &&
                   tok != "ZONE"      &&
                   tok != "GEOMETRY"  &&
                   tok != "TEXT")
            {
                // Skipping token
                tok = GetNextToken();
            }
            got_next_token_already = true;
        }
        else if (tok == "TEXT")
        {
            // unsupported
            tok = GetNextToken();
            while (!next_char_eof     &&
                   tok != "TITLE"     &&
                   tok != "VARIABLES" &&
                   tok != "ZONE"      &&
                   tok != "GEOMETRY"  &&
                   tok != "TEXT")
            {
                // Skipping token
                tok = GetNextToken();
            }
            got_next_token_already = true;
        }
        else if (tok == "VARIABLES")
        {
            // variable lists
            tok = GetNextToken();
            while (token_was_string)
            {
                int l = tok.length();
                for (int i=0;i<l;i++)
                {
                    if (tok[i]=='(')
                        tok[i] = '[';
                    else if (tok[i]==')')
                        tok[i] = ']';
                    else if (tok[i]=='/')
                        tok[i] = '_';
                }

                if (tok == "X" || tok == "x")
                {
                    Xindex = numTotalVars;
                }
                else if (tok == "Y" || tok == "y")
                {
                    Yindex = numTotalVars;
                    spatialDimension = (spatialDimension < 2) ? 2 : spatialDimension;
                }
                else if (tok == "Z" || tok == "z")
                {
                    Zindex = numTotalVars;
                    spatialDimension = (spatialDimension < 3) ? 3 : spatialDimension;
                }
                else
                    variableNames.push_back(tok);

                allVariableNames.push_back(tok);
                numTotalVars++;
                numVars = variableNames.size();
                tok = GetNextToken();
            }
            if (numTotalVars==0)
            {
                // If we didn't get any quoted variables, then
                // it's probably just because they forgot to quote
                // them.  Assume all we get are variables till EOL.

                // FIND A WAY TO DISPLAY THIS TO THE USER AS A WARNING!
                //cerr << "Didn't get any quoted variables.  Assuming"
                //     << " they weren't quoted and continuing until "
                //     << "the end-of-line.\n";

                while (true)
                {
                    if (tok == "X" || tok == "x")
                    {
                        Xindex = numTotalVars;
                    }
                    else if (tok == "Y" || tok == "y")
                    {
                        Yindex = numTotalVars;
                        spatialDimension = (spatialDimension < 2) ? 2 : spatialDimension;
                    }
                    else if (tok == "Z" || tok == "z")
                    {
                        Zindex = numTotalVars;
                        spatialDimension = (spatialDimension < 3) ? 3 : spatialDimension;
                    }
                    else
                        variableNames.push_back(tok);

                    allVariableNames.push_back(tok);
                    numTotalVars++;
                    numVars = variableNames.size();
                    if (next_char_eol)
                    {
                        tok = GetNextToken();
                        break;
                    }
                    else
                        tok = GetNextToken();
                }
            }
            got_next_token_already = true;
        }
        else if (tok == "ZONE")
        {
            // Parse a zone!
            char untitledZoneTitle[40];
            sprintf(untitledZoneTitle, "zone%05d", zoneIndex);

            string zoneTitle = untitledZoneTitle;
            string elemType = "";
            string format = "";
            int numNodes = 0;
            int numElements = 0;
            int numI = 1, numJ = 1, numK = 1;

            tok = GetNextToken();
            while (!(tok != "T"  &&
                     tok != "I"  &&
                     tok != "J"  &&
                     tok != "K"  &&
                     tok != "N"  &&
                     tok != "E"  &&
                     tok != "ET" &&
                     tok != "F"  &&
                     tok != "DT" &&
                     tok != "D"))
            {
                if (tok == "T")
                {
                    zoneTitle = GetNextToken();
                    if (!token_was_string)
                    {
                        EXCEPTION2(InvalidFilesException, filename,
                                   "Zone titles MUST be quoted.");
                    }
                }
                else if (tok == "I")
                {
                    numI = atoi(GetNextToken().c_str());
                }
                else if (tok == "J")
                {
                    numJ = atoi(GetNextToken().c_str());
                }
                else if (tok == "K")
                {
                    numK = atoi(GetNextToken().c_str());
                }
                else if (tok == "N")
                {
                    numNodes = atoi(GetNextToken().c_str());
                }
                else if (tok == "E")
                {
                    numElements = atoi(GetNextToken().c_str());
                }
                else if (tok == "ET")
                {
                    elemType = GetNextToken();
                }
                else if (tok == "F")
                {
                    format = GetNextToken();
                }
                else if (tok == "DT")
                {
                    for (int i=0; i<numTotalVars; i++)
                        GetNextToken();
                }
                else if (tok == "D")
                {
                    EXCEPTION2(InvalidFilesException, filename,
                               "TecPlot Zone record parameter 'D' is "
                               "currently unsupported.  Please contact a "
                               "VisIt developer if you need support for this "
                               "parameter.");
                }
                tok = GetNextToken();
            }
            PushBackToken(tok);

            zoneTitles.push_back(zoneTitle);
            if (format=="FEBLOCK")
            {
                ParseFEBLOCK(numNodes, numElements, elemType);
            }
            else if (format=="FEPOINT")
            {
                ParseFEPOINT(numNodes, numElements, elemType);
            }
            else if (format=="BLOCK")
            {
                ParseBLOCK(numI,numJ,numK);
            }
            else if (format=="POINT")
            {
                ParsePOINT(numI,numJ,numK);
            }
            else if (format=="")
            {
                // No format; we should probably assume we got a POINT block
            }
            else
            {
                // UNKNOWN FORMAT
                char msg[200];
                sprintf(msg, "The format '%s' found in the file was unknown.",
                        format.c_str());
                EXCEPTION2(InvalidFilesException, filename, msg);
            }
        }
        else if (first_token && token_was_string)
        {
            // Robust: assume it's a title
            title = tok;
        }
        else
        {
            // UNKNOWN RECORD TYPE
            char msg[200];
            sprintf(msg, "The record type '%s' found in the file was unknown.",
                    tok.c_str());
            EXCEPTION2(InvalidFilesException, filename, msg);
        }

        first_token = false;
        if (!got_next_token_already)
        {
            tok = GetNextToken();
        }
    }
    file.close();
    file_read = true;

    //
    // Fix up spatial and topological dimension.
    // It may be that a file reported that it had 2-d data
    // but we never found the "X" or "Y" variables, so we
    // must treat it as a curve.  See simpscat.tec as an example.
    //
    if (topologicalDimension > spatialDimension)
    {
        topologicalDimension = spatialDimension;
    }
}

// ****************************************************************************
//  Method: avtTecPlot constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   November  7, 2004
//
// ****************************************************************************

avtTecPlotFileFormat::avtTecPlotFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    file_read = false;
    filename = fname;
    saved_token = "";
    next_char_valid = false;
    next_char_eof = false;
    next_char_eol = false;
    token_was_string = false;
    Xindex = -1;
    Yindex = -1;
    Zindex = -1;
    topologicalDimension = 1;
    spatialDimension = 1;
    numTotalVars = 0;
}


// ****************************************************************************
//  Method: avtTecPlotFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Nov 5 15:44:16 PST 2004
//
// ****************************************************************************

void
avtTecPlotFileFormat::FreeUpResources(void)
{
    file_read = false;
    filename = "";
    saved_token = "";
    next_char_valid = false;
    next_char_eof = false;
    next_char_eol = false;
    token_was_string = false;
    Xindex = -1;
    Yindex = -1;
    Zindex = -1;
    topologicalDimension = 1;
    spatialDimension = 1;
    numTotalVars = 0;

    int i,j;
    for (i=0; i<meshes.size(); i++)
    {
        meshes[i]->Delete();
    }
    meshes.clear();
    for (i=0; i<variableNames.size(); i++)
    {
        for (j=0; j<vars[variableNames[i]].size(); j++)
        {
            vars[variableNames[i]][j]->Delete();
        }
        vars[variableNames[i]].clear();
    }
    vars.clear();
    variableNames.clear();
    allVariableNames.clear();
    curveNames.clear();
    curveFirstVar.clear();
    curveSecondVar.clear();
    zoneTitles.clear();
}


// ****************************************************************************
//  Method: avtTecPlotFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: meredith -- generated by xml2avt
//  Creation:   Fri Nov 5 15:44:16 PST 2004
//
// ****************************************************************************

void
avtTecPlotFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!file_read)
        ReadFile();

    if (topologicalDimension==2 || topologicalDimension==3)
    {
        avtMeshMetaData *mesh = new avtMeshMetaData;
        mesh->name = "mesh";
        mesh->topologicalDimension = topologicalDimension;
        mesh->spatialDimension = spatialDimension;

        if (meshes.size() > 0 && 
            meshes[0]->GetDataObjectType()==VTK_STRUCTURED_GRID)
        {
            mesh->meshType = AVT_CURVILINEAR_MESH;
            // See '5756 for the reason for this next line
            if (spatialDimension > 2)
                mesh->topologicalDimension = 3;
        }
        else
        {
            mesh->meshType = AVT_UNSTRUCTURED_MESH;
        }

        mesh->numBlocks = meshes.size();
        mesh->blockOrigin = 1;
        mesh->cellOrigin = 1;
        mesh->blockTitle = "Zones";
        mesh->blockPieceName = "Zone";
        mesh->hasSpatialExtents = false;
        md->Add(mesh);

        for (int i=0; i<variableNames.size(); i++)
        {
            AddScalarVarToMetaData(md, variableNames[i],
                                   "mesh", AVT_NODECENT);
        }
    }
    else
    {
        //
        // We're pretty sneaky about 1D plots -- we
        // actual populate every n^2 (roughly) pairing
        // of 1D values in the metadata but only 
        // construct the meshes upon request.
        //
        char s[200];
        for (int z = 0 ; z < zoneTitles.size(); z++)
        {
            if (Xindex < 0)
            {
                for (int i=0; i<allVariableNames.size(); i++)
                {
                    for (int j=0; j<allVariableNames.size(); j++)
                    {
                        if (i==j) 
                            continue;
                        if (zoneTitles.size() > 1)
                        {
                            sprintf(s, "%s/%s vs/%s",
                                    zoneTitles[z].c_str(),
                                    allVariableNames[i].c_str(),
                                    allVariableNames[j].c_str());
                        }
                        else
                        {
                            sprintf(s, "%s vs/%s",
                                    allVariableNames[i].c_str(),
                                    allVariableNames[j].c_str());
                        }
                        curveIndices[s] = curveNames.size();
                        curveNames.push_back(s);
                        curveDomains.push_back(z);
                        curveFirstVar.push_back(i);
                        curveSecondVar.push_back(j);
                        avtCurveMetaData *curve = new avtCurveMetaData;
                        curve->name = s;
                        md->Add(curve);
                    }
                }
            }
            else
            {
                for (int i=0; i<allVariableNames.size(); i++)
                {
                    if (i==Xindex)
                        continue;

                    if (zoneTitles.size() > 1)
                    {
                        sprintf(s, "%s/%s vs/X",
                                zoneTitles[z].c_str(),
                                allVariableNames[i].c_str());
                    }
                    else
                    {
                        sprintf(s, "%s vs/X", allVariableNames[i].c_str());
                    }
                    curveIndices[s] = curveNames.size();
                    curveNames.push_back(s);
                    curveDomains.push_back(z);
                    curveFirstVar.push_back(i);
                    curveSecondVar.push_back(-1);
                    avtCurveMetaData *curve = new avtCurveMetaData;
                    curve->name = s;
                    md->Add(curve);
                }
            }
        }
    }
}


// ****************************************************************************
//  Method: avtTecPlotFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      domain      The index of the domain.  If there are NDomains, this
//                  value is guaranteed to be between 0 and NDomains-1,
//                  regardless of block origin.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
// ****************************************************************************

vtkDataSet *
avtTecPlotFileFormat::GetMesh(int domain, const char *meshname)
{
    if (topologicalDimension == 2 || topologicalDimension == 3)
    {
        meshes[domain]->Register(NULL);
        return meshes[domain];
    }
    else
    {
        //
        // We're pretty sneaky about 1D plots -- we
        // actual populated every n^2 (roughly) pairing
        // of 1D values in the metadata but only 
        // construct the meshes here upon request.
        //

        if (curveIndices.count(meshname) <= 0)
            EXCEPTION1(InvalidVariableException, meshname);

        int curve = curveIndices[meshname];
        int index1 = curveFirstVar[curve];
        int index2 = curveSecondVar[curve];
        // override domain
        int curveDomain = curveDomains[curve];

        if (index2 < 0)
            EXCEPTION1(InvalidVariableException, meshname);

        vtkFloatArray *var1 = vars[allVariableNames[index1]][curveDomain];
        vtkFloatArray *var2 = vars[allVariableNames[index2]][curveDomain];
        int nPts = var1->GetNumberOfTuples();

        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();
        pd->SetPoints(pts);

        pts->SetNumberOfPoints(nPts);
        for (int j = 0 ; j < nPts ; j++)
        {
            pts->SetPoint(j, var2->GetValue(j), var1->GetValue(j), 0.);
        }
 
        //
        // Connect the points up with line segments.
        //
        vtkCellArray *line = vtkCellArray::New();
        pd->SetLines(line);
        for (int k = 1 ; k < nPts ; k++)
        {
            line->InsertNextCell(2);
            line->InsertCellPoint(k-1);
            line->InsertCellPoint(k);
        }

        pts->Delete();
        line->Delete();

        return pd;
    }
}


// ****************************************************************************
//  Method: avtTecPlotFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
// ****************************************************************************

vtkDataArray *
avtTecPlotFileFormat::GetVar(int domain, const char *varname)
{
    vars[varname][domain]->Register(NULL);
    return vars[varname][domain];
}


// ****************************************************************************
//  Method: avtTecPlotFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      domain     The index of the domain.  If there are NDomains, this
//                 value is guaranteed to be between 0 and NDomains-1,
//                 regardless of block origin.
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   November 16, 2004
//
// ****************************************************************************

vtkDataArray *
avtTecPlotFileFormat::GetVectorVar(int domain, const char *varname)
{
    return NULL;
}
