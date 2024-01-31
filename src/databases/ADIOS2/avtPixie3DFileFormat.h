// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                          avtPixie3DFileFormat.h                           //
// ************************************************************************* //

#ifndef AVT_PIXIE3D_FILE_FORMAT_H
#define AVT_PIXIE3D_FILE_FORMAT_H

#include <avtMTMDFileFormatInterface.h>
#include <avtMTMDFileFormat.h>

#include <adios2.h>
#include <vector>
#include <map>

// ****************************************************************************
//  Class: avtPixie3DFileFormat
//
//  Purpose:
//      Reads in ADIOS2 files as a plugin to VisIt.
//
//  Programmer: Dave Pugmire
//  Creation:   Fri Apr  3 14:55:01 EDT 2020
//
//  Modifications:
//
// ****************************************************************************

class avtPixie3DFileFormat : public avtMTMDFileFormat
{
  public:
    static bool        Identify(const char *fname);
    static avtFileFormatInterface *CreateInterface(const char *const *list,
                                                   int nList,
                                                   int nBlock);

                       avtPixie3DFileFormat(const char *);

    // interface creator with first file already opened
    static avtFileFormatInterface *CreateInterfaceADIOS2(
            const char *const *list,
            int nList,
            int nBlock,
            std::shared_ptr<adios2::ADIOS> adios,
            adios2::Engine &reader,
            adios2::IO &io,
            std::map<std::string, adios2::Params> &variables,
            std::map<std::string, adios2::Params> &attributes
            );

    // constructor with already-opened stream
    avtPixie3DFileFormat(std::shared_ptr<adios2::ADIOS> adios,
            adios2::Engine &reader,
            adios2::IO &io,
            std::map<std::string, adios2::Params> &variables,
            std::map<std::string, adios2::Params> &attributes,
            const char *);

    virtual           ~avtPixie3DFileFormat() {}

    virtual int            GetNTimesteps() {Initialize(); return numTimeSteps;}
    virtual void           GetCycles(std::vector<int> &c) {Initialize(); c = cycles;}
    virtual void           GetTimes(std::vector<double> &t) {Initialize(); t = times;}
    virtual const char    *GetType(void)   {return "Pixie3D";}
    virtual void           FreeUpResources(void);

    virtual vtkDataSet    *GetMesh(int, int, const char *);
    virtual vtkDataArray  *GetVar(int, int, const char *);
    virtual vtkDataArray  *GetVectorVar(int, int, const char *);

    static bool IdentifyADIOS2(
                   std::map<std::string, adios2::Params> &variables,
                   std::map<std::string, adios2::Params> &attributes);

  protected:
    virtual void           PopulateDatabaseMetaData(avtDatabaseMetaData *, int);

    void Initialize();
    std::string GetMeshName(const std::string &varName) const;

    std::shared_ptr<adios2::ADIOS> adios;
    adios2::IO io;
    adios2::Engine reader;
    std::map<std::string, adios2::Params> variables, attributes;
    int numTimeSteps;
private:
    bool initialized;
    std::vector<int> cycles;
    std::vector<double> times;

    struct meshInfo
    {
        meshInfo() : meshType(AVT_UNKNOWN_MESH) {}

        avtMeshType meshType;
        std::vector<std::string> coordVars;

        //coords and dims for THIS timestep.
        std::vector<std::string> currCoords;
        std::vector<int> currDims;
    };

    std::map<std::string, meshInfo> meshes;
    std::map<std::string, std::string> varMesh;
};


#endif
