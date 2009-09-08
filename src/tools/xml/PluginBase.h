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
#ifndef PLUGIN_BASE_H
#define PLUGIN_BASE_H
#include <QTextStream>

#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

#include <snprintf.h>
#if defined(_WIN32)
#include <windows.h>
#endif

// ****************************************************************************
// Class: PluginBase
//
// Purpose:
//   This is a base class for all of the plugin code generator classes. We now
//   use a base class because we formerly had to add new members in all of the
//   various code generation tools instead of adding the new members in a central
//   base class.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 5 11:37:27 PDT 2008
//
// Modifications:
//    Jeremy Meredith, Thu Aug  7 14:55:48 EDT 2008
//    Use const char * for string literals.
//
//    Brad Whitlock, Wed Oct 15 14:20:36 PDT 2008
//    Added support for custom Java files.
//
//    Cyrus Harrison, Fri Sep 19 13:46:16 PDT 2008
//    Added support for custom libs for gui,engine, mdserver, viewer
//
//    Jeremy Meredith, Tue Sep  8 15:11:35 EDT 2009
//    Split custom engine libs into serial and parallel versions.
//
// ****************************************************************************

class PluginBase
{
public:
    QString name;
    QString type;
    QString label;
    QString version;
    QString vartype;
    QString dbtype;
    QString iconFile;

    bool    haswriter;
    bool    hasoptions;
    bool    specifiedFilenames;  // For DB plugins
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    hasEngineSpecificCode;
    bool    onlyEnginePlugin;
    bool    noEnginePlugin;

    std::vector<QString> cxxflags;
    std::vector<QString> ldflags;
    std::vector<QString> libs;
    std::vector<QString> extensions; // for DB plugins
    std::vector<QString> filenames;  // for DB plugins
    bool customgfiles;
    std::vector<QString> gfiles;     // gui files
    bool customglibs;
    std::vector<QString> glibs;      // gui libs
    bool customsfiles;
    std::vector<QString> sfiles;     // scripting
    bool customvfiles;
    std::vector<QString> vfiles;     // viewer files
    bool customvlibs;
    std::vector<QString> vlibs;      // viewer libs
    bool custommfiles;
    std::vector<QString> mfiles;     // mdserver files
    bool custommlibs;
    std::vector<QString> mlibs;      // mdserver libs
    bool customefiles;
    std::vector<QString> efiles;     // engine files
    bool customelibsSer;
    std::vector<QString> elibsSer;      // engine libs
    bool customelibsPar;
    std::vector<QString> elibsPar;      // engine libs
    bool customwfiles;
    std::vector<QString> wfiles;     // widgets
    bool customvwfiles;
    std::vector<QString> vwfiles;    // viewer widgets
    bool customjfiles;
    std::vector<QString> jfiles;     // Java code

public:
    PluginBase(const QString &n,const QString &l,const QString &t,
               const QString &vt,const QString &dt,
               const QString &v, const QString &ifile,
               bool hw, bool ho, bool onlyengine, bool noengine)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt), 
          iconFile(ifile),
          haswriter(hw),
          hasoptions(ho),
          specifiedFilenames(false),
          enabledByDefault(true),
          has_MDS_specific_code(false),
          hasEngineSpecificCode(false),
          onlyEnginePlugin(onlyengine), 
          noEnginePlugin(noengine),
          cxxflags(),
          ldflags(),
          libs(),
          extensions(),
          filenames(),
          customgfiles(false),
          gfiles(),
          customglibs(false),
          glibs(),
          customsfiles(false),
          sfiles(),
          customvfiles(false),
          vfiles(),
          customvlibs(false),
          vlibs(),
          custommfiles(false),
          mfiles(),
          custommlibs(false),
          mlibs(),
          customefiles(false),
          efiles(),
          customelibsSer(false),
          elibsSer(),
          customelibsPar(false),
          elibsPar(),
          customwfiles(false),
          wfiles(),
          customvwfiles(false),
          vwfiles(),
          customjfiles(false),
          jfiles()
    {
    }

    virtual ~PluginBase()
    {
    }

    static QString
    CurrentTime()
    {
        const char *tstr[] = {"PDT", "PST"};
        char s1[10], s2[10], s3[10], tmpbuf[200];
        time_t t;
        char *c = NULL;
        int h,m,s,y;
        t = time(NULL);
        c = asctime(localtime(&t));
        // Read the hour.
        sscanf(c, "%s %s %s %d:%d:%d %d", s1, s2, s3, &h, &m, &s, &y);
        // Reformat the string a little.
        sprintf(tmpbuf, "%s %s %s %02d:%02d:%02d %s %d",
                s1, s2, s3, h, m, s, tstr[h > 12], y);

        return QString(tmpbuf);
    }

    const char *
    UserName()
    {
         static char user_buffer[100];
         const char *user = getenv("USER");
         if(user != 0)
             SNPRINTF(user_buffer, 100, "%s -- ", user);
         else
         {
#if defined(_WIN32)
             char tmp[100];
             DWORD maxLen = 100;
             GetUserName((LPTSTR)tmp, (LPDWORD)&maxLen);
             SNPRINTF(user_buffer, 100, "%s -- ", tmp);
#else
             user_buffer[0] = '\0';
#endif
         }
         return user_buffer;
    }
};

#endif
