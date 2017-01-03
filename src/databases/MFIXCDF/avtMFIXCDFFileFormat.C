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
//                            avtMFIXCDFFileFormat.C                         //
// ************************************************************************* //

#include <avtMFIXCDFFileFormat.h>
#include <avtMFIXCDFOptions.h>

#include <cstring>
#include <math.h>
#include <visitstream.h>
#include <sstream>

#include <vtkCellData.h>
#include <vtkDataArray.h>
#include <vtkDemandDrivenPipeline.h>
#include <vtkDoubleArray.h>
#include <vtkExecutive.h>
#include <vtkFloatArray.h>
#include <vtkInformation.h>
#include <vtkInformationVector.h>
#include <vtkIntArray.h>
#include <vtkLongLongArray.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStreamingDemandDrivenPipeline.h>
#include <vtkStringArray.h>
#include <vtkStructuredGrid.h>
#include <vtkUnsignedCharArray.h>

#include <avtCallback.h>
#include <avtDatabaseMetaData.h>
#include <avtGhostData.h>
#include <avtMaterial.h>
#include <avtIntervalTree.h>
#include <avtParallel.h>

#include <DBOptionsAttributes.h>
#include <DebugStream.h>
#include <Expression.h>

#include <InvalidVariableException.h>
#include <InvalidFilesException.h>
#include <UnexpectedValueException.h>

#ifdef _WIN32
#define rint(x) (floor(x+0.5))
#define cbrt(x) (pow(x, 1.0/3.0))
#endif

using std::auto_ptr;
using std::vector;
using std::ostringstream;

void checkFile(NcFile* ncFile, const char* fname, int iSz, int jSz, int kSz)
{
    NcDim* iDim= ncFile->get_dim("x");
    if (iDim->size() != iSz-1)
    {
        debug5 << "File has wrong x dimension size" << endl;
        EXCEPTION1(InvalidFilesException,fname);
    }

    NcDim* jDim= ncFile->get_dim("y");
    if (jDim->size() != jSz-1)
    {
        debug5 << "File has wrong y dimension size" << endl;
        EXCEPTION1(InvalidFilesException,fname);
    }

    NcDim* kDim= ncFile->get_dim("z");
    if (kDim->size() == 1)
    {
        if (kSz != 1)
        {
            debug5 << "File has wrong z dimension size" << endl;
            EXCEPTION1(InvalidFilesException,fname);
        }
    }
    else
    {
        if (kDim->size() != kSz-1)
        {
            debug5 << "File has wrong z dimension size" << endl;
            EXCEPTION1(InvalidFilesException,fname);
        }
    }
    debug5 << "checkFile succeeded for " << fname << endl;
}

NcFile* openFile( const char* filename )
{
    NcFile* ncFile= new NcFile(filename, NcFile::ReadOnly);
    if (!ncFile->is_valid())
    {
        debug5 << "NcFile constructor failed for " << filename << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }

    int nAttr= ncFile->num_atts();
    if (nAttr != 0)
    {
        debug5 << "Unexpectedly found " << nAttr <<
               " attributes in the input file (expected zero)" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }

    NcDim* iDim= ncFile->get_dim("x");
    if (!iDim->is_valid())
    {
        debug5 << "Invalid dim 'x'" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }
    debug5 << "Size of " << filename << " " << iDim->name()
           << " is " << iDim->size() << endl;
    if (iDim->size()<2)
    {
        debug5 << "dimension of 1 in x is too small" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }

    NcDim* jDim= ncFile->get_dim("y");
    if (!jDim->is_valid())
    {
        debug5 << "Invalid dim 'y'" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }
    debug5 << "Size of " << filename << " " << jDim->name()
           << " is " << jDim->size() << endl;
    if (jDim->size()<2)
    {
        debug5 << "dimension of 1 in y is too small" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }

    NcDim* kDim= ncFile->get_dim("z");
    if (!kDim->is_valid())
    {
        debug5 << "Invalid dim 'z'" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }
    debug5 << "Size of " << filename << " " << kDim->name()
           << " is " << kDim->size() << endl;

    return ncFile;
}

// ****************************************************************************
//  Method: avtMFIXCDFFileFormat constructor
//
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

avtMFIXCDFFileFormat::avtMFIXCDFFileFormat(const char *filename, DBOptionsAttributes *readOpts)
    : avtSTMDFileFormat(&filename, 1)
{
    // INITIALIZE DATA MEMBERS
    numXDomains = DEF_N_X_DOMAINS;
    numYDomains = DEF_N_Y_DOMAINS;
    numZDomains = DEF_N_Z_DOMAINS;
    for (int i = 0; readOpts != 0 && i < readOpts->GetNumberOfOptions(); ++i)
    {
        if (readOpts->GetName(i) == N_X_DOMAINS)
            numXDomains = readOpts->GetInt(N_X_DOMAINS);
        if (readOpts->GetName(i) == N_Y_DOMAINS)
            numYDomains = readOpts->GetInt(N_Y_DOMAINS);
        if (readOpts->GetName(i) == N_Z_DOMAINS)
            numZDomains = readOpts->GetInt(N_Z_DOMAINS);
    }

    // Open the file, or try
    dataFile= openFile(filename);
    filePath= new std::string(filename);
    iSz= dataFile->get_dim("x")->size()+1;
    jSz= dataFile->get_dim("y")->size()+1;
    kSz= dataFile->get_dim("z")->size()+1;
    if (kSz==2)
    {
        kSz= 1; // special case of 2D
        numZDomains= 1;
    }

    Dx = Lx = NULL;
    Dy = Ly = NULL;
    Dz = Lz = NULL;

    par_size = PAR_Size();
    par_rank = PAR_Rank();

    // Get coordCode, which specifies coordinate system type
    NcDim* coordDim= dataFile->get_dim("coordinates");
    if (!coordDim->is_valid() || coordDim->size()!=1)
    {
        debug5 << "Invalid dim 'coordinates' or size!=1" << endl;
        EXCEPTION1(InvalidFilesException,filePath->c_str());
    }
    NcVar* coordVar= dataFile->get_var("coordinates");
    if (!coordVar->get(&coordCode,1L))
    {
        debug5 << "'coordinates' data is not an int" << endl;
        EXCEPTION1(InvalidFilesException,filePath->c_str());
    }
    if (coordCode<0 || coordCode>1)
    {
        debug5 << "'coordinates' data is invalid: " << coordCode << endl;
        EXCEPTION1(InvalidFilesException,filePath->c_str());
    }

    //     if (kSz == 1) {
    //       CalcDomainBreakdown2D(numDomainsPerProc * par_size, iSz-2, jSz-2,
    //                  &(numXDomains), &(numYDomains));
    //       numZDomains= 1;
    //     }
    //     else {
    //       CalcDomainBreakdown3D(numDomainsPerProc * par_size,
    //                 iSz-2, jSz-2, kSz-2,
    //                 &(numXDomains), &(numYDomains), &(numZDomains));
    //     }

    // We will keep a copy of the time.
    NcVar* timeVar= dataFile->get_var("t");
    if (!timeVar)
    {
        debug5 << "Data file has no 'time' variable" << endl;
        EXCEPTION1(InvalidFilesException,filename);
    }
    if (!(timeVar->get(&timeNow,1L)))
    {
        debug5 << "failed to read 1 double from the 'time' variable" << endl;
        EXCEPTION2(UnexpectedValueException,"get failed","t");
    }

    // Keeping the open dataFile turns out to be kind of costly when there
    // are a lot of files involved.
    FreeUpResources();

    debug5 << "finished constructor for time " << timeNow << endl;

}

