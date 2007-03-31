// ************************************************************************* //
//                            avtCurve2DFileFormat.C                         //
// ************************************************************************* //

#include <avtCurve2DFileFormat.h>

#include <vector>
#include <string>

#include <vtkPolyData.h>

#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>


using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtCurve2DFileFormat constructor
//
//  Arguments:
//      fname    The name of the curve file.
//
//  Programmer:  Hank Childs
//  Creation:    May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Do not do so much work in the constructor.
//
// ****************************************************************************

avtCurve2DFileFormat::avtCurve2DFileFormat(const char *fname)
    : avtSTSDFileFormat(fname)
{
    filename = fname;
    readFile = false;
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
// ****************************************************************************

avtCurve2DFileFormat::~avtCurve2DFileFormat()
{
    for (int i = 0 ; i < curves.size() ; i++)
    {
        curves[i]->Delete();
    }
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetMesh
//
//  Purpose:
//      Returns the curve associated with a curve name.
//
//  Arguments:
//      name       The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jan 15 10:54:14 PST 2003
//    Increment the reference count for the curve, since we own the memory for
//    it and the calling function believes it also owns it.
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Make sure we have read in the file first.
//
//    Hank Childs, Fri Aug  1 21:19:28 PDT 2003
//    Retro-fit for STSD.
//
// ****************************************************************************

vtkDataSet *
avtCurve2DFileFormat::GetMesh(const char *name)
{
    if (!readFile)
    {
        ReadFile();
    }

    for (int i = 0 ; i < curves.size() ; i++)
    {
        if (strcmp(curveNames[i].c_str(), name) == 0)
        {
            //
            // The calling function will think it owns the return mesh, so
            // increment its reference count.
            //
            curves[i]->Register(NULL);
        
            return curves[i];
        }
    }

    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetVar
//
//  Purpose:
//      Returns a variable.
//
//  Notes:      This is not meaningful for this file type and is here only to
//              meet the base type's interface.
//
//  Arguments:
//      <unused>  The domain number.
//      name      The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Fri Aug  1 21:19:28 PDT 2003
//    Retro-fit for STSD.
//
// ****************************************************************************

vtkDataArray *
avtCurve2DFileFormat::GetVar(const char *name)
{
    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this curve file.  There is only one
//      mesh, the curve.  Each curve gets its own domain,for easy subselection.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Make sure we have read in the file first.
//
//    Hank Childs, Fri Aug  1 21:01:51 PDT 2003
//    Mark curves as "curve" type.
//
// ****************************************************************************

void
avtCurve2DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readFile)
    {
        ReadFile();
    }

    for (int i = 0 ; i < curves.size() ; i++)
    {
        avtCurveMetaData *curve = new avtCurveMetaData;
        curve->name = curveNames[i];
        md->Add(curve);
    }
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::ReadFile
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
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Wed Jan 15 13:44:39 PST 2003
//    Fix parsing error where whitespace was getting prepended to the curve
//    names.
//
//    Hank Childs, Thu Apr  3 16:58:33 PST 2003
//    It is now acceptable to have whitespace between the curves to indicate
//    a break in the curve.  Add parsing for this.
//
//    Hank Childs, Fri Apr  4 09:04:17 PST 2003
//    Fixed bug from yesterday where wrong string is being sent into GetPoint.
//
//    Hank Childs, Tue Apr  8 11:09:03 PDT 2003
//    Do some of the work formerly done by the constructor.
//
// ****************************************************************************

void
avtCurve2DFileFormat::ReadFile(void)
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
    vector<bool>  breakpoint_following;
    vector<int>   cutoff;
    string  headerName = "";
    while (!ifile.eof())
    {
        float   x, y;
        string  lineName;
        CurveToken t = GetPoint(ifile, x, y, lineName);
        switch (t)
        {
          case VALID_POINT:
          {
            if (headerName.find_first_not_of("#") != string::npos)
            {
                string str1 = 
                          headerName.substr(headerName.find_first_not_of("#"));
                string str2 = str1.substr(str1.find_first_not_of(" \t"));
                curveNames.push_back(str2);
                headerName = "";
            }
            int len = xl.size();
            bool shouldAddPoint = true;
            if (len > 0)
            {
                if (x == xl[len-1] && y == yl[len-1])
                {
                    shouldAddPoint = false;
                }
            }
            if (shouldAddPoint)
            {
                xl.push_back(x);
                yl.push_back(y);
                breakpoint_following.push_back(false);
            }
            break;
          }
          case HEADER:
          {
            if (lineName.find_first_not_of("#") != string::npos)
            {
                headerName = lineName;
            }
            cutoff.push_back(xl.size());
            break;
          }
          case WHITESPACE:
          {
            breakpoint_following[breakpoint_following.size()-1] = true;
            break;
          }
          case INVALID_POINT:
          {
            EXCEPTION1(InvalidFilesException, filename.c_str());
          }
       }
    }  

    //
    // Now we can construct the curve as vtkPolyData.
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
            pts->SetPoint(j, xl[start+j], yl[start+j], 0.);
        }
 
        //
        // Connect the points up with line segments.
        //
        vtkCellArray *line = vtkCellArray::New();
        pd->SetLines(line);
        for (int k = 1 ; k < nPts ; k++)
        {
            if (k < (nPts-1) && breakpoint_following[start+k-1])
            {
                continue;
            }
            line->InsertNextCell(2);
            line->InsertCellPoint(k-1);
            line->InsertCellPoint(k);
        }

        pts->Delete();
        line->Delete();
        curves.push_back(pd);

        //
        // Set ourselves up for the next iteration.
        //
        start = cutoff[i];
    }

    readFile = true;
}


// ****************************************************************************
//  Method: avtCurve2DFileFormat::GetPoint
//
//  Purpose:
//      Gets a point from a line.
//
//  Programmer: Hank Childs
//  Creation:   May 28, 2002
//
//  Modifications:
//
//    Hank Childs, Thu Apr  3 16:58:33 PST 2003
//    Identify different return types to make parsing easier.
//
// ****************************************************************************

CurveToken
avtCurve2DFileFormat::GetPoint(ifstream &ifile, float &x, float &y, string &ln)
{
    char line[256];
    ifile.getline(line, 256, '\n');
    ln = line;

    //
    // Pick out some of the harder to parse cases.
    //
    if (strstr(line, "#") != NULL)
    {
        return HEADER;
    }
    bool allSpace = true;
    int len = strlen(line);
    for (int i = 0 ; i < len ; i++)
    {
        if (!isspace(line[i]))
        {
            allSpace = false;
        }
    }
    if (allSpace)
    {
        return WHITESPACE;
    }

    char *ystr = NULL;

    x = (float) strtod(line, &ystr);
    if (ystr == NULL)
    {
        return INVALID_POINT;
    }
    ystr = strstr(ystr, " ");
    if (ystr == NULL || ystr == line)
    {
        return INVALID_POINT;
    }
    
    // Get past the space.
    ystr++;

    y = (float) strtod(ystr, NULL);

    ln = "";
    return VALID_POINT;
}


