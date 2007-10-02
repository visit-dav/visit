/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

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