avtMFIXCDFFileFormat::~avtMFIXCDFFileFormat()
{
    debug5 << "destructor for time " << timeNow << endl;
    delete dataFile;
    delete filePath;
}

// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

void
avtMFIXCDFFileFormat::FreeUpResources(void)
{
    debug5 << "FreeUpResources for time " << timeNow << endl;
    // null values signal the uninitialized state
    if (Dx)
    {
        Dx->Delete();
        Dx= NULL;
    }
    if (Lx)
    {
        Lx->Delete();
        Lx= NULL;
    }
    if (Dy)
    {
        Dy->Delete();
        Dy= NULL;
    }
    if (Ly)
    {
        Ly->Delete();
        Ly= NULL;
    }
    if (Dz)
    {
        Dz->Delete();
        Dz= NULL;
    }
    if (Lz)
    {
        Lz->Delete();
        Lz= NULL;
    }
    delete dataFile;
    dataFile= NULL;
}

double
avtMFIXCDFFileFormat::GetTime(void)
{
    return timeNow;
}

// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::GetAuxiliaryData

//  Purpose:
//      Gets the auxiliary data specified.
//
//  Arguments:
//      var        The variable of interest.
//      timestep   The timestep of interest.
//      domain     The domain of interest.
//      type       The type of auxiliary data.
//      args       The arguments for that type.
//
//  Returns:    The auxiliary data.
//
//  Programmer: Joel Welling
//  Creation:   July 1, 2010
//
// ****************************************************************************
void *
avtMFIXCDFFileFormat::GetAuxiliaryData(const char * var,
                                       int domain, const char *type, void *args,
                                       DestructorFunction &df)
{
    void *retval = NULL;

    // Decompose the domain
    int xDomain, yDomain, zDomain;
    decompose_domains(domain, &xDomain, &yDomain, &zDomain);

    // these are the unghosted cell index limits
    int widths[3];
    int offsets[3];
    double *larrays[3];

    checkCoordArrays();
    get_limit(iSz-1, xDomain, numXDomains, Lx, &widths[0], &offsets[0],
              &larrays[0]);
    get_limit(jSz-1, yDomain, numYDomains, Ly, &widths[1], &offsets[1],
              &larrays[1]);
    get_limit(kSz-1, zDomain, numZDomains, Lz, &widths[2], &offsets[2],
              &larrays[2]);

    if (strcmp(type, AUXILIARY_DATA_MATERIAL) == 0)
    {
        int *matlist = NULL;
        int *matnos = NULL;
        int nmats = 0;
        char** names = NULL;
        NcFile* meshFile = NULL;
        TRY
        {
            int dims[3] = {1,1,1}, ndims = 1, nzvals = 0;
            if ( coordCode==0 )
            {
                // Cartesian
                ndims = 3;
                dims[0] = widths[0]+2;
                dims[1] = widths[1]+2;
                if (this->kSz == 1)
                    dims[2]= nzvals= 1;
                else
                    dims[2]= nzvals= widths[2]+2;
            }
            else
            {
                // Cylindrical
                ndims = 1;
                int nzvals= 0; (void) nzvals;
                if (this->kSz == 1)
                {
                    dims[0] = (widths[0]+2)*(widths[1]+2);
                    nzvals= 1;
                }
                else
                {
                    dims[0] = (widths[0]+2)*(widths[1]+2)*(widths[2]+2);
                    nzvals= widths[2]+2;
                }
            }

            // Read the matlist array, which tells what the material
            // is for each zone in the mesh.
            int nzones = dims[0] * dims[1] * dims[2];
            if (!strcmp(var,"flagclass"))
            {
                nmats = 5;
                matnos = new int[nmats];
                for (int i =0; i<nmats; i++)
                    matnos[i] = i+1; // numbers are 1-based
                names = new char*[5];
                names[0] = strdup("Fluid");
                names[1] = strdup("Inlet");
                names[2] = strdup("Outlet");
                names[3] = strdup("Obstruction");
                names[4] = strdup("Other");
                matlist = new int[nzones];
                char meshFileName[256];
                strncpy(meshFileName,filePath->c_str(),sizeof(meshFileName)-1);
                char* ind= strrchr(meshFileName,'_');
                *ind= '\0'; // string now ends before '_'
                strncat(meshFileName,"_MESH.nc",sizeof(meshFileName)-1);
                meshFile= openFile(meshFileName);
                checkFile(meshFile, meshFileName, iSz, jSz, kSz);
                getBlockOfInts3D( meshFile, "flag", matlist,
                                  offsets[0], widths[0]+2,
                                  offsets[1], widths[1]+2, offsets[2], widths[2]+2 );

                // Convert from flag to flag class
                for (int i = 0; i < nzones; i++)
                {
                    if (matlist[i] < 10)
                        matlist[i] = 1; // fluid
                    else if (matlist[i] == 10 || matlist[i] == 20)
                        matlist[i] = 2; // inlet
                    else if (matlist[i] == 11 || matlist[i] == 21 || matlist[i] == 31)
                        matlist[i] = 3; // outlet
                    else if (matlist[i] >= 100)
                        matlist[i] = 4; // obstruction
                    else
                        matlist[i] = 5; // other
                }
            }
            else
                EXCEPTION1(InvalidVariableException, var);

            avtMaterial* mat = new avtMaterial(nmats, matnos, names, ndims,
                                               dims, 0, matlist,
                                               0, // length of mix arrays
                                               0, // mix_mat array
                                               0, // mix_next array
                                               0, // mix_zone array
                                               0 // mix_vf array
                                              );

            // Clean up.
            delete meshFile;
            delete [] matlist;
            delete [] matnos;
            for(int i = 0; i < nmats; ++i) delete [] names[i];
            delete [] names;
            // Set the return values.
            retval = (void *)mat;
            df = avtMaterial::Destruct;
        }
        CATCH(InvalidFilesException)
        {
            // Clean up.
            delete meshFile;
            delete [] matlist;
            delete [] matnos;
            for(int i = 0; i < nmats; ++i) delete [] names[i];
            delete [] names;

            avtCallback::IssueWarning("Unable to locate and load MESH file!");
        }
        CATCHALL
        {
            // Clean up.
            delete meshFile;
            delete [] matlist;
            delete [] matnos;
            for(int i = 0; i < nmats; ++i) delete [] names[i];
            delete [] names;

            RETHROW;
        }
        ENDTRY
    }
    else if (strcmp(type, AUXILIARY_DATA_SPATIAL_EXTENTS) == 0)
    {
        // Read the number of domains for the mesh.
        // Read the spatial extents for each domain of the
        // mesh. This information should be in a single
        // and should be available without having to
        // read the real data. The expected format for
        // the data in the spatialextents array is to
        // repeat the following pattern for each domain:
        // xmin, xmax, ymin, ymax, zmin, zmax.

        int nDoms = numXDomains * numYDomains * numZDomains;

        double *spatialextents = new double[nDoms * 6];

        double* runner= spatialextents;
        for (int iDom=0; iDom<nDoms; iDom++)
        {
            // Be careful to work with local version, not the method's
            // top level domain info!
            int xDom, yDom, zDom;
            decompose_domains(iDom, &xDom, &yDom, &zDom);

            // these are the unghosted cell index limits
            int w[3];
            int o[3];
            double *l[3];
            double xmin, xmax, ymin, ymax, zmin, zmax;

            get_limit(iSz-1, xDom, numXDomains, Lx, &w[0], &o[0], &l[0]);
            get_limit(jSz-1, yDom, numYDomains, Ly, &w[1], &o[1], &l[1]);
            get_limit(kSz-1, zDom, numZDomains, Lz, &w[2], &o[2], &l[2]);

            ymin= l[1][o[1]];
            ymax= l[1][o[1]+w[1]+2];
            if (coordCode==0)
            {
                // Cartesian
                xmin= l[0][o[0]];
                xmax= l[0][o[0]+w[0]+2];
                if (kSz == 1)
                {
                    // 2D
                    zmin= zmax= 0.0;
                }
                else
                {
                    zmin= l[2][o[2]];
                    zmax= l[2][o[2]+w[2]+2];
                }
            }
            else
            {
                // Cylindrical
                double rMin= l[0][o[0]];
                double rMax= l[0][o[0]+w[0]+2];
                if (kSz == 1)
                {
                    // 2D
                    xmin= rMin;
                    xmax= rMax;
                    zmin= zmax= 0.0;
                }
                else
                {
                    double thetaMin= l[2][o[2]];
                    double thetaMax= l[2][o[2]+w[2]+2];
                    // Walk through the corners building bounding box-
                    // boring but straightforward.
                    double xTry= rMin*cos(thetaMin);
                    double zTry= -rMin*sin(thetaMin);
                    xmin= xmax= xTry;
                    zmin= zmax= zTry;

                    xTry= rMin*cos(thetaMax);
                    zTry= -rMin*sin(thetaMax);
                    if (xTry<xmin) xmin= xTry;
                    if (xTry>xmax) xmax= xTry;
                    if (zTry<zmin) zmin= zTry;
                    if (zTry>zmax) zmax= zTry;

                    xTry= rMax*cos(thetaMin);
                    zTry= -rMax*sin(thetaMin);
                    if (xTry<xmin) xmin= xTry;
                    if (xTry>xmax) xmax= xTry;
                    if (zTry<zmin) zmin= zTry;
                    if (zTry>zmax) zmax= zTry;

                    xTry= rMax*cos(thetaMax);
                    zTry= -rMax*sin(thetaMax);
                    if (xTry<xmin) xmin= xTry;
                    if (xTry>xmax) xmax= xTry;
                    if (zTry<zmin) zmin= zTry;
                    if (zTry>zmax) zmax= zTry;

                    if (thetaMin<=0.0 && thetaMax>=0.0) xmax= rMax;
                    if (thetaMin<=0.5*M_PI && thetaMax>=0.5*M_PI) zmax= rMax;
                    if (thetaMin<=M_PI && thetaMax>=M_PI) xmin= -rMax;
                    if (thetaMin<=1.5*M_PI && thetaMax>=1.5*M_PI) zmin= -rMax;
                }

            }

            *runner++= xmin;
            *runner++= xmax;
            *runner++= ymin;
            *runner++= ymax;
            *runner++= zmin;
            *runner++= zmax;
        }

        // Create an interval tree
        avtIntervalTree *itree = new avtIntervalTree(nDoms, 3);
        runner = spatialextents;
        for(int iDom = 0; iDom < nDoms; iDom++)
        {
            itree->AddElement(iDom, runner);
            runner += 6;
        }
        itree->Calculate(true);
        // Delete temporary array.
        delete [] spatialextents;
        // Set return values
        retval = (void *)itree;
        df = avtIntervalTree::Destruct;
    }

    return retval;
}

