// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef AVT_OpenFOAM_METADATA_HELPER_H
#define AVT_OpenFOAM_METADATA_HELPER_H

#include <string>
#include <vector>

// ****************************************************************************
//  Class: avtOpenFOAMMetaDataHelper
//
//  Purpose:
//    Reads in OpenFOAM metadata in a manner useable with VisIt.
//
//  Programmer: CODEX
//  Creation:   Aug 5, 2026
//
//  Modifications:
//
// ****************************************************************************

class avtOpenFOAMMetaDataHelper
{
  public:
    struct TimeInfo
    {
        double      value;
        std::string name;
    };

    struct FieldInfo
    {
        std::string name;
        std::string className;
    };

    struct MetaData
    {
        std::vector<std::string> patchNames;
        std::vector<std::string> lagrangianPatches;
        std::vector<std::string> pointZoneNames;
        std::vector<std::string> faceZoneNames;
        std::vector<std::string> cellZoneNames;
        std::vector<FieldInfo>   cellFields;
        std::vector<FieldInfo>   pointFields;
        std::vector<FieldInfo>   lagrangianFields;
    };

    struct HeaderInfo
    {
        std::string className;
        std::string objectName;
    };

    avtOpenFOAMMetaDataHelper(const std::string &fileName, int caseType);

    bool                        IsValid() const;
    const std::string          &GetError() const;
    const std::vector<TimeInfo>&GetTimes() const;
    bool                        ReadMetaData(int timeState,
                                             bool readZones,
                                             MetaData &metaData) const;
  private:

    bool                Initialize();
    bool                ResolveControlDictAndCasePath();
    bool                SelectMetadataRootPath();
    bool                DiscoverRegions();
    bool                DiscoverTimes();
    std::string         RegionPath(const std::string &regionName) const;
    std::string         RegionPrefix(const std::string &regionName) const;
    std::string         TimeRegionPath(const std::string &timeName,
                                       const std::string &regionName) const;
    std::string         ConstantRegionPath(const std::string &regionName) const;
    bool                HasMeshFiles(const std::string &path) const;
    std::string         ResolveMeshInstance(int timeState,
                                            const std::string &regionName) const;
    bool                ReadEntryNames(const std::string &path,
                                       std::vector<std::string> &names) const;
    bool                ReadHeader(const std::string &path,
                                   HeaderInfo &header,
                                   bool tryGzipSuffix) const;
    void                GatherBoundaryMetaData(int timeState,
                                               MetaData &metaData) const;
    void                GatherZoneMetaData(int timeState,
                                           MetaData &metaData) const;
    void                GatherFieldMetaData(int timeState,
                                            MetaData &metaData) const;
    void                GatherFieldMetaDataForTime(const std::string &timeName,
                                                   MetaData &metaData) const;
    void                GatherLagrangianMetaDataForTime(const std::string &timeName,
                                                        MetaData &metaData) const;
    bool                GatherLagrangianMetaDataForRegion(
                            const std::string &timeName,
                            const std::string &regionName,
                            MetaData &metaData) const;
    void                ReadEulerianFieldDirectory(
                            const std::string &path,
                            std::vector<FieldInfo> &cellFields,
                            std::vector<FieldInfo> &pointFields) const;
    void                ReadLagrangianFieldDirectory(
                            const std::string &path,
                            std::vector<FieldInfo> &lagrangianFields) const;

    std::string               inputFileName;
    std::string               controlDictPath;
    std::string               casePath;
    std::string               metadataRootPath;
    std::vector<std::string>  regionNames;
    std::vector<TimeInfo>     times;
    std::string               error;
    int                       caseType;
};

#endif
