#include <QString>
#include <QTextStream>
QTextStream cOut(stdout);
QTextStream cErr(stderr);
QString     Endl("\n");

#include <qxml.h>
#include "Field.h"
#include "Attribute.h"
#include "Enum.h"
#include "Plugin.h"

#include <BJHash.h>
#include <stdio.h>
#ifndef WIN32
#include <unistd.h>
#endif

vector<EnumType*> EnumType::enums;

bool print   = true;
bool clobber = false;
bool using_dev = false;
bool installpublic  = false;
bool installprivate = false;
bool outputtoinputdir = false;
QString currentInputDir = "";
QString preHeaderLeader = "pre_";

const char *copyright_str = 
"/*****************************************************************************\n"
"*\n"
"* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC\n"
"* Produced at the Lawrence Livermore National Laboratory\n"
"* LLNL-CODE-400124\n"
"* All rights reserved.\n"
"*\n"
"* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The\n"
"* full copyright notice is contained in the file COPYRIGHT located at the root\n"
"* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n"
"*\n"
"* Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n"
"* modification, are permitted provided that the following conditions are met:\n"
"*\n"
"*  - Redistributions of  source code must  retain the above  copyright notice,\n"
"*    this list of conditions and the disclaimer below.\n"
"*  - Redistributions in binary form must reproduce the above copyright notice,\n"
"*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n"
"*    documentation and/or other materials provided with the distribution.\n"
"*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may\n"
"*    be used to endorse or promote products derived from this software without\n"
"*    specific prior written permission.\n"
"*\n"
"* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n"
"* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n"
"* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,\n"
"* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY\n"
"* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL\n"
"* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n"
"* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n"
"* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n"
"* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n"
"* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n"
"* DAMAGE.\n"
"*\n"
"*****************************************************************************/\n";