// This code implements an algorithm to find X, Y and Z domain divisions
// given a target number of domains.  Unfortunately VisIt doesn't seem to
// support this unless the number of domains is exactly PAR_Rank.  I am
// leaving it here out of optimism for the future.
// void
// avtMFIXCDFFileFormat::CalcDomainBreakdown2D(long targetDomains,
//     int cellsX, int cellsY, int* nX, int* nY)
// {
//     long totalCells= cellsX*cellsY;
//     double approxCellsPerDomain= (double)totalCells/targetDomains;
//     double approxCellsPerEdge= sqrt(approxCellsPerDomain);
//     if (approxCellsPerEdge>cellsX) approxCellsPerEdge= (double)cellsX;
//     if (approxCellsPerEdge>cellsY) approxCellsPerEdge= (double)cellsY;
//     if (cellsX>=cellsY) {
//         *nY= (int)rint(cellsY/approxCellsPerEdge);
//         *nX= (int)rint((double)targetDomains/(*nY));
//     }
//     else {
//         *nX= (int)rint(cellsX/approxCellsPerEdge);
//         *nY= (int)rint((double)targetDomains/(*nX));
//     }
// }

// void
// avtMFIXCDFFileFormat::CalcDomainBreakdown3D(long targetDomains,
//     int cellsX, int cellsY, int cellsZ, int* nX, int* nY, int* nZ)
// {
//     long totalCells= cellsX*cellsY*cellsZ;
//     double approxCellsPerDomain= (double)totalCells/targetDomains;
// //     debug5 << "Calculating domain sizes in 3D" << endl;
// //     debug5 << "cellsX " << cellsX << " cellsY " << cellsY << " cellsZ "
// //        << cellsZ << " totalCells " << totalCells << endl;
//     double approxCellsPerEdge= cbrt(approxCellsPerDomain);
// //     debug5 << "approxCellsPerDomain " << approxCellsPerDomain
// //        << " approxCellsPerEdge " << approxCellsPerEdge << endl;
//     int zTargetDomains= (int)rint((double)cellsZ/approxCellsPerEdge);
// //    debug5 << "zTargetDomains " << zTargetDomains << endl;
//     if (zTargetDomains<1) zTargetDomains= 1;
//     int inPlaneTargetDomains= (int)rint((double)targetDomains/(double)zTargetDomains);
// //     debug5 << "inPlaneTargetDomains " << inPlaneTargetDomains << endl;
//     CalcDomainBreakdown2D(inPlaneTargetDomains, cellsX, cellsY, nX, nY);
// //     debug5 << "nX " << *nX << " nY " << *nY << endl;
//     *nZ= (int)rint((double)targetDomains/(*nX * *nY));
// //     debug5 << "initial nZ " << *nZ << endl;
//     if (*nZ<1) *nZ= 1;
//     if (*nZ>cellsZ) *nZ= cellsZ;
// //     debug5 << "nZ " << *nZ << endl;
// }

