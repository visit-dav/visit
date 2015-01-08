/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/
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
