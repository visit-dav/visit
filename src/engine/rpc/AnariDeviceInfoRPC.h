// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_DEVICE_INFO_RPC_H
#define ANARI_DEVICE_INFO_RPC_H
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>

// ****************************************************************************
//  Class:  AnariDeviceInfoRPC
//
//  Purpose:
//    Implements an RPC that lets a client enumerate the ANARI libraries,
//    device subtypes, renderer subtypes, and renderer parameters that are
//    actually available on the engine, without the client needing to load
//    ANARI backend libraries itself. The engine is the only process
//    guaranteed to have ANARI backend libraries installed (e.g. it may run
//    on a remote HPC node with the GPU/backends, while the client/viewer
//    runs locally without them).
//
//    libraryName / librarySubtype / rendererSubtype may be passed empty to
//    request only the information that can be determined without them
//    (e.g. an empty librarySubtype returns the list of available device
//    subtypes for libraryName instead of parameter info).
//
//  Programmer:  Kevin Griffin
//  Creation:    Thu 27 Aug 2026
//
// ****************************************************************************

class ENGINE_RPC_API AnariDeviceInfoRPC : public BlockingRPC
{
public:
    struct ENGINE_RPC_API MapNodeString : public AttributeSubject
    {
        std::string s;
      public:
        MapNodeString();
        MapNodeString(std::string);
        virtual ~MapNodeString();
        virtual void SelectAll();
        virtual const std::string TypeName() const
           { return "AnariDeviceInfoRPC::MapNodeString"; }
    };

    AnariDeviceInfoRPC();
    virtual ~AnariDeviceInfoRPC();

    virtual const std::string TypeName() const { return "AnariDeviceInfoRPC"; }

    // Invocation method
    std::string operator()(const std::string &libraryName,
                           const std::string &librarySubtype,
                           const std::string &rendererSubtype);

    // Property selection methods
    virtual void SelectAll();

    // Property getting methods
    const std::string &GetLibraryName() const {return libraryName; };
    const std::string &GetLibrarySubtype() const {return librarySubtype; };
    const std::string &GetRendererSubtype() const {return rendererSubtype; };

private:
    std::string libraryName;
    std::string librarySubtype;
    std::string rendererSubtype;
    MapNodeString mapNodeString;
};

#endif
