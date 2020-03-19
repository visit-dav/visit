// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef VIEWER_DATABASE_CORRELATION_METHODS_H
#define VIEWER_DATABASE_CORRELATION_METHODS_H
#include <ViewerBase.h>
#include <string>
#include <map>
#include <vectortypes.h>

class DatabaseCorrelation;
class DataNode;

// ****************************************************************************
// Class: ViewerDatabaseCorrelationMethods
//
// Purpose:
//   Contains methods that manipulate database correlations.
//
// Notes:    
//
// Programmer: Brad Whitlock
// Creation:   Mon Sep  8 09:43:39 PDT 2014
//
// Modifications:
//
// ****************************************************************************

class VIEWERCORE_API ViewerDatabaseCorrelationMethods : public ViewerBase
{
public:
    ViewerDatabaseCorrelationMethods();
    virtual ~ViewerDatabaseCorrelationMethods();

    // Methods for manipulating database correlations.
    DatabaseCorrelation       *CreateDatabaseCorrelation(const std::string &,
                                                         const stringVector &,
                                                         int, int=-1);
    void                       RemoveDatabaseCorrelation(const std::string &);
    void                       UpdateDatabaseCorrelation(const std::string &);
    DatabaseCorrelation       *GetMostSuitableCorrelation(const stringVector &) const;

    std::string                CreateNewCorrelationName() const;
    bool                       PreviouslyDeclinedCorrelationCreation(
                                   const stringVector &) const;
    void                       DeclineCorrelationCreation(const stringVector &);

    void                       CreateNode(DataNode *parentNode, 
                                   const std::map<std::string, std::string> &dbToSource,
                                   bool detailed);
    void                       SetFromNode(DataNode *parentNode,
                                   const std::map<std::string,std::string> &sourceToDB,
                                   const std::string &configVersion);
private:
    stringVector declinedFiles;
    intVector    declinedFilesLength;
};

#endif
