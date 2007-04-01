// ************************************************************************* //
//                              TimingsManager.h                             //
// ************************************************************************* //

#ifndef TIMINGS_MANAGER_H
#define TIMINGS_MANAGER_H

#include <iostream.h>
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
#define DELTA_TOA_HERE TimingsManager::TimeSinceLastCall(__FILE__, __LINE__)

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
// ****************************************************************************

class MISC_API TimingsManager
{
  public:
                               TimingsManager();
    virtual                   ~TimingsManager() {;};

    static double              TimeSinceLastCall(const char *file, int line);

    static TimingsManager     *Initialize(const char *);
    void                       SetFilename(const std::string &s);

    void                       Enable(void);
    void                       Disable(void);

    int                        StartTimer(void);
    double                     StopTimer(int, const std::string &);

    static double              DiffTime();

    void                       DumpTimings(void);
    void                       DumpTimings(ostream &);

  protected:
    std::string                filename;
    bool                       openedFile;
    int                        numCurrentTimings;
    int                        numTimings;
    bool                       enabled;

    std::vector<double>        times;
    std::vector<std::string>   summaries;

    static double              DiffTime(const struct TIMEINFO &startTime,
                                        const struct TIMEINFO &endTime);

    virtual void               PlatformStartTimer(void) = 0;
    virtual double             PlatformStopTimer(int) = 0;
};

class MISC_API SystemTimingsManager : public TimingsManager
{
  public:
                               SystemTimingsManager() {;};
    virtual                   ~SystemTimingsManager() {;};

  protected:
    std::vector<TIMEINFO>      values;
    virtual void               PlatformStartTimer(void);
    virtual double             PlatformStopTimer(int);
};

class MISC_API MPITimingsManager : public TimingsManager
{
  public:
                               MPITimingsManager() {;};
    virtual                   ~MPITimingsManager() {;};

  protected:
    std::vector<double>        values;

    virtual void               PlatformStartTimer(void);
    virtual double             PlatformStopTimer(int);
};


MISC_API extern TimingsManager   *visitTimer;


#endif


