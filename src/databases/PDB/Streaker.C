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
#include <Streaker.h>
#include <PP_ZFileReader.h>

#include <set>
#include <math.h>
#include <snprintf.h>

#include <vtkStructuredGrid.h>
#include <vtkCellData.h>
#include <vtkPointData.h>
#include <vtkPoints.h>
#include <vtkFloatArray.h>
#include <vtkIntArray.h>

#include <visitstream.h>
#include <avtDatabaseMetaData.h>
#include <avtMaterial.h>
#include <DebugStream.h>

#include <PP_ZFileReader.h>

#define STREAK_MATERIAL "streak_material"

// ****************************************************************************
// Method: Streaker::StreakInfo::StreakInfo
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:39:55 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Dec  2 16:11:54 PST 2008
//   I added material vars.
//
//   Brad Whitlock, Wed Feb 25 16:11:10 PST 2009
//   I added x_scale, x_translate. I changed log's type too.
//
// ****************************************************************************

Streaker::StreakInfo::StreakInfo() : xvar(), yvar(), zvar(), hasMaterial(false),
    cellCentered(true), matchSilo(false), slice(0), sliceIndex(0), hsize(0), dataset(0), integrate(false),
    log(Streaker::LOGTYPE_NONE), x_scale(1.), x_translate(0.), y_scale(1.), y_translate(0.),
    z_scale(1.), z_translate(0.)
{
}

// ****************************************************************************
// Method: Streaker::Streaker
//
// Purpose: 
//   Constructor.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:40:10 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Dec  3 14:05:29 PST 2008
//   Added material related vars.
//
// ****************************************************************************

Streaker::Streaker() : streaks(), matvar(), matnos(), matNames(), matToStreak()
{
}

// ****************************************************************************
// Method: Streaker::~Streaker
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:40:28 PST 2008
//
// Modifications:
//   
// ****************************************************************************

Streaker::~Streaker()
{
    FreeUpResources();
}

// ****************************************************************************
// Method: Streaker::FreeUpResources
//
// Purpose: 
//   Frees up resources, including the cached VTK datasets. The streak info
//   is also cleared.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:40:56 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Dec  3 14:05:29 PST 2008
//   Added material related vars.
//   
// ****************************************************************************

void
Streaker::FreeUpResources()
{
    // Clear our caches, etc.
    for(std::map<std::string, StreakInfo>::iterator it = streaks.begin();
        it != streaks.end(); ++it)
    {
        if(it->second.dataset != 0)
            it->second.dataset->Delete();
    }
    streaks.clear();
    matvar = "";
    matnos.clear();
    matNames.clear();
    matToStreak.clear();
}

// ****************************************************************************
// Method: Streaker::ReadStreakFile
//
// Purpose: 
//   Reads the specified streak file that contains the "streakplot" commands.
//
// Arguments:
//   filename : The streak file.
//   pdb      : The PDB file object corresponding to the first real PDB file.
//              The file is used to determine whether all of the information
//              for the streak plot is available.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:41:37 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Feb 25 16:11:10 PST 2009
//   I added x_scale, x_translate.
//
//   Brad Whitlock, Tue May  5 16:28:35 PDT 2009
//   I added cellcentered and nodecentered commands.
//
//   Brad Whitlock, Tue May 26 13:45:40 PDT 2009
//   I added the "matchsilo" option.
//
// ****************************************************************************

