// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
//   Eric Brugger, Tue Aug 14 11:29:38 PDT 2018
//   I modified the reader to create a single regular block in the rotational
//   direction, interpolating the field values onto it based on zshift instead
//   of distorting the zones by zshift and using unmodified field values.
//   This reduces the size of the mesh and also eliminates the rendering
//   artifacts caused by the large aspect zones created by distorting the
//   zones.
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
    void                   CreateDiverterMesh(Subgrid &, int, float *);
    void                   CreateMesh(Subgrid &, int, int, float *);
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

    float                 *Rxy, *Zxy, *zShift;
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
