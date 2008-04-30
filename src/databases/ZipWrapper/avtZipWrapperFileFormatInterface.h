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
//                   avtZipWrapperFileFormatInterface.h                      //
// ************************************************************************* //

#ifndef AVT_ZIPWRAPPER_FILE_FORMAT_INTERFACE_H
#define AVT_ZIPWRAPPER_FILE_FORMAT_INTERFACE_H

#include <avtDatabaseMetaData.h>
#include <avtFileFormatInterface.h>
#include <MRUCache.h>

#include <string>
#include <vector>

using std::string;
using std::vector;

class avtZipWrapperFileFormat;
class DBOptionsAttributes;

// ****************************************************************************
//  Class: avtZipWrapperFileFormatInterface
//
//  Purpose: Override the FileFormatInterface class to support on-the-fly
//  decompression of database files.
//
//  If you are looking for good examples of database plugins, ZipWrapper is
//  NOT a good choice. Amoung database plugins, ZipWrapper is very unusual.
//  First, it overrides a file format interface object and not a file format
//  object. It does define its own file format object but that object's role
//  is basically to serve as a proxy for real format objects.
//
//  The main trick ZipWrapper does is to NOT instantiate real format objects
//  until data from them is actually needed. Then, it will decompress the
//  appropriate file with a call to system(), and instantiate the appropriate
//  real file format interface to read the file. To do this, ZipWrapper
//  behaves a little like an MTMD file format interface. It manages
//  translation of queries from VisIt of the form (timestep, domain) to the
//  actual files to be read.
//
//  Managing decompression of files, ensuring that not too many files are
//  decompressed (before freeing older, lesser used ones) as well as ensuring
//  decompressed files are removed, espeically upon exit, is a big part of
//  what ZipWrapper does. An MRUCache object is used to help manage this.
//
//  ZipWrapper recognizes some enviornment variables. These are...
//      VISIT_ZIPWRAPPER_TMPDIR specifies the temporary directory
//                              for decompressed files. 
//      TMPDIR                  Used if immediate above is not defined.
//      HOME                    Used if immediate above is not defined.
//      "/usr/tmp"              Used if immediate above is not defined.
//
//      VISIT_ZIPWRAPPER_USER   specifies the username
//      USER                    Used if immediate above is not defined.
//      USERNAME                Used if immediate above is not defined.
//      "user"                  Used if immediate above is not defined.
//
//      VISIT_ZIPWRAPPER_MAXFILES specifies the maximum number of
//                              decompressed files that can be in existance
//                              at any one time. Default is 50. In
//                              parallel, this is a total summed over
//                              all processors unless a negative number
//                              is specified in which case it is the
//                              total per processor (useful for
//                              processor local tmp directories).
//
//      VISIT_ZIPWRAPPER_DCMD   specifies the decompression command
//                              to use to decompress files. Default is
//                              to use extension to determine command.
//
//  Be aware that in a client/server setting where the command you use to
//  start VisIt is your desktop machine (the client) but the mdserver and
//  engine run on another machine (the server), any environment variables
//  you specify on the client will not defined for VisIt on the server.
//  You will have to modify your login shell to define them.
//
//  Programmer: Mark C. Miller 
//  Creation:   Thu Jul 26 08:34:49 PDT 2007 
//
//  Modifications:
//    Mark C. Miller, Wed Aug  8 17:19:14 PDT 2007
//    Added procNum/procCount args to Initialize
//
//    Hank Childs, Sun Oct 21 09:51:53 PDT 2007
//    Templates with default arguments confuse xlC, so add the default
//    arguments explicitly.
//
//    Mark C. Miller, Tue Apr 29 23:33:55 PDT 2008
//    Added read options to constructor and Initialize
// ****************************************************************************

class avtZipWrapperFileFormatInterface : public avtFileFormatInterface 
{
  public:
                       avtZipWrapperFileFormatInterface(const char *const *list, int nList, int nBlock,
                           const DBOptionsAttributes *readOpts);
    virtual           ~avtZipWrapperFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *, int, int, 
                                             const char *, void *, 
                                             DestructorFunction &);

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false);
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts);

    virtual void            FreeUpResources(int, int);
    virtual void            ActivateTimestep(int ts);
    virtual void            PopulateIOInformation(int ts, avtIOInformation &) { return; };

  protected:
    virtual int             GetNumberOfFileFormats(void) { return 1; };
    virtual avtFileFormat  *GetFormat(int) const;

  private:
      static void                  Initialize(int procNum, int procCount,
                                       const DBOptionsAttributes *readOpts);
      static void                  Finalize();
      static void                  CleanUpAtExit();
      static vector<avtZipWrapperFileFormatInterface*>
                                   objList;
      static string                tmpDir;
      static string                decompCmd;
      static int                   maxDecompressedFiles;

      int                          procNum;
      int                          procCount;

      avtFileFormatInterface      *GetRealInterface(int ts, int dom, bool dontCache = false);
      void                         UpdateRealFileFormatInterface(avtFileFormatInterface *iface) const;

      avtDatabaseMetaData          mdCopy;

      avtZipWrapperFileFormat     *dummyFileFormat;
      avtFileFormatInterface      *dummyInterface;

      vector<vector<string> >      inputFileList;
      int                          inputFileListSize;
      int                          inputFileBlockCount;
      int                          nTimesteps;
      int                          dbType;
      string                       pluginId;
      bool                         realPluginWasLoadedByMe;

      MRUCache<string, avtFileFormatInterface*, MRUCache_CallbackDelete, 20>
                                   decompressedFilesCache;
};

#endif
