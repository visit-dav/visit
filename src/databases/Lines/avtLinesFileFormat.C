// ************************************************************************* //
//                             avtLinesFileFormat.C                          //
// ************************************************************************* //

#include <avtLinesFileFormat.h>

#include <vector>
#include <string>

#include <vtkCellArray.h>
#include <vtkPolyData.h>

#include <avtDatabaseMetaData.h>

#include <BadIndexException.h>
#include <DebugStream.h>
#include <InvalidVariableException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtLinesFileFormat constructor
//
//  Arguments:
//      fname    The name of lines file.
//
//  Programmer:  Hank Childs
//  Creation:    August 22, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 09:10:58 PDT 2003
//    Defer reading in file.
//
// ****************************************************************************

avtLinesFileFormat::avtLinesFileFormat(const char *fname)
    : avtSTMDFileFormat(&fname, 1)
{
    filename = fname;
    readInFile = false;
}


// ****************************************************************************
//  Method: avtLinesFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
// ****************************************************************************

avtLinesFileFormat::~avtLinesFileFormat()
{
    for (int i = 0 ; i < lines.size() ; i++)
    {
        lines[i]->Delete();
    }
}


// ****************************************************************************
//  Method: avtLinesFileFormat::GetMesh
//
//  Purpose:
//      Returns the line associated with a domain number.
//
//  Arguments:
//      dom     The domain number.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 09:10:58 PDT 2003
//    Make sure we have read in the file.
//
// ****************************************************************************

vtkDataSet *
avtLinesFileFormat::GetMesh(int dom, const char *name)
{
    if (!readInFile)
    {
        ReadFile();
    }

    if (dom < 0 || dom >= lines.size())
    {
        EXCEPTION2(BadIndexException, dom, lines.size());
    }

    if (strcmp(name, "Lines") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    return lines[dom];
}


// ****************************************************************************
//  Method: avtLinesFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Notes:      This would be a cool feature.  Variables on a line are not
//              currently supported.
//
//  Arguments:
//      <unused>  The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     August 22, 2001
//
//  Modifications:
//    Kathleen Bonnell, Fri Feb  8 11:03:49 PST 2002
//    vtkScalars has been deprecated in VTK 4.0, use vtkDataArray instead.
//
// ****************************************************************************

vtkDataArray *
avtLinesFileFormat::GetVar(int, const char *name)
{
    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtLinesFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this line file.  There is only one
//      mesh, the lines.  Each line gets its own domain, for easy subselection.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 12:33:08 PDT 2001
//    Add block names when specified in the file.
//
//    Hank Childs, Fri Sep  7 15:22:59 PDT 2001
//    Initialize hasSpatialExtents.
//
//    Hank Childs, Mon Mar 11 08:52:59 PST 2002
//    Renamed to PopulateDatabaseMetaData.
//
//    Hank Childs, Tue May 28 14:13:45 PDT 2002
//    Override the "domains" heading with "lines".
//
//    Hank Childs, Tue Apr  8 09:10:58 PDT 2003
//    Make sure we have read in the file.
//
// ****************************************************************************

void
avtLinesFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readInFile)
    {
        ReadFile();
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Lines";
    mesh->meshType = AVT_UNSTRUCTURED_MESH;
    mesh->numBlocks = lines.size();
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 1;
    mesh->blockNames = lineNames;
    mesh->blockTitle = "lines";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);
}


// ****************************************************************************
//  Method: avtLinesFileFormat::ReadFile
//
//  Purpose:
//      Actually reads in from a file.  This is pretty dependent on formats
//      that have one point per line.  When there are runs of points, followed
//      by non-points, that is assumed to be a new line.
//
//  Arguments:
//      ifile   The file to read in.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Wed Aug 29 12:11:21 PDT 2001
//    Adding conversion to STL string to deal with dumb compilers.
//
//    Hank Childs, Tue Oct 30 15:44:34 PST 2001
//    Fix problems with white space between headers and remove identical 
//    points.
//
//    Hank Childs, Tue Apr  8 09:10:58 PDT 2003
//    Open the file here.
//
// ****************************************************************************

void
avtLinesFileFormat::ReadFile(void)
{
    ifstream ifile(filename.c_str());

    if (ifile.fail())
    {
        debug1 << "Unable to open file " << filename.c_str() << endl;
        return;
    }

    //
    // Read in all of the points and store where there are lines between them
    // so we can re-construct them later.
    //
    vector<float> xl;
    vector<float> yl;
    vector<float> zl;
    vector<int>   cutoff;
    string  headerName = "";
    while (!ifile.eof())
    {
        float   x, y, z;
        string  lineName;
        if (GetPoint(ifile, x, y, z, lineName))
        {
            if (headerName.find_first_not_of(" ") != string::npos)
            {
                lineNames.push_back(headerName);
                headerName = "";
            }
            int len = xl.size();
            bool shouldAddPoint = true;
            if (len > 0)
            {
                if (x == xl[len-1] && y == yl[len-1] && z == zl[len-1])
                {
                    shouldAddPoint = false;
                }
            }
            if (shouldAddPoint)
            {
                xl.push_back(x);
                yl.push_back(y);
                zl.push_back(z);
            }
        }
        else
        {
            if (lineName.find_first_not_of(" ") != string::npos)
            {
                headerName = lineName;
            }
            cutoff.push_back(xl.size());
        }
    }  

    //
    // Now we can construct the lines as vtkPolyData.
    //
    int start = 0;
    cutoff.push_back(xl.size());  // Make logic easier.
    for (int i = 0 ; i < cutoff.size() ; i++)
    {
        if (start == cutoff[i])
        {
            continue;
        }
       
        //
        // Add all of the points to an array.
        //
        vtkPolyData *pd  = vtkPolyData::New();
        vtkPoints   *pts = vtkPoints::New();
        pd->SetPoints(pts);
        int nPts = cutoff[i] - start;
        pts->SetNumberOfPoints(nPts);
        for (int j = 0 ; j < nPts ; j++)
        {
            pts->SetPoint(j, xl[start+j], yl[start+j], zl[start+j]);
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
        lines.push_back(pd);

        //
        // Set ourselves up for the next iteration.
        //
        start = cutoff[i];
    }

    readInFile = true;
}


// ****************************************************************************
//  Method: avtLinesFileFormat::GetPoint
//
//  Purpose:
//      Gets a point from a line.
//
//  Programmer: Hank Childs
//  Creation:   August 22, 2001
//
//  Modifications:
//
//    Hank Childs, Tue Aug 28 10:58:15 PDT 2001
//    Added string argument for the line name.
//
// ****************************************************************************

bool
avtLinesFileFormat::GetPoint(ifstream &ifile, float &x, float &y, float &z,
                             string &ln)
{
    char line[256];
    ifile.getline(line, 256, '\n');
    ln = line;

    char *ystr = NULL;

    x = (float) strtod(line, &ystr);
    if (ystr == NULL)
    {
        return false;
    }
    ystr = strstr(ystr, ",");
    if (ystr == NULL || ystr == line)
    {
        return false;
    }
    
    // Get past the comma.
    ystr++;

    char *zstr = NULL;
    y = (float) strtod(ystr, &zstr);
    if (zstr == NULL)
    {
        return false;
    }
    zstr = strstr(zstr, ",");
    if (zstr == NULL || zstr == ystr)
    {
        return false;
    }
    zstr++;

    z = (float) strtod(zstr, NULL);

    // Should probably check z better, but...
    if (z == 0. && (strstr(zstr, "0") == NULL) )
    {
        return false;
    }

    ln = "";
    return true;
}