void
Streaker::ReadStreakFile(const std::string &filename, PDBFileObject *pdb)
{
    FreeUpResources();

    // Open the file.
    ifstream ifile(filename.c_str());
    if (ifile.fail())
    {
        debug4 << "Streaker::ReadStreakFile: Could not open streak file: "
               << filename.c_str() << endl;
        return;
    }

    // process the file.
    char  line[1024];
    bool  cellCentered = true;
    bool  matchSilo = false;
    for(int lineIndex = 0; !ifile.eof(); ++lineIndex)
    {
        // Get the line
        ifile.getline(line, 1024);

        if(line[0] == '#')
            continue;
        else if(strncmp(line, "cellcentered", 12) == 0 ||
                strncmp(line, "zonecentered", 12) == 0)
            cellCentered = true;
        else if(strncmp(line, "nodecentered", 12) == 0)
            cellCentered = false;
        else if(strncmp(line, "matchsilo", 9) == 0)
            matchSilo = true;
        else if(strncmp(line, "streakplot", 10) == 0)
        {
            bool invalidStreak = true;
            char varname[100];
            char xvar[100];
            char yvar[100];
            char zvar[100];
            char slice[100];
            char integrate[100];
            char log[100];
            char cs[100];
            const char *ptr = line + 10;
            int ci;
            StreakInfo s;
            if(sscanf(ptr, "%s %s %s %s %s %d %g %g %g %g %g %g %s %s", 
                      varname, xvar, yvar, zvar, cs, &ci,
                      &s.x_scale, &s.x_translate, &s.y_scale, &s.y_translate,
                      &s.z_scale, &s.z_translate, integrate, log) == 14)
            {
                invalidStreak = false;
            }
            else if(sscanf(ptr, "%s %s %s %s %s %d %g %g %g %g %s %s", 
                      varname, xvar, yvar, zvar, cs, &ci,
                      &s.x_scale, &s.x_translate, &s.y_scale, &s.y_translate,
                      integrate, log) == 12)
            {
                invalidStreak = false;
            }
            else if(sscanf(ptr, "%s %s %s %s %s %d %g %g %s %s", 
                      varname, xvar, yvar, zvar, cs, &ci,
                      &s.y_scale, &s.y_translate, integrate, log) == 10)
            {
              
                invalidStreak = false;
            }

            if(invalidStreak)
            {
                debug4 << "Streak file has an invalid line " << lineIndex << endl;
                cerr << "Streak file has an invalid line " << lineIndex << endl;
            }
            else
            {
                s.xvar = xvar;
                s.yvar = yvar;
                s.zvar = zvar;

                if((strcmp(cs, "J")==0 || strcmp(cs, "j")==0) && (ci > 0))
                {
                    s.slice = 1;
                    s.sliceIndex = ci-1;
                }
                else if((strcmp(cs, "I")==0 || strcmp(cs, "i")==0) && (ci > 0))
                {
                    s.slice = 0;
                    s.sliceIndex = ci-1;
                }
                else
                    invalidStreak = true;

                s.integrate = (strcmp(integrate, "on") == 0 ||
                               strcmp(integrate, "On") == 0 ||
                               strcmp(integrate, "ON") == 0);

                if(strcmp(log, "on") == 0 ||
                   strcmp(log, "On") == 0 ||
                   strcmp(log, "ON") == 0 ||
                   strcmp(log, "log") == 0)
                {
                    s.log = LOGTYPE_LOG;
                }
                else if(strcmp(log, "log10") == 0)
                    s.log = LOGTYPE_LOG10;
                else
                    s.log = LOGTYPE_NONE;
                s.cellCentered = cellCentered;
                s.matchSilo = matchSilo;

                AddStreak(varname, s, pdb);
            }
        }       
    }
}

// ****************************************************************************
// Method: Streaker::AddStreak
//
// Purpose: 
//   Takes the input StreakInfo record and ensures that the file has the
//   required variables to make the plot and that the arrays all have
//   compatible sizes.
//
// Arguments:
//   varname : The variable name we're creating.
//   s       : The streak info needed to create the streak plot.
//   pdb     : The PDB file to check for data.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:42:46 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void
Streaker::AddStreak(const std::string &varname, StreakInfo &s, 
    PDBFileObject *pdb)
{
    const char *mName = "Streaker::AddStreak: ";
    // Make sure that all variables exist and that their sizes are good.
    TypeEnum    t[3];
    std::string typeString[3];
    int         nTotalElements[3];
    int        *dimensions[3];
    int         nDims[3];
    bool xExists = pdb->SymbolExists(s.xvar.c_str(), &t[0], typeString[0], 
                       &nTotalElements[0], &dimensions[0], &nDims[0]);
    bool yExists = pdb->SymbolExists(s.yvar.c_str(), &t[1], typeString[1], 
                       &nTotalElements[1], &dimensions[1], &nDims[1]);
    bool zExists = pdb->SymbolExists(s.zvar.c_str(), &t[2], typeString[2], 
                       &nTotalElements[2], &dimensions[2], &nDims[2]);
    if(xExists && yExists && zExists)
    {
        debug4 << mName << "All streakplot variables exist." << endl;

        // Make sure that xvar is 1d and that the others are 2/3d (inc. time)
        // and that yvar is the same size as zvar.
        if(nDims[0] > 1)
        {
            debug4 << mName << "The time variable " << s.xvar.c_str() << " has "
                   << nDims[0] << " dimensions instead of 1." << endl;
            goto invalid;
        }

        if(nDims[1] == 1 || nDims[1] > 3)
        {
            debug4 << mName << "The yvar variable " << s.yvar.c_str() << " has "
                   << nDims[1] << " dimensions; not 2 or 3." << endl;
            goto invalid;
        }

        if(nDims[2] == 1 || nDims[2] > 3)
        {
            debug4 << mName << "The zvar variable " << s.zvar.c_str() << " has "
                   << nDims[1] << " dimensions; not 2 or 3." << endl;
            goto invalid;
        }

        if(nDims[1] != nDims[2])
        {
            debug4 << mName << "The yvar,zvar dimensions do not match." << endl;
            goto invalid;
        }

        // Make sure that yvar's last dimension is the same size as xvar if
        // we have an xvar is an array.
        if(nDims[0] == 1 &&
           dimensions[0][0] != dimensions[1][nDims[1]-1])
        {
            debug4 << mName << "Time dims: " << dimensions[0][0]
                   << " are not the same as yvar's last dimension: "
                   << dimensions[1][nDims[1]-1] << endl;
            goto invalid;
        }

        // Make sure yvar, zvar are the same size.
        for(int i = 0; i < nDims[1]; ++i)
            if(dimensions[1][i] != dimensions[2][i])
            {
                debug4 << mName << "yvar's dim[" << i << "]=" << dimensions[1][i]
                       << " and that is not equal to zvar's dim[" << i
                       << "]=" << dimensions[2][i] << endl;
                goto invalid;
            }

        // Make sure that the slice is valid.
        if(s.slice == 0)
        {
            if(s.sliceIndex >= dimensions[1][0])
            {
                debug4 << mName << "X slice=" << s.sliceIndex 
                       << " is larger than yvar's dim[0]=" << dimensions[1][0] << endl;
                goto invalid;
            }
            s.hsize = dimensions[1][1];
        }
        else if(s.slice == 1)
        {
            if(s.sliceIndex >= dimensions[1][1])
            {
                debug4 << mName << "Y slice=" << s.sliceIndex 
                       << " is larger than yvar's dim[1]=" << dimensions[1][1] << endl;
                goto invalid;
            }
            s.hsize = dimensions[1][0];
        }

        s.hasMaterial = FindMaterial(pdb, dimensions[2], nDims[2]);

        // Okay, it looks good. Add the streak.
        streaks[varname] = s;
    }
    else
        debug4 << mName << "One or more streakplot variables is missing." << endl;
invalid:
    for(int i = 0; i < 3; ++i)
        delete [] dimensions[i];    
}

