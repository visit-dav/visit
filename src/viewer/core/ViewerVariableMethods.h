// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_VARIABLE_METHODS_H
#define VIEWER_VARIABLE_METHODS_H
#include <viewercore_exports.h>
#include <ViewerBase.h>
#include <avtTypes.h>

#include <string>

class ExpressionList;

// ****************************************************************************
// Class: ViewerVariableMethods
//
// Purpose:
//   This class provides some variable-related convenience methods using 
//   metadata services from the file server.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 16:13:19 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerVariableMethods : public ViewerBase
{
public:
    ViewerVariableMethods();
    virtual ~ViewerVariableMethods();

    avtVarType DetermineVarType(const std::string &host,
                                const std::string &db,
                                const std::string &var,
                                int state);

    avtVarType DetermineRealVarType(const std::string &host,
                                    const std::string &db,
                                    const std::string &var,
                                    int state);

    void GetUserExpressions(ExpressionList &newList);
    void GetDatabaseExpressions(ExpressionList &newList,
                                const std::string &host,
                                const std::string &db,
                                int state);
    void GetOperatorCreatedExpressions(ExpressionList &newList,
                                const std::string &host,
                                const std::string &db,
                                int state);
    void GetAllExpressions(ExpressionList &newList,
                           const std::string &host,
                           const std::string &db,
                           int state);
};

#endif