void
avtMFIXCDFFileFormat::inferVectorVariableNames(avtDatabaseMetaData *md,
        std::set<std::string>& varNames)
{
    // U_g, V_g, W_g make up gas velocity; U_s_nnn etc. makes up
    // solid velocity for solid nnn.
    std::set<std::string>::iterator it;
    for (it=varNames.begin(); it!=varNames.end(); it++)
    {
        std::string s= *it;
        if (!strcmp(s.c_str(),"U_g"))
        {
            if ((varNames.find("V_g") != varNames.end())
                    && (varNames.find("W_g") != varNames.end()))
            {

                AddVectorVarToMetaData(md, "Vel_g", "Mesh", AVT_ZONECENT,3);
            }
        }
        else if (!strncmp(s.c_str(),"U_s_",4))
        {
            long index= strtol(s.c_str()+4, NULL, 10);
            if (index==0 || index==LONG_MIN || index==LONG_MAX)
                EXCEPTION1(InvalidFilesException,filePath->c_str());
            char buf[100];
            SNPRINTF(buf,sizeof(buf),"Vel_s_%03ld",index);
            AddVectorVarToMetaData(md, buf, "Mesh", AVT_ZONECENT,3);
        }
    }
}

void
avtMFIXCDFFileFormat::checkCoordArrays()
// ****************************************************************************
// Verify that the coordinate arrays are valid, building them
// if necessary.
// ****************************************************************************
{
    if (!dataFile)   // null value signals that file is not open
    {
        dataFile= openFile(filePath->c_str());
    }

    if (!Dx)   // null values signal the uninitialized state
    {

        Dx = vtkDoubleArray::New();
        Lx = vtkDoubleArray::New();
        Dy = vtkDoubleArray::New();
        Ly = vtkDoubleArray::New();
        Dz = vtkDoubleArray::New();
        Lz = vtkDoubleArray::New();

        Lx->Resize(iSz);
        Ly->Resize(jSz);
        Lz->Resize(kSz);

        Dx->Resize(iSz-1);
        Dy->Resize(jSz-1);

        getBlockOfDoubles1D(dataFile,"x", (double*)Lx->GetVoidPointer(0), 0, iSz-1);
        Lx->SetValue(iSz-1,(2*Lx->GetValue(iSz-2))-Lx->GetValue(iSz-3));
        getBlockOfDoubles1D(dataFile,"y", (double*)Ly->GetVoidPointer(0), 0, jSz-1);
        Ly->SetValue(jSz-1,(2*Ly->GetValue(jSz-2))-Ly->GetValue(jSz-3));

        for (int i=0; i<iSz-1; i++)
            Dx->SetValue(i, Lx->GetValue(i+1)-Lx->GetValue(i));
        for (int j=0; j<jSz-1; j++)
            Dy->SetValue(j, Ly->GetValue(j+1)-Ly->GetValue(j));

        if (kSz==1)
        {
            Dz->Resize(1);
            getBlockOfDoubles1D(dataFile, "z", (double*)Lz->GetVoidPointer(0), 0, 1);
            Dz->SetValue(0, 0.0);
        }
        else
        {
            Dz->Resize(kSz-1);
            getBlockOfDoubles1D(dataFile,"z", (double*)Lz->GetVoidPointer(0), 0,
                                kSz-1);
            Lz->SetValue(kSz-1,(2*Lz->GetValue(kSz-2))-Lz->GetValue(kSz-3));
            for (int k=0; k<kSz-1; k++)
                Dz->SetValue(k, Lz->GetValue(k+1)-Lz->GetValue(k));
        }
    }
}

// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

void
avtMFIXCDFFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    avtMeshMetaData *mesh = new avtMeshMetaData;
    mesh->name = "Mesh";

    checkCoordArrays();
    int nVar= dataFile->num_vars();

    if (coordCode==0)
        mesh->meshType = AVT_RECTILINEAR_MESH;
    else if (coordCode==1)
        mesh->meshType = AVT_CURVILINEAR_MESH;
    else
    {
        // Actually this was caught in constructor, but for clarity...
        debug5 << "'coordinates' data is invalid: " << coordCode << endl;
        EXCEPTION1(InvalidFilesException,filePath->c_str());
    }

    if (kSz == 1)
    {
        kSz= 1;
        mesh->topologicalDimension = 2;
        mesh->spatialDimension = 2;
        mesh->numBlocks = numXDomains*numYDomains;
    }
    else
    {
        mesh->topologicalDimension = 3;
        mesh->spatialDimension = 3;
        mesh->numBlocks = numXDomains * numYDomains * numZDomains;
    }
    // debug5 << "numBlocks calculation: " << par_size << " yields " << mesh->numBlocks << endl;
    mesh->blockOrigin = 0;
    mesh->blockTitle = "blocks";
    mesh->blockPieceName = "block";
    mesh->hasSpatialExtents = false;
    md->Add(mesh);

    // The 'flag' data will become a VisIt 'material'
    avtMaterialMetaData *matmd = new avtMaterialMetaData;
    matmd->name = "flagclass";
    matmd->meshName = "Mesh";
    matmd->numMaterials = 5;
    matmd->materialNames.push_back("Fluid");
    matmd->materialNames.push_back("Inlet");
    matmd->materialNames.push_back("Outlet");
    matmd->materialNames.push_back("Obstruction");
    matmd->materialNames.push_back("Other");
    md->Add(matmd);

    // Dirty trick: provide space to return the domain number
    // parallel rank and cell type flag as if they were data.
    AddScalarVarToMetaData(md, "domain", "Mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "par_rank", "Mesh", AVT_ZONECENT);
    AddScalarVarToMetaData(md, "flagclass_var", "Mesh", AVT_ZONECENT);

    std::set<std::string> varNames;
    for (int i=0; i<nVar; i++)
    {
        NcVar* v= dataFile->get_var(i);
        NcToken t= v->name();
        std::string typeNames[]= {"nat","byte","char","short","int","float","double"};
        debug5 << "Variable " << v->name() << " is of type "
               << typeNames[v->type()] << endl;
        if (strcmp(t,"x") && strcmp(t,"y") && strcmp(t,"z")
                && strcmp(t,"coordinates") && strcmp(t,"t"))
        {
            // Well, I guess this must be a real variable, then.
            AddScalarVarToMetaData(md, v->name(), "Mesh", AVT_ZONECENT);
            varNames.insert(v->name());
            for (int l=0; l<v->num_dims(); l++)
            {
                debug5 << "Variable " << v->name() << " dim " << l << " size is "
                       << v->get_dim(l)->size() << endl;
            }
        }
    }
    // debug5 << "Finished defining scalar variables" << endl;

    // Vector variables come in as sets of 3 similarly-named scalar variables
    inferVectorVariableNames(md, varNames);

    // debug5 << "Finished defining vector variables" << endl;
}

