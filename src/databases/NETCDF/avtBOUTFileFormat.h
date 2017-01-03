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

#ifndef AVT_BOUT_FILE_FORMAT_H
#define AVT_BOUT_FILE_FORMAT_H
#include <avtMTMDFileFormat.h>
#include <vectortypes.h>

class NETCDFFileObject;
class avtFileFormatInterface;

// ****************************************************************************
// Class: avtBOUTFileFormat
//
// Purpose:
//   Reads BOUT data from a NETCDF file.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Thu Aug  1 16:16:10 PDT 2013
//
// Modifications:
//   Eric Brugger, Mon Dec  2 15:43:26 PST 2013
//   I added the ability to handle circular grids.
//   
//   Eric Brugger, Tue Dec  3 10:20:02 PST 2013
//   I added the ability to handle grids with two X points.
//
//   Eric Brugger, Fri Apr 11 10:32:26 PDT 2014
//   I modified the display of the diverter to display all of the lower
//   diverter and also display the upper diverter with a two X point grid.
//
// ****************************************************************************

#define MAX_SUB_MESHES 7

#define N_DIVERTER_ONEX_SUB_MESHES 4
#define N_DIVERTER_TWOX_SUB_MESHES 6

enum GridType {circularGrid, oneXGrid, twoXGrid};

struct Subgrid
{
    int                nb;
    int                istart, iend;
    int                jstart[2], jend[2];
    int                nxIn, nyIn;
    int                nxOut, nyOut;
    int               *jindex;
    int               *ijindex;
    int               *inrep;
    int               *jnrep;
};

class avtBOUTFileFormat : public avtMTMDFileFormat
{
public:
    static bool        Identify(NETCDFFileObject *);
    static avtFileFormatInterface *CreateInterface(NETCDFFileObject *f,
                       const char *const *list, int nList, int nBlock);

                       avtBOUTFileFormat(const char *filename,
                                         NETCDFFileObject *);
                       avtBOUTFileFormat(const char *filename);
    virtual           ~avtBOUTFileFormat();

    virtual const char    *GetType(void) { return "BOUT"; }
    virtual void           ActivateTimestep(int ts);
    virtual void           FreeUpResources(void);

    virtual int            GetNTimesteps(void);
    virtual void           GetTimes(std::vector<double> &);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);

  private:
    bool                   ReadTimes();
    void                   DetermineMeshReplication(Subgrid &);
    void                   ReadMeshMetaData();
    bool                   ReadMesh();
    void                   CreateDiverterMesh(Subgrid &, int, float *, float *);
    void                   CreateMesh(Subgrid &, int, int, float *, float *);
    void                   CreateDiverterVar(Subgrid &, int, float *, float *);
    void                   CreateVar(Subgrid &, int, int, float *, float *);

    char                  *filePath;

    NETCDFFileObject      *fileObject;
    NETCDFFileObject      *meshFile;

    doubleVector           times;
    bool                   timesRead;

    int                    ixseps1, ixseps2;
    int                    jyseps1_1, jyseps1_2;
    int                    jyseps2_1, jyseps2_2;
    GridType               gridType;
    int                    nSubMeshes;
    Subgrid                subgrid[MAX_SUB_MESHES];

    float                 *Rxy, *Zxy, *zShift, *zShiftZero;
    int                    nx2d, ny2d;
    int                    zperiod;
    bool                   meshRead;

    int                    nxRaw, nyRaw, nz, nzOut;

    int                    cacheTime;
    std::string            cacheVar;
    std::string            cacheVarRaw;
    vtkDataArray          *cacheData[MAX_SUB_MESHES];
    float                 *cacheDataRaw;
};

#endif
