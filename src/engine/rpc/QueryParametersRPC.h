// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QUERY_PARAMETERS_RPC_H 
#define QUERY_PARAMETERS_RPC_H 
#include <engine_rpc_exports.h>
#include <VisItRPC.h>
#include <string>

// ****************************************************************************
//  Class:  QueryParametersRPC
//
//  Purpose:
//    Implements an RPC to get query parameters 
//
//  Programmer:  Kathleen Biagas 
//  Creation:    July 15, 2011
//
// ****************************************************************************

class ENGINE_RPC_API QueryParametersRPC : public BlockingRPC
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
           { return "QueryParametersRPC::MapNodeString"; }
    };

    QueryParametersRPC();
    virtual ~QueryParametersRPC();

    virtual const std::string TypeName() const { return "QueryParametersRPC"; }

    // Invocation method
    std::string operator()(const std::string &qName); 

    // Property selection methods
    virtual void SelectAll();

    // Property getting methods
    const std::string &GetQueryName() const {return queryName; };

private:
    std::string queryName;
    MapNodeString mapNodeString;
};

#endif
