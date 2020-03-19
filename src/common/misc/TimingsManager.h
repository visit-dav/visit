// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
#else
#    define TIMEINFO timeval
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
//    Dave Pugmire, Mon Feb 23 13:29:49 EST 2009
//    Added LookupTimer.
//    
//    Hank Childs, Sat Apr 11 23:41:27 CDT 2009
//    Add mode for NoOutputs.
//
//    Hank Childs, Sat Feb 20 16:45:48 CST 2010
//    Add method for getting the NeverOutput data member.
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
    void                       NoForcedTiming(bool v=true);
    bool                       Enabled() const {return enabled;};

    int                        StartTimer(bool force=false);
    double                     StopTimer(int, const std::string &, bool force=false);
    double                     LookupTimer(const std::string &);

    static double              DiffTime();

    void                       DumpTimings(void);
    void                       DumpTimings(ostream &);

    void                       WithholdOutput(bool);
    void                       NeverOutput(bool);
    bool                       GetNeverOutputValue(void) {return neverOutput;};
    void                       OutputAllTimings();

  protected:
    std::string                filename;
    bool                       openedFile;
    int                        numCurrentTimings;
    bool                       enabled;
    bool                       noForcedTiming;
    bool                       withholdOutput;
    bool                       neverOutput;
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
                                   { return static_cast<int>(values.size()); };

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
                                   { return static_cast<int>(values.size()); };

  protected:
    std::vector<double>        values;
    virtual int                PlatformStartTimer(void);
    virtual double             PlatformStopTimer(int);
};


MISC_API extern TimingsManager   *visitTimer;


#endif


