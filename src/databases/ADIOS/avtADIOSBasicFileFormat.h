// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_ADIOS_BASIC_FILE_FORMAT_H
#define AVT_ADIOS_BASIC_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>

class ADIOSFileObject;
class avtFileFormatInterface;
class vtkRectilinearGrid;

// ****************************************************************************
//  Class: avtADIOSBasicFileFormat
//
//  Purpose:
//      Reads in ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Thu Sep 17 11:23:05 EDT 2009
//
//  Modifications:
//
//   Dave Pugmire, Tue Mar  9 12:40:15 EST 2010
//   Use uint64_t for start/count arrays.
//
// ****************************************************************************

class avtADIOSBasicFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    avtADIOSBasicFileFormat(const char *);
    virtual  ~avtADIOSBasicFileFormat();

    //
    // This is used to return unconvention data -- ranging from material
    // information to information about block connectivity.
    //
    // virtual void      *GetAuxiliaryData(const char *var, int timestep, 
    //                                     const char *type, void *args, 
    //                                     DestructorFunction &);
    //

    //
    // If you know the times and cycle numbers, overload this function.
    // Otherwise, VisIt will make up some reasonable ones for you.
    //
    virtual void        GetCycles(std::vector<int> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS"; };
    virtual void           FreeUpResources(void); 

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    ADIOSFileObject *fileObj;
    bool             initialized;
    std::map<std::string, std::string> complexVarMap;


    void                   Initialize();
    std::string            GenerateMeshName(const ADIOS_VARINFO *avi);

    class block
    {
    public:
        block() {start[0]=start[1]=start[2]=0; count[0]=count[1]=count[2]=0;}
        block(int n, ADIOS_VARBLOCK &b)
        {
            for (int i = 0; i < n; i++)
            {
                start[i] = b.start[i];
                count[i] = b.count[i];
            }
            for (int i = n; i < 3; i++)
                start[i] = count[i] = 0;
        }
        uint64_t start[3], count[3];
    };
    class meshInfo
    {
      public:
        meshInfo() {dim=0; dims[0]=dims[1]=dims[2]=0;}
        meshInfo(ADIOS_VARINFO *avi)
        {
            dim = avi->ndim;
            for (int i = 0; i < 3; i++)
                if (i < dim) dims[i] = avi->dims[i];
                else dims[i] = 0;
            
            for (int i = 0; i < avi->sum_nblocks; i++)
                blocks.push_back(block(dim, avi->blockinfo[i]));
        }
        ~meshInfo() {}

        int dim;
        uint64_t dims[3];
        std::vector<block> blocks;
    };

    std::map<std::string, meshInfo> meshes;

    vtkRectilinearGrid    *CreateUniformGrid(meshInfo &mi, int ts, int dom);
    
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);



    bool GetDomBounds(int nBlocks, int dim, uint64_t *dims, 
                      uint64_t *sIn, uint64_t *cIn,
                      uint64_t *sOut, uint64_t *cOut, bool *gFlags);
    void AddGhostZones(vtkRectilinearGrid *grid, bool *g);
};

#endif
