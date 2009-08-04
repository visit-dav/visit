/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#ifndef CONFIGURATION_MANAGER_H
#define CONFIGURATION_MANAGER_H
#include <state_exports.h>
#include <stdio.h>
#include <DataNode.h>
#include <string>

// ****************************************************************************
// Class: ConfigManager
//
// Purpose:
//   This class contains logic to read and write config files.
//
// Notes:
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 28 10:29:28 PDT 2000
//
// Modifications:
//   Brad Whitlock, Thu Mar 29 17:21:51 PST 2001
//   Modified the GetDefaultConfigFile method so it accepts an argument.
//
//   Brad Whitlock, Tue Feb 19 12:29:17 PDT 2002
//   Added a method to get the name of the system config file.
//
//   Brad Whitlock, Thu Jul 3 16:14:04 PST 2003
//   Rewrote the methods that read in the file.
//
//   Brad Whitlock, Thu Jul 3 17:43:47 PST 2003
//   I added a convenience method to get the user's home .visit dir.
//
//   Brad Whitlock, Fri Oct 3 16:11:27 PST 2003
//   Added internal methods to handle string vectors better.
//
//   Brad Whitlock, Wed Feb 16 09:13:21 PDT 2005
//   Removed GetUserVisItDirectory, GetDefaultConfigFile, GetSystemConfigFile
//   methods and made them part of the utility library (Utility.h).
//
//   Brad Whitlock, Thu Feb 17 15:45:19 PST 2005
//   I made WriteConfigFile return a bool.
//
//   Jeremy Meredith, Tue Aug  2 16:05:09 PDT 2005
//   Added WriteEscapedString method.
//
// ****************************************************************************

class STATE_API ConfigManager
{
public:
    ConfigManager();
    virtual ~ConfigManager();

    virtual bool WriteConfigFile(const char *filename) = 0;
    virtual DataNode *ReadConfigFile(const char *filename) = 0;
protected:
    // Functions to write out the tree.
    void WriteObject(DataNode *node, int indentLevel = 0);
    void WriteData(DataNode *node);
    void WriteQuotedStringData(const std::string &str);
    void WriteEscapedString(const std::string &str);
    void WriteIndent(int indentLevel);
    void WriteBack(DataNode *root);

    // Functions to read in the tree.
    bool ReadObject(DataNode *);
    bool ReadObjectHelper(DataNode *, bool &);
    char ReadChar();
    void PutBackChar(char c);
    void FinishTag();
    bool ReadField(DataNode *parentNode, const std::string &tagName,
                   NodeTypeEnum tagType, int tagLength);
    DataNode *ReadFieldData(const std::string &tagName, NodeTypeEnum tagType,
                            int tagLength);
    bool ReadTag(std::string &tagName, NodeTypeEnum &tagType,
                   int &tagLength, bool &tagIsReturnTag);
    stringVector ReadStringVector(char termChar);
    void RemoveLeadAndTailQuotes(stringVector &sv);

    // File attributes used in reading.
    bool  putback;
    char  putbackChar;
    FILE *fp;
};

#endif