//   A note on how the domain index range stuff works: Comparing
//   get_limit to the old MFIX version, 'max' in the calls to
//   get_limit should be the size of the Dx array, which is iSz-1
//   rather than iSz.  For the highest domain in that direction,
//   iCellHigh is then max-3, which is iSz-4.  That makes the highest
//   offset+(width+3) iSz-1, which is the last entry in the array of
//   size iSz, which sounds right.  So all those mesh indices should
//   be width+3, even though that sounds too high.

void
avtMFIXCDFFileFormat::decompose_domains(int dom, int *xd, int *yd, int *zd)
{
    *zd = dom / (numXDomains * numYDomains);
    dom -= *zd * (numXDomains * numYDomains);
    *yd = dom / numXDomains;
    dom -= *yd * numXDomains;
    *xd = dom;
}

void
avtMFIXCDFFileFormat::get_limit(int max, int dom, int num_domains,
                                vtkDoubleArray *l, int *pwidth, int *poff,
                                double **larr)
{
    // these are the unghosted cell index limits
    if (max==0)
    {
        // special case of degenerate dimension, used for 2D data
        *pwidth = -1;
        *poff = 0;
        *larr = (double *)l->GetVoidPointer(0);
    }
    else
    {
        // general case
        int width = (max - 2) / num_domains;
        int iCellLow = dom * width;
        int iCellHigh = iCellLow + width - 1;
        if (dom == num_domains - 1)
            iCellHigh = max - 3; // get leftovers
        *pwidth = iCellHigh + 1 - iCellLow;
        *poff = iCellLow;
        *larr = (double *)l->GetVoidPointer(0);
    }
}

// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::GetMesh
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
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

