/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                             avtPoint3DFileFormat.C                        //
// ************************************************************************* //

#include <avtPoint3DFileFormat.h>

#include <visitstream.h>

#include <vtkCellTypes.h>
#include <vtkUnstructuredGrid.h>
#include <vtkFloatArray.h>

#include <avtDatabase.h>
#include <avtDatabaseMetaData.h>

#include <InvalidFilesException.h>
#include <InvalidVariableException.h>

#include <InstallationFunctions.h>
#include <DebugStream.h>
#include <StringHelpers.h>

const char      *avtPoint3DFileFormat::MESHNAME = "points";

#define COORDINATE_ORDER_DEFAULT 0
#define COORDINATE_ORDER_XYZV    0
#define COORDINATE_ORDER_XYVZ    1

using std::string;
using std::vector;

// ****************************************************************************
//  Method: avtPoint3DFileFormat constructor
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Do not open files in the constructor.
//
// ****************************************************************************

avtPoint3DFileFormat::avtPoint3DFileFormat(const char *fname)
    : avtSTSDFileFormat(fname)
{
    haveReadData = false;
    column1 = NULL;
    column2 = NULL;
    column3 = NULL;
    column4 = NULL;
    points  = NULL;
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Mar 17 16:29:45 PDT 2007
//    Call FreeUpResources instead of doing destructing here.
//
// ****************************************************************************

avtPoint3DFileFormat::~avtPoint3DFileFormat()
{
    FreeUpResources();
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::FreeUpResources
//
//  Purpose:
//      Frees up the resources associated with this format's file.
//
//  Programmer: Hank Childs
//  Creation:   March 17, 2007
//
// ****************************************************************************

void
avtPoint3DFileFormat::FreeUpResources(void)
{
    haveReadData = false;

    if (column1 != NULL)
    {
        column1->Delete();
        column1 = NULL;
    }
    if (column2 != NULL)
    {
        column2->Delete();
        column2 = NULL;
    }
    if (column3 != NULL)
    {
        column3->Delete();
        column3 = NULL;
    }
    if (column4 != NULL)
    {
        column4->Delete();
        column4 = NULL;
    }
    if (points != NULL)
    {
        points->Delete();
        points = NULL;
    }
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh (a bunch of 3D points).
//
//  Notes:   The calling function believes that it owns the returned memory.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

vtkDataSet *
avtPoint3DFileFormat::GetMesh(const char *mesh)
{
    if (!haveReadData)
    {
        ReadData();
    }

    if (strcmp(mesh, MESHNAME) != 0)
    {
        EXCEPTION1(InvalidVariableException, mesh);
    }

    points->Register(NULL);
    return points;
}


// ****************************************************************************
//  Method; avtPoint3DFileFormat::GetVar
//
//  Purpose:
//      Gets a variable.
//
//  Notes:   The calling function believes that it owns the returned memory.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
// ****************************************************************************

vtkDataArray *
avtPoint3DFileFormat::GetVar(const char *var)
{
    if (!haveReadData)
    {
        ReadData();
    }

    vtkDataArray *rv = NULL;
    if (varnames[0] == var)
        rv = column1;
    if (varnames[1] == var)
        rv = column2;
    if (varnames[2] == var)
        rv = column3;
    if (varnames[3] == var)
        rv = column4;

    if (rv == NULL)
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    rv->Register(NULL);
    return rv;
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Declares which variables are available for this file format.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Read in the variable names before populating the meta-data.
//
// ****************************************************************************

void
avtPoint3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    if (!haveReadData)
    {
        ReadData();
    }

    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = MESHNAME;
    mesh->meshType = AVT_POINT_MESH;
    mesh->blockOrigin = 0;
    mesh->spatialDimension = 3;
    mesh->topologicalDimension = 0;
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    for (int i = 0 ; i < 4 ; i++)
    {
        AddScalarVarToMetaData(md, varnames[i], MESHNAME, AVT_NODECENT, NULL);
    }
}


// ****************************************************************************
//  Method: avtPoint3DFileFormat::ReadData
//
//  Purpose:
//      Does the work of actually reading in the data.
//
//  Programmer: Hank Childs
//  Creation:   March 15, 2003
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:21:32 PDT 2003
//    Read in the variable names as well.
//
//    Hank Childs, Wed May  7 08:45:08 PDT 2003 
//    We are consistently adding one bogus point at the end of the array.
//    Remove that.
//
//    Hank Childs, Wed Oct  8 15:36:36 PDT 2003
//    Removed another bogus line, this time it was at the beginning, with
//    the variable declaration.
//
//    Brad Whitlock, Mon Jun 5 10:58:49 PDT 2006
//    Added support for a config file as well as setting coordflag in the
//    file itself.
//
//    Hank Childs, Sat Mar 17 16:29:45 PDT 2007
//    Make reading for meta-data more lightweight.
//
//    Jeremy Meredith, Thu Jan  7 12:04:12 EST 2010
//    Check some lines to make sure it's ASCII.
//
//    Jeremy Meredith, Fri Jan  8 16:17:34 EST 2010
//    Added extra error checking and forced more parsing in strict mode.
//
// ****************************************************************************

void
avtPoint3DFileFormat::ReadData(void)
{
    const char *mName = "avtPoint3DFileFormat::ReadData: ";
    int      i;

    ifstream ifile(filename);
    if (ifile.fail())
    {
        EXCEPTION1(InvalidFilesException, filename);
    }

    // Read the coordinate ordering out of the file.
    int coordFlag = COORDINATE_ORDER_XYZV;
    ReadConfigFile(coordFlag);

    // Read the variable names.
    for (i = 0 ; i < 4 ; i++)
    {
        char buf[1024];
        ifile >> buf;
        if (!StringHelpers::IsPureASCII(buf, 1024))
            EXCEPTION2(InvalidFilesException, filename, "Not ASCII.");
        varnames.push_back(buf);
    }

    if (avtDatabase::OnlyServeUpMetaData() && !GetStrictMode())
        return;

    char     line[1024];

    // We read out the four variables, but not the newline at the end.
    // Get that now.
    ifile.getline(line, 1024);
    if (!StringHelpers::IsPureASCII(line, 1024))
        EXCEPTION2(InvalidFilesException, filename, "Not ASCII.");

    vector<float> var1;
    vector<float> var2;
    vector<float> var3;
    vector<float> var4;

    int linesToCheckForAscii = 100;
    while (!ifile.eof())
    {
        line[0] = '\0';
        ifile.getline(line, 1024);
        if (--linesToCheckForAscii > 0 &&
            !StringHelpers::IsPureASCII(line, 1024))
            EXCEPTION2(InvalidFilesException, filename, "Not ASCII.");

        // Allow the user to specify "coordflag" in the file.
        if(line[0] == '#')
        {
            if(strncmp(line+1, "coordflag", 9) == 0)
            {
                debug4 << mName << "Reading coordflag value from file: ";
                if(strncmp(line+9+2, "xyzv", 4) == 0)
                {
                    coordFlag = COORDINATE_ORDER_XYZV;
                    debug4 << "xyzv";
                }
                else if(strncmp(line+9+2, "xyvz", 4) == 0)
                {
                    coordFlag = COORDINATE_ORDER_XYVZ;
                    debug4 << "xyvz";
                }
                debug4 << endl;
            }
        }
        else
        {
            float a=0, b=0, c=0, d=0;
            int n = sscanf(line, "%f %f %f %f", &a, &b, &c, &d);
            if (GetStrictMode() && n != 4)
            {
                EXCEPTION2(InvalidFilesException, filename,
                           "Bad line in file; less than four values");
            }
            var1.push_back(a);
            var2.push_back(b);
            var3.push_back(c);
            var4.push_back(d);
        }
    }

    int npts = var1.size();
    // The EOF logic always seems to get an extra point. Take off the last one.
    npts -= 1;

    column1 = vtkFloatArray::New();
    column1->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column1->SetTuple1(i, var1[i]);
    }

    column2 = vtkFloatArray::New();
    column2->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column2->SetTuple1(i, var2[i]);
    }

    column3 = vtkFloatArray::New();
    column3->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column3->SetTuple1(i, var3[i]);
    }

    column4 = vtkFloatArray::New();
    column4->SetNumberOfTuples(npts);
    for (i = 0 ; i < npts ; i++)
    {
        column4->SetTuple1(i, var4[i]);
    }

    vtkPoints *p = vtkPoints::New();
    p->SetNumberOfPoints(npts);
    if(coordFlag == COORDINATE_ORDER_XYZV)
    {
        for (i = 0 ; i < npts ; i++)
             p->SetPoint(i, var1[i], var2[i], var3[i]);
    }
    else // COORDINATE_ORDER_XYVZ
    {
        for (i = 0 ; i < npts ; i++)
             p->SetPoint(i, var1[i], var2[i], var4[i]);
    }

    points = vtkUnstructuredGrid::New();
    points->SetPoints(p);
    p->Delete();

    points->Allocate(npts);
    vtkIdType onevertex[1];
    for (i = 0 ; i < npts ; i++)
    {
         onevertex[0] = i;
         points->InsertNextCell(VTK_VERTEX, 1, onevertex);
    }

    haveReadData = true;
}

