// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCurve3DFileFormat.C                         //
// A reader for reading files that contain 3D curves. These may be named using
// the same convention as in Curve2D, or not.
// White space can be considered as a segmentation when MULTSEG is set.
// In that case, we get the same behaviour as the one of gnuplot with the splot
// function.
// 2023/10/05 Olivier Cessenat.
// Verbosity for poor man's debugging
#define INTERACTIVEPLOT 0
// Consider white space indicates a split
#define MULTSEG 1
// ************************************************************************* //

#include <avtCurve3DFileFormat.h>

#include <vector>
#include <string>

#include <vtkPointData.h>
#include <vtkUnstructuredGrid.h>

#include <vtkCellArray.h>
#include <vtkPoints.h>
#include <vtkCellType.h>

#include <avtIntervalTree.h>
#include <avtMaterial.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>

#include <DebugStream.h>
#include <DBOptionsAttributes.h>
#include <StringHelpers.h>
using StringHelpers::vstrtonum;
using StringHelpers::NO_OSTREAM;
#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include "visit_gzstream.h"

#include <float.h>
#include <stdlib.h>

#ifdef PARALLEL
#include <mpi.h>
#include <avtParallel.h>
#endif

using std::vector;
using std::string;
using namespace std;

// ****************************************************************************
//  Method: avtCurve3DFileFormat constructor
//
//  Arguments:
//      fname    The name of the curve file.
//
//  Programmer:  Olivier Cessenat
//  Creation:    Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

