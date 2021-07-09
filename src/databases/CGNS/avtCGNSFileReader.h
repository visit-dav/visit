// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtCGNSFileReader.h                            //
// ************************************************************************* //

#ifndef AVT_CGNS_FILE_READER_H
#define AVT_CGNS_FILE_READER_H

#include <cgnslib.h>
#if CGNS_VERSION <= 3000
#define cgsize_t int
#else
#include <cgnstypes.h>
#endif
#include <ostream>
#include <vector>
#include <vectortypes.h>
#include <map>

class vtkDataArray;
class vtkDataSet;
class vtkUnstructuredGrid;
class avtDatabaseMetaData;

using namespace std;

// ****************************************************************************
//  Class: avtCGNSFileReader
//
//  Purpose:
//      Reads in CGNS files as a plugin to VisIt.
//
//  Programmer: Brad Whitlock
//  Creation:   Tue Aug 30 16:08:44 PST 2005
//
//  Modifications:
//    Brad Whitlock, Tue Apr 15 10:20:10 PDT 2008
//    Added methods that help us inspect the file contents.
//
//    Brad Whitlock, Thu Oct 13 11:14:18 PDT 2011
//    I added InitializeMaps so we can call it from GetMesh, GetVar to ensure
//    the file grouping works.
//
//    Eric Brugger, Fri Feb 28 12:41:40 PST 2020
//    Pulled out all the CGNS specific code from avtCGNSFileFormat into
//    this class.
//
//    Eric Brugger, Thu Jul  2 10:56:36 PDT 2020
//    Corrected a bug that caused a crash when doing a Subset plot of "zones"
//    when reading data decomposed across multiple CGNS files.
//
//    Alister Maguire, Tue Mar  2 08:01:12 PST 2021
//    Added ReadMixedAndNamedElementSections, ReadNGonSections,
//    ReadNGonAndNFaceSections.
//
//    Eric Brugger, Tue Jul  6 10:27:03 PDT 2021
//    Added support for reading rind data.
//
// ****************************************************************************

class avtCGNSFileReader
{
public:
                           avtCGNSFileReader(const char *, bool);
                          ~avtCGNSFileReader();

    virtual void           GetCycles(std::vector<int> &);
    virtual void           GetTimes(std::vector<double> &);

    virtual int            GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "CGNS"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

protected:
    struct VarInfo
    {
        intVector   zoneList; // The zones that contain the variable
        int         cellCentering;
        int         nodeCentering;
        int         badCentering;
        bool        hasUnits;
        std::string units;
    };
    typedef std::map<std::string, VarInfo> StringVarInfoMap;

    struct BaseInformation
    {
        std::string      name;
        int              cellDim;
        int              physicalDim;
        int              meshType;
        stringVector     zoneNames;
        StringVarInfoMap vars;
    };
    typedef std::vector<BaseInformation> BaseInformationVector;

    struct BaseAndZoneList
    {
        int       base;
        intVector zones;
    };

    int                    GetFileHandle();
    void                   ReadTimes();
    bool                   GetCoords(int timestate, int base, int zone,
                               const cgsize_t *zsize, int cell_dim,
                               int phys_dim, bool structured, float **coords);
    void                   AddReferenceStateExpressions(avtDatabaseMetaData *md,
                               int base, int nBases,
                               const std::string &baseName,
                               const std::string &meshName);
    void                   AddVectorExpressions(avtDatabaseMetaData *md,
                               bool *haveVelocity, bool *haveMomentum,
                               int nBases, const std::string &baseName);
    void                   AddVectorExpression(avtDatabaseMetaData *md,
                               bool *haveComponent, int nBases,
                               const std::string &baseName,
                               const std::string &vecName);
    bool                   GetVariablesForBase(int base, BaseInformation &baseInfo);
    bool                   BaseContainsUnits(int base);
    void                   InitializeMaps(int timeState);

    void                   GetQuadGhostZones(int base, int zone,
                               const cgsize_t *zsize, int cell_dim,
                               vtkDataSet *ds);
    vtkDataSet *           GetCurvilinearMesh(int, int, int, const char *,
                                              const cgsize_t *, int, int);
    vtkDataSet *           GetUnstructuredMesh(int, int, int, const char *,
                                               const cgsize_t *, int, int);
    void                   ReadMixedAndNamedElementSections(
                               vtkUnstructuredGrid *,
                               const char *, std::vector<int> &,
                               int, int, int, int);
    void                   ReadNGonSections(
                               vtkUnstructuredGrid *,
                               const char *, std::vector<int> &,
                               int, int, int, int);
    void                   ReadNGonAndNFaceSections(
                               vtkUnstructuredGrid *,
                               const char *,
                               std::vector<int> &,
                               std::vector<int> &,
                               int, int, int, int);

    void PrintVarInfo(ostream &out, const VarInfo &var, const char *indent);
    void PrintStringVarInfoMap(ostream &out, const StringVarInfoMap &vars, const char *indent);
    void PrintBaseInformation(ostream &out, const BaseInformation &baseInfo);

    char                                  *cgnsFileName;
    int                                    fn;
    bool                                   timesRead;
    bool                                   cgnsCyclesAccurate;
    bool                                   cgnsTimesAccurate;
    doubleVector                           times;
    intVector                              cycles;
    std::map<std::string, BaseAndZoneList> MeshDomainMapping;
    std::map<std::string, int>             BaseNameToIndices;
    std::map<std::string, std::string>     VisItNameToCGNSName;
    bool                                   initializedMaps;
    bool                                   cgnsIsMTMD;
};

#endif