// ****************************************************************************
// Method: avtPoint3DFileFormat::ReadConfigFile
//
// Purpose: 
//   Reads a config file out of the home VisIt directory that allows the user
//   to configure the Point3D file format.
//
// Arguments:
//   coordFlag : The order of the coordinates.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 5 10:51:59 PDT 2006
//
// Modifications:
//   
// ****************************************************************************

bool
avtPoint3DFileFormat::ReadConfigFile(int &coordFlag)
{
    const char *mName = "avtPoint3DFileFormat::ReadConfigFile: ";
    bool retval = false;

    std::string configFile(GetUserVisItDirectory());
    configFile += "Point3D.ini";
 
    coordFlag = COORDINATE_ORDER_DEFAULT;
 
    // Open the file.
    ifstream ifile(configFile.c_str());
    if (ifile.fail())
    {
        debug4 << mName << "Could not open config file: "
               << configFile.c_str() << endl;
    }
    else
    {
        debug4 << mName << "Opened config file: "
               << configFile.c_str() << endl;

        char line[1024];
        for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
        {
            // Get the line
            ifile.getline(line, 1024);

            if(strncmp(line, "coordflag", 9) == 0)
            {
                debug4 << mName << "Reading coordflag value from file: ";
                if(strncmp(line+9+1, "xyzv", 4) == 0)
                {
                    coordFlag = COORDINATE_ORDER_XYZV;
                    debug4 << "xyzv";
                }
                else if(strncmp(line+9+1, "xyvz", 4) == 0)
                {
                    coordFlag = COORDINATE_ORDER_XYVZ;
                    debug4 << "xyvz";
                }
                debug4 << endl;
            }
        }

        retval = true;
    }

    debug4 << mName << "coordFlag=" << coordFlag << endl;

    return retval;
}