// ****************************************************************************
// Method: Streaker::FindMaterial
//
// Purpose: 
//   Looks through the file for "ireg" and if it matches the size of the
//   current z variable, we add a material for it.
//
// Arguments:
//   s           : The streak info that we're modifying.
//   pdb         : The PDB file that contains the data.
//   zDimensions : The size of the zvar.
//   zDims       : The number of dimensions in Z.
//
// Programmer: Brad Whitlock
// Creation:   Tue Dec  2 16:38:01 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Apr  1 11:18:07 PDT 2009
//   I removed a pointer increment that caused an ABR.
//
// ****************************************************************************

bool
Streaker::FindMaterial(PDBFileObject *pdb, int *zDimensions, int zDims)
{
    // Let's see if there's an ireg variable.
    bool hasMaterial = false;
    PDBfile *pdbPtr = pdb->filePointer();
    int numVars = 0;
    char *m = 0;
    char **varList = PD_ls(pdbPtr, NULL /*path*/, NULL /*pattern*/, &numVars);
    if(varList != NULL)
    {
        for(int j = 0; j < numVars; ++j)
        {
            if(strncmp(varList[j], "ireg@", 5) == 0)
            {
                // Get more information about ireg.
                TypeEnum    t;
                std::string typeString;
                int         nTotalElements;
                int        *dimensions;
                int         nDims;
                pdb->SymbolExists(varList[j], &t, typeString, 
                       &nTotalElements, &dimensions, &nDims);

                // Check whether ireg is the same size as zvar.
                bool sameSize = nDims == nDims;
                if(sameSize)
                {
                    for(int k = 0; k < nDims; ++k)
                        sameSize &= (zDimensions[k] == dimensions[k]);
                }

                // If ireg is the same size then we can add a material var.
                if(sameSize && t == INTEGERARRAY_TYPE)
                {
                    m = varList[j];
                    hasMaterial = true;
                    break;
                }
            }
        }
    }

    // If we have not yet read the matnos then do that now.
    if(hasMaterial && matvar.size() == 0)
    {
        // Read the matvar array and determine a list of material numbers. This can
        // conceivably be a lot of data since we're reading the whole array for 
        // this PDB file, which could contain several time steps.
        int *ireg = 0, nvals = 0;
        if(pdb->GetIntegerArray(m, &ireg, &nvals))
        {
            // Get a unique set of material numbers.
            std::set<int> mats;
            int *ptr = ireg;
            for(int i = 0; i < nvals; ++i)
            {
                int mat = *ptr++;
                if(mat > 0)
                    mats.insert(mat);
            }
            delete [] ireg;

            if(mats.size() > 0)
            {
                this->matvar = std::string(m);
                for(std::set<int>::const_iterator it = mats.begin();
                    it != mats.end(); ++it)
                {
                    this->matnos.push_back(*it);
                }

                // Set up the material names. Try and be consistent with the
                // PPZ reader.
                stringVector names;
                if(PP_ZFileReader::ReadMaterialNames(pdb, matnos.size(), names))
                {
                    for(size_t i = 0; i < names.size(); ++i)
                        this->matNames.push_back(names[i]);
                }
                else
                {
                    for(size_t i = 0; i < matnos.size(); ++i)
                    {
                        char tmp[10];
                        SNPRINTF(tmp, 10, "%d", matnos[i]);
                        this->matNames.push_back(tmp);
                    }
                }
            }
            else
                hasMaterial = false;
        }
    }

    SFREE(varList);

    return hasMaterial;
}

