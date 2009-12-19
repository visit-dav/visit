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
bool generateVersion7Projects = true;
#ifdef _WIN32
    bool buildforvisitsource = false;
    QString fullVisItDir = "";
    QString fullCurrentDir = "";
    QString privatePluginDir = "";
#endif
#include "GenerateProjectFile.h"

#define EXTRA_PRINTUSAGE
void
ExtraPrintUsage()
{
    cErr << "        -version7      (xml2projectfile only) make MSVC .Net "
         << "2003 projects (default)" << Endl;
    cErr << "        -version8      (xml2projectfile only) COMING SOON "
          << "make MSVC 2005 projects " << Endl;
}

#define HANDLE_ARGV
void
HandleArgv(int &argc, char **argv)
{
    for (int i=1; i<argc; i++)
    {
        if (strcmp(argv[i], "-version7") == 0)
        {
            generateVersion7Projects = true;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
        else if (strcmp(argv[i], "-version8") == 0)
        {
            generateVersion7Projects = false;
            argc--;
            for (int j=i; j<argc; j++)
                argv[j] = argv[j+1];
            i--;
        }
    }
}

#include "main.C"

void
CallGenerator(const QString &docType, Attribute *attribute, Plugin *plugin, const QString &file)
{
    if (docType == "Plugin")
    {
#ifdef _WIN32
        // find full path to this executable
        fullVisItDir = "";
        char *tmp = new char[MAX_PATH];
        if (GetModuleFileName(NULL, tmp, 100) != 0)
        {
            fullVisItDir = tmp;
            int lastslash = fullVisItDir.lastIndexOf("\\");
            if (lastslash >= 0)
                fullVisItDir = fullVisItDir.left(lastslash+1);
        }
        // find full path to file
        if (GetFullPathName(file.toStdString().c_str(), 100, tmp, NULL) > 0)
        {
            fullCurrentDir = tmp;
        }
#endif
        // project file writer mode
        plugin->WriteProjectFiles(Open, generateVersion7Projects);
    }
    else
    {
        cErr << "No project files to generate for non-plugins." << Endl;
    }
}
