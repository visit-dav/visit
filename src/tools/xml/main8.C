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
#include "main.h"
const char *java_copyright_str = 
"// ***************************************************************************\n"
"//\n"
"// Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC\n"
"// Produced at the Lawrence Livermore National Laboratory\n"
"// LLNL-CODE-400142\n"
"// All rights reserved.\n"
"//\n"
"// This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The\n"
"// full copyright notice is contained in the file COPYRIGHT located at the root\n"
"// of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.\n"
"//\n"
"// Redistribution  and  use  in  source  and  binary  forms,  with  or  without\n"
"// modification, are permitted provided that the following conditions are met:\n"
"//\n"
"//  - Redistributions of  source code must  retain the above  copyright notice,\n"
"//    this list of conditions and the disclaimer below.\n"
"//  - Redistributions in binary form must reproduce the above copyright notice,\n"
"//    this  list of  conditions  and  the  disclaimer (as noted below)  in  the\n"
"//    documentation and/or other materials provided with the distribution.\n"
"//  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may\n"
"//    be used to endorse or promote products derived from this software without\n"
"//    specific prior written permission.\n"
"//\n"
"// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS \"AS IS\"\n"
"// AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE\n"
"// IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE\n"
"// ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,\n"
"// LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY\n"
"// DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL\n"
"// DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR\n"
"// SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER\n"
"// CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT\n"
"// LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY\n"
"// OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH\n"
"// DAMAGE.\n"
"//\n"
"// ***************************************************************************\n";

#include "GenerateJava.h"
#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &)
{
    if (docType == "Plugin" && plugin->type == "database")
    {
        cErr << "No java to generate for database plugins\n";
    }
    else
    {
        if (docType == "Plugin")
        {
            attribute->pluginVersion = plugin->version;
            attribute->pluginName = plugin->name;
            attribute->pluginType = plugin->type;
        }

        // java atts writer mode
        QFile *fj;
        if ((fj = Open(attribute->name+".java")) != 0)
        {
            QTextStream j(fj);
            attribute->WriteSource(j);
            fj->close();
            delete fj;
        }
    }
}
