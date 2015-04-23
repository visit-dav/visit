/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
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
// ****************************************************************************

avtPLOT3DFileFormat::avtPLOT3DFileFormat(const char *fname,
    DBOptionsAttributes *readOpts)
    : avtSTMDFileFormat(&fname, 1), visitMetaFile("")
{
    char *s_file = NULL;
    char *x_file = NULL;
    bool guessedQFile = false;
    haveSolutionFile = false;
    if (strstr(fname, ".vp3d") != NULL)
    {
        visitMetaFile = filenames[0];
    }
    else if (strstr(fname, ".x") != NULL)
    {
        x_file = filenames[0];

        char soln_file[1024];
        const char *q = strstr(fname, ".x");
        strncpy(soln_file, fname, q-fname);
        strcpy(soln_file + (q-fname), ".q");
        // see if this q file exists
        FILE *file = fopen(soln_file, "r");
        if (file != NULL)
        {
            fclose(file);
            AddFile(soln_file);
            s_file = filenames[1];
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

        AddFile(points_file);

        x_file = filenames[1];
        s_file = filenames[0];
        haveSolutionFile = true;
    }
    else
    {
        if (readOpts && readOpts->FindIndex("Solution (Q) File Name") >= 0)
        {
            string qFileName = readOpts->GetString("Solution (Q) File Name");
            if (!qFileName.empty())
            {
                string xFileName(fname);
                int pos = xFileName.rfind(VISIT_SLASH_STRING);
                string solnFile = xFileName.substr(0, pos+1);
                solnFile += qFileName;
                AddFile(solnFile.c_str());
                x_file = filenames[0];
                s_file = filenames[1];
                haveSolutionFile = true;
            }
        }
        if (x_file == NULL)
        {
            // so give up.
            //
            debug1 << "PLOT3D reader giving up with file identification of "
                   << fname << endl;
            EXCEPTION0(ImproperUseException);
        }
    }

    reader = vtkPLOT3DReader::New();

    if (x_file)
        reader->SetXYZFileName(x_file);
    if (s_file)
        reader->SetQFileName(s_file);

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
            string qFileName = readOpts->GetString("Solution (Q) File Name");
            if (!qFileName.empty())
            {
                if (s_file == NULL || guessedQFile)
                {
                    string xFileName(fname);
                    int pos = xFileName.rfind(VISIT_SLASH_STRING);
                    string solnFile = xFileName.substr(0, pos+1);
                    solnFile += qFileName;
                    reader->SetQFileName(solnFile.c_str());
                    haveSolutionFile = true;
                }
            }
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
}


// *****************************************************************************
//  Method: avtPLOT3DFileFormat::GetMesh
//
//  Purpose:
//      Returns the grid associated with a domain number.
//
//  Arguments:
//      dom     The domain number.
//      name    The mesh name.
//
//  Programmer: Hank Childs
//  Creation:   May 3, 2002
//
//  Modifications:
//    Kathleen Bonnell, Fri Dec 13 16:31:30 PST 2002
//    Use NewInstance instead of MakeObject, to match vtk's new api.
//
// ****************************************************************************

vtkDataSet *
avtPLOT3DFileFormat::GetMesh(int dom, const char *name)
{
    if (dom < 0 || dom >= reader->GetNumberOfGrids())
    {
        EXCEPTION2(BadIndexException, dom, reader->GetNumberOfGrids());
    }

    if (strcmp(name, "mesh") != 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetGridNumber(dom);
    reader->SetScalarFunctionNumber(-1);
    reader->SetVectorFunctionNumber(-1);
    reader->Update();
    vtkDataSet *rv = (vtkDataSet *) reader->GetOutput()->NewInstance();
    if(rv != NULL)
        rv->ShallowCopy(reader->GetOutput());

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVar
//
//  Purpose:
//      Returns the variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
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
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVar(int dom, const char *name)
{
    int var = -1;
    reader->SetVectorFunctionNumber(-1);
    if (strcmp(name, "Density") == 0)
    {
        var = 100;
    }
    else if (strcmp(name, "Pressure") == 0)
    {
        var = 110;
    }
    else if (strcmp(name, "Temperature") == 0)
    {
        var = 120;
    }
    else if (strcmp(name, "Enthalpy") == 0)
    {
        var = 130;
    }
    else if (strcmp(name, "InternalEnergy") == 0)
    {
        var = 140;
    }
    else if (strcmp(name, "KineticEnergy") == 0)
    {
        var = 144;
    }
    else if (strcmp(name, "VelocityMagnitude") == 0)
    {
        var = 153;
    }
    else if (strcmp(name, "StagnationEnergy") == 0)
    {
        var = 163;
    }
    else if (strcmp(name, "Entropy") == 0)
    {
        var = 170;
    }
    else if (strcmp(name, "Swirl") == 0)
    {
        var = 184;
    }
    else if (strcmp(name, "VorticityMagnitude") == 0)
    {
        var = 211;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetScalarFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetArray(name);
    if (dat == NULL)
    {
        debug1 << "Internal error -- could not read variable: " << name << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
}


// ****************************************************************************
//  Method: avtPLOT3DFileFormat::GetVectorVar
//
//  Purpose:
//      Returns the vector variable associated with a domain number.
//
//  Arguments:
//      dom       The domain number.
//      name      The variable name.
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
// ****************************************************************************

vtkDataArray *
avtPLOT3DFileFormat::GetVectorVar(int dom, const char *name)
{
    int var = -1;
    reader->SetScalarFunctionNumber(-1);
    if (strcmp(name, "Velocity") == 0)
    {
        var = 200;
    }
    else if (strcmp(name, "Vorticity") == 0)
    {
        var = 201;
    }
    else if (strcmp(name, "Momentum") == 0)
    {
        var = 202;
    }
    else if (strcmp(name, "PressureGradient") == 0)
    {
        var = 210;
    }
    else if (strcmp(name, "StrainRate") == 0)
    {
        var = 212;
    }

    if (var < 0)
    {
        EXCEPTION1(InvalidVariableException, name);
    }

    reader->SetVectorFunctionNumber(var);
    reader->SetGridNumber(dom);
    reader->Update();

    vtkDataArray *dat = reader->GetOutput()->GetPointData()->GetArray(name);
    if (dat == NULL)
    {
        debug1 << "Internal error -- could not read variable: " << name << "!" << endl;
        EXCEPTION0(ImproperUseException);
    }

    vtkDataArray *rv = dat->NewInstance();
    rv->DeepCopy(dat);
    rv->SetName(name);

    return rv;
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
// ****************************************************************************

void
avtPLOT3DFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "mesh";
    mesh->meshType = AVT_CURVILINEAR_MESH;

    if (visitMetaFile != "")
    {
        if (ReadVisItMetaFile())
        {
            AddFile(reader->GetXYZFileName());
            AddFile(reader->GetQFileName());
        }
        else
        {
            EXCEPTION2(InvalidFilesException, visitMetaFile.c_str(),
                       "Could not parse the .vp3d meta file");
        }
    }
    else
    {
        // If we have the auto-detect implemented use that, otherwise
        // add DBOptions
        //reader->SetAutoDetectFormatOn();
    }

    reader->UpdateInformation();
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
        const int NUM_SCALARS = 11;
        const char *scalar_names[NUM_SCALARS] = { "Density", "Pressure",
             "Temperature", "Enthalpy", "InternalEnergy", "KineticEnergy",
             "VelocityMagnitude", "StagnationEnergy", "Entropy", "Swirl",
             "VorticityMagnitude" };

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
// ****************************************************************************

bool
avtPLOT3DFileFormat::ReadVisItMetaFile()
{
    const int lineSize = 1024;
    bool fileFound = false;
    char infoLine [lineSize];
    FILE * vp3dFp;

    if (visitMetaFile == "")
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
        int pos = visitMetaFile.rfind(VISIT_SLASH_STRING);
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
            }
        }
        fclose(vp3dFp);
        // try default names if meta file didn't provide them.
        if (reader->GetXYZFileName() == NULL)
        {
            string grid_file = base + ".x";
            reader->SetXYZFileName(grid_file.c_str());
        }
        if (reader->GetQFileName() == NULL)
        {
            string soln_file = base + ".q";
            FILE *file = fopen(soln_file.c_str(), "r");
            if (file != NULL)
            {
                reader->SetQFileName(soln_file.c_str());
                haveSolutionFile = true;
                flcose(file);
            }
        }
    }
    return fileFound;
}


