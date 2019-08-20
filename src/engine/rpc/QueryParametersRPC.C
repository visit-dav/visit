// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QueryParametersRPC.h" 
#include <string>
using std::string;

// ****************************************************************************
//  Method: QueryParametersRPC::QueryParametersRPC
//
//  Purpose: 
//    This is the RPC's constructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
// ****************************************************************************

QueryParametersRPC::QueryParametersRPC() : BlockingRPC("sa", &mapNodeString), queryName("")
{
    // nothing here;
}


// ****************************************************************************
//  Method: QueryParametersRPC::~QueryParameterRPC
//
//  Purpose: 
//    This is the RPC's destructor
//
//  Programmer: Kathleen Biagas 
//  Creation:   July 15, 2011 
//
// ****************************************************************************

QueryParametersRPC::~QueryParametersRPC() 
{
    // nothing here;
}

// ****************************************************************************
// Method: QueryParametersRPC::operator()
//
// Purpose: 
//   Invocation method for the QueryParametersRPC class. Clears the caches that 
//   relate to the specified database.
//
// Arguments:
//   queryName : The name of the query for which we're requesting information.
//
// Programmer: Kathleen Biagas 
// Creation:   July 15, 2011
//
// Modifications:
//   
// ****************************************************************************

string
QueryParametersRPC::operator()(const std::string &queryname)
{
    queryName = queryname;
    Execute();
    return mapNodeString.s;
}

// ****************************************************************************
// Method: QueryParametersRPC::SelectAll
//
// Purpose: 
//   Selects all components in the RPC.
//
// Programmer: Kathleen Biagas 
// Creation:   July 15, 2011 
//
// Modifications:
//   
// ****************************************************************************

void
QueryParametersRPC::SelectAll()
{
    Select(0, (void*)&queryName);
}

// ****************************************************************************
//  Constructor:  QueryParametersRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::MapNodeString() : AttributeSubject("s")
{
    s = "";
}

// ****************************************************************************
//  Constructor:  QueryParametersRPC::MapNodeString::MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::MapNodeString(string s_) : AttributeSubject("s")
{
    s = s_;
}

// ****************************************************************************
//  Destructor:  QueryParametersRPC::MapNodeString::~MapNodeString
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
QueryParametersRPC::MapNodeString::~MapNodeString()
{
}


// ****************************************************************************
//  Destructor:  QueryParametersRPC::MapNodeString::SelectAll
//
//  Purpose:
//    Select all attributes.
//
//  Programmer:  Kathleen Biagas
//  Creation:    July 15, 2011
//
// ****************************************************************************
void
QueryParametersRPC::MapNodeString::SelectAll()
{
    Select(0, (void*)&s);
}


