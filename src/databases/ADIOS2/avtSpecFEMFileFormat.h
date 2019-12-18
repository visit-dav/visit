// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_SpecFEM_FILE_FORMAT_H
#define AVT_SpecFEM_FILE_FORMAT_H

#include <avtMTMDFileFormat.h>
#include <vector>
#include <string>
#include <map>
#include <adios2.h>

class avtFileFormatInterface;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtSpecFEMFileFormat
//
//  Purpose:
//      Reads in SpecFEM-ADIOS files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
// ****************************************************************************

class avtSpecFEMFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);
    static bool        GenerateFileNames(const std::string &nm,
                                         std::string &meshNm, std::string &dataNm);
    static bool        IsMeshFile(const std::string &fname);
    static bool        IsDataFile(const std::string &fname);

    avtSpecFEMFileFormat(const char *);
    virtual  ~avtSpecFEMFileFormat();

    virtual void        GetCycles(std::vector<int> &);
    virtual void        GetTimes(std::vector<double> &);
    virtual int         GetNTimesteps(void);

    virtual const char    *GetType(void)   { return "ADIOS-SpecFEM"; };
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

  protected:
    std::shared_ptr<adios2::ADIOS> adiosMesh, adiosData;
    adios2::IO meshIO, dataIO;
    adios2::Engine meshReader, dataReader;

    int ngllx, nglly, ngllz, numBlocks;

    std::vector<int> regions;
    bool             initialized;

    void             Initialize();
    virtual void     PopulateDatabaseMetaData(avtDatabaseMetaData *, int);
    vtkDataSet *     GetWholeMesh(int ts, int dom, bool xyzMesh);
    vtkDataSet *     GetRegionMesh(int ts, int dom, int region, bool xyzMesh);
    void             AddRegionMesh(int ts, int dom, int region, vtkDataSet *ds,
                                   bool xyzMesh, int ptOffset=0);
    vtkDataSet *     GetHotSpotsMesh(bool xyzMesh);
    vtkDataSet *     GetVolcanoMesh(bool xyzMesh);
    vtkDataSet *     GetContinents(bool xyzMesh);
    vtkDataSet *     GetPlates(bool xyzMesh);
    vtkDataSet *     LatLonClip(vtkDataSet *ds);

    vtkDataArray *   GetVarRegion(std::string &nm, int ts, int dom);
    vtkDataArray *   GetVectorVarRegion(std::string &nm, int ts, int dom);

    std::vector<std::string> variables;
    std::vector<std::pair<std::string, int> > domainVarPaths;
    bool kernelFile;

    int GetRegion(const std::string &str);
    std::string GetVariable(const std::string &str);
    static int NUM_REGIONS;
};
#endif
