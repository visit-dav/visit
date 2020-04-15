// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtChomboFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_Chombo_FILE_FORMAT_H
#define AVT_Chombo_FILE_FORMAT_H

#include <avtSTMDFileFormat.h>
#include <Expression.h>

#include <vector>
#include <list>

// Define this symbol BEFORE including hdf5.h to indicate the HDF5 code
// in this file uses version 1.6 of the HDF5 API. This is harmless for
// versions of HDF5 before 1.8 and ensures correct compilation with
// version 1.8 and thereafter. When, and if, the HDF5 code in this file
// is explicitly upgraded to the 1.8 API, this symbol should be removed.
#define H5_USE_16_API
#include <hdf5.h>

struct {
  int i;
  int j;
} typedef intvect2d;

struct {
  int i;
  int j;
  int k;
} typedef intvect3d;

struct {
  int i;
  int j;
  int k;
  int l;
} typedef intvect4d;

struct {
  intvect2d lo;
  intvect2d hi;
} typedef box2d;

struct{
  intvect3d lo;
  intvect3d hi;
} typedef box3d;

struct{
  intvect4d lo;
  intvect4d hi;
} typedef box4d;

union
{
  box2d b2;
  box3d b3;
  box4d b4;
} typedef box;

struct {
  double x;
  double y;
} typedef doublevect2d;

struct {
  double x;
  double y;
  double z;
} typedef doublevect3d;

struct {
  double x;
  double y;
  double z;
  double u;
} typedef doublevect4d;

union
{
  doublevect2d dv2;
  doublevect3d dv3;
  doublevect3d dv4;
} typedef doublevect;

class DBOptionsAttributes;


// ****************************************************************************
//  Class: avtChomboFileFormat
//
//  Purpose:
//      Reads in Chombo files as a plugin to VisIt.
//
//  Programmer: Hank Childs
//  Creation:   Thu Jan 19 11:17:14 PDT 2006
//
//  Modifications:
//
//    Hank Childs, Mon Jun 19 16:44:06 PDT 2006
//    Add support for ghost zones.
//
//    Brad Whitlock, Mon Sep 25 13:54:59 PST 2006
//    I added some fixes for getting cycle,time and for time-varying metadata.
//
//    Gunther H. Weber, Tue Aug  7 15:56:32 PDT 2007
//    Added material support
//
//    Hank Childs, Mon Oct  8 17:17:24 PDT 2007
//    Added options for reading.
//
//    Gunther H. Weber, Mon Oct 22 11:22:35 PDT 2007
//    Added information about problem domain [low|hi]Prob[I|J|K] needed
//    to figure out whether a ghost zone is external to the problem.
//
//    Gunther H. Weber, Mon Mar 24 20:45:05 PDT 2008
//    Added support for node centered Chombo files.
//
//    Gunther H. Weber, Tue Apr 15 17:43:30 PDT 2008
//    Add support to automatically import a coordinate mapping file via conn_cmfe
//
//    Hank Childs, Sun Jan 25 15:38:50 PST 2009
//    Improve support for ghost data.
//
//    Gunther H. Weber, Wed Jun 10 18:25:34 PDT 2009
//    Added support for particle data in Chombo files
//
//    Gunther H. Weber, Tue Sep 15 11:25:21 PDT 2009
//    Added support for 3D mappings for 2D files
//
//    Gunther H. Weber, Thu Jun 17 10:10:17 PDT 2010
//    Added ability to connect particle mesh based on polymer_id and
//    particle_nid
//
//    Tom Fogal, Fri Aug  6 16:39:18 MDT 2010
//    Implement method to handle data selections.
//
//    Gunther H. Weber, Thu Aug 15 11:37:51 PDT 2013
//    Initial bare-bones support for 4D Chombo files (fairly limited and 
//    "hackish")
//
//    Gunther H. Weber, Tue Feb 10 21:06:28 PST 2015
//    Add support for vec_dx for 4D Chombo files
//
// ****************************************************************************

class avtChomboFileFormat : public avtSTMDFileFormat
{
  public:
                       avtChomboFileFormat(const char *, const DBOptionsAttributes*);
    virtual           ~avtChomboFileFormat();

    virtual const char    *GetType(void)   { return "Chombo"; };
    virtual void           FreeUpResources(void); 
    virtual void           ActivateTimestep(void);

    virtual vtkDataSet    *GetMesh(int, const char *);
    virtual vtkDataArray  *GetVar(int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, const char *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *);
    virtual void          *GetAuxiliaryData(const char *var, int,
                                            const char *type, void *args,
                                            DestructorFunction &);

    void                  *GetMaterial(const char *var, int patch, 
                                       const char *type,
                                       DestructorFunction &df);

    void                   RegisterDataSelections(
                             const std::vector<avtDataSelection_p>&,
                             std::vector<bool>* applied
                           );
  
  protected:
    bool                               initializedReader;
    int                                dimension;
    hid_t                              file_handle;
    std::vector<std::string>           varnames;
    int                                nMaterials;
    bool                               hasParticles;
    bool                               connectParticles;
    bool                               alwaysComputeDomainBoundaries;
    std::vector<std::string>           particleVarnames;
    double                             dtime;
    int                                cycle;
    int                                num_levels;
    bool                               nodeCentered;
    std::vector<int>                   numGhosts;
    std::vector<int>                   patchesPerLevel;
    std::vector< std::vector<int> >    refinement_ratio;
    std::vector< std::vector<double> > dx;
    std::list<Expression*>             expressions;
    bool                               allowedToUseGhosts;
    bool                               fileContainsGhosts;
    bool                               enableOnlyRootLevel;
    bool                               enableOnlyExplicitMaterials;
    bool                               checkForMappingFile;
    bool                               mappingFileExists;
    bool                               mappingIs3D;

    // Information to group boxes for 4D data sets to create array variable
    // from 4th dimension
    std::vector<int>                   listOfRepresentativeBoxes;
    std::vector<int>                   representativeBox;
    std::vector< std::vector<int> >    representedBoxes;

    std::vector<int>                   lowProbI;
    std::vector<int>                   hiProbI;
    std::vector<int>                   lowProbJ;
    std::vector<int>                   hiProbJ;
    std::vector<int>                   lowProbK;
    std::vector<int>                   hiProbK;
    std::vector<int>                   lowProbL;
    std::vector<int>                   hiProbL;

    std::vector<int>                   lowI;
    std::vector<int>                   hiI;
    std::vector<int>                   lowJ;
    std::vector<int>                   hiJ;
    std::vector<int>                   lowK;
    std::vector<int>                   hiK;
    std::vector<int>                   lowL;
    std::vector<int>                   hiL;

    double                             probLo[3];
    double                             aspectRatio[3];

    size_t                             resolution; // for user selection of resolution

    void                               InitializeReader(void);
    void                               GetLevelAndLocalPatchNumber(int global_patch,
                                          int &level, int &local_patch) const;
    void                               CalculateDomainNesting(void);

    virtual int                        GetCycle(void);
    virtual double                     GetTime(void);
    virtual int                        GetCycleFromFilename(const char *f) const;

    virtual bool                       HasInvariantMetaData(void) const { return false; };
    virtual bool                       HasInvariantSIL(void) const      { return false; };
};
#endif
