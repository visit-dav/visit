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

#ifndef COLORTABLE_MANAGER_H
#define COLORTABLE_MANAGER_H
#include <ConfigManager.h>
#include <ColorControlPointList.h>
#include <ColorTableAttributes.h>
#include <string>

class DataNode;

// ****************************************************************************
// Class: ColorTableManager
//
// Purpose:
//   Reads and writes ColorControlPointLists to/from files.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 3 17:49:53 PST 2003
//
// Modifications:
//   Brad Whitlock, Thu Nov 13 11:51:47 PDT 2003
//   I changed how messages are passed out of Export.
//
//   Brad Whitlock, Thu Feb 17 15:54:26 PST 2005
//   Made WriteConfigFile return bool.
//
// ****************************************************************************

class STATE_API ColorTableManager : public ConfigManager
{
public:
    ColorTableManager();
    virtual ~ColorTableManager();

    bool              Export(const std::string &ctName,
                             const ColorControlPointList &ccpl,
                             std::string &message);
    bool              ImportColorTables(ColorTableAttributes *cta);

    virtual bool      WriteConfigFile(const char *filename);
    virtual DataNode *ReadConfigFile(const char *filename);
protected:
    void              ImportColorTable(const std::string &ctFileName);
    static void       ImportHelper(void *, const std::string &,
                                   bool, bool, long);
private:
    ColorTableAttributes *ctAtts;
    ColorControlPointList ccpl;
};

#endif