// ****************************************************************************
// Method: Streaker::PopulateDatabaseMetaData
//
// Purpose: 
//   Populates metadata for the streak plot objects that we're creating.
//
// Arguments:
//   md  : The metadata object to populate.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:45:03 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Dec  2 16:52:24 PST 2008
//   Added material support.
//
//   Brad Whitlock, Wed Feb 25 16:06:40 PST 2009
//   Changed to use log10.
//
//   Brad Whitlock, Thu Apr  2 16:37:08 PDT 2009
//   Added cell-centered data support.
//
//   Brad Whitlock, Tue May  5 16:25:02 PDT 2009
//   I made cell-centering support be dynamic instead of compiled in.
//
//   Brad Whitlock, Thu May  7 16:58:42 PDT 2009
//   I set the cell and node origins to 1.
//
// ****************************************************************************

void
Streaker::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    std::map<std::string, StreakInfo>::const_iterator it;
    for(it = streaks.begin(); it != streaks.end(); ++it)
    {
        std::string meshName = it->first + "_mesh";

        avtMeshMetaData *mmd = new avtMeshMetaData;
        mmd->name = meshName;
        mmd->meshType = AVT_CURVILINEAR_MESH;
        mmd->spatialDimension = 2;
        mmd->topologicalDimension = 2;
        mmd->xLabel = it->second.xvar;
        if(it->second.integrate && it->second.log)
            mmd->yLabel = std::string("log10(integrated ") + it->second.yvar + std::string(")");
        else if(it->second.integrate)
            mmd->yLabel = std::string("integrated ") +  it->second.yvar;
        else if(it->second.log)
            mmd->yLabel = std::string("log10(") +  it->second.yvar + std::string(")");
        else
            mmd->yLabel = it->second.yvar;
        mmd->cellOrigin = 1;
        mmd->nodeOrigin = 1;
        md->Add(mmd);

        avtScalarMetaData *smd = new avtScalarMetaData;
        smd->name = it->first;
        smd->meshName = meshName;
        smd->centering = it->second.cellCentered ? AVT_ZONECENT : AVT_NODECENT;
        md->Add(smd);

        // Add a material for this streak plot if its ireg matched the size of zvar.
        if(it->second.hasMaterial)
        {
            std::string matName(it->first + "_mat");
            matToStreak[matName] = it->first;

            avtMaterialMetaData *mmd = new avtMaterialMetaData(matName,
                meshName, matNames.size(), matNames);
            md->Add(mmd);
        }
    }
}

// ****************************************************************************
// Method: Streaker::GetMesh
//
// Purpose: 
//   Gets the streak plot mesh.
//
// Arguments:
//   mesh : The name of the mesh to get.
//   pdb  : The vector of PDB files that contain the data over time.
//
// Returns:    A VTK dataset
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:45:46 PST 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataSet *
Streaker::GetMesh(const std::string &mesh, const PDBFileObjectVector &pdb)
{
#ifdef MDSERVER
    return 0;
#else
    if(mesh.size() <= 5)
        return 0;
    std::string varName(mesh.substr(0, mesh.size()-5));

    std::map<std::string, StreakInfo>::iterator pos = streaks.find(varName);
    if(pos == streaks.end())
        return 0;

    // Create the dataset if needed.
    if(pos->second.dataset == 0)
        pos->second.dataset = ConstructDataset(varName, pos->second, pdb);

    // Increment the reference count.
    pos->second.dataset->Register(NULL);

    return pos->second.dataset;
#endif
}

// ****************************************************************************
// Method: Streaker::GetVar
//
// Purpose: 
//   Gets the streak plot var.
//
// Arguments:
//   var : The name of the var to get.
//   pdb : The vector of PDB files that contain the data over time.
//
// Returns:    A VTK dataset
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:45:46 PST 2008
//
// Modifications:
//   
// ****************************************************************************

