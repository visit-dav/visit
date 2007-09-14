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
//                              TimingsManager.h                             //
// ************************************************************************* //

#ifndef TIMINGS_MANAGER_H
#define TIMINGS_MANAGER_H

#include <visitstream.h>
#if defined(_WIN32)
#include <sys/timeb.h>
#else
#include <sys/time.h>
#include <sys/timeb.h>
#endif

#include <string>
#include <vector>
#include <misc_exports.h>

// useful macro for declaring time info structs
#if defined(_WIN32)
#    define TIMEINFO _timeb
#elif defined(__APPLE__)
#    define TIMEINFO timeval
#else
#    define TIMEINFO timeb
#endif

// useful macro for computing time of arrival at a particular line of code
#define DELTA_TOA_THIS_LINE TimingsManager::TimeSinceLine(__FILE__, __LINE__)
#define TOA_THIS_LINE TimingsManager::TimeSinceInit()

// ****************************************************************************
//  Class: TimingsManager
//
//  Purpose:
//      Allows other modules in VisIt to not worry about timings.
//
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:14:12 PST 2001
//    I added the methods Enable and Disable.
//
//    Brad Whitlock, Thu Mar 14 13:37:34 PST 2002
//    Added exports.
//
//    Brad Whitlock, Fri Sep 20 16:14:30 PST 2002
//    I made StopTimer return the time.
//
//    Jeremy Meredith, Fri Oct  4 16:48:26 PDT 2002
//    Added number of current timings and indented output based on that value.
//
//    Brad Whitlock, Mon May 19 12:36:50 PDT 2003
//    Added MacOS X code.
//
//    Mark C. Miller, Wed Apr 21 12:42:13 PDT 2004
//    I added static methods, TimeSinceLastArrival, DiffTime
//    I defined the TIMEINFO macro and restructured code to use it 
//
//    Mark C. Miller, Wed Nov  2 09:07:05 PST 2005
//    Added optional force argument to Start/Stop methods to permit
//    getting timer info in return value even if not logging to file
//
//    Mark C. Miller, Fri Nov 11 09:45:42 PST 2005
//    Added TimeSinceInit method, changed name of TimeSinceLastCall to
//    TimeSinceLine
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added Finalize method
//
//    Mark C. Miller, Thu Aug  3 13:33:20 PDT 2006
//    Added StopAllUnstoppedTimers, ClearValues and GetNValues()
//
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Eliminated numTimings. Fixed bug where DumpTimings would ClearValues
//    but not also reset numTimings.
// 
//    Hank Childs, Fri Sep 14 12:58:59 PDT 2007
//    Overhauled the handling of the "values" array, since it has been
//    mis-used for over a year and been giving incorrect results.
//
// ****************************************************************************

class MISC_API TimingsManager
{
  public:
                               TimingsManager();
    virtual                   ~TimingsManager() {;};

    static double              TimeSinceLine(const char *file, int line);
    static double              TimeSinceInit();

    static TimingsManager     *Initialize(const char *);
    static void                Finalize();
    void                       SetFilename(const std::string &s);

    void                       Enable(void);
    void                       Disable(void);
    bool                       Enabled() const {return enabled;};

    int                        StartTimer(bool force=false);
    double                     StopTimer(int, const std::string &, bool force=false);

    static double              DiffTime();

    void                       DumpTimings(void);
    void                       DumpTimings(ostream &);

    void                       WithholdOutput(bool);
    void                       OutputAllTimings();

  protected:
    std::string                filename;
    bool                       openedFile;
    int                        numCurrentTimings;
    bool                       enabled;
    bool                       withholdOutput;
    bool                       outputAllTimings;

    std::vector<double>        times;
    std::vector<std::string>   summaries;
    std::vector<bool>          usedEntry;

    static double              DiffTime(const struct TIMEINFO &startTime,
                                        const struct TIMEINFO &endTime);
    int                        FindFirstUnusedEntry(void);

    void                       StopAllUnstoppedTimers();

    virtual int                PlatformStartTimer(void) = 0;
    virtual double             PlatformStopTimer(int) = 0;
    virtual int                GetNValues() const = 0;
    virtual void               ClearValues() = 0;
};

class MISC_API SystemTimingsManager : public TimingsManager
{
  public:
                               SystemTimingsManager() {;};
    virtual                   ~SystemTimingsManager() {;};
    void                       ClearValues()
                                   { values.clear(); };
    int                        GetNValues() const
                                   { return values.size(); };

  protected:
    std::vector<struct TIMEINFO> values;
    virtual int                PlatformStartTimer(void);
    virtual double             PlatformStopTimer(int);
};

class MISC_API MPITimingsManager : public TimingsManager
{
  public:
                               MPITimingsManager() {;};
    virtual                   ~MPITimingsManager() {;};
    void                       ClearValues()
                                   { values.clear(); };
    int                        GetNValues() const
                                   { return values.size(); };

  protected:
    std::vector<double>        values;
    virtual int                PlatformStartTimer(void);
    virtual double             PlatformStopTimer(int);
};


MISC_API extern TimingsManager   *visitTimer;


#endif


