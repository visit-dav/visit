// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtPlainTextFileFormat.C                           //
// ************************************************************************* //

#include <avtPlainTextFileFormat.h>

#include <cstring>
#include <string>

#include <vtkPointData.h>
#include <vtkDoubleArray.h>
#include <vtkRectilinearGrid.h>
#include <vtkUnstructuredGrid.h>
#include <vtkPolyData.h>
#include <vtkCellArray.h>
#include <vtkVisItUtility.h>

#include <avtDatabaseMetaData.h>

#include <DBOptionsAttributes.h>
#include <Expression.h>
#include <StringHelpers.h>
using StringHelpers::vstrtonum;

#include "visit_gzstream.h"

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>


using     std::string;
using     std::vector;

string
TrimLeadingandTrailingWhitespace(string str)
{
    size_t start = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");
    size_t range = end - start + 1;
    if (start == string::npos || range <= 1)
    {
        static size_t cnt = 0;
        char tmp[16];
        snprintf(tmp, sizeof(tmp), "unknown_%zu", cnt ++);
        return tmp;
    }
    return str.substr(start, range);
}

// ****************************************************************************
//  Method: avtPlainTextFileFormat constructor
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct 29 12:32:11 PDT 2008
//    Made it possible for curves to have any column as the abscissa
//
//    Jeremy Meredith, Fri Apr 30 09:57:29 EDT 2010
//    Changed data layout field to have a new default, and so I renamed
//    it so it wouldn't conflict with the old values.
//
// ****************************************************************************

