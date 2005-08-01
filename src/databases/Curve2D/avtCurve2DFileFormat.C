// ************************************************************************* //
//                            avtCurve2DFileFormat.C                         //
// ************************************************************************* //

#include <avtCurve2DFileFormat.h>

#include <vector>
#include <string>

#include <vtkCellArray.h>
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
//    Hank Childs, Thu Sep 23 14:54:07 PDT 2004
//    Add support for files with extra whitespace and files that have no 
//    headers.
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
            if (breakpoint_following.size() > 0)
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

    //
    // It is possible to have a file that has no header.  This should be
    // interpreted as one curve.  Check for this, since we will later assume
    // the number of curves and the number of curve names is the same.
    //
    if (curves.size() == 1 && curveNames.size() == 0)
    {
        curveNames.push_back("curve");
    }

    if (curves.size() != curveNames.size())
    {
        debug1 << "The number of curves does not match the number of curve "
               << "names.  Cannot continue with this file." << endl;
        EXCEPTION1(InvalidFilesException, filename.c_str());
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
//    Hank Childs, Mon May 24 14:45:14 PDT 2004
//    Treat the line "end" as white space, since it screws up our parsing
//    and is not really necessary for us.  Also treat parenthesis as square
//    brackets, since parenthesis are special for us.
//
//    Brad Whitlock, Tue Jun 29 11:50:41 PDT 2004
//    Fixed for Windows compiler.
//
//    Hank Childs, Thu Sep 23 14:54:07 PDT 2004
//    Add support for Fortran-style scientific notation (5.05D-2).
//
//    Hank Childs, Fri Jul 29 14:34:39 PDT 2005
//    Add support for tabs.
//
// ****************************************************************************

CurveToken
avtCurve2DFileFormat::GetPoint(ifstream &ifile, float &x, float &y, string &ln)
{
    char line[256];
    ifile.getline(line, 256, '\n');

    //
    // Parenthesis are special characters for variables names, etc, so just
    // change them to square brackets to "go with the flow"...
    //
    int i, nchars = strlen(line);
    for (i = 0 ; i < nchars ; i++)
    {
        if (line[i] == '(')
            line[i] = '<';
        else if (line[i] == ')')
            line[i] = '>';
    }
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
    for (i = 0 ; i < len ; i++)
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
    if (strncmp(line, "end", strlen("end")) == 0)
    {
        // We will infer that we have hit the end when we find a new token.
        // Just treat this as white space to make our parsing rules easier.
        return WHITESPACE;
    }

    //
    // We are assuming that we a number.  Fortran-style scientific notation
    // uses 'D' when we are used to seeing 'E'.  So just switch them out.
    //
    for (i = 0 ; i < len ; i++)
    {
        if (line[i] == 'D' || line[i] == 'd')
            line[i] = 'E';
        if (line[i] == '\t')
            line[i] = ' ';
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


