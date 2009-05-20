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

// ************************************************************************* //
//                          avtDatabaseFactory.h                             //
// ************************************************************************* //

#ifndef AVT_DATABASE_FACTORY_H
#define AVT_DATABASE_FACTORY_H

#include <database_exports.h>

#include <stdlib.h> // For NULL
#include <string>
#include <vector>

#include <FileOpenOptions.h>

class avtDatabase;
class CommonDatabasePluginInfo;
class DatabasePluginManager;

// ****************************************************************************
//  Class: avtDatabaseFactory
//
//  Purpose:
//      Takes a file list, determines the type of file and instantiates the
//      correct avtDatabase.
//
//  Programmer: Hank Childs
//  Creation:   September 12, 2000
//
//  Modifications:
//
//    Jeremy Meredith, Tue Aug 27 15:08:29 PDT 2002
//    Removed all formats and placed them in database plugins.
//
//    Hank Childs, Mon Mar  1 08:48:26 PST 2004
//    Added the timestep as an argument.
//
//    Hank Childs, Mon Mar 22 09:41:34 PST 2004
//    Added string specifying format type.  Also removed outdated comments
//    from the era when the DB factory knew about specific formats.
//
//    Hank Childs, Sun May  9 11:41:45 PDT 2004
//    Allow for default file format type to be set.
//
//    Mark C. Miller, Tue May 31 20:12:42 PDT 2005
//    Added bool args for forcing reading of all cycles/times
//
//    Hank Childs, Thu Jan 11 15:56:53 PST 2007
//    Added argument for list of plugins attempted.
//
//    Mark C. Miller, Thu Jun 14 10:26:37 PDT 2007
//    Added bool to support to treat all databases as time varying to various
//    methods
//
//    Kathleen Bonnell, Tue Oct  9 14:40:10 PDT 2007 
//    Added support for turning on/off automatic creation of MeshQuality and
//    TimeDerivative expressions. 
//
//    Cyrus Harrison, Wed Nov 28 10:38:17 PST 2007
//    Added preference for automatic creation of Vector Magnitude expressions. 
//
//    Jeremy Meredith, Wed Jan 23 16:14:13 EST 2008
//    Store the default file opening options for database plugins.
//
//    Jeremy Meredith, Thu Jan 24 14:45:14 EST 2008
//    Added ability to specify a format to try *before* the ones guessed
//    from file extensions.
//
//    Jeremy Meredith, Wed Mar 19 14:06:16 EDT 2008
//    Renamed default format to fallback format for clarity.
//
//    Brad Whitlock, Tue Jun 24 15:49:33 PDT 2008
//    Pass the database plugin manager in because it's no longer a singleton.
//
//    Mark C. Miller, Tue May 19 21:26:53 PDT 2009
//    Added methods to retrieve fallback and first format as well as default
//    file open options.
// ****************************************************************************

class DATABASE_API avtDatabaseFactory
{
  public:
    static avtDatabase          *FileList(DatabasePluginManager *,
                                          const char * const *, int, int,
                                          std::vector<std::string> &,
                                          const char * = NULL, bool = false,
                                          bool = false);
    static avtDatabase          *VisitFile(DatabasePluginManager *,
                                           const char *, int,
                                           std::vector<std::string> &,
                                           const char * = NULL, bool = false,
                                           bool = false);
    static void                  SetFallbackFormat(const char *);
    static void                  SetFormatToTryFirst(const char *);
    static void                  SetDefaultFileOpenOptions(const
                                                           FileOpenOptions &f);

    static const char           *GetFormatToTryFirst()
                                     {return formatToTryFirst;};
    static const char           *GetFallbackFormat()
                                     {return fallbackFormat;};
    static const FileOpenOptions &GetDefaultFileOpenOptions()
                                     { return defaultFileOpenOptions;};

    static void                  SetCreateMeshQualityExpressions(bool f)
                                     {createMeshQualityExpressions = f;}
    static void                  SetCreateTimeDerivativeExpressions(bool f)
                                     {createTimeDerivativeExpressions = f;}
    static void                  SetCreateVectorMagnitudeExpressions(bool f)
                                     {createVectorMagnitudeExpressions= f;}
                                     
    static bool                  GetCreateMeshQualityExpressions(void)
                                  {return createMeshQualityExpressions;}
    static bool                  GetCreateTimeDerivativeExpressions(void)
                                  {return createTimeDerivativeExpressions;}
    static bool                  GetCreateVectorMagnitudeExpressions(void)
                                  {return createVectorMagnitudeExpressions;}

  protected:
    static avtDatabase          *SetupDatabase(CommonDatabasePluginInfo *,
                                               const char * const *, int,
                                               int, int, int, bool, bool);

    static char                 *fallbackFormat;
    static char                 *formatToTryFirst;
    static bool                  createMeshQualityExpressions;
    static bool                  createTimeDerivativeExpressions;
    static bool                  createVectorMagnitudeExpressions;
    static FileOpenOptions       defaultFileOpenOptions;
};
#endif
