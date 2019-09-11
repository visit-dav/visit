// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef STREAKER_H
#define STREAKER_H
#include <map>
#include <vectortypes.h>
#include <void_ref_ptr.h>

class avtDatabaseMetaData;
class PDBFileObject;
class vtkDataSet;
class vtkDataArray;
class vtkFloatArray;

// ****************************************************************************
// Class: Streaker
//
// Purpose:
//   This class provides support for Streak plots coming from PDB files. A
//   file is read that contains information needed to construct streak datasets.
//   If all of the needed information is available, this class provides methods
//   to add streak variables to metadata and to construct and return the 
//   streak datasets using data from a vector of PDB files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Nov  7 10:42:02 PST 2008
//
// Modifications:
//   Brad Whitlock, Tue Dec  2 15:53:13 PST 2008
//   Added GetAuxiliaryData so we can return materials.
//
//   Brad Whitlock, Wed Feb 25 16:07:32 PST 2009
//   I added x_scale and x_translate to the StreakInfo. I also added support
//   for different kinds of log scaling.
//
//   Brad Whitlock, Tue May  5 16:22:56 PDT 2009
//   I added support for setting the streak plot centering.
//
//   Brad Whitlock, Wed May 13 13:21:57 PDT 2009
//   I added z_scale, z_translate.
//
// ****************************************************************************

class Streaker
{
public:
    Streaker();
    ~Streaker();

    typedef std::vector<PDBFileObject *> PDBFileObjectVector;

    void ReadStreakFile(const std::string &, PDBFileObject *pdb);
    void PopulateDatabaseMetaData(avtDatabaseMetaData *md);

    vtkDataSet   *GetMesh(const std::string &mesh, const PDBFileObjectVector &pdb);
    vtkDataArray *GetVar(const std::string &mesh, const PDBFileObjectVector &pdb);
    void         *GetAuxiliaryData(const std::string &var, 
                                   const char *type, void *args, DestructorFunction &df, 
                                   const PDBFileObjectVector &pdb);
    void FreeUpResources();
private:
    typedef enum {LOGTYPE_NONE, LOGTYPE_LOG, LOGTYPE_LOG10} logtype;

    struct StreakInfo
    {
        StreakInfo();

        std::string xvar;
        std::string yvar;
        std::string zvar;

        bool        hasMaterial;
        bool        cellCentered;
        bool        matchSilo;

        int         slice;
        int         sliceIndex;
        int         hsize;
        bool        integrate;
        logtype     log;
        float       x_scale;
        float       x_translate;
        float       y_scale;
        float       y_translate;
        float       z_scale;
        float       z_translate;

        vtkDataSet *dataset;
    };

    void AddStreak(const std::string &varname, StreakInfo &s, PDBFileObject *pdb);
    bool FindMaterial(PDBFileObject *pdb, int *zDimensions, int zDims);

#ifndef MDSERVER
    vtkDataSet *ConstructDataset(const std::string &, const StreakInfo &,
                                 const PDBFileObjectVector &pdb);
    vtkFloatArray *AssembleData(const std::string &var, int *sdims, int slice, 
                                int sliceIndex, const PDBFileObjectVector &pdb) const;
#endif

    std::map<std::string, StreakInfo> streaks;
    std::string                       matvar;
    intVector                         matnos;
    stringVector                      matNames;
    std::map<std::string,std::string> matToStreak;
};

#endif
