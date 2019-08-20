// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtBoxlibFileFormat.h                             //
// ************************************************************************* //

#ifndef AVT_BOXLIB_FILE_FORMAT_H
#define AVT_BOXLIB_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>

#include <vector>
#include <string>

// Map AVTBOXLIBFILEFORMAT to avtBoxlib2DFileFormat or avtBoxlib3DFileFormat
// based on BL_SPACEDIM
#if BL_SPACEDIM==3
#define AVTBOXLIBFILEFORMAT avtBoxlib3DFileFormat
#else
#define AVTBOXLIBFILEFORMAT avtBoxlib2DFileFormat
#endif

#ifdef __APPLE__
// For MacOS, the BoxLib VisMF class is mapped to VisMF2D or VisMF3D based on
// BL_SPACEDIM. Use typedef to select the appropriate version as VisMF.
#if BL_SPACEDIM==3
class VisMF3D;
typedef VisMF3D VisMF;
#else
class VisMF2D;
typedef VisMF2D VisMF;
#endif
#else
// Forward decleration of VisMF for non-MacOS BoxLib builds.
class VisMF;
#endif

// ****************************************************************************
//  Class: avtBoxlibFileFormat
//
//  Purpose:
//      A file format reader for single-timestep, 3D AMR Boxlib files.
//
//  Programmer:  Akira Haddox
//  Creation:    July 25, 2003
//
//  Modifications:
//
//    Hank Childs, Sat Nov  8 18:38:08 PST 2003
//    Overhauled class.  Made a STMD (from MTMD) to clean up class and also
//    to overcome current (overcome-able) infrastructure limitations with
//    SILs changing in MTMD.  Also added true AMR capabilities.
//
//    Hank Childs, Sun Mar  6 16:21:15 PST 2005
//    Add support for GeoDyne material names.
//
//    Hank Childs, Thu Jun 23 14:46:22 PDT 2005
//    Broadcast Header from proc. 0
//
//    Mark C. Miller, Wed Nov  9 12:35:15 PST 2005
//    Added GetCycleFromFilename
//
//    Kathleen Bonnell, Thu Sep 25 09:09:54 PDT 2008 
//    Removed GetType, as it is redefined in derived classes
//    avtBoxlib2DFileFormat and avtBoxlib3DFileFormat.
//
//    Hank Childs, Wed Oct  8 16:56:47 PDT 2008
//    Add data member for coordinate system.
//
//    David Camp, Tue Feb  1 09:47:31 PST 2011
//    Added the GetTime() function for the pathline code.
//
//    Gunther H. Weber, Thu Dec 15 19:08:35 PST 2011
//    Added initialization for BoxLib (including static memeber
//    boxLibInitialized) since the new version of BoxLib no longer works if
//    not initialized.
//    Use a single class for 2D and 3D and map this to 2D and 3D file formats.
//    This change avoids duplicate symbols in the 2D and 3D version of the
//    plugin which can confuse MacOS. On MacOS use "#defines" to ensure that
//    all classes in BoxLib that depend on dimension have different names for
//    2D and 3D, thus avoiding symbol clashes when linking. These were
//    required to make the BoxLib file reader work on MacOS.
//
// ****************************************************************************

class AVTBOXLIBFILEFORMAT : public avtSTMDFileFormat
{
  public:
                          AVTBOXLIBFILEFORMAT(const char *);
    virtual              ~AVTBOXLIBFILEFORMAT();

    virtual bool          HasInvariantSIL(void) const { return false; };
    virtual bool          HasInvariantMetaData(void) const { return false; };

    virtual int           GetCycle(void) { return cycle; };
    virtual int           GetCycleFromFilename(const char *f) const;

    virtual double        GetTime(void);
 
    virtual vtkDataSet   *GetMesh(int, const char *);
    virtual vtkDataArray *GetVar(int, const char *);
    virtual vtkDataArray *GetVectorVar(int, const char *);

    virtual void          PopulateDatabaseMetaData(avtDatabaseMetaData *);

    virtual void         *GetAuxiliaryData(const char *var, int,
                                           const char *type, void *args,
                                           DestructorFunction &);

    virtual void          FreeUpResources(void);
    virtual void          ActivateTimestep(void);

#if BL_SPACEDIM==3
    virtual const char   *GetType(void) { return "Boxlib3D File Format"; };
#else
    virtual const char   *GetType(void) { return "Boxlib2D File Format"; };
#endif

  protected:
    // This relative location of the multifab files.  It contains entries for
    // all levels. There is no predefined length.
    std::vector<std::string>                multifabFilenames;

    // For each level and variable, which multifab file the variable
    // is stored in, and at what index (component in the file it is stored as.)
    // Indexed by level, variable.
    std::vector<std::vector<int> >          fabfileIndex;
    std::vector<std::vector<int> >          componentIds;

    std::string                             rootPath;

    int                                     nLevels;
    int                                     coordSys;
    std::vector<int>                        patchesPerLevel;
    // These entries are per patch.
    std::vector<double>                     xMin;
    std::vector<double>                     xMax;
    std::vector<double>                     yMin;
    std::vector<double>                     yMax;
#if BL_SPACEDIM==3
    std::vector<double>                     zMin;
    std::vector<double>                     zMax;
#endif
    // These entries are per level.
    std::vector<double>                     deltaX;
    std::vector<double>                     deltaY;
#if BL_SPACEDIM==3
    std::vector<double>                     deltaZ;
#endif
    // This entry is per level, but level 0 is omitted.
    std::vector<int>                        refinement_ratio;

    bool                                    haveReadTimeAndCycle;
    double                                  time;
    int                                     cycle;
    std::string                             timestepPath;
    bool                                    initializedReader;
    enum { none, vf, frac, vfrac }          varnames_for_materials;

    // Scalar vars listed in header.
    int                                     nVars;
    std::vector<std::string>                varNames;
    std::vector<int>                        varCentering;
    std::vector<bool>                       varUsedElsewhere;

    // Vectors are combinations of scalar vars. The index to those
    // variables are stored in vectorComponents.
    int                                     nVectors;
    std::vector<std::string>                vectorNames;
    std::vector<std::vector<int> >          vectorComponents;
    std::vector<int>                        vectorCentering;

    // Variable readers.
    // Indexed by multifabFile
    std::vector<VisMF *>                    mfReaders;

    // The VisMF class is buggy when it comes to freeing up
    // memory.  The "clearlist" is used to sidestep those issues.
    std::vector<int>                        clearlist;

    // Problem range
    double                                  probLo[BL_SPACEDIM];
    double                                  probHi[BL_SPACEDIM];

    int                                     nMaterials;

    static bool                             boxLibInitialized;
    static const int                        dimension = BL_SPACEDIM;

    void                                   *GetMaterial(const char *, int,
                                                        const char *, 
                                                        DestructorFunction &);
    void                                   *GetSpatialIntervalTree(
                                                        DestructorFunction &);
    VisMF*                                  GetVisMF(int index);
    void                                    ReadHeader(void);
    void                                    InitializeReader(void);
    void                                    CalculateDomainNesting(void);

    vtkDataSet *CreateGrid(double lo[BL_SPACEDIM],
                           double hi[BL_SPACEDIM],
                           double delta[BL_SPACEDIM]) const;

    int                 GetGlobalPatchNumber(int level, int patch) const;
    void                GetLevelAndLocalPatchNumber(int global_patch,
                                  int &level, int &local_patch) const;
    void                GetDimensions(int *, int level, int patch) const;
    void                GetDimensions(int *, double*, double*, double*) const;
};

#endif
