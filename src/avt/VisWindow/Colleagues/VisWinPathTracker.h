// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            VisWinPathTracker.h                            //
// ************************************************************************* //

#include <string>
#include <map>
#include <vectortypes.h>

// ****************************************************************************
//  Class: VisWinPathTracker
//
//  Purpose:
//    This is a singleton used by the VisWinPlots and VisWinLegends colleagues
//    to keep track of the active database path names across vis windows.
//
//  Notes:
//    This is a singleton and not a colleague for a few reasons:
//     The UpdatePlotList method does not provide the right level of info
//     for the desired functionality. We need to track when plots go inactive
//     across windows, and keep a reference count for each of the used database
//     paths. To do this the necessary granularity is at the AddPlot/RemovePlot
//     level of VisWinPlots.
//
//     The database paths must be updated before the VisWinLegends executes
//     its UpdatePlotList. I was not sure if that order was guaranteed by
//     the current colleague setup.
//
//
//  Programmer: Cyrus Harrison
//  Creation:   Sunday June 18, 2007
//
//  Modifications:
//
//    Cyrus Harrison, Tue Sep 25 09:41:20 PDT 2007
//    Added support for directory and smart directory options,
//    added GetCommonPath and removed GetUniquePath.
//
// ****************************************************************************

class VisWinPathTracker
{
  public:

    // ***********************************************************************
    //  Class: VisWinPathTracker::Entry
    //
    //  Purpose:
    //    Holds reference count and path info for currently active paths.
    //
    //  Programmer: Cyrus Harrison
    //  Creation:   Sunday June 18, 2007
    //
    //  Modifications:
    //
    //    Cyrus Harrison, Tue Sep 25 09:41:20 PDT 2007
    //    Added support for directory and smart directory options
    //
    // ***********************************************************************

    class Entry
    {
      public:
                       Entry();
                       Entry(const std::string &path);
        virtual       ~Entry();

        void           IncrementRefCount() { refCount++; }
        bool           DecrementRefCount() { refCount--; return refCount > 0; }

        std::string   &GetFileName()       { return fileName; }
        std::string   &GetDirectory()      { return directory; }
        std::string   &GetPath()           { return fullPath; }
        std::string   &GetSmartPath()      { return smartPath; }
        std::string   &GetSmartDirectory() { return smartDir; }
        void           SetSmartPath(const std::string &spath)
                                           { smartPath = spath; }
        void           SetSmartDirectory(const std::string &sdir)
                                            { smartDir = sdir; }
      private:
        int            refCount;
        std::string    fileName;
        std::string    directory;
        std::string    fullPath;
        std::string    smartPath;
        std::string    smartDir;
    };

    virtual                     ~VisWinPathTracker();
    static VisWinPathTracker    *Instance();
    static void                  DeleteInstance();

    void                         AddPath(const std::string &path);
    void                         RemovePath(const std::string &path);

    std::string                  GetFileName(const std::string &path);
    std::string                  GetDirectory(const std::string &path);
    std::string                  GetSmartPath(const std::string &path);
    std::string                  GetSmartDirectory(const std::string &path);

    void                         UpdatePaths();

  protected:
                                 VisWinPathTracker();

  private:
    static std::string           GetSubPath(const std::string &,int);
    static std::string           GetCommonPath(stringVector &);
    
    static VisWinPathTracker    *instance;
    std::map<std::string,Entry>  entires;
};

