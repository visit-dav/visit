/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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

#ifndef VIEWER_CONFIGURATION_MANAGER_H
#define VIEWER_CONFIGURATION_MANAGER_H
#include <viewer_exports.h>
#include <ConfigManager.h>
#include <ViewerBase.h>
#include <string>
#include <vectortypes.h>

// Forward declarations
class AttributeSubject;
class DataNode;
class ViewerSubject;

// ****************************************************************************
//  Class: ViewerConfigManager
//
//  Purpose:
//    This class writes the viewer's portion of the VisIt configuration
//    file.
//
//  Notes:
//    Objects of this class do not own the state objects that they save
//    to the config file. They just maintain pointers to them so the
//    config file can be written easily.
//
//  Programmer: Brad Whitlock
//  Creation:   Thu Sep 28 10:29:28 PDT 2000
//
//  Modifications:
//    Kathleen Bonnell, Wed Oct 11 09:03:03 PDT 2000
//    I added OnionPeelAttributes state object.
//
//    Brad Whitlock, Fri Nov 10 15:25:38 PST 2000
//    I added the MaterialAttributes state object.
//
//    Eric Brugger, Thu Dec 21 08:00:03 PST 2000
//    I modified the class to accept a list of AttributeSubjects
//    instead of specific ones.
//
//    Brad Whitlock, Wed Sep 5 09:58:05 PDT 2001
//    Modified the class so it splits the reading of the config file from
//    the processing of the config file.
//
//    Brad Whitlock, Tue Feb 19 13:02:55 PST 2002
//    Made it conform to the base class's new interface.
//
//    Brad Whitlock, Fri Mar 21 10:04:30 PDT 2003
//    I removed the ProcessOldVersions method.
//
//    Brad Whitlock, Mon Jun 30 12:24:43 PDT 2003
//    I made it require a pointer to its parent ViewerSubject.
//
//    Brad Whitlock, Wed Jul 9 12:44:08 PDT 2003
//    I added methods to export the entire state and import the entire state.
//
//    Brad Whitlock, Mon Aug 25 14:28:23 PST 2003
//    Added NotifyIfSelected.
//
//    Brad Whitlock, Thu Feb 17 16:06:02 PST 2005
//    I made WriteConfig return a bool.
//
//    Brad Whitlock, Fri Nov 10 10:58:11 PDT 2006
//    I added arguments to ImportEntireState.
//
//    Brad Whitlock, Mon Feb 12 18:01:20 PST 2007
//    Use ViewerBase base class.
//
// ****************************************************************************

class VIEWER_API ViewerConfigManager : public ViewerBase, public ConfigManager
{
public:
    ViewerConfigManager(ViewerSubject *);
    virtual ~ViewerConfigManager();

    virtual bool WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
    void ProcessConfigSettings(DataNode *);
    void Notify();
    void NotifyIfSelected();
    void ClearSubjects();

    void Add(AttributeSubject *subject);

    void ExportEntireState(const std::string &filename);
    void ImportEntireState(const std::string &filename, bool,
                           const stringVector &, bool);

private:
    bool                             writeDetail;
    std::vector<AttributeSubject *>  subjectList;
    ViewerSubject                   *parent;
};

#endif
