/*****************************************************************************
*
* Copyright (c) 2000 - 2017, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
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
//                            avtPLOT3DFileFormat.C                          //
// ************************************************************************* //

#include <avtPLOT3DFileFormat.h>

#include <vector>
#include <string>

#include <vtkDataArray.h>
#include <vtkDataSet.h>
#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPointData.h>
#include <vtkPLOT3DReader.h>

#include <Expression.h>

#include <avtDatabaseMetaData.h>
#include <visit-config.h> // FOR VISIT_SLASH_STRING

#include <BadIndexException.h>
#include <DebugStream.h>
#include <ImproperUseException.h>
#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <DBOptionsAttributes.h>
#include <FileFunctions.h>
#include <TimingsManager.h>

using std::vector;
using std::string;


// ****************************************************************************
//  Method: avtPLOT3DFileFormat constructor
//
//  Arguments:
//      fname    The name of the PLOT3D file.
//
//  Programmer:  Hank Childs
//  Creation:    May 3, 2002
//
//  Modifications:
//
//    Hank Childs, Mon Apr  7 18:37:59 PDT 2003
//    Do not read in the file in the constructor.
//
//    Abel Gezahegne, Sept 12, 2004
//    Add the Visit plot3d file extension.
//
//    Kathleen Biagas, Tue Aug 19 14:30:10 PDT 2014
//    Save metafile for later use. Reader is now external, has updated api.
//
//    Kathleen Biagas, Thu Apr 23 10:38:31 PDT 2015
//    Added haveSolutionFile.
//
//    Kathleen Biagas, Wed Jun 17 17:50:22 PDT 2015
//    Turn on the reader's auto-detection
//
//    Kathleen Biagas, Fri Jun 26 10:24:26 PDT 2015
//    Change this from type STMD to MTMD.
//    Add solutionFiles, times, haveReadMetaFile, haveProcessedQ, previousTS,
//    xFileName, qFileName.
//
//    Kathleen Biagas, Thu Aug 27 12:33:56 PDT 2015
//    Change 'times' to single 'time'.  Add solutionHasValidTime flag. Add
//    Read option for non-record based fortran binaries (no byte count).
//
// ****************************************************************************

avtPLOT3DFileFormat::avtPLOT3DFileFormat(const char *fname,
    DBOptionsAttributes *readOpts)
    : avtMTMDFileFormat(fname),
       visitMetaFile(),
       xFileName(),
       qFileName(),
       solutionRoot(),
       solutionFiles()
{
    bool guessedQFile = false;
    haveSolutionFile = false;
    haveReadMetaFile = false;
    haveProcessedQ = false;
    previousTS = -1;
    solutionHasValidTime = true;
    time = INVALID_TIME;
    if (strstr(fname, ".vp3d") != NULL)
    {
        visitMetaFile = fname;
    }
    else if (strstr(fname, ".x") != NULL)
    {
        xFileName = fname;

        char soln_file[1024];
        const char *q = strstr(fname, ".x");
        strncpy(soln_file, fname, q-fname);
        strcpy(soln_file + (q-fname), ".q");
        // see if this q file exists
        FILE *file = fopen(soln_file, "r");
        if (file != NULL)
        {
            fclose(file);
            qFileName = soln_file;
            guessedQFile = true;
            haveSolutionFile = true;
        }
    }
    else if (strstr(fname, ".q") != NULL)
    {
        char points_file[1024];
        const char *x = strstr(fname, ".q");
        strncpy(points_file, fname, x-fname);
        strcpy(points_file + (x-fname), ".x");

        xFileName = points_file;
        haveSolutionFile = true;
    }
    else
    {
        if (readOpts && readOpts->FindIndex("Solution (Q) File Name") >= 0)
        {
            qFileName = readOpts->GetString("Solution (Q) File Name");
            if (!qFileName.empty())
            {
                xFileName = fname;
                string::size_type pos = xFileName.rfind(VISIT_SLASH_STRING);
                string solnFile = xFileName.substr(0, pos+1);
                qFileName = solnFile + qFileName;
                haveSolutionFile = true;
            }
        }
        if (xFileName.empty())
        {
            // so give up.
            //
            debug1 << "PLOT3D reader giving up with file identification of "
                   << fname << endl;
            EXCEPTION0(ImproperUseException);
        }
    }

    reader = vtkPLOT3DReader::New();
    
    if (!xFileName.empty())
        reader->SetXYZFileName(xFileName.c_str());
    if (!qFileName.empty())
        reader->SetQFileName(qFileName.c_str());

    // we assume 
    if (visitMetaFile.empty() && readOpts)
    {
        if (readOpts->FindIndex("File Format") >= 0)
        {
            int index = readOpts->GetEnum("File Format");
            switch(index) 
            {
                case 0: reader->BinaryFileOff();
                        reader->HasByteCountOff();
                        break;
                case 1: reader->BinaryFileOn();
                        reader->HasByteCountOff();
                        break;
                case 2: reader->BinaryFileOn();
                        reader->HasByteCountOn();
                        break;
                case 3: reader->BinaryFileOn();
                        reader->HasByteCountOff();
                        break;
            }
        }
        if (readOpts->FindIndex("Big Endian") >= 0)
        {
            bool bigEnd = readOpts->GetBool("Big Endian");
            if (bigEnd)
                reader->SetByteOrderToBigEndian();
            else 
                reader->SetByteOrderToLittleEndian();
        }
        if (readOpts->FindIndex("Multi Grid") >= 0)
        {
            bool mg = readOpts->GetBool("Multi Grid");
            if (mg)
                reader->MultiGridOn();
            else 
                reader->MultiGridOff();
        }
        if (readOpts->FindIndex("Double Precision") >= 0)
        {
            bool dp = readOpts->GetBool("Double Precision");
            if (dp)
                reader->DoublePrecisionOn();
            else 
                reader->DoublePrecisionOff();
        }
        if (readOpts->FindIndex("3D") >= 0)
        {
            bool td = readOpts->GetBool("3D");
            if (td)
                reader->TwoDimensionalGeometryOff();
            else 
                reader->TwoDimensionalGeometryOn();
        }
        if (readOpts->FindIndex("IBlanking") >= 0)
        {
            bool ib = readOpts->GetBool("IBlanking");
            if (ib)
                reader->IBlankingOn();
            else 
                reader->IBlankingOff();
        }
        if (readOpts->FindIndex("Solution (Q) File Name") >= 0)
        {
            string qFN = readOpts->GetString("Solution (Q) File Name");
            if (!qFN.empty())
            {
                if (qFileName.empty() || guessedQFile)
                {
                    string xFN(fname);
                    string::size_type pos = xFN.rfind(VISIT_SLASH_STRING);
                    string solnFile = xFN.substr(0, pos+1);
                    qFileName = solnFile + qFN;
                    reader->SetQFileName(qFileName.c_str());
                    haveSolutionFile = true;
                }
            }
        }
        if (readOpts->FindIndex("Solution Time field accurate") >= 0)
        {
            solutionHasValidTime = readOpts->GetBool("Solution Time field accurate");
        }
        if (readOpts->FindIndex("Gas constant R") >= 0)
        {
            double R = readOpts->GetDouble("Gas constant R");
            reader->SetR(R);
        }
        if (readOpts->FindIndex("Gas constant Gamma") >= 0)
        {
            double gamma = readOpts->GetDouble("Gas constant Gamma");
            reader->SetGamma(gamma);
        }
    }
    else
    {
        // set up some assumed defaults that may or may not be specified in 
        // the Meta File 
        reader->MultiGridOff();
        reader->BinaryFileOn();
        reader->SetByteOrderToBigEndian();
        reader->IBlankingOff();
        reader->TwoDimensionalGeometryOff();
        reader->DoublePrecisionOff();
    }
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat destructor
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
// ****************************************************************************

avtPLOT3DFileFormat::~avtPLOT3DFileFormat()
{
    reader->Delete();
    reader = NULL;
    solutionFiles.clear();
}

// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetTime
//
//  Purpose:
//      Returns all the time values for the requested timestep.
//
//  Programmer: Kathleen Biagas
//  Creation:   June 26, 2015
//
// ****************************************************************************

double
avtPLOT3DFileFormat::GetTime(int ts)
{
    if (solutionFiles.empty())
       return INVALID_TIME;

    if(solutionHasValidTime)
        time = reader->GetTime();
    else if ((int)solutionFiles.size() > 1 && 
        ts >= 0 && ts < (int)solutionFiles.size())
    time = ((double) GuessCycle(solutionFiles[ts].c_str()));
    return time;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Thu Jun 18 10:40:24 PDT 2015
//
// ****************************************************************************

int
avtPLOT3DFileFormat::GetNTimesteps()
{
    int nT = 1;
    if (!visitMetaFile.empty() && !haveReadMetaFile)
    {
        if (!ReadVisItMetaFile())
        {
            EXCEPTION2(InvalidFilesException, visitMetaFile.c_str(),
                       "Could not parse the .vp3d meta file");
        }
    }
    if (!haveProcessedQ)
    {
        ProcessQForTimeSeries();
    }
    if (!solutionFiles.empty())
    {
        nT = (int)solutionFiles.size();
    }
    return nT;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: biagas2 -- generated by xml2avt
//  Creation:   Thu Jun 18 10:40:24 PDT 2015
//
// ****************************************************************************

void
avtPLOT3DFileFormat::FreeUpResources(void)
{
  // don't think we want to do anything here if this is called at every
  // timestep change.  
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetMesh
//
//  Purpose:
//      Returns the grid associated with a timestate (if applicable) and
//      domain number.
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      domain      The domain number.
//      name         The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
//    Kathleen Biagas, Wed Jun 17 17:51:21 PDT 2015
//    Change how output is retrieved from reader.
//
// ****************************************************************************

vtkDataSet *
avtPLOT3DFileFormat::GetMesh(int timestate, int domain, const char *name)
{
    if (domain < 0 || domain >= reader->GetNumberOfGrids())
    {
        EXCEPTION2(BadIndexException, domain, reader->GetNumberOfGrids());
    }

    if (strcmp(name, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    vtkDataSet *rv = NULL;
    reader->SetGridNumber(domain);
    if (reader->ReadGrid())
    {
        vtkDataSet *cb = reader->GetOutput();
        if (cb != NULL)
        {
            rv = cb->NewInstance();
            rv->ShallowCopy(cb);
        }
    }
    else
    {
        debug3 << "avtPLOT3DFileFormat::GetMesh: Error retreiving Grid for"
               << " timestate " << timestate << " domain " << domain << endl;
    }
    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a timesate and domain number.
//
//  Arguments:
//      timestate  The time state index.
//      domain     The domain number.
//      name       The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
//    Kathleen Biagas, Tue Aug 19 14:33:38 PDT 2014
//    Changed capitalization of var names, to match the vtk reader.
//    Add VorticityMagnitude.
//
//    Kathleen Biagas, Wed Jun 17 15:38:30 PDT 2015
//    Since we process one var at a time, make sure function numbers are reset.
//    Change how output is retrieved from reader.
//
//    Kathleen Biagas, Thu Aug 27 13:16:20 PDT 2015
//    Reader now processes var name to retrieve correct function.
//    Memory allocation is handled here instead of reader.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVar(int timestate, int domain, const char *name)
{
    reader->SetGridNumber(domain);
    reader->ReadGrid();
    int nPts = reader->GetNumberOfPoints();
    vtkDataArray *rv = NULL;
    int success;
    if (reader->GetPrecision() == 4)
    {
        rv  = vtkFloatArray::New();
        rv->SetNumberOfTuples(nPts);
        success = reader->GetFunction_float(name, (float*)rv->GetVoidPointer(0));
    }
    else
    {
        rv  = vtkDoubleArray::New();
        rv->SetNumberOfTuples(nPts);
        success = reader->GetFunction_double(name, (double*)rv->GetVoidPointer(0));
    }

    if (success == VTK_OK)
    {
        rv->Register(NULL);
    }
    else
    {
        rv->Delete();
        rv = NULL;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the vector variable associated with a timestate and domain number.
//
//  Arguments:
//      timestate  The time state index.
//      dom        The domain number.
//      name       The variable name.
//
//  Programmer:   Hank Childs
//  Creation:     May 6, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
//    Kathleen Biagas, Tue Aug 19 14:33:38 PDT 2014
//    Changed capitalization of var names, to match the vtk reader.
//    Add StrainRate.
//
//    Kathleen Biagas, Wed Jun 17 15:38:30 PDT 2015
//    Since we process one var at a time, make sure function numbers are reset.
//    Change how output is retrieved from reader.
//
//    Kathleen Biagas, Thu Aug 27 13:16:20 PDT 2015
//    Reader now processes var name to retrieve correct function.
//    Memory allocation is handled here instead of reader.
//
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVectorVar(int timestate, int domain,const char *name)
{
    reader->SetGridNumber(domain);
    reader->ReadGrid();
    int nPts = reader->GetNumberOfPoints();
    vtkDataArray *rv = NULL;
    int success; 
    if (reader->GetPrecision() == 4)
    {
        rv  = vtkFloatArray::New();
        rv->SetNumberOfComponents(3);
        rv->SetNumberOfTuples(nPts);
        success = reader->GetFunction_float(name, (float*)rv->GetVoidPointer(0));
    }
    else
    {
        rv  = vtkDoubleArray::New();
        rv->SetNumberOfComponents(3);
        rv->SetNumberOfTuples(nPts);
        success = reader->GetFunction_double(name, (double*)rv->GetVoidPointer(0));
    }

    if (success == VTK_OK)
    {
        rv->Register(NULL);
    }
    else
    {
        rv->Delete();
        rv = NULL;
    }

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::ActivateTimestep
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 26, 2015
//
// ****************************************************************************

void
avtPLOT3DFileFormat::ActivateTimestep(int ts)
{
    SetTimeStep(ts);
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      Sets the database meta-data for this PLOT3D file.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//
//  Modifications:
//     Abel Gezahenge,  Sept 12, 2004
//     Read information from Visit Plot3d file; read global expressions and
//     account for 2 and 3D files.
//  
//    Jeremy Meredith, Thu Aug  7 15:47:31 EDT 2008
//    Use const char*'s for string literlas.
//
//    Kathleen Biagas, Tue Aug 19 14:35:29 PDT 2014
//    Reading of the MetaFile is now internal to this class.
//    vtk reader api changed.
//
//    Kathleen Biagas, Thu Aug 27 13:18:22 PDT 2015
//    Removed 'Magnitude' vars, since VisIt automatically generates
//    vector magnitude expressions.
//
// ****************************************************************************

void
avtPLOT3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, 
    int timeState)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;

    if (!visitMetaFile.empty() && !haveReadMetaFile)
    {
        if (!ReadVisItMetaFile())
        {
            EXCEPTION2(InvalidFilesException, visitMetaFile.c_str(),
                       "Could not parse the .vp3d meta file");
        }
    }

    reader->SetGridNumber(0);
    reader->RequestInformation();
    mesh->numBlocks = reader->GetNumberOfGrids();
    mesh->blockOrigin = 0;

    if (reader->GetTwoDimensionalGeometry() == 0)
    {
        mesh->spatialDimension = 3;
        mesh->topologicalDimension = 3;
    }
    else
    {
        mesh->spatialDimension = 2;
        mesh->topologicalDimension =2;
    }

    debug3 <<"PLOT3D  geometry file:    " << reader->GetXYZFileName() << endl;
    debug3 <<"PLOT3D  solution file:    " << reader->GetQFileName() << endl;
    debug3 <<"PLOT3D  multigrid:        " << reader->GetMultiGrid() << endl;
    debug3 <<"PLOT3D  double precision: " << reader->GetDoublePrecision() << endl;
    debug3 <<"PLOT3D  binary:           " << reader->GetBinaryFile() << endl;
    debug3 <<"PLOT3D  fortran binary:   " << reader->GetHasByteCount() << endl;
    debug3 <<"PLOT3D  byte order:       " << reader->GetByteOrderAsString() << endl;
    debug3 <<"PLOT3D  has iblanking:    " << reader->GetIBlanking() << endl;
    debug3 <<"PLOT3D  2D geometry:      " << reader->GetTwoDimensionalGeometry() << endl;

    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    if(haveSolutionFile)
    {
        const int NUM_SCALARS = 9;
        const char *scalar_names[NUM_SCALARS] = { "Density", "Pressure",
             "Temperature", "Enthalpy", "InternalEnergy", "KineticEnergy",
             "StagnationEnergy", "Entropy", "Swirl"};

        int i;
        for (i = 0; i < NUM_SCALARS; i++)
        {
            avtScalarMetaData *sd1 = new avtScalarMetaData;
            sd1->name = scalar_names[i];
            sd1->meshName = "mesh";
            sd1->centering = AVT_NODECENT;
            sd1->hasDataExtents = false;
            md->Add(sd1);
        }

        const int NUM_VECTORS = 5;
        const char *vector_names[NUM_VECTORS] = { "Velocity", "Vorticity",
             "Momentum", "PressureGradient", "StrainRate"};
        for (i = 0; i < NUM_VECTORS; i++)
        {
            avtVectorMetaData *vd1 = new avtVectorMetaData;
            vd1->name = vector_names[i];
            vd1->meshName = "mesh";
            vd1->centering = AVT_NODECENT;
            vd1->hasDataExtents = false;
            vd1->varDim = 3;
            md->Add(vd1);
        }

        // Retrieve the "Properties" array.
        vtkDataArray *props = reader->GetProperties();
        if (props != NULL && props->GetNumberOfTuples() == 4)
        {
            char def[1024];
            Expression exp;

            exp.SetName("Free-stream mach number");
            sprintf(def,"%f",props->GetTuple1(0));
            exp.SetDefinition(def);
            exp.SetType(Expression::Unknown);
            md->AddExpression(&exp);

            exp.SetName("Angle of attack");
            sprintf(def,"%f",props->GetTuple1(1));
            exp.SetDefinition(def);
            exp.SetType(Expression::Unknown);
            md->AddExpression(&exp);

            exp.SetName("Reynold's number");
            sprintf(def,"%f",props->GetTuple1(2));
            exp.SetDefinition(def);
            exp.SetType(Expression::Unknown);
            md->AddExpression(&exp);

            exp.SetName("Integration time");
            sprintf(def,"%f",props->GetTuple1(3));
            exp.SetDefinition(def);
            exp.SetType(Expression::Unknown);
            md->AddExpression(&exp);
        }

        // logic for overflow field names from vtkMultiblockPLOT3DReader:RequestData
        if (reader->GetIsOverflow())
        {
            int nq = reader->GetOverflowNQ();
            int nqc = reader->GetOverflowNQC();
            if(nq >= 6)
            {
                avtScalarMetaData *sd = new avtScalarMetaData;
                sd->name = "Gamma";
                sd->meshName = "mesh";
                sd->centering = AVT_NODECENT;
                sd->hasDataExtents = false;
                md->Add(sd);

                // Quantities derived from Gamma
                avtScalarMetaData *sd2 = new avtScalarMetaData;
                sd2->name = "PressureCoefficient";
                sd2->meshName = "mesh";
                sd2->centering = AVT_NODECENT;
                sd2->hasDataExtents = false;
                md->Add(sd2);

                avtScalarMetaData *sd3 = new avtScalarMetaData;
                sd3->name = "MachNumber";
                sd3->meshName = "mesh";
                sd3->centering = AVT_NODECENT;
                sd3->hasDataExtents = false;
                md->Add(sd3);

                avtScalarMetaData *sd4 = new avtScalarMetaData;
                sd4->name = "SoundSpeed";
                sd4->meshName = "mesh";
                sd4->centering = AVT_NODECENT;
                sd4->hasDataExtents = false;
                md->Add(sd4);
            }
            char fieldname[100];
            for (int i = 0; i <  nqc; ++i)
            {
                SNPRINTF(fieldname, 100, "Species Density #%d", i+i);
                avtScalarMetaData *sd1 = new avtScalarMetaData;
                sd1->name = fieldname;
                sd1->meshName = "mesh";
                sd1->centering = AVT_NODECENT;
                sd1->hasDataExtents = false;
                md->Add(sd1);

                SNPRINTF(fieldname, 100, "Spec Dens #%d / rho", i+i);
                avtScalarMetaData *sd2 = new avtScalarMetaData;
                sd2->name = fieldname;
                sd2->meshName = "mesh";
                sd2->centering = AVT_NODECENT;
                sd2->hasDataExtents = false;
                md->Add(sd2);
            }
            for (int i = 0; i < nq - 6 - nqc; ++i)
            {
                SNPRINTF(fieldname, 100, "Turb Field Quant #%d", i+i);
                avtScalarMetaData *sd = new avtScalarMetaData;
                sd->name = fieldname;
                sd->meshName = "mesh";
                sd->centering = AVT_NODECENT;
                sd->hasDataExtents = false;
                md->Add(sd);
            }
        }

    } // if haveSolutionFile
}


// ****************************************************************************
//  Method: getInfoLine
//
//  Arguments:
//      line     storage for a line read from a vp3d file.
//      size     number of characters to read for one line.
//      fp       file stream for the vp3d file
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept 9, 2004
//
// ****************************************************************************

int
getInfoLine(char * line, int size, FILE*fp)
{
  //  skip all comments and empty lines in the file.
  do{
    int res = fscanf(fp,"%*[^0-9a-zA-Z#]s"); (void) res;
    line = fgets(line,size,fp);
  }while(line && line[0] =='#');

  
  if (line)
    return 1;
  else 
    return 0;
}

bool
MatchesSubstring(const char *c1, const char *c2)
{
    return (strncmp(c1, c2, strlen(c2)) == 0);
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::ReadVisItMetaFile
//
//  Notes:  Copied from vtkVisItPLOT3DReader::CollectInfo, and modified
//          for use with vtkPLOT3DReader.
//
//  Programmer:  Abel Gezahegne
//  Creation:    Sept. 9, 2004
//
//  Modifications:
//
//    Hank Childs, Tue Jan 18 13:19:28 PST 2005
//    Added support for extended pathnames.  Also beefed up substring matching.
//
//    Jeremy Meredith, Tue Feb 22 13:27:06 PST 2005
//    Made lineSize a const.  Compilers other than g++ (like xlC) choke on
//    automatic array variable allocation of sizes that are non-const, even
//    if they are provably known at compile time.
//
//    Hank Childs, Thu Jun  8 14:04:15 PDT 2006
//    Change BIG_ENDIAN to VTK_BIG_ENDIAN because of namespace conflict with
//    endian.h on SUSE.
//
//    Kathleen Biagas, Mon Aug 11 10:57:06 PDT 2014
//    Added to avtPLOT3DFileFormat class, and modified to be used with
//    vtkMultiBlockPLOT3DReader.
//
//    Kathleen Biagas, Thu Apr 23 10:40:03 PDT 2015
//    Set haveSolutionFile.
//
//    Kathleen Biagas, Thu Aug 27 13:20:47 PDT 2015
//    Add FORTRAN_BINARY_STREAM, to indicate non-record based fortran binary.
//    Add SOLUTION_TIME_ACCURATE, and Gas constants R and Gamma.
//
// ****************************************************************************

bool
avtPLOT3DFileFormat::ReadVisItMetaFile()
{
    const int lineSize = 1024;
    bool fileFound = false;
    char infoLine [lineSize];
    FILE * vp3dFp;

    if (visitMetaFile.empty())
    {
        return fileFound;
    }

    vp3dFp = fopen(visitMetaFile.c_str(),"r");

    //  Note the order of these checks is important because we are using
    //  strstr.  ie  check for BINARY must not come before C_BINARY
    if (vp3dFp != NULL)
    {
        fileFound = true;
        string base =  visitMetaFile.substr(0, visitMetaFile.length()-5);
        string::size_type pos = visitMetaFile.rfind(VISIT_SLASH_STRING);
        string path = visitMetaFile.substr(0, pos);

        while (getInfoLine(infoLine,lineSize,vp3dFp))
        {
            if (MatchesSubstring(infoLine,"SINGLE_GRID"))
            {
                reader->MultiGridOff();
            }
            else if (MatchesSubstring(infoLine,"MULTI_GRID"))
            {
                reader->MultiGridOn();
            }
            else if (MatchesSubstring(infoLine,"BIG_ENDIAN"))
            {
                reader->SetByteOrderToBigEndian();
            }
            else if (MatchesSubstring(infoLine,"LITTLE_ENDIAN"))
            {
                reader->SetByteOrderToLittleEndian();
            }
            else if (MatchesSubstring(infoLine,"NO_IBLANKING"))
            {
                reader->IBlankingOff();
            }
            else if (MatchesSubstring(infoLine,"IBLANKING"))
            {
                reader->IBlankingOn();
            }
            else if (MatchesSubstring(infoLine,"3D"))
            {
                reader->TwoDimensionalGeometryOff();
            }
            else if (MatchesSubstring(infoLine,"2D"))
            {
                reader->TwoDimensionalGeometryOn();
            }
            else if (MatchesSubstring(infoLine,"SINGLE_PRECISION"))
            {
                reader->DoublePrecisionOff();
            }
            else if (MatchesSubstring(infoLine,"DOUBLE_PRECISION"))
            {
                reader->DoublePrecisionOn();
            }
            else if (MatchesSubstring(infoLine,"C_BINARY"))
            {
                reader->BinaryFileOn();
                reader->HasByteCountOff();
            }
            else if (MatchesSubstring(infoLine,"FORTRAN_BINARY"))
            {
                reader->BinaryFileOn();
                reader->HasByteCountOn();
            }
            else if (MatchesSubstring(infoLine,"FORTRAN_BINARY_STREAM"))
            {
                reader->BinaryFileOn();
                reader->HasByteCountOff();
            }
            else if (MatchesSubstring(infoLine,"BINARY"))
            {
                reader->BinaryFileOn();
            }
            else if (MatchesSubstring(infoLine,"ASCII"))
            {
                reader->BinaryFileOff();
            }
            else if (MatchesSubstring(infoLine,"GRID"))
            {
                char tmp[1024];
                char x_file[1024];
                sscanf(infoLine + strlen("GRID"),"%s",tmp);
                if (tmp[0] == '/')
                    strcpy(x_file, tmp);
                else
                    sprintf(x_file, "%s/%s", path.c_str(), tmp);
                reader->SetXYZFileName(x_file);
                xFileName = x_file;
            }
            else if (MatchesSubstring(infoLine,"SOLUTION_TIME_ACCURATE"))
            {
                int t;
                sscanf(infoLine + strlen("SOLUTION_TIME_ACCURATE"),"%d", &t);
                solutionHasValidTime = (bool)t;
            }
            else if (MatchesSubstring(infoLine,"SOLUTION"))
            {
                char tmp[1024];
                char s_file[1024];
                sscanf(infoLine + strlen("SOLUTION"),"%s",tmp);
                if (tmp[0] == '/')
                    strcpy(s_file, tmp);
                else
                    sprintf(s_file, "%s/%s", path.c_str(), tmp);
                reader->SetQFileName(s_file);
                haveSolutionFile = true;
                qFileName = s_file;
            }
            else if (MatchesSubstring(infoLine,"R"))
            {
                double R;
                sscanf(infoLine + strlen("R"),"%lg",&R);
                reader->SetR(R);
            }
            else if (MatchesSubstring(infoLine,"GAMMA"))
            {
                double G;
                sscanf(infoLine + strlen("GAMMA"),"%lg",&G);
                reader->SetGamma(G);
            }
        }
        fclose(vp3dFp);
        // try default names if meta file didn't provide them.
        if (reader->GetXYZFileName() == NULL)
        {
            string grid_file = base + ".x";
            reader->SetXYZFileName(grid_file.c_str());
            xFileName = grid_file;
        }
        if (reader->GetQFileName() == NULL)
        {
            string soln_file = base + ".q";
            FILE *file = fopen(soln_file.c_str(), "r");
            if (file != NULL)
            {
                reader->SetQFileName(soln_file.c_str());
                haveSolutionFile = true;
                fclose(file);
                qFileName = soln_file;
            }
        }
    }
    haveReadMetaFile = true;
    return fileFound;
}

// ****************************************************************************
//  Method: avtPLOT3DFileFormat::ProcessQForTimeSeries
//
//  Purpose:
//     Parses the qFileName for '*' or '?', indicating a time-series, then
//     finds the individual solution files matching the wildcards.
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 26, 2015
//
// ****************************************************************************

bool
avtPLOT3DFileFormat::ProcessQForTimeSeries()
{
    bool haveTimeSeries = false;
    if (!qFileName.empty() && (qFileName.find('*') != string::npos ||
        qFileName.find('?') != string::npos ))
    {
        solutionRoot = FileFunctions::Dirname(qFileName);
        string basename = FileFunctions::Basename(qFileName);
        int t2 = visitTimer->StartTimer();
        int returnFullPath = 0;
        void *cb_data[3] = {(void *)&solutionFiles,
                            (void*)&basename,
                            (void*)&returnFullPath};
        FileFunctions::ReadAndProcessDirectory(solutionRoot,
            FileFunctions::FileMatchesPatternCB, (void*) cb_data, false);
        if (!solutionFiles.empty())
        {
            haveTimeSeries = true;
            std::sort(solutionFiles.begin(), solutionFiles.end());
        }
        visitTimer->StopTimer(t2, "avtPLOT3DFileFormat::ReadDirectory");
    }
    haveProcessedQ = true;
    return haveTimeSeries;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::SetTimeStep
//
//  Purpose:
//    Sets the correct QFile name in the reader, based on timestep.
//
//  Programmer: Kathleen Biagas 
//  Creation:   June 26, 2015 
//
// ****************************************************************************

void
avtPLOT3DFileFormat::SetTimeStep(int timeState)
{
    // Currently only only time-series for solution files is supported,
    // could be expanded to also support time-series for xyz (grid) files,
    // if they are named similarly.
    if (!solutionFiles.empty() && previousTS != timeState) 
    {
        string timeFile = solutionRoot + VISIT_SLASH_STRING + 
                          solutionFiles[timeState];
        reader->SetQFileName(timeFile.c_str());
        reader->Modified();
    }
    previousTS = timeState;
}