vtkDataArray *
Streaker::GetVar(const std::string &var, const PDBFileObjectVector &pdb)
{
#ifdef MDSERVER
    return 0;
#else
    std::map<std::string, StreakInfo>::iterator pos = streaks.find(var);
    if(pos == streaks.end())
        return 0;

    // Create the dataset if needed.
    if(pos->second.dataset == 0)
        pos->second.dataset = ConstructDataset(var, pos->second, pdb);

    // Look up the variable in the node data.
    vtkDataSet *ds = pos->second.dataset;
    vtkDataArray *arr = 0;
    if(pos->second.cellCentered)
        arr = ds->GetCellData()->GetArray(var.c_str());
    else 
        arr = ds->GetPointData()->GetArray(var.c_str());
    if(arr != 0)
        arr->Register(NULL);

    return arr;
#endif
}

#ifndef MDSERVER
// ****************************************************************************
// Method: StoreValues
//
// Purpose: 
//   This function stores a slice of data from one array into a destination array.
//
// Arguments:
//   dest : The destination array.
//   src  : The source array.
//   ti   : The starting "X" value in the destination array.
//   sdims : The size of the destination array.
//   dims  : The size of the source array.
//   slice : The slice dimension in the source array.
//   sliceIndex : The index of the slice in the source array.
//
// Returns:    The new X value in the destination array.
//
// Note:       We call this method to store the slice data from various source
//             arrays into a single destination array where we write data
//             into windows of the destination array.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:47:01 PST 2008
//
// Modifications:
//   
// ****************************************************************************

template<class T>
static int
StoreValues(float *dest, const T * const src, int ti, const int * const sdims, 
            const int * const dims, int ndims, int slice, int sliceIndex)
{
    int newTi = ti;
    int nx = dims[0];
    int ny = (ndims > 1) ? dims[1] : 1;
    int nz = (ndims > 2) ? dims[2] : 1;
    int nxny = nx*ny;
    if(slice == 0)
    {
        int i = sliceIndex;
        for(int k = 0; k < nz; ++k)
        {
            for(int j = 0; j < ny; ++j)
            {
                int srcindex = k*nxny + j*nx + i;
                int destindex = j*sdims[0] + k + ti;
                dest[destindex] = float(src[srcindex]);
//                debug4 << "src[" << srcindex << "](" << src[srcindex] << ") -> dest[" << destindex << "]" << endl;
            }
        }
        newTi += nz;
    }
    else if(slice == 1)
    {
        int j = sliceIndex;
        for(int k = 0; k < nz; ++k)
        {
            for(int i = 0; i < nx; ++i)
            {
                int srcindex = k*nxny + j*nx + i;
                int destindex = i*sdims[0] + k + ti;
                dest[destindex] = float(src[srcindex]);
//                debug4 << "src[" << srcindex << "](" << src[srcindex] << ") -> dest[" << destindex << "]" << endl;
            }
        }
        newTi += nz;
    }

    return newTi;
}

// ****************************************************************************
// Method: Streaker::AssembleData
//
// Purpose: 
//   This method reads the values for a PDB variable for all of the PDB files
//   and assembles the sliced values into a vtkFloatArray shaped like the
//   final data.
//
// Arguments:
//   var        : The variable to read.
//   sdims      : The size of the destination array.
//   slice      : The slice dimension.
//   sliceIndex : The slice index.
//   pdb        : A vector of PDB file objects from which to get data.
//
// Returns:    The assembled data array.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:50:02 PST 2008
//
// Modifications:
//   
// ****************************************************************************

vtkFloatArray *
Streaker::AssembleData(const std::string &var, int *sdims, int slice, int sliceIndex, 
    const PDBFileObjectVector &pdb) const
{
    const char *mName = "Streaker::AssembleData: ";

    // Let's assemble vtkDataArray from the slices over time.
    debug4 << mName << "Creating new double array sized: "
           << (sdims[0] * sdims[1] * sdims[2]) << endl;
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(sdims[0] * sdims[1] * sdims[2]);
    arr->SetName(var.c_str());
    float *dest = (float *)arr->GetVoidPointer(0);
    memset(dest, 0, sizeof(float) * sdims[0] * sdims[1] * sdims[2]);
    int ti = 0;
    for(int r = 0; r < pdb.size(); ++r)
    {
        int *dims = 0;
        int nDims = 0;
        int length = 0;
        TypeEnum t = NO_TYPE;
        void *val = pdb[r]->ReadValues(var.c_str(), &t, &length, &dims, &nDims);
        if(val)
        {
            debug4 << mName << "Read " << var.c_str() << " from " << pdb[r]->GetName().c_str() << endl;
            debug4 << "\tdims = {";
            for(int i = 0; i < nDims; ++i)
                debug4 << dims[i] << " ";
            debug4 << "}" << endl;
            if(t == DOUBLEARRAY_TYPE)
            {
                debug4 << "Storing double values" << endl;
                double *src = (double *)val;
                ti = StoreValues<double>(dest, src, ti, sdims, dims, nDims, slice, sliceIndex);
            }
            else if(t == FLOATARRAY_TYPE)
            {
                debug4 << "Storing float values" << endl;
                float *src = (float *)val;
                ti = StoreValues<float>(dest, src, ti, sdims, dims, nDims, slice, sliceIndex);
            }
            else if(t == INTEGERARRAY_TYPE)
            {
                debug4 << "Storing int values" << endl;
                int *src = (int *)val;
                ti = StoreValues<int>(dest, src, ti, sdims, dims, nDims, slice, sliceIndex);
            }
            else
                debug4 << "Unsupported type" << endl;

            free_void_mem(val, t);
            free_mem(dims);
        }

        // Close the file so we don't get too many files open.
        pdb[r]->Close();
    }

    return arr;
}

