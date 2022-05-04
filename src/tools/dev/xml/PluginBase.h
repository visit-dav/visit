// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef PLUGIN_BASE_H
#define PLUGIN_BASE_H
#include <QTextStream>

#include <vector>

#include <stdlib.h>
#include <sys/types.h>
#include <time.h>

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
//    Kathleen Bonnell, Wed May 26 9:03:27 MST 2009
//    Added support for custom windows files for mdserver and engine.
//
//    Jeremy Meredith, Tue Sep  8 15:11:35 EDT 2009
//    Split custom engine libs into serial and parallel versions.
//
//    Jeremy Meredith, Tue Dec 29 11:21:30 EST 2009
//    Replaced "Extensions" and "Filenames" with "FilePatterns".  Removed
//    specifiedFilenames.  Added filePatternsStrict and opensWholeDirectory.
//
//    Brad Whitlock, Thu Feb  4 16:09:24 PST 2010
//    I added category.
//
//    Hank Childs, Thu Dec 30 13:33:19 PST 2010
//    Add support for expression-creating operators.
//
//    Kathleen Biagas, Thu Nov  6 11:24:21 PST 2014
//    Add support for DEFINES tag.
//
//    Kathleen Biagas, Thu Jan  2 09:31:08 PST 2020
//    Added hasLicense.
//
//    Kathleen Biagas, Fri July 16, 2021
//    Added windefs storage for WIN32DEFINES.
//
//    Kathleen Biagas, Tue April 27, 2022
//    Added skipInfoGen, allowing plugins to specify that xml2info gen
//    targets should not automatically created.
//    For plugins that have custom code that isn't currently supported by
//    generation tools.
//
//    Kathleen Biagas, Tue May 3, 2022
//    Added support for component-specific CXXFLAGS, LDFLAGS and DEFINES.
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
    QString category;

    bool    haswriter;
    bool    hasoptions;
    bool    haslicense;
    bool    enabledByDefault;
    bool    has_MDS_specific_code;
    bool    hasEngineSpecificCode;
    bool    onlyEnginePlugin;
    bool    noEnginePlugin;
    bool    skipInfoGen;

    bool    createExpression;   // for Operator plugins
    QString exprInType;         // for Operator plugins
    QString exprOutType;        // for Operator plugins

    std::vector<QString> cxxflags;
    std::vector<QString> ldflags;
    std::vector<QString> libs;
    std::vector<QString> defs;
    std::vector<QString> windefs;
    std::vector<QString> filePatterns;        // for DB plugins
    bool                 filePatternsStrict;  // for DB plugins
    bool                 opensWholeDirectory; // for DB plugins
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
    bool customwmfiles;
    std::vector<QString> wmfiles;    // mdserver files for windows
    bool custommlibs;
    std::vector<QString> mlibs;      // mdserver libs
    std::vector<QString> mcxxflags;  // mdserver cxxflags
    std::vector<QString> mldflags;   // mdserver ldflags
    std::vector<QString> mdefs;      // mdserver defines
    bool customefiles;
    std::vector<QString> efiles;     // engine files
    bool customwefiles;
    std::vector<QString> wefiles;    // engine files for windows
    bool customelibsSer;
    std::vector<QString> elibsSer;      // serial engine libs
    std::vector<QString> ecxxflagsSer;  // serial engine cxxflags
    std::vector<QString> eldflagsSer;   // serial engine ldflags
    std::vector<QString> edefsSer;      // serial engine defines
    bool customelibsPar;
    std::vector<QString> elibsPar;      // parallel engine libs
    std::vector<QString> ecxxflagsPar;  // parallel engine cxxflags
    std::vector<QString> eldflagsPar;   // parallel engine ldflags
    std::vector<QString> edefsPar;      // parallel engine defines
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
               bool hw, bool ho, bool hl, bool onlyengine, bool noengine)
        : name(n), type(t), label(l), version(v), vartype(vt), dbtype(dt),
          iconFile(ifile),
          category(),
          haswriter(hw),
          hasoptions(ho),
          haslicense(hl),
          enabledByDefault(true),
          has_MDS_specific_code(false),
          hasEngineSpecificCode(false),
          onlyEnginePlugin(onlyengine),
          noEnginePlugin(noengine),
          createExpression(false),
          exprInType(),
          exprOutType(),
          cxxflags(),
          ldflags(),
          libs(),
          defs(),
          windefs(),
          filePatterns(),
          filePatternsStrict(false),
          opensWholeDirectory(false),
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
          customwmfiles(false),
          wmfiles(),
          custommlibs(false),
          mlibs(),
          mcxxflags(),
          mldflags(),
          mdefs(),
          customefiles(false),
          efiles(),
          customwefiles(false),
          wefiles(),
          customelibsSer(false),
          elibsSer(),
          ecxxflagsSer(),
          eldflagsSer(),
          edefsSer(),
          customelibsPar(false),
          elibsPar(),
          ecxxflagsPar(),
          eldflagsPar(),
          edefsPar(),
          customwfiles(false),
          wfiles(),
          customvwfiles(false),
          vwfiles(),
          customjfiles(false),
          jfiles(),
          skipInfoGen(false)
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
             snprintf(user_buffer, 100, "%s -- ", user);
         else
         {
#if defined(_WIN32)
             char tmp[100];
             DWORD maxLen = 100;
             GetUserName((LPTSTR)tmp, (LPDWORD)&maxLen);
             snprintf(user_buffer, 100, "%s -- ", tmp);
#else
             user_buffer[0] = '\0';
#endif
         }
         return user_buffer;
    }
};

#endif
