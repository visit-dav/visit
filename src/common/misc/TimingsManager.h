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
// ****************************************************************************

class MISC_API TimingsManager
{
  public:
                               TimingsManager();
    virtual                   ~TimingsManager() {;};

    static TimingsManager     *Initialize(const char *);
    void                       SetFilename(const std::string &s);

    void                       Enable(void);
    void                       Disable(void);

    int                        StartTimer(void);
    double                     StopTimer(int, const std::string &);

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

    virtual void               PlatformStartTimer(void) = 0;
    virtual double             PlatformStopTimer(int) = 0;
};

class MISC_API SystemTimingsManager : public TimingsManager
{
  public:
                               SystemTimingsManager() {;};
    virtual                   ~SystemTimingsManager() {;};

  protected:
#if defined(_WIN32)
    std::vector<_timeb>        values;
#elif defined(__APPLE__)
    std::vector<timeval>       values;
#else
    std::vector<timeb>         values;
#endif
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