// ****************************************************************************
// Method: TransposeArray
//
// Purpose: 
//   Transposes the data array in I,J.
//
// Arguments:
//   arr  : The data array to transpose.
//   dims : The dimensions of the data array.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue May 26 14:02:16 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static vtkDataArray *
TransposeArray(vtkDataArray *arr, const int *dims)
{
    vtkDataArray *newarr = arr->NewInstance();
    newarr->SetNumberOfComponents(arr->GetNumberOfComponents());
    newarr->SetNumberOfTuples(arr->GetNumberOfTuples());
    newarr->SetName(arr->GetName());
    for(int j = 0; j < dims[1]; ++j)
        for(int i = 0; i < dims[0]; ++i)
        {
            int dest_index = j * dims[0] + i;
            int src_index = i * dims[1] + j;
            newarr->SetTuple(dest_index, arr->GetTuple(src_index));
        }
    return newarr;
}

// ****************************************************************************
// Method: TransposeIJ
//
// Purpose: 
//   Transposes a structured grid in I,J.
//
// Arguments: 
//   grid : The grid to transpose.
//
// Returns:    A new grid that has been transposed.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Tue May 26 13:57:32 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

static vtkStructuredGrid *
TransposeIJ(vtkStructuredGrid *grid)
{
    int dims[3];
    grid->GetDimensions(dims);
    int newdims[3];
    newdims[0] = dims[1]; // Swap I,J dimensions
    newdims[1] = dims[0];
    newdims[2] = dims[2];

    int i, j;
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(grid->GetPoints()->GetNumberOfPoints());
    float *coords_dest = (float *)points->GetVoidPointer(0);
    float *coords_src = (float *)grid->GetPoints()->GetVoidPointer(0);
    // Transpose points when copying into the new points
    for(j = 0; j < newdims[1]; ++j)
        for(i = 0; i < newdims[0]; ++i)
        {
            int dest_index = (j * newdims[0] + i) * 3;
            int src_index = (i * newdims[1] + j) * 3;
            coords_dest[dest_index + 0]  = coords_src[src_index + 0];
            coords_dest[dest_index + 1]  = coords_src[src_index + 1];
            coords_dest[dest_index + 2]  = coords_src[src_index + 2];
        }

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();
    sgrid->SetDimensions(newdims);

    // Now create transposed copies of the cell arrays.
    int celldims[3];
    celldims[0] = newdims[0] - 1;
    celldims[1] = newdims[1] - 1;
    celldims[2] = 1;
    vtkDataArray *scalars = grid->GetCellData()->GetScalars();
    for(i = 0; i < grid->GetCellData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = TransposeArray(grid->GetCellData()->GetArray(i), celldims);
        if(grid->GetCellData()->GetArray(i) == scalars)
            sgrid->GetCellData()->SetScalars(arr);
        else
            sgrid->GetCellData()->AddArray(arr);
    }

    // Now create transposed copies of the cell arrays.
    scalars = grid->GetPointData()->GetScalars();
    for(i = 0; i < grid->GetPointData()->GetNumberOfArrays(); ++i)
    {
        vtkDataArray *arr = TransposeArray(grid->GetPointData()->GetArray(i), newdims);
        if(grid->GetPointData()->GetArray(i) == scalars)
            sgrid->GetPointData()->SetScalars(arr);
        else
            sgrid->GetPointData()->AddArray(arr);
    }

    return sgrid;
}