avtPlainTextFileFormat::avtPlainTextFileFormat(const char *fn,
                                               const DBOptionsAttributes *readOpts)
    : avtSTSDFileFormat(fn)
{
    fileRead = false;
    filename = fn;
    ncolumns = 0;
    nrows = 0;

    // Set the options from the DBOptionsAttributes
    format = Grid;
    firstRowIsHeader = false;
    skipLines = 0;
    xcol = -1;
    ycol = -1;
    zcol = -1;

    if (readOpts &&
        readOpts->FindIndex("Data layout")>=0)
    {
        int index = readOpts->GetEnum("Data layout");
        if (index==0) format = Columns;
        if (index==1) format = Grid;
    }
    if (readOpts &&
        readOpts->FindIndex("First row has variable names")>=0)
    {
        firstRowIsHeader = readOpts->GetBool("First row has variable names");
    }
    if (readOpts &&
        readOpts->FindIndex("Lines to skip at beginning of file")>=0)
    {
        skipLines = readOpts->GetInt("Lines to skip at beginning of file");
    }
    if (readOpts &&
        readOpts->FindIndex("Column for X coordinate (or -1 for none)")>=0)
    {
        xcol = readOpts->GetInt("Column for X coordinate (or -1 for none)");
    }
    if (readOpts &&
        readOpts->FindIndex("Column for Y coordinate (or -1 for none)")>=0)
    {
        ycol = readOpts->GetInt("Column for Y coordinate (or -1 for none)");
    }
    if (readOpts &&
        readOpts->FindIndex("Column for Z coordinate (or -1 for none)")>=0)
    {
        zcol = readOpts->GetInt("Column for Z coordinate (or -1 for none)");
    }

    acol = -2; // abscissa column for curves
    if (xcol<0 && ycol<0 && zcol<0)
        acol = -1;
    else if (xcol>=0 && ycol<0  && zcol<0)
        acol = xcol;
    else if (xcol<0  && ycol>=0 && zcol<0)
        acol = ycol;
    else if (xcol<0  && ycol<0  && zcol>=0)
        acol = zcol;
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Modifications:
//
//    Hank Childs, Wed May 26 09:05:44 PDT 2010
//    Fix memory bloat issue from STL.
//
//    Mark C. Miller, Tue Dec  1 13:07:23 PST 2020
//    Switched to double precision data
// ****************************************************************************

void
avtPlainTextFileFormat::FreeUpResources(void)
{
    variableNames.clear();
    data.clear();
    std::vector< std::vector<double> > tmp;
    data.swap(tmp); // this makes capacity() drop to 0, which is better than
                    // what clear() does.
    fileRead = false;
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Modifications:
//
//    Mark C. Miller, Wed Oct 29 12:32:11 PDT 2008
//    Made it possible for curves to have any column as the abscissa
// ****************************************************************************

void
avtPlainTextFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    ReadFile();

    if (format == Grid)
    {
        // rectilinear grid
        AddMeshToMetaData(md, "mesh", AVT_RECTILINEAR_MESH, NULL,
                          1,0, 2,2);
        AddScalarVarToMetaData(md, variableNames[0], "mesh", AVT_NODECENT);
    }
    else if (format == Columns)
    {
        if (acol > -2)
        {
            // curves
            for (int i=0; i<ncolumns; i++)
            {
                if (i == acol)
                    continue;
                avtCurveMetaData *curve = new avtCurveMetaData;
                curve->name = variableNames[i];
                md->Add(curve);
            }
        }
        else
        {
            // pointmesh
            int spatialdim = (zcol<0) ? 2 : 3;
            AddMeshToMetaData(md, "mesh", AVT_POINT_MESH, NULL,
                              1,0, spatialdim,1);

            // variables
            for (int i=0; i<ncolumns; i++)
            {
                AddScalarVarToMetaData(md, variableNames[i], "mesh",
                                       AVT_NODECENT);
            }
        }
    }
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Modifications:
//    Jeremy Meredith, Thu Jun 26 17:26:22 EDT 2008
//    Changed curves to be created as rectilinear grids instead of polydata
//    (so that they work with expressions).
//
//    Mark C. Miller, Wed Oct 29 12:32:11 PDT 2008
//    Made it possible for curves to have any column as the abscissa
//
//    Jeremy Meredith, Thu Jan 20 13:26:04 EST 2011
//    Fixed a copy/paste bug with creating 1D/2D point meshes.  (3D was okay.)
//
//    Mark C. Miller, Tue Dec  1 13:05:46 PST 2020
//    Switched to double precision for coords
// ****************************************************************************

vtkDataSet *
avtPlainTextFileFormat::GetMesh(const char *meshname)
{
    ReadFile();
    if (format == Grid)
    {
        // rectilinear grid
        vtkRectilinearGrid   *rgrid   = vtkRectilinearGrid::New(); 

        vtkDoubleArray *coords[3];
        int dims[3] = {ncolumns, nrows, 1};
        for (int i = 0 ; i < 3 ; i++)
        {
            coords[i] = vtkDoubleArray::New();
            coords[i]->SetNumberOfTuples(dims[i]);

            for (int j = 0 ; j < dims[i] ; j++)
            {
                coords[i]->SetComponent(j, 0, j);
            }
        }
        rgrid->SetDimensions(dims);
        rgrid->SetXCoordinates(coords[0]);
        rgrid->SetYCoordinates(coords[1]);
        rgrid->SetZCoordinates(coords[2]);
        coords[0]->Delete();
        coords[1]->Delete();
        coords[2]->Delete();

        return rgrid;        
    }
    else if (format == Columns)
    {
        if (acol > -2)
        {
            // curves
            int index = -1;
            for (int i=0; i< (int)variableNames.size(); i++)
            {
                if (meshname == variableNames[i])
                    index = i;
            }
            if (index < 0)
                EXCEPTION1(InvalidVariableException, meshname);


            vtkDoubleArray *vals = vtkDoubleArray::New();
            vals->SetNumberOfComponents(1);
            vals->SetNumberOfTuples(nrows);
            vals->SetName(meshname);

            vtkRectilinearGrid *rg =
                vtkVisItUtility::Create1DRGrid(nrows,VTK_DOUBLE);
            rg->GetPointData()->SetScalars(vals);

            vtkDataArray *xc = rg->GetXCoordinates();
            for (int j = 0 ; j < nrows ; j++)
            {
                if (acol == -1)
                    xc->SetComponent(j, 0, j);
                else
                    xc->SetComponent(j, 0, data[j][acol]);
                vals->SetValue(j, data[j][index]);
            }
            vals->Delete();

            return rg;
        }
        else
        {
            // pointmesh
            vtkPolyData *pd  = vtkPolyData::New();
            vtkPoints   *pts = vtkPoints::New();

            pts->SetNumberOfPoints(nrows);
            pd->SetPoints(pts);
            pts->Delete();
            for (int j = 0 ; j < nrows ; j++)
            {
                double x = (xcol<0 || xcol>=ncolumns) ? 0 : data[j][xcol];
                double y = (ycol<0 || ycol>=ncolumns) ? 0 : data[j][ycol];
                double z = (zcol<0 || zcol>=ncolumns) ? 0 : data[j][zcol];
                pts->SetPoint(j, x, y, z);
            }
 
            vtkCellArray *verts = vtkCellArray::New();
            pd->SetVerts(verts);
            verts->Delete();
            for (int k = 0 ; k < nrows ; k++)
            {
                verts->InsertNextCell(1);
                verts->InsertCellPoint(k);
            }

            return pd;
        }
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Mark C. Miller, Tue Dec  1 13:06:00 PST 2020
//  Switched to double precision for data
// ****************************************************************************

vtkDataArray *
avtPlainTextFileFormat::GetVar(const char *varname)
{
    ReadFile();

    if (format == Grid)
    {
        // just one var; do it blindly
        vtkDoubleArray *values = vtkDoubleArray::New();
        values->SetNumberOfTuples(nrows*ncolumns);
        int ctr = 0;
        for (int i=0; i<nrows; i++)
        {
            for (int j=0; j<ncolumns; j++)
            {
                values->SetComponent(ctr, 0, data[i][j]);
                ctr++;
            }
        }
        return values;
    }
    else if (format == Columns)
    {
        int index = -1;
        for (int i=0; i<(int)variableNames.size(); i++)
        {
            if (varname == variableNames[i])
                index = i;
        }
        if (index < 0)
            EXCEPTION1(InvalidVariableException, varname);

        vtkDoubleArray *values = vtkDoubleArray::New();
        values->SetNumberOfTuples(nrows);
        int ctr = 0;
        for (int i=0; i<nrows; i++)
        {
            values->SetComponent(ctr, 0, data[i][index]);
            ctr++;
        }
        return values;
    }

    return NULL;
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::GetVectorVar
//
//  Purpose:
//      Gets a vector variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      varname    The name of the variable requested.
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
// ****************************************************************************

vtkDataArray *
avtPlainTextFileFormat::GetVectorVar(const char *varname)
{
    return NULL;
}


// ****************************************************************************
//  Method: avtPlainTextFileFormat::ReadFile
//
//  Purpose:
//      Read the file if it's not already been read.
//
//  Note: the mdserver will shortcut out once it reads the first few rows.
//
//  Arguments:
//      none
//
//  Programmer: Jeremy Meredith
//  Creation:   January 24, 2008
//
//  Modifications:
//    Jeremy Meredith, Fri Jan 25 17:53:13 EST 2008
//    Support both CSV and whitespace-delimited files.  Sped up
//    parsing as well.
//
//    Brad Whitlock, Mon Apr 27 09:00:52 PDT 2009
//    I increased the line length from 4K to 32K.
//
//    Jeremy Meredith, Wed Jan  6 13:26:52 EST 2010
//    Check for an ASCII file or empty data.
//
//    Hank Childs, Mon Feb  4 10:52:01 PST 2013
//    Improve logic for files with variable names.
//
//    Mark C. Miller, Tue Dec  1 13:06:57 PST 2020
//    Switched to double precision for rows of data
// 
//    Justin Privitera, Fri Apr  1 11:18:38 PDT 2022
//    Added TrimLeadingandTrailingWhitespace function
//    to remove whitespace from variable names
//    as they are read.
// 
// ****************************************************************************

void
avtPlainTextFileFormat::ReadFile()
{
    if (fileRead)
        return;

    visit_ifstream in(filename.c_str());
    if (!in())
        EXCEPTION1(InvalidFilesException, filename.c_str());

    // skip the first lines if asked
    int linelen = 32768;
    char *buff = new char[linelen];
    memset(buff, 0, sizeof(char) * linelen);
    for (int l=0; l<skipLines; l++)
    {
        in().getline(buff, linelen);
        if (!StringHelpers::IsPureASCII(buff, linelen))
            EXCEPTION2(InvalidFilesException, filename.c_str(), "Not ASCII.");
    }


    // actually read the data; one vector per row
    ncolumns = 0;
    nrows = 0;
    bool firstRow = true;
    in().getline(buff, linelen);
    if (!StringHelpers::IsPureASCII(buff, linelen))
        EXCEPTION2(InvalidFilesException, filename.c_str(), "Not ASCII.");

    bool comma = false;
    for (char *p=buff; *p!='\0'; p++)
        if (*p == ',')
            comma = true;

    while (in())
    {
        int len = (int)strlen(buff);
        char *start = buff;
        vector<double> row;

        if (comma)
        {
            while ((start-buff)<=len)
            {
                char *end = start;
                while (*end != '\0' && (end-buff)<=len &&
                       !(*end == ','))
                {
                    end++;
                }
                *end = '\0';
                if (firstRowIsHeader && firstRow)
                {
                    variableNames.push_back(
                        TrimLeadingandTrailingWhitespace(start));
                }
                else
                {
                    double value = vstrtonum<double>(start);
                    row.push_back(value);
                }
                start = end+1;
            }
        }
        else
        {
            while (*start != '\0' && (start-buff)<=len &&
                   (*start == ' ' || *start == '\t'))
            {
                start++;
            }
            while ((start-buff)<=len)
            {
                char *end = start;
                while (*end != '\0' && (end-buff)<=len &&
                       !(*end == ' ' || *end == '\t'))
                {
                    end++;
                }
                *end = '\0';
                while (*(end+1) != '\0' && (end+1-buff)<=len &&
                       (*(end+1) == ' ' || *(end+1) == '\t'))
                {
                    end++;
                    *end = '\0';
                }
                if (start != end)
                {
                    if (firstRowIsHeader && firstRow)
                    {
                        variableNames.push_back(
                            TrimLeadingandTrailingWhitespace(start));
                    }
                    else
                    {
                        double value = vstrtonum<double>(start);
                        row.push_back(value);
                    }
                }
                start = end+1;
            }
        }

        int rowlen = (firstRowIsHeader && firstRow) ?(int) variableNames.size() : (int)row.size();
        if (firstRow)
        {
            ncolumns = rowlen;
        }
        else if (rowlen != ncolumns)
        {
            break;
        }
        if (!(firstRowIsHeader && firstRow))
        {
            data.push_back(row);
            nrows++;
        }
        firstRow = false;
        in().getline(buff, linelen);

        if (nrows < 5)
        {
            // Check the first several rows to make sure it's ASCII.
            if (!StringHelpers::IsPureASCII(buff, linelen))
                EXCEPTION2(InvalidFilesException, filename.c_str(), "Not ASCII.");
        }
        else
        {
#ifdef MDSERVER
            // We don't need to know how many rows there are in the
            // MDServer, nor do we need to read all the data.  We do
            // want to make sure there's at least one row of data,
            // though, because if not, its probably not even a plain
            // text file.  And we probably don't want to break out
            // until we've got a good clue this file is truly ASCII.
            break;
#endif
        }
    }
    delete [] buff;

    if (ncolumns == 0 || (!firstRowIsHeader && nrows == 0))
    {
        EXCEPTION2(InvalidFilesException, filename.c_str(),
                   "Couldn't parse anything meaningful from the file.");
    }

    // fix up variable names
    if (format == Grid)
    {
        // we need exactly one variable name for 2D grid files
        if (variableNames.size() > 1)
            variableNames.resize(1);
        if (variableNames.size() == 0)
            variableNames.push_back("var");
    }
    else if (format == Columns)
    {
        // If we didn't get enough variable names, fill it out with
        // generic values; and if it's too much, trim it
        for (int i=(int)variableNames.size(); i<ncolumns; i++)
        {
            char str[20];
            sprintf(str, "var%02d", i);
            variableNames.push_back(str);
        }
        if (variableNames.size() > (size_t)ncolumns)
            variableNames.resize(ncolumns);
    }

    //in().close();
    fileRead = true;
}