vtkDataSet *
avtMFIXCDFFileFormat::GetMesh(int domain, const char *meshname)
{
    // Mesh does not depend on time.  There is only one mesh,
    // so we need not check its name.

    debug5 << "Enter GetMesh for domain " << domain << " time " << timeNow
           << endl;

    // Decompose the domain
    int xDomain, yDomain, zDomain;
    decompose_domains(domain, &xDomain, &yDomain, &zDomain);

    // We'll be using these to mark ghost zones
    unsigned char vInternalGhost = 0;
    avtGhostData::AddGhostZoneType(vInternalGhost,
                                   DUPLICATED_ZONE_INTERNAL_TO_PROBLEM);
    unsigned char vExternalGhost = 0;
    avtGhostData::AddGhostZoneType(vExternalGhost, ZONE_EXTERIOR_TO_PROBLEM);

    // these are the unghosted cell index limits
    int widths[3];
    int offsets[3];
    double* larrays[3];

    checkCoordArrays();
    get_limit(iSz-1, xDomain, numXDomains, Lx, &widths[0], &offsets[0],
              &larrays[0]);
    get_limit(jSz-1, yDomain, numYDomains, Ly, &widths[1], &offsets[1],
              &larrays[1]);
    get_limit(kSz-1, zDomain, numZDomains, Lz, &widths[2], &offsets[2],
              &larrays[2]);

    if ( coordCode==0 && kSz!=1 )
    {
        debug5 << "Generating Cartesian 3D mesh" << endl;
        // 3-D cartesian grid
        vtkDoubleArray *coords[3] = {NULL,NULL,NULL};

        for (int iCoord =0; iCoord<3; iCoord++)
        {
            coords[iCoord] = vtkDoubleArray::New();
            coords[iCoord]->SetNumberOfTuples(widths[iCoord]+3);
            double* varray = (double*)(coords[iCoord]->GetVoidPointer(0));
            double* larray = larrays[iCoord];
            for (int i =0; i<widths[iCoord]+3; i++)
                varray[i] = larray[i+offsets[iCoord]];
        }

        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        rgrid->SetDimensions(widths[0]+3,widths[1]+3,widths[2]+3);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfComponents(1);
        int totCells = (widths[0]+2)*(widths[1]+2)*(widths[2]+2);
        ghostCells->SetNumberOfTuples(totCells);
        unsigned char *buf = ghostCells->GetPointer(0);
        int xLeftGhost = (xDomain ==0 ? vExternalGhost:vInternalGhost);
        int xRightGhost = (xDomain ==numXDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int yLeftGhost = (yDomain ==0 ? vExternalGhost:vInternalGhost);
        int yRightGhost = (yDomain ==numYDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int zLeftGhost = (zDomain ==0 ? vExternalGhost:vInternalGhost);
        int zRightGhost = (zDomain ==numZDomains-1 ?
                           vExternalGhost:vInternalGhost);
        for (int i =0; i<totCells; i++) buf[i] = 0;

        for (int k =0; k<widths[2]+2; k++)
            for (int j =0; j<widths[1]+2; j++)
            {
                int i = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= xLeftGhost;
                i = widths[0]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= xRightGhost;
            }

        for (int k =0; k<widths[2]+2; k++)
            for (int i =0; i<widths[0]+2; i++)
            {
                int j = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= yLeftGhost;
                j = widths[1]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= yRightGhost;
            }

        for (int j =0; j<widths[1]+2; j++)
            for (int i =0; i<widths[0]+2; i++)
            {
                int k = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= zLeftGhost;
                k = widths[2]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= zRightGhost;
            }
        rgrid->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete(); // held alive by ref count
        rgrid->GetInformation()->Set(
            vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

        return rgrid;

    }
    else if ( coordCode==0 && kSz==1 )
    {
        // 2-D cartesian grid

        debug5 << "Generating Cartesian 2D Mesh" << endl;
        vtkDoubleArray *coords[3] = {NULL,NULL,NULL};

        for (int iCoord =0; iCoord<2; iCoord++)
        {
            coords[iCoord] = vtkDoubleArray::New();
            coords[iCoord]->SetNumberOfTuples(widths[iCoord]+3);
            double* varray = (double*)(coords[iCoord]->GetVoidPointer(0));
            double* larray = larrays[iCoord];
            for (int i =0; i<widths[iCoord]+3; i++)
                varray[i] = larray[i+offsets[iCoord]];
        }
        coords[2]= vtkDoubleArray::New();
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetValue(0,0.0);

        vtkRectilinearGrid *rgrid = vtkRectilinearGrid::New();
        rgrid->SetDimensions(widths[0]+3,widths[1]+3,1);
        rgrid->SetXCoordinates(coords[0]);
        coords[0]->Delete();
        rgrid->SetYCoordinates(coords[1]);
        coords[1]->Delete();
        rgrid->SetZCoordinates(coords[2]);
        coords[2]->Delete();

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfComponents(1);
        int totCells = (widths[0]+2)*(widths[1]+2);
        ghostCells->SetNumberOfTuples(totCells);
        unsigned char *buf = ghostCells->GetPointer(0);
        int xLeftGhost = (xDomain ==0 ? vExternalGhost:vInternalGhost);
        int xRightGhost = (xDomain ==numXDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int yLeftGhost = (yDomain ==0 ? vExternalGhost:vInternalGhost);
        int yRightGhost = (yDomain ==numYDomains-1 ?
                           vExternalGhost:vInternalGhost);
        for (int i =0; i<totCells; i++) buf[i] = 0;

        for (int j =0; j<widths[1]+2; j++)
        {
            int i = 0;
            int index = (j)*(widths[0]+2)+i;
            buf[index] |= xLeftGhost;
            i = widths[0]+1;
            index = (j)*(widths[0]+2)+i;
            buf[index] |= xRightGhost;
        }

        for (int i =0; i<widths[0]+2; i++)
        {
            int j = 0;
            int index = (j)*(widths[0]+2)+i;
            buf[index] |= yLeftGhost;
            j = widths[1]+1;
            index = (j)*(widths[0]+2)+i;
            buf[index] |= yRightGhost;
        }

        rgrid->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete(); // held alive by ref count
        rgrid->GetInformation()->Set(
            vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

        return rgrid;
    }
    else if ( coordCode==1 && kSz==1 )
    {
        // 2-D cylindrical grid
        debug5 << "Generating Cylindrical 2D mesh" << endl;
        vtkDoubleArray *coords[3] = {NULL,NULL,NULL};

        for (int iCoord =0; iCoord<2; iCoord++)
        {
            coords[iCoord] = vtkDoubleArray::New();
            coords[iCoord]->SetNumberOfTuples(widths[iCoord]+3);
            double* varray = (double*)(coords[iCoord]->GetVoidPointer(0));
            double* larray = larrays[iCoord];
            for (int i =0; i<widths[iCoord]+3; i++)
                varray[i] = larray[i+offsets[iCoord]];
        }
        coords[2]= vtkDoubleArray::New();
        coords[2]->SetNumberOfTuples(1);
        coords[2]->SetValue(0,0.0);

        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        sgrid->SetPoints(points);
        sgrid->SetDimensions(widths[0]+3,widths[1]+3,1);
        points->Delete();
        points->SetNumberOfPoints((widths[0]+3)*(widths[1]+3));
        float *pts = (float*)points->GetVoidPointer(0);
        for (int j =0; j<widths[1]+3; j++)
            for (int i =0; i<widths[0]+3; i++)
            {
                double r = coords[0]->GetValue(i);
                double y = coords[1]->GetValue(j);
                double theta = 0.0;
                double x = r*cos(theta);
                double z = -r*sin(theta);
                *pts++= (float)x;
                *pts++= (float)y;
                *pts++= (float)z;
            }
        for (int iCoord =0; iCoord<3; iCoord++) coords[iCoord]->Delete();

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfComponents(1);
        int totCells = (widths[0]+2)*(widths[1]+2);
        ghostCells->SetNumberOfTuples(totCells);
        unsigned char *buf = ghostCells->GetPointer(0);
        int xLeftGhost = vInternalGhost;
        int xRightGhost = (xDomain ==numXDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int yLeftGhost = (yDomain ==0 ? vExternalGhost:vInternalGhost);
        int yRightGhost = (yDomain ==numYDomains-1 ?
                           vExternalGhost:vInternalGhost);
        for (int i =0; i<totCells; i++) buf[i] = 0;

        for (int j =0; j<widths[1]+2; j++)
        {
            int i = 0;
            int index = (j)*(widths[0]+2)+i;
            buf[index] |= xLeftGhost;
            i = widths[0]+1;
            index = (j)*(widths[0]+2)+i;
            buf[index] |= xRightGhost;
        }

        for (int i =0; i<widths[0]+2; i++)
        {
            int j = 0;
            int index = (j)*(widths[0]+2)+i;
            buf[index] |= yLeftGhost;
            j = widths[1]+1;
            index = (j)*(widths[0]+2)+i;
            buf[index] |= yRightGhost;
        }

        sgrid->GetCellData()->AddArray(ghostCells);
        ghostCells->Delete(); // held alive by ref count
        sgrid->GetInformation()->Set(
            vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

        return sgrid;
    }
    else
    {
        // 3-D cylindrical grid
        debug5 << "Generating Cylindrical 3D mesh" << endl;
        vtkDoubleArray *coords[3] = {NULL,NULL,NULL};

        for (int iCoord =0; iCoord<3; iCoord++)
        {
            coords[iCoord] = vtkDoubleArray::New();
            coords[iCoord]->SetNumberOfTuples(widths[iCoord]+3);
            double* varray = (double*)(coords[iCoord]->GetVoidPointer(0));
            double* larray = larrays[iCoord];
            for (int i =0; i<widths[iCoord]+3; i++)
                varray[i] = larray[i+offsets[iCoord]];
        }

        vtkStructuredGrid *sgrid = vtkStructuredGrid::New();
        vtkPoints *points = vtkPoints::New();
        sgrid->SetPoints(points);
        sgrid->SetDimensions(widths[0]+3,widths[1]+3,widths[2]+3);
        points->Delete();
        points->SetNumberOfPoints((widths[0]+3)*(widths[1]+3)*(widths[2]+3));
        float *pts = (float*)points->GetVoidPointer(0);
        for (int k =0; k<widths[2]+3; k++)
            for (int j =0; j<widths[1]+3; j++)
                for (int i =0; i<widths[0]+3; i++)
                {
                    double r = coords[0]->GetValue(i);
                    double y = coords[1]->GetValue(j);
                    double theta = coords[2]->GetValue(k);
                    double x = r*cos(theta);
                    double z = -r*sin(theta);
                    *pts++= (float)x;
                    *pts++= (float)y;
                    *pts++= (float)z;
                }
        for (int iCoord =0; iCoord<3; iCoord++) coords[iCoord]->Delete();

        vtkUnsignedCharArray *ghostCells = vtkUnsignedCharArray::New();
        ghostCells->SetName("avtGhostZones");
        ghostCells->SetNumberOfComponents(1);
        int totCells = (widths[0]+2)*(widths[1]+2)*(widths[2]+2);
        ghostCells->SetNumberOfTuples(totCells);
        unsigned char *buf = ghostCells->GetPointer(0);
        int xLeftGhost = vInternalGhost;
        int xRightGhost = (xDomain == numXDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int yLeftGhost = (yDomain ==0 ? vExternalGhost:vInternalGhost);
        int yRightGhost = (yDomain ==numYDomains-1 ?
                           vExternalGhost:vInternalGhost);
        int zLeftGhost = vInternalGhost;
        int zRightGhost = vInternalGhost;

        for (int i =0; i<totCells; i++) buf[i] = 0;

        for (int k =0; k<widths[2]+2; k++)
            for (int j =0; j<widths[1]+2; j++)
            {
                int i = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= xLeftGhost;
                i = widths[0]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= xRightGhost;
            }

        for (int k =0; k<widths[2]+2; k++)
            for (int i =0; i<widths[0]+2; i++)
            {
                int j = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= yLeftGhost;
                j = widths[1]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= yRightGhost;
            }

        for (int j =0; j<widths[1]+2; j++)
            for (int i =0; i<widths[0]+2; i++)
            {
                int k = 0;
                int index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= zLeftGhost;
                k = widths[2]+1;
                index = (k*(widths[1]+2)+j)*(widths[0]+2)+i;
                buf[index] |= zRightGhost;
            }
        sgrid->GetCellData()->AddArray(ghostCells);
        sgrid->GetInformation()->Set(
            vtkStreamingDemandDrivenPipeline::UPDATE_NUMBER_OF_GHOST_LEVELS(), 0);

        return sgrid;
    }
}


// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::GetVar
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
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

vtkDataArray *
avtMFIXCDFFileFormat::GetVar(int domain, const char *varname)
{
    // Decompose the domain
    int xDomain, yDomain, zDomain;
    decompose_domains(domain, &xDomain, &yDomain, &zDomain);

    // these are the unghosted cell index limits
    int widths[3];
    int offsets[3];
    double *larrays[3];

    checkCoordArrays();
    get_limit(iSz-1, xDomain, numXDomains, Lx,
              &widths[0], &offsets[0], &larrays[0]);
    get_limit(jSz-1, yDomain, numYDomains, Ly,
              &widths[1], &offsets[1], &larrays[1]);
    get_limit(kSz-1, zDomain, numZDomains, Lz,
              &widths[2], &offsets[2], &larrays[2]);

    vtkFloatArray *arr = vtkFloatArray::New();
    int totNodes= 0; (void) totNodes;
    int totZones= 0;
    int nzvals= 0; (void) nzvals;

    if (kSz==1)
    {
        // 2D case
        totNodes = (widths[0]+3) * (widths[1]+3);
        totZones = (widths[0]+2) * (widths[1]+2);
        nzvals= 1;
    }
    else
    {
        // 3D case
        totNodes = (widths[0]+3) * (widths[1]+3) * (widths[2]+3);
        totZones = (widths[0]+2) * (widths[1]+2) * (widths[2]+2);
        nzvals= (widths[2]+3);
    }

    TRY
    {
        if (!strcmp(varname, "domain"))
        {
            arr->SetNumberOfTuples(totZones);
            for (int i = 0; i < totZones; i++) arr->SetValue(i, domain);
        }
        else if (!strcmp(varname, "par_rank"))
        {
            arr->SetNumberOfTuples(totZones);
            for (int i = 0; i < totZones; i++) arr->SetValue(i, par_rank);
        }
        else if (!strcmp(varname, "flagclass_var"))
        {
            DestructorFunction matDF = NULL;
            avtMaterial* mat = NULL;
            TRY
            {
                mat= (avtMaterial*)GetAuxiliaryData("flagclass",
                                                    domain,
                                                    AUXILIARY_DATA_MATERIAL,
                                                    NULL, matDF);
                if (mat->GetNZones() != totZones)
                {
                    debug5 << "Internal error: material GetNZones " << mat->GetNZones()
                           << " vs. totZones " << totZones << endl;
                    EXCEPTION1(InvalidVariableException, varname);
                }
                const int* buf= mat->GetMatlist();
                arr->SetNumberOfTuples(totZones);
                float *data = (float*)arr->GetVoidPointer(0);
                for (int i=0; i<totZones; i++) data[i]= (float)buf[i]+1.0;
                if (matDF) matDF(mat);
            }
            CATCHALL
            {
                if (matDF) matDF(mat);
                RETHROW;
            }
            ENDTRY
        }
        else
        {
            arr->SetNumberOfTuples(totZones);
            getBlockOfFloats3D(dataFile, varname, (float*)arr->GetVoidPointer(0),
                               offsets[0], widths[0]+2,
                               offsets[1], widths[1]+2, offsets[2], widths[2]+2);
        }

        return arr;
    }
    CATCHALL
    {
        arr->Delete();
        RETHROW;
    }
    ENDTRY
}


// ****************************************************************************
//  Method: avtMFIXCDFFileFormat::GetVectorVar
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
//  Programmer: welling -- generated by xml2avt
//  Creation:   Wed Aug 3 16:35:13 PST 2011
//
// ****************************************************************************

vtkDataArray *
avtMFIXCDFFileFormat::GetVectorVar(int domain, const char *varname)
{
    // Decompose the domain
    int xDomain, yDomain, zDomain;
    decompose_domains(domain, &xDomain, &yDomain, &zDomain);

    // these are the unghosted cell index limits
    int widths[3];
    int offsets[3];
    double *larrays[3];

    checkCoordArrays();
    get_limit(iSz-1, xDomain, numXDomains, Lx,
              &widths[0], &offsets[0], &larrays[0]);
    get_limit(jSz-1, yDomain, numYDomains, Ly,
              &widths[1], &offsets[1], &larrays[1]);
    get_limit(kSz-1, zDomain, numZDomains, Lz,
              &widths[2], &offsets[2], &larrays[2]);

    int totNodes= 0; (void) totNodes;
    int totZones= 0;
    int nzvals= 0; (void) nzvals;
    if (kSz==1)
    {
        // 2D case
        totNodes = (widths[0]+3) * (widths[1]+3);
        totZones = (widths[0]+2) * (widths[1]+2);
        nzvals= 1;
    }
    else
    {
        // 3D case
        totNodes = (widths[0]+3) * (widths[1]+3) * (widths[2]+3);
        totZones = (widths[0]+2) * (widths[1]+2) * (widths[2]+2);
        nzvals= (widths[2]+3);
    }

    // This trick with auto_ptr makes xvec, yvec and zvec get deleted
    // when they go out of scope.
    auto_ptr< vector<float> > xvec(new vector<float>(totZones));
    float* xdata= &(*xvec)[0];
    auto_ptr< vector<float> > yvec(new vector<float>(totZones));
    float* ydata= &(*yvec)[0];
    auto_ptr< vector<float> > zvec(new vector<float>(totZones));
    float* zdata= &(*zvec)[0];

    if (!strncmp(varname,"Vel_",4))
    {
        if (!strcmp(varname,"Vel_g"))
        {
            getBlockOfFloats3D(dataFile, "U_g", xdata, offsets[0], widths[0]+2,
                               offsets[1], widths[1]+2, offsets[2], widths[2]+2);
            getBlockOfFloats3D(dataFile, "V_g", ydata, offsets[0], widths[0]+2,
                               offsets[1], widths[1]+2, offsets[2], widths[2]+2);
            getBlockOfFloats3D(dataFile, "W_g", zdata, offsets[0], widths[0]+2,
                               offsets[1], widths[1]+2, offsets[2], widths[2]+2);
        }
        else
        {
            long index= strtol(varname+6, NULL, 10);

            if (!strncmp(varname,"Vel_s_",6)
                    && (index!=0) && (index!=LONG_MIN) && (index!=LONG_MAX))
            {
                // one of the solid velocities
                char buf[100];
                SNPRINTF(buf,sizeof(buf),"U_s_%03ld",index);
                getBlockOfFloats3D(dataFile, buf, xdata, offsets[0], widths[0]+2,
                                   offsets[1], widths[1]+2, offsets[2], widths[2]+2);

                SNPRINTF(buf,sizeof(buf),"V_s_%03ld",index);
                getBlockOfFloats3D(dataFile, buf, ydata, offsets[0], widths[0]+2,
                                   offsets[1], widths[1]+2, offsets[2], widths[2]+2);

                SNPRINTF(buf,sizeof(buf),"W_s_%03ld",index);
                getBlockOfFloats3D(dataFile, buf, zdata, offsets[0], widths[0]+2,
                                   offsets[1], widths[1]+2, offsets[2], widths[2]+2);
            }
            else
            {
                EXCEPTION1(InvalidVariableException, varname);
            }
        }
    }
    else
    {
        EXCEPTION1(InvalidVariableException, varname);
    }

    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(totZones);
    float *data = (float *)arr->GetVoidPointer(0);

    float *runner = data;

    if (coordCode==0)
    {
        // Cartesian
        for (int i = 0; i < totZones; i++)
        {
            *runner++ = xdata[i];
            *runner++ = ydata[i];
            *runner++ = zdata[i];
        }
    }
    else if (coordCode==1)
    {
        if (kSz==1)
        {
            // 2D case
            long arrayOff = 0;
            double theta = 0.0; // cell center
            for (int j=0; j<widths[1]+2; j++)
            {
                for (int i=0; i<widths[0]+2; i++)
                {
                    float vx = (xdata[arrayOff]*cos(theta))-(zdata[arrayOff]*sin(theta));
                    float vy = ydata[arrayOff];
                    float vz = (xdata[arrayOff]*sin(theta))+(zdata[arrayOff]*cos(theta));
                    *runner++ = vx;
                    *runner++ = vy;
                    *runner++ = vz;
                    arrayOff += 1;
                }
            }
        }
        else
        {
            // 3D case
            long arrayOff = 0;
            for (int k=0; k<widths[2]+2; k++)
            {
                double thetaLow = larrays[2][k+offsets[2]];
                double thetaHigh = larrays[2][k+1+offsets[2]];
                double theta = 0.5*(thetaLow+thetaHigh); // cell center
                for (int j=0; j<widths[1]+2; j++)
                {
                    for (int i=0; i<widths[0]+2; i++)
                    {
                        float vx = (xdata[arrayOff]*cos(theta))-(zdata[arrayOff]*sin(theta));
                        float vy = ydata[arrayOff];
                        float vz = (xdata[arrayOff]*sin(theta))+(zdata[arrayOff]*cos(theta));
                        *runner++ = vx;
                        *runner++ = vy;
                        *runner++ = vz;
                        arrayOff += 1;
                    }
                }
            }
        }
    }
    else
    {
        EXCEPTION2(UnexpectedValueException, "Should be 0 or 1", "coordCode");
    }

    return arr;
}

//void avtMFIXCDFFileFormat::getBlockOfDoubles1D(NcVar* in, vtkDoubleArray *v,
//                        long offset, int n)
void avtMFIXCDFFileFormat::getBlockOfDoubles1D(NcFile* file,
        const char* varname,
        double* data,
        long offset, int n)
{
    NcVar* inVar= file->get_var(varname);
    if (!inVar)
    {
        debug5 << "Data file nas no variable " << varname << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

    if (!inVar->set_cur(offset))
    {
        debug5 << "set_cur failed with offset " << offset << endl;
        EXCEPTION2(UnexpectedValueException,"set_cur failed",(int)offset);
    }
    if (!inVar->get(data,n))
    {
        debug5 << "get failed with offset,n values " << offset
               << " " << n << endl;
        EXCEPTION2(UnexpectedValueException,"get failed",(int)offset);
    }
//   debug5 << "read " << n << " doubles at offset " << offset << " from "
//      << inVar->name() << endl;
}

void avtMFIXCDFFileFormat::getBlockOfFloats3D(NcFile* file,
        const char* varname,
        float *data,
        long iOffset, int iN,
        long jOffset, int jN,
        long kOffset, int kN)
{
    NcVar* inVar= file->get_var(varname);
    if (!inVar)
    {
        debug5 << "Data file nas no variable " << varname << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

    debug5 << "Setting cur for " << inVar->name() << " to " << kOffset
           << " " << jOffset << " " << iOffset << endl;
    if (!(inVar->set_cur(0,kOffset,jOffset,iOffset)))
    {
        debug5 << "Failure doing set_cur of " << inVar->name() << " to "
               << kOffset << " " << jOffset << " " << iOffset << endl;
        ostringstream oss;
        oss << kOffset << " " << jOffset << " " << iOffset;
        EXCEPTION2(UnexpectedValueException,"set_cur failed",oss.str());
    }
    debug5 << "Getting " << kN << " " << jN << " " << iN << endl;
    if (!(inVar->get(data, 1, kN, jN, iN)))
    {
        debug5 << "Failure doing get from " << inVar->name() << " of "
               << kN << " " << jN << " " << iN << endl;
        ostringstream oss;
        oss << kN << " " << jN << " " << iN;
        EXCEPTION2(UnexpectedValueException,"get failed",oss.str());
    }
    debug5 << "Get complete: " << data[0] << " " << data[1] << " "
           << data[2] << " ..." << endl;
}

void avtMFIXCDFFileFormat::getBlockOfInts3D(NcFile* file,
        const char* varname,
        int *data,
        long iOffset, int iN,
        long jOffset, int jN,
        long kOffset, int kN)
{
    NcVar* inVar= file->get_var(varname);
    if (!inVar)
    {
        debug5 << "Data file nas no variable " << varname << endl;
        EXCEPTION1(InvalidVariableException, varname);
    }

//   debug5 << "Setting cur to " << kOffset << " " << jOffset << " "
//      << iOffset << endl;
    if (!(inVar->set_cur(0,kOffset,jOffset,iOffset)))
    {
        debug5 << "Failure doing set_cur of " << inVar->name() << " to "
               << kOffset << " " << jOffset << " " << iOffset << endl;
        ostringstream oss;
        oss << kOffset << " " << jOffset << " " << iOffset;
        EXCEPTION2(UnexpectedValueException,"set_cur failed",oss.str());
    }
//   debug5 << "Getting " << kN << " " << jN << " " << iN << endl;
    if (!(inVar->get(data, 1, kN, jN, iN)))
    {
        debug5 << "Failure doing get from " << inVar->name() << " of "
               << kN << " " << jN << " " << iN << endl;
        ostringstream oss;
        oss << kN << " " << jN << " " << iN;
        EXCEPTION2(UnexpectedValueException,"get failed",oss.str());
    }
//   debug5 << "Get complete" << endl;
}