// ****************************************************************************
// Method: Streaker::ConstructDataset
//
// Purpose: 
//   This method constructs the VTK dataset for the requested streak plot and
//   caches the results.
//
// Arguments:
//   var : The streak plot variable.
//   s   : The information about the streak plot variable.
//   pdb : The vector of PDB files from which to get the data.
//
// Returns:    The VTK dataset that makes up the streak plot.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 12 13:52:50 PST 2008
//
// Modifications:
//   Brad Whitlock, Wed Dec  3 14:02:10 PST 2008
//   Added material support.
//
//   Brad Whitlock, Wed Feb 25 16:14:33 PST 2009
//   I added x_scale and x_translate. I also added support for different logs.
//
//   Brad Whitlock, Tue May  5 16:27:54 PDT 2009
//   Do the centering dynamically.
//
//   Brad Whitlock, Wed May 13 13:41:33 PDT 2009
//   I added z_scale, z_translate.
//
//   Brad Whitlock, Tue May 26 13:47:39 PDT 2009
//   I added matchSilo support.
//
// ****************************************************************************

vtkDataSet *
Streaker::ConstructDataset(const std::string &var, const StreakInfo &s, const PDBFileObjectVector &pdb)
{
    const char *mName = "Streaker::ConstructDataset: ";

    // Gather up the times so we know how wide the new mesh is.
    std::vector<double> times;
    debug4 << mName << "Times = " << endl;
    for(size_t i = 0; i < pdb.size(); ++i)
    {
        double *vals = 0; int nvals = 0;
        debug4 << "    ";
        if(pdb[i]->GetDoubleArray(s.xvar.c_str(), &vals, &nvals))
        {
            for(int j = 0; j < nvals; ++j)
            {
                times.push_back(vals[j]);
                debug4 << vals[j] << " ";
            }
            delete [] vals;
        }
        else
             debug4 << "** Could not read times in " << pdb[i]->GetName().c_str() << " **";
        debug4 << endl;
    }

    int sdims[3];
    sdims[0] = (int)times.size();
    sdims[1] = s.hsize;
    sdims[2] = 1;
    int nnodes = sdims[0] * sdims[1];
    debug4 << mName << "sdims = {" << sdims[0] << ","<<sdims[1] <<","<<sdims[2]<<"}"<<endl;
    // Let's assemble vtkDataArray for the y and "z" values.
    vtkFloatArray *yvar = AssembleData(s.yvar, sdims, s.slice, s.sliceIndex, pdb);
    vtkFloatArray *zvar = AssembleData(s.zvar, sdims, s.slice, s.sliceIndex, pdb);

    // Use the yvar and time to create the points.
    vtkPoints *points = vtkPoints::New();
    points->SetNumberOfPoints(nnodes);
    float *coords = (float *)points->GetVoidPointer(0);
    float *yc = (float *)yvar->GetVoidPointer(0);
    if(s.integrate)
    {
        double *ysum = new double[sdims[0]];
        memset(ysum, 0, sizeof(double) * sdims[0]);
        for(int j = 0; j < sdims[1]; ++j)
            for(int i = 0; i < sdims[0]; ++i)
            {
                double xval = times[i];
                xval *= s.x_scale;
                xval += s.x_translate;

                ysum[i] += (double)*yc++;
                double yval = ysum[i];
                yval *= s.y_scale;
                yval += s.y_translate;
                if(s.log == LOGTYPE_LOG)
                    yval = log((yval > 0.) ? yval : 1.e-9);
                else if(s.log == LOGTYPE_LOG10)
                    yval = log10((yval > 0.) ? yval : 1.e-9);

                *coords++ = (float)xval;
                *coords++ = (float)yval;
                *coords++ = 0.;
            }
        delete [] ysum;
    }
    else
    {
        for(int j = 0; j < sdims[1]; ++j)
            for(int i = 0; i < sdims[0]; ++i)
            {
                double xval = times[i];
                xval *= s.x_scale;
                xval += s.x_translate;

                double yval = (double)*yc++;
                yval *= s.y_scale;
                yval += s.y_translate;
                if(s.log == LOGTYPE_LOG)
                    yval = log((yval > 0.) ? yval : 1.e-9);
                else if(s.log == LOGTYPE_LOG10)
                    yval = log10((yval > 0.) ? yval : 1.e-9);

                *coords++ = (float)xval;
                *coords++ = (float)yval;
                *coords++ = 0.;
            }
    }

    vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
    sgrid->SetPoints(points);
    points->Delete();
    sgrid->SetDimensions(sdims);

    // Transform zvar.
    float *zarr = (float *)zvar->GetVoidPointer(0);
    for(int i = 0; i < zvar->GetNumberOfTuples(); ++i)
        zarr[i] = zarr[i] * s.z_scale + s.z_translate;

    if(s.cellCentered)
    {
        // Create a cell-centered version of zvar. We can ignore the first row of
        // data since it's no good.
        vtkFloatArray *cvar = vtkFloatArray::New();
        int cx = sdims[0] - 1;
        int cy = sdims[1] - 1;
        cvar->SetNumberOfTuples(cx * cy);
        float *dest = (float *)cvar->GetVoidPointer(0);
        float *f = (float *)zvar->GetVoidPointer(0);
        for(int j = 0; j < cy; ++j)
        {
            float *src = f + ((j+1) * sdims[0]);
            for(int i = 0; i < cx; ++i)
                *dest++ = *src++;
        }
        zvar->Delete();

        // Add the cvar array to the dataset.
        cvar->SetName(var.c_str());
        sgrid->GetCellData()->AddArray(cvar);
    }
    else
    {
        // Add the zvar array to the dataset.
        zvar->SetName(var.c_str());
        sgrid->GetPointData()->AddArray(zvar);
    }

    // Assemble the vtkDataArray for the matvar if this streak plot has a material.
    if(s.hasMaterial)
    {
        vtkFloatArray *nodecent_matvar = AssembleData(matvar, sdims, s.slice, s.sliceIndex, pdb);

        // Convert the node centered matvar into a cell centered var. We just convert
        // 1 fewer columns and rows from the node centered data to make the cell centered
        // data. Not ideal.
        vtkIntArray *cellcent_matvar = vtkIntArray::New();
        int cx = sdims[0] - 1;
        int cy = sdims[1] - 1;
        cellcent_matvar->SetNumberOfTuples(cx * cy);
        int *dest = (int *)cellcent_matvar->GetVoidPointer(0);
        int ghostMat = matnos[0];
        for(int j = 0; j < cy; ++j)
        {
            float *f = (float *)nodecent_matvar->GetVoidPointer(0);
            float *src = f + ((j+1) * sdims[0]);
            for(int i = 0; i < cx; ++i)
            {
                // mat==0 is a ghost zone. Safeguard against that value appearing
                // in the material data or VisIt will complain.
                int mat = (int)(*src++);
                if(mat <= 0)
                    mat = ghostMat;
                *dest++ = mat;
            }
        }
        cellcent_matvar->SetName(STREAK_MATERIAL);
        sgrid->GetCellData()->AddArray(cellcent_matvar);

        nodecent_matvar->Delete();
    }

    // cleanup
    yvar->Delete();

    // Transpose I,J if the data should match Silo.
    if(s.matchSilo)
    {
        vtkStructuredGrid *ds = TransposeIJ(sgrid);
        sgrid->Delete();
        sgrid = ds;
    }

    return sgrid;
}
#endif