avtCurve3DFileFormat::avtCurve3DFileFormat(const char *fname,
                                           const DBOptionsAttributes *opts)
    : avtSTSDFileFormat(fname)
{
    filename = fname;
    readFile = false;
    curveTime = INVALID_TIME;
    curveCycle = INVALID_CYCLE;
    gnuplotStyle = opts->GetBool("Use blank line as another curve as in gnuplot");
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat destructor
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
// ****************************************************************************

avtCurve3DFileFormat::~avtCurve3DFileFormat()
{
    for (size_t i = 0 ; i < curves.size() ; i++)
    {
        curves[i]->Delete();
    }
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat::GetMesh
//
//  Purpose:
//      Returns the curve associated with a curve name.
//
//  Arguments:
//      name       The mesh name.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 1, 2003 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

vtkDataSet *
avtCurve3DFileFormat::GetMesh(const char *name)
{
    if (strcmp("mesh", name) == 0)
    {
        // This is the global mesh :
        int npoints = 0 ;
        int nelements = 0 ;
        int j ;
        // Gets the number of nodes and elements :
        for (size_t i=0; i<curves.size(); i++)
        {
            vtkUnstructuredGrid *rg = curves[i] ;
            vtkPoints *lpts = rg->GetPoints() ;
            int npl = lpts->GetNumberOfPoints();
            npoints += npl ;
            nelements += (npl-1) ;
        }
        // Fill the data structure "grid"
        vtkUnstructuredGrid *grid = vtkUnstructuredGrid::New();
        vtkPoints   *gpts = vtkPoints::New();
        gpts->SetNumberOfPoints(npoints);
        grid->SetPoints(gpts);
        gpts->Delete();
        // Now gets the points and write them to "gpts"
        j = 0 ;
        for (size_t i=0; i<curves.size(); i++)
        {
            vtkUnstructuredGrid *rg = curves[i] ;
            vtkPoints *lpts = rg->GetPoints() ;
            int npl = lpts->GetNumberOfPoints();
            for (int k=0; k<npl; k++)
            {
                double * positions = lpts->GetPoint (k) ;
                gpts->SetPoint(j, positions) ;
                j++ ;
            }
        }
        grid->Allocate(nelements);
        vtkIdType verts[2];
        j = 0 ;
        for (size_t i=0; i<curves.size(); i++)
        {
            vtkUnstructuredGrid *rg = curves[i] ;
            vtkPoints *lpts = rg->GetPoints() ;
            int npl = lpts->GetNumberOfPoints();
            for (int k=1; k<npl; k++)
            {
                verts[0] = j + k-1;
                verts[1] = j + k;
                grid->InsertNextCell(VTK_LINE, 2, verts);
            }
            j = j + npl ;
        }
        return(grid) ;
    }
    for (size_t i = 0 ; i < curves.size() ; i++)
    {
        if (strcmp(("cur_" + curveNames[i]).c_str(), name) == 0)
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
//  Method: avtCurve3DFileFormat::GetVar
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
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 1, 2003 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

vtkDataArray *
avtCurve3DFileFormat::GetVar(const char *name)
{
    EXCEPTION1(InvalidVariableException, name);
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this curve file.  There is only one
//      mesh, the curve.  Each curve gets its own domain,for easy subselection.
//      One unstructured mesh per segment, then a gloval mesh with materials.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 23, 2012 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
// ****************************************************************************

void
avtCurve3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!readFile)
    {
#ifdef MDSERVER 
        bool const clearData = true;
        ReadFile(clearData);
#else
        bool const clearData = false;
        ReadFile(clearData);
#endif  
    }

    avtMeshType mt = AVT_UNSTRUCTURED_MESH;
    int nblocks = 1;
    int block_origin = 0;
    int spatial_dimension = 3;
    int topological_dimension = 1;
    for (size_t i = 0 ; i < curves.size() ; i++)
    {
        string meshname = "cur_" + curveNames[i] ;
        double extents[6] ; // Local extents
        extents[0] = xExtents[i*2] ;
        extents[1] = xExtents[i*2+1] ;
        extents[2] = yExtents[i*2] ;
        extents[3] = yExtents[i*2+1] ;
        extents[4] = zExtents[i*2] ;
        extents[5] = zExtents[i*2+1] ;
        AddMeshToMetaData(md, meshname, mt, extents, nblocks, block_origin,
                          spatial_dimension, topological_dimension);
    }
    if (curves.size() > 1)
    {
        // Make a global mesh with all the sub elements:
        string meshname = "mesh" ;
        AddMeshToMetaData(md, meshname, mt, range, nblocks, block_origin,
                          spatial_dimension, topological_dimension);
        
        int nmats = curves.size() ;
        vector<string> mnames;
        for (int i = 0; i < nmats; i++)
            mnames.push_back("mat_" + curveNames[i]);
        string matname = "segments" ;
        AddMaterialToMetaData(md, matname, meshname, nmats, mnames);
    }
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat::ReadFile
//
//  Purpose:
//      Actually reads in from a file.  This is pretty dependent on formats
//      that have one point per line.  When there are runs of points, followed
//      by non-points, that is assumed to be a new line.
//      We segment the curve when a whitespace line is met
//      to mimic splot behaviour of Gnuplot.
//
//  Arguments:
//      ifile   The file to read in.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Jul 5, 2022 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
//  Modifications:
//    Mark C. Miller, Fri Jan 12 17:04:46 PST 2024
//    Replace atoX/strtoX with vstrtonum
// ****************************************************************************

#define INVALID_POINT_WARNING(X)                                        \
{                                                                       \
    if (invalidPointCount++ < 6)                                        \
    {                                                                   \
        char msg[512] = "Further warnings will be suppressed";          \
        if (invalidPointCount < 6)                                      \
        {                                                               \
            snprintf(msg, sizeof(msg),"Encountered invalid point "      \
                "at or near line %d beginning with \"%s\"",             \
                lineCount, lineName.c_str());                           \
        }                                                               \
        TRY                                                             \
        {                                                               \
            if (!avtCallback::IssueWarning(msg))                        \
                cerr << msg << endl;                                    \
        }                                                               \
        CATCH(VisItException)                                           \
        {                                                               \
            cerr << msg << endl;                                        \
        }                                                               \
        ENDTRY                                                          \
    }                                                                   \
    xl.push_back(X);                                                    \
    if (yl.size()) {                                                    \
        yl.push_back(yl[yl.size()-1]);                                  \
        zl.push_back(zl[zl.size()-1]);                                  \
    } else {                                                            \
        yl.push_back(X);                                                \
        zl.push_back(X);                                                \
    }                                                                   \
    breakpoint_following.push_back(false);                              \
}

void
avtCurve3DFileFormat::ReadFile(bool clearData = false)
{
    int invalidPointCount = 0;
    int lineCount = 1;
    visit_ifstream ifile(filename.c_str());

    if (ifile().fail())
    {
        debug1 << "Unable to open file " << filename.c_str() << endl;
        return;
    }

    //
    // Read in all of the points and store where there are lines between them
    // so we can re-construct them later.
    //
    vector<double> xl;
    vector<double> yl;
    vector<double> zl;
    vector<bool>  breakpoint_following;
    vector<int>   cutoff;
    vector<avtCentering> centering;
    avtCentering useCentering = AVT_NODECENT;
    string headerName = "";
    string peaderName = "";
    int nseg = 0 ; 
    char segname[128] ;
    curveTime = INVALID_TIME;
    curveCycle = INVALID_CYCLE;
    Curve3DToken lastt = VALID_POINT;
    bool justStartedNewCurve = false;
    xl.reserve(1000);
    yl.reserve(1000);
    zl.reserve(1000);

    peaderName = filename ;
    size_t pos = peaderName.find(".");
    peaderName = (string::npos == pos) ? peaderName : peaderName.substr(0, pos);
    pos = peaderName.find_last_of("/") ;
    if (pos != string::npos)
        peaderName = peaderName.substr(pos+1);
    // Better set the header prior to reading 
    headerName = "#" + peaderName ;
#if INTERACTIVEPLOT
    fprintf(stdout,"peaderName=%s\n", peaderName.c_str()) ;
#endif
    while (!ifile().eof())
    {
        double  x, y, z;
        string  lineName;
        Curve3DToken t = GetPoint(ifile(), x, y, z, lineName);
        switch (t)
        {
        case VALID_POINT:
        {
#if INTERACTIVEPLOT
            fprintf(stdout,"vp ") ;
#endif
            if (headerName.find_first_not_of("#%") != string::npos)
            {
                string str1 = 
                          headerName.substr(headerName.find_first_not_of("#%"));
                string str2 = str1.substr(str1.find_first_not_of(" \t"));
                curveNames.push_back(str2);
#if MULTSEG
                if (gnuplotStyle)
                {
                    centering.push_back(useCentering);
                    cutoff.push_back((int)xl.size());
                    justStartedNewCurve = true;
                    if (nseg == 0)
                        // Normal way: store the previous value
                        peaderName = str1 ;
                }
#endif
                headerName = "";
            }
            size_t len = xl.size();
            bool shouldAddPoint = true;
            if (len > 0)
            {
                if (x == xl[len-1] && y == yl[len-1] && z == zl[len-1])
                {
                    if (justStartedNewCurve == false)
                        shouldAddPoint = false;
                }
            }
            if (shouldAddPoint)
            {
                xl.push_back(x);
                yl.push_back(y);
                zl.push_back(z);
                breakpoint_following.push_back(false);
                useCentering = AVT_NODECENT;
                justStartedNewCurve = false;
            }
            break;
          }
          case HEADER:
          {
            if (headerName != "")
            {
                // If we parsed a header followed by another header,
                // see if it has TIME. 
                size_t timePos = headerName.find("TIME");
                if ( timePos != string::npos)
                {
                    string tStr = headerName.substr(timePos+4);
                    char *endstr = NULL;
                    curveTime = vstrtonum<double>(tStr.c_str(),10,INVALID_TIME);
                }
                else
                {
                    size_t cyclePos = headerName.find("CYCLE");
                    if ( cyclePos != string::npos)
                    {
                        string cyStr = headerName.substr(cyclePos+5);
                        char *endstr = NULL;
                        curveCycle = vstrtonum<int>(cyStr.c_str(),10,INVALID_CYCLE);
                    }
                }
            }
  
            if (lineName.find_first_not_of("#%") != string::npos)
            {
                headerName = lineName;
            }
#if !MULTSEG
#if INTERACTIVEPLOT
            fprintf(stdout,"header %s\n",headerName.c_str()) ;
#endif
            centering.push_back(useCentering);
            cutoff.push_back((int)xl.size());
            justStartedNewCurve = true;
#endif
#if MULTSEG
            if (gnuplotStyle)
                nseg = 0 ;
            else
            {
                centering.push_back(useCentering);
                cutoff.push_back((int)xl.size());
                justStartedNewCurve = true;
            }
#endif
            break;
          }
          case WHITESPACE:
          {
#if MULTSEG
              if (gnuplotStyle)
              {
#if INTERACTIVEPLOT
                  fprintf(stdout,"white: ") ;
#endif
                  sprintf(segname, "%d", nseg) ;
                  nseg++ ;
                  headerName = "#" + peaderName + "_" + string(segname) ;
#if INTERACTIVEPLOT
                  fprintf(stdout,"headerName=%s\n", headerName.c_str()) ;
#endif
              }
#endif
            if (breakpoint_following.size() > 0)
                breakpoint_following[breakpoint_following.size()-1] = true;
            break;
          }
          case INVALID_POINT:
          {
#if INTERACTIVEPLOT
              fprintf(stdout,"invalid\n") ;
#endif
              if (this->GetStrictMode())
                  EXCEPTION2(InvalidFilesException, GetFilename(),
                             "Found a bad point");
              if (xl.size())
              {
                  INVALID_POINT_WARNING(xl[xl.size()-1]);
              }
              else
              {
                  INVALID_POINT_WARNING(0);
              }
              break;
          }
          case VALID_XVALUE:
          {
#if INTERACTIVEPLOT
              fprintf(stdout,"xalid\n") ;
#endif
              if (lastt == VALID_XVALUE)
              {
                  if (this->GetStrictMode())
                      EXCEPTION2(InvalidFilesException, GetFilename(),
                                 "Found a bad point");
                  INVALID_POINT_WARNING(x);
                  useCentering = AVT_NODECENT;
              }
              else
              {
                  xl.push_back(x);
                  yl.push_back(yl[yl.size()-1]);
                  zl.push_back(zl[zl.size()-1]);
                  breakpoint_following.push_back(false);
                  useCentering = AVT_ZONECENT;
              }
              break;
          }
       }
       lastt = t;
       //lastx = x;
       lineCount++;
    }  

    // If we parsed a header not followed by data values, see if
    // it is TIME. 
    if (headerName != "")
    {
        size_t timePos = headerName.find("TIME");
        if ( timePos != string::npos)
        {
            string tStr = headerName.substr(timePos+4);
            char *endstr = NULL;
            curveTime = vstrtonum<double>(tStr.c_str(),10,INVALID_TIME);
        }
        else
        {
            size_t cyclePos = headerName.find("CYCLE");
            if ( cyclePos != string::npos)
            {
                string cyStr = headerName.substr(cyclePos+5);
                char *endstr = NULL;
                curveCycle = vstrtonum<int>(cyStr.c_str(),10,INVALID_CYCLE);
            }
        }
    }

    //
    // Now we can construct the curve as vtkPolyData.
    //
    int start = 0;
    cutoff.push_back((int)xl.size());       // Make logic easier.
    centering.push_back(useCentering); //      ditto
    int curveIndex = 0; (void) curveIndex;
    if (cutoff.size() > 1)
    {
        // Get ready for the global mesh ?
    }
    for (size_t i = 0 ; i < cutoff.size() ; i++)
    {
#if INTERACTIVEPLOT
        fprintf(stdout,"i=%ld, cutoff=%d\n", i, cutoff[i]) ;
#endif
        if (start == cutoff[i])
        {
            continue;
        }
       
        //
        // Add all of the points to an array.
        //
        int npoints = cutoff[i] - start - (centering[i] == AVT_NODECENT ? 0 : 1);
#ifdef MDSERVER
        vtkUnstructuredGrid *rg = vtkUnstructuredGrid::New();
#else
        vtkUnstructuredGrid *rg = vtkUnstructuredGrid::New();
        vtkPoints   *pts = vtkPoints::New();

        pts->SetNumberOfPoints(npoints);
        rg->SetPoints(pts);
        pts->Delete();
        
        rg->Allocate(npoints);
#endif
#if INTERACTIVEPLOT
        fprintf(stdout,"\t npoints=%d\n", npoints) ;
#endif
        double xmin = DBL_MAX;
        double xmax = -DBL_MAX;
        double ymin = DBL_MAX;
        double ymax = -DBL_MAX;
        double zmin = DBL_MAX;
        double zmax = -DBL_MAX;
        for (int j = 0 ; j < npoints ; j++)
        {
            double X, Y, Z;
            if (centering[i] == AVT_NODECENT)
            {
                X = xl[start+j];
                Y = yl[start+j];
                Z = zl[start+j];
            } else {
                X = (xl[start+j]+xl[start+j+1])/2.0;
                Y = (yl[start+j]+yl[start+j+1])/2.0;
                Z = (zl[start+j]+zl[start+j+1])/2.0;
            }
            if (Z < zmin)
                zmin = Z;
            if (Z > zmax)
                zmax = Z;
            if (Y < ymin)
                ymin = Y;
            if (Y > ymax)
                ymax = Y;
            if (X < xmin)
                xmin = X;
            if (X > xmax)
                xmax = X;
#ifndef MDSERVER
            pts->SetPoint(j, X, Y, Z);
#endif
        }
#ifndef MDSERVER
        vtkIdType verts[2];
        for (int j = 1 ; j < npoints ; j++)
        {
            verts[0] = j-1;
            verts[1] = j;
            rg->InsertNextCell(VTK_LINE, 2, verts);
        }
#endif

        curves.push_back(rg);
#if INTERACTIVEPLOT
        fprintf(stdout,"\t nom=%s\n", curveNames[curves.size()-1].c_str()) ;
#endif
        zExtents.push_back(zmin);
        zExtents.push_back(zmax);
        yExtents.push_back(ymin);
        yExtents.push_back(ymax);
        xExtents.push_back(xmin);
        xExtents.push_back(xmax);
        //
        // Set ourselves up for the next iteration.
        //
        start = cutoff[i];
    }
    // Sets the global range now
    for (size_t i=0; i<curves.size(); i++)
    {
        double extents[6] ;
        extents[0] = xExtents[i*2] ;
        extents[1] = xExtents[i*2+1] ;
        extents[2] = yExtents[i*2] ;
        extents[3] = yExtents[i*2+1] ;
        extents[4] = zExtents[i*2] ;
        extents[5] = zExtents[i*2+1] ;
        if (i == 0) {
            range[0] = extents[0] ;
            range[1] = extents[1] ;
            range[2] = extents[2] ;
            range[3] = extents[3] ;
            range[4] = extents[4] ;
            range[5] = extents[5] ;
        } else {
            range[0] = min(range[0], extents[0]) ;
            range[1] = max(range[1], extents[1]) ;
            range[2] = min(range[2], extents[2]) ;
            range[3] = max(range[3], extents[3]) ;
            range[4] = min(range[4], extents[4]) ;
            range[5] = max(range[5], extents[5]) ;
        }
    }
    //
    // It is possible to have a file that has no header.  This should be
    // interpreted as one curve.  Check for this, since we will later assume
    // the number of curves and the number of curve names is the same.
    //
    if (curves.size() == 1 && curveNames.size() == 0)
    {
        // curveNames.push_back("curve");
        curveNames.push_back(peaderName) ;
    }

    if (curves.size() != curveNames.size())
    {
        debug1 << "The number of curves does not match the number of curve "
               << "names.  Cannot continue with this file." << endl;
        EXCEPTION1(InvalidFilesException, filename.c_str());
    }
    if (clearData == true)
    {
        xl.clear() ;
        yl.clear() ;
        zl.clear() ;
    }
    
    readFile = true;
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat::GetPoint
//
//  Purpose:
//      Gets a point from a line.
//      Go 3D now.
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
//  Copied from Curve2D plugin as of Aug 31, 2018 and adjusted for 3D
//  and make a global mesh with materials out of the figures.
//
//  Modifications:
//    Mark C. Miller, Fri Jan 12 17:04:46 PST 2024
//    Replace atoX/strtoX with vstrtonum
// ****************************************************************************

Curve3DToken
avtCurve3DFileFormat::GetPoint(istream &ifile, double &x, double &y, double &z, string &ln)
{
    char line[256];
    ifile.getline(line, 256, '\n');

    // Do an ASCII check.  We only support text files.
    if (GetStrictMode() && !StringHelpers::IsPureASCII(line,256))
        EXCEPTION2(InvalidFilesException, filename, "Not ASCII.");

    //
    // Parenthesis are special characters for variables names, etc, so just
    // change them to square brackets to "go with the flow"...
    //
    size_t i, nchars = strlen(line);
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
    if (strstr(line, "%") != NULL)
    {
        return HEADER;
    }
    bool allSpace = true;
    size_t len = strlen(line);
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
    char *tmpstr;

    x = vstrtonum<double>(line,10,0,NO_OSTREAM,&ystr);
    if ((x == 0.0) && (ystr == line))
    {
        return INVALID_POINT;
    }
    if (ystr == NULL)
    {
        return VALID_XVALUE;
    }
    ystr = strstr(ystr, " ");
    if (ystr == NULL || ystr == line)
    {
        return VALID_XVALUE;
    }
    
    // Get past the space.
    ystr++;

    y = vstrtonum<double>(ystr,10,0,NO_OSTREAM,&tmpstr);
    if ((y == 0.0) && (tmpstr == ystr))
    {
        return INVALID_POINT;
    }
    if (ystr == NULL)
    {
        return VALID_XVALUE;
    }
    ystr = strstr(ystr, " ");
    if (ystr == NULL || ystr == line)
    {
        return VALID_XVALUE;
    }
    
    // Get past the space.
    ystr++;

    z = vstrtonum<double>(ystr,10,0,NO_OSTREAM,&tmpstr);
    if ((z == 0.0) && (tmpstr == ystr))
    {
        return INVALID_POINT;
    }

    ln = "";
    return VALID_POINT;
}    


// ****************************************************************************
//  Method: avtCurve3DFileFormat::GetTime
//
//  Purpose: Return the time associated with this curve file
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
// ****************************************************************************

double 
avtCurve3DFileFormat::GetTime(void)
{
    return curveTime;
}


// ****************************************************************************
//  Method: avtCurve3DFileFormat::GetCycle
//
//  Purpose: Return the cycle associated with this curve file
//
//  Programmer: Olivier Cessenat
//  Creation:   Oct 5, 2023
//
// ****************************************************************************

int 
avtCurve3DFileFormat::GetCycle(void)
{
    return curveCycle;
}

// ****************************************************************************
//  Method: avtCurve3DFileFormat::GetAuxiliaryData
//
//  Purpose: Manage materials
//
//  Programmer: Olivier Cessenat
//  Creation:   October 5, 2023
//
// ****************************************************************************

void *
avtCurve3DFileFormat::GetAuxiliaryData(const char *var, const char *type, void *,DestructorFunction &df)
{
    void *retval = 0;
#ifdef MDSERVER
    return retval;
#endif
#ifdef PARALLEL
    if (PAR_Rank() != 0)
        return retval ;
#endif
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        if (strcmp(var, "segments") == 0)
        {
            avtMaterial *mat = NULL;
            int ndims = 1 ;
            int dims[3];
            int nelements = 0 ;
            size_t i,j,k ;
            size_t nmats = curves.size() ;
            // Gets the number of nodes and elements :
            for (i=0 ; i<nmats; i++)
            {
                vtkUnstructuredGrid *rg = curves[i] ;
                vtkPoints *lpts = rg->GetPoints() ;
                int npl = lpts->GetNumberOfPoints();
                nelements += (npl-1) ;
            }
            dims[0] = nelements ; 
            int *matnos = new int[nmats];
            for (i=0; i<nmats; i++)
                matnos[i] = i+1;

            char **names = new char *[nmats];
            for (i=0; i<nmats; i++)
            {
                names[i] = new char[strlen(curveNames[i].c_str())+1+4];
                sprintf(names[i], "%s", ("mat_" + curveNames[i]).c_str());
            }

            int *matlist = new int[dims[0]];
            j = -1;
            for (i=0 ; i<nmats; i++)
            {
                vtkUnstructuredGrid *rg = curves[i] ;
                vtkPoints *lpts = rg->GetPoints() ;
                int npl = lpts->GetNumberOfPoints();
                for (k=1; k< npl; k++)
                    matlist[j+k] = 1+i ;
                j += (npl - 1) ;
            }
#if INTERACTIVEPLOT
            fprintf(stdout,"nmats=%ld\n",nmats);
            for (i=0 ; i<nmats; i++)
                fprintf(stdout,"\ti=%ld, no=%d, name=%s\n", i, matnos[i], names[i]);
#endif
            
            mat = new avtMaterial(nmats,matnos,names,ndims,dims,0,matlist,
                                  0, // length of mix arrays
                                  0, // mix_mat array
                                  0, // mix_next array
                                  0, // mix_zone array
                                  0, // mix_vf array
                                  "mesh",0);
            delete [] matnos;
            delete [] matlist;
            for(i=0; i<nmats; ++i)
                delete [] names[i];
            delete [] names;
            
            // Set the return values.
            retval = (void *)mat;
            df = avtMaterial::Destruct;
        }
    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        if (strstr(var, "mesh") != NULL)
        {
            avtIntervalTree *itree = new avtIntervalTree(1, 3);
            itree->AddElement(0, range);
            itree->Calculate(true);
            df = avtIntervalTree::Destruct;
            return ((void *) itree);
        }
        else
            return retval;

    }
    return retval;
}
