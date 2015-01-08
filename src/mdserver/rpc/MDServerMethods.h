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

// ************************************************************************* //
//                              MDServerMethods.h                              //
// ************************************************************************* //

#ifndef MDSERVER_METHODS_H
#define MDSERVER_METHODS_H


#include <mdsrpc_exports.h>
#include <string>
#include <maptypes.h>
#include <vectortypes.h>
#include <FileOpenOptions.h>
#include <SILAttributes.h>
#include <avtDatabaseMetaData.h>
#include <DBPluginInfoAttributes.h>

class MDServerState;


class MDSERVER_RPC_API MDServerMethods
{

public:
    struct FileEntry
    {
        FileEntry();
        FileEntry(const FileEntry &);
        ~FileEntry();
        void operator = (const FileEntry &);

        static bool LessThan(const FileEntry &f1, const FileEntry &f2);

        std::string   name;
        int           size;

        bool          CanAccess() const    { return (flag & 1) == 1; }
        void          SetAccess(bool val)  { flag = ((flag & 0xfe) | (val?1:0)); }
        bool          IsVirtual() const    { return (flag & 2) == 2; }
        void          SetVirtual(bool val) { flag = ((flag & 0xfd) | (val?2:0)); }
    private:
        // Access  is bit 0
        // Virtual is bit 1
        unsigned char flag;
    };

    typedef std::vector<FileEntry> FileEntryVector;

    struct MDSERVER_RPC_API FileList
    {
        FileList();
        FileList(const FileList &);
        ~FileList();
        void operator = (const FileList &);
        void Sort();
        void Clear();

        FileEntryVector       files;
        FileEntryVector       dirs;
        FileEntryVector       others;
        StringStringVectorMap virtualFiles;
    };
    MDServerMethods(MDServerState*);
    ~MDServerMethods();

    void                       ChangeDirectory(const std::string &);
    void                       Connect(const stringVector &args);
    void                       CreateGroupList(const std::string &filename,
                                               const stringVector &groupList);
    std::string                GetDirectory();
    const FileList             *GetFileList(const std::string &filter, bool,
                                           bool=true);
    const avtDatabaseMetaData *GetMetaData(const std::string &, int=0,
                               bool forceReadAllCyclesTimes = false,
                               const std::string &forcedFileType = "",
                               bool treatAllDBsAsTimeVarying = false,
                               bool createMeshQualityExpressions = true,
                               bool createTimeDerivativeExpressions = true,
                               bool createVectorMagnitudeExpressions = true);
    const SILAttributes       *GetSIL(const std::string &, int=0,
                                   bool treatAllDBsAsTimeVarying = false);
    std::string                ExpandPath(const std::string &);
    void                       CloseDatabase();
    void                       CloseDatabase(const std::string &);
    void                       LoadPlugins();
    const DBPluginInfoAttributes *GetDBPluginInfo(void);
    void                       SetDefaultFileOpenOptions(
                                                       const FileOpenOptions&);

    std::string                GetPluginErrors();

    char                       GetSeparator() const;
    std::string                GetSeparatorString() const;
private:
    MDServerState* state;

    FileList                   fileList;
    avtDatabaseMetaData        metaData;
    SILAttributes              sil;
};
#endif