// ****************************************************************************
// Method: Streaker::GetAuxiliaryData
//
// Purpose: 
//   This method gets the streak data associated with the material. The material
//   data was added to the streak data when the data was read -- otherwise it
//   gets created when we get the mesh via GetMesh here. We use the material
//   data stored on the mesh to create a material that we then pass back to
//   VisIt.
//
// Arguments:
//   var  : The variable to read.
//   type : The type of data to read.
//   args : unused
//   df   : The destructor function needed to destroy the return value.
//   pdb  : The PDB files that we'll use to construct the data.
//
// Returns:    An avtMaterial.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Dec  3 14:51:30 PST 2008
//
// Modifications:
//   
// ****************************************************************************

void *
Streaker::GetAuxiliaryData(const std::string &var, const char *type, void *args,
    DestructorFunction &df, const PDBFileObjectVector &pdb)
{
#ifndef MDSERVER
    if(strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        // Get the streak associated with the material var.
        std::map<std::string,std::string>::const_iterator it;
        it = matToStreak.find(var);
        if(it != matToStreak.end())
        {
            // Get the streak's dataset, which carries with it the cell-centered
            // material data.
            vtkDataSet *ds = GetMesh(it->second + "_mesh", pdb);
            if(ds != 0)
            {
                vtkIntArray *matlist = (vtkIntArray *)ds->GetCellData()->
                    GetArray(STREAK_MATERIAL);

                int *mnos = new int[matNames.size()];
                char **names = new char *[matNames.size()];
                for(size_t i = 0; i < matNames.size(); ++i)
                {
                    mnos[i] = matnos[i];
                    names[i] = (char *)matNames[i].c_str();
                }

                int dims = matlist->GetNumberOfTuples();
                int ndims = 1;
                df = avtMaterial::Destruct;

                avtMaterial *retval = new avtMaterial(
                    (int)matNames.size(),
                    mnos,
                    names,
                    ndims,
                    &dims,
                    0,
                    (int*)matlist->GetVoidPointer(0),
                    0,
                    0,
                    0,
                    0,
                    0
                );

                delete [] mnos;
                delete [] names;
                ds->Delete();

                return retval;
            }
        }
    }
#endif
    return 0;
}

