// ************************************************************************* //
//                         avtBoxlib2DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_BOXLIB_2D_FILE_FORMAT_H
#define AVT_BOXLIB_2D_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>

#include <vector>
#include <string>
#include <fstream.h>

class VisMF;

// ****************************************************************************
//  Class: avtBoxlib2DFileFormat
//
//  Purpose:
//      A file format reader for multi-timestep, 2D AMR Boxlib files.
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
// ****************************************************************************

class avtBoxlib2DFileFormat : public avtMTMDFileFormat
{
  public:
                          avtBoxlib2DFileFormat(const char *);
    virtual              ~avtBoxlib2DFileFormat();
    
    virtual const char   *GetType(void) { return "Boxlib2D File Format"; };
    
    virtual void          GetCycles(std::vector<int> &);
    virtual int           GetNTimesteps(void);
 
    virtual vtkDataSet   *GetMesh(int, int, const char *);
    virtual vtkDataArray *GetVar(int, int, const char *);
    virtual vtkDataArray *GetVectorVar(int, int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void          FreeUpResources(void);

  protected:
    // Patches are indexed by timestep, level, patch.
    std::vector<std::vector<std::vector<vtkDataSet *> > >    patches;

    // This relative location of the multifab files.
    // There is one for all timesteps, and it contains entries for
    // all levels. There is no predefined length.
    std::vector<std::string>                multifabFilenames;

    // For each level and variable, which multifab flle the variable
    // is stored in, and at what index (component in the file it is stored as.
    // Indexed by level, variable.
    std::vector<std::vector<int> >          fabfileIndex;
    std::vector<std::vector<int> >          componentIds;

    // Flags for whether or not a header has been read for a given timestep.
    std::vector<bool>                       readTimeHeader;

    std::string                             rootPath;

    int                                     nLevels;
    std::vector<int>                        patchesPerLevel;

    int                                     nTimesteps;
    std::vector<int>                        cycles;
    std::vector<std::string>                timestepPaths;
    
    // Vars do not include vectors.
    int                                     nVars;
    std::vector<std::string>                varNames;
    std::vector<int>                        varCentering;

    // Vectors are combinations of scalar vars. The index to those
    // variables are stored in vectorComponents.
    int                                     nVectors;
    std::vector<std::string>                vectorNames;
    std::vector<std::vector<int> >          vectorComponents;
    std::vector<int>                        vectorCentering;

    // Variable readers.
    // Indexed by timestep, multifabFile
    std::vector<std::vector<VisMF *> >      mfReaders;

    VisMF* GetVisMF(int ts, int index);
    
    void ReadTimeHeader(int ts, bool populate);

    vtkDataSet *CreateGrid(double lo[2], double hi[2], double delta[2]);

    const static int                        dimension = 2;
};


#endif

