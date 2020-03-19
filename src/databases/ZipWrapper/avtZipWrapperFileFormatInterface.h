// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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

class avtZipWrapperFileFormat;
class DBOptionsAttributes;
class CommonDatabasePluginInfo;
class DatabasePluginManager;

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
//
//    Brad Whitlock, Tue Jun 24 16:43:17 PDT 2008
//    Added common database info to the constructor so the constructor can
//    call the plugin manager.
//
//    Hank Childs, Tue Dec 20 15:55:05 PST 2011
//    Add method CreateCacheNameIncludingSelections.
//
// ****************************************************************************

class avtZipWrapperFileFormatInterface : public avtFileFormatInterface 
{
  public:
                       avtZipWrapperFileFormatInterface(const char *const *list, int nList, int nBlock,
                           const DBOptionsAttributes *readOpts,
                           CommonDatabasePluginInfo *);
    virtual           ~avtZipWrapperFileFormatInterface();

    virtual vtkDataSet     *GetMesh(int, int, const char *);
    virtual vtkDataArray   *GetVar(int, int, const char *);
    virtual vtkDataArray   *GetVectorVar(int, int, const char *);
    virtual void           *GetAuxiliaryData(const char *, int, int, 
                                             const char *, void *, 
                                             DestructorFunction &);
    virtual std::string     CreateCacheNameIncludingSelections(std::string, int, int);

    virtual const char     *GetFilename(int);
    virtual void            SetDatabaseMetaData(avtDatabaseMetaData *md,
                                int timeState = 0,
                                bool forceReadAllCyclesTimes = false);
    virtual void            SetCycleTimeInDatabaseMetaData(
                                avtDatabaseMetaData *md, int ts);

    virtual void            FreeUpResources(int, int);
    virtual void            ActivateTimestep(int ts);
    virtual bool            PopulateIOInformation(int ts, 
                               const std::string &meshName,avtIOInformation &)
                               { return false; }

  protected:
    virtual int             GetNumberOfFileFormats(void) { return 1; };
    virtual avtFileFormat  *GetFormat(int) const;

  private:
      static void                  Initialize(int procNum, int procCount,
                                       const DBOptionsAttributes *readOpts);
      static void                  Finalize();
      static void                  CleanUpAtExit();
      static std::vector<avtZipWrapperFileFormatInterface*>
                                   objList;
      static std::string                tmpDir;
      static std::string                decompCmd;
      static int                   maxDecompressedFiles;

      int                          procNum;
      int                          procCount;

      avtFileFormatInterface      *GetRealInterface(int ts, int dom, bool dontCache = false);
      void                         UpdateRealFileFormatInterface(avtFileFormatInterface *iface) const;

      avtDatabaseMetaData          mdCopy;

      avtZipWrapperFileFormat     *dummyFileFormat;
      avtFileFormatInterface      *dummyInterface;

      DatabasePluginManager       *pluginManager;
      std::vector<std::vector<std::string> >      inputFileList;
      int                          inputFileBlockCount;
      int                          nTimesteps;
      int                          dbType;
      std::string                       pluginId;
      bool                         realPluginWasLoadedByMe;

      MRUCache<std::string, avtFileFormatInterface*, MRUCache_CallbackDelete, 20>
                                   decompressedFilesCache;
};

#endif
