// ************************************************************************* //
//                               TimingsManager.C                            //
// ************************************************************************* //

#include <TimingsManager.h>
#include <snprintf.h>

#include <map>
#include <float.h>
#include <visitstream.h>
#include <time.h>
#include <stdio.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <DebugStream.h>


#if !defined(_WIN32)
#ifndef HAVE_FTIME_PROTOTYPE
extern "C" {
int ftime(struct timeb *);
}
#endif
#endif


TimingsManager   *visitTimer = TimingsManager::Initialize("default");


// ****************************************************************************
//  Function:GetCurrentTimeInfo 
//
//  Programmer: Mark C. Miller
//  Creation:   April 20, 2004 
//
// ****************************************************************************

static void
GetCurrentTimeInfo(struct TIMEINFO &timeInfo)
{
#if defined(_WIN32)
    _ftime(&timeInfo);
#elif defined(__APPLE__)
    gettimeofday(&timeInfo, 0);
#else
    ftime(&timeInfo);
#endif
}

// ****************************************************************************
//  Method: TimingsManager::TimeSinceLastCall
//
//  Purpose:
//      Computes time since last called with same key 
// 
//  Programmer: Mark C. Miller 
//  Creation:   April 20, 2004 
//
// ****************************************************************************
double
TimingsManager::TimeSinceLastCall(const char *file, int line)
{
    static std::map<std::string, TIMEINFO> keyMap;

    // a way to re-initilize this static function's static member if
    // ever needed
    if ((file == 0) && (line < 0))
    {
        keyMap.clear();
        return 0.0;
    }

    struct TIMEINFO currentTime;
    GetCurrentTimeInfo(currentTime);

    char key[256];
    SNPRINTF(key, sizeof(key), "%s#%d", file, line);
    if (keyMap.find(key) == keyMap.end())
    {
        keyMap[key] = currentTime;
        return DBL_MAX;
    }
    else
    {
        struct TIMEINFO lastTime = keyMap[key];
        keyMap[key] = currentTime;
        return DiffTime(lastTime, currentTime);
    }
}

// ****************************************************************************
//  Method: TimingsManager constructor
//
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:17:24 PST 2001
//    I added the ability to enable and disable timings.
//
//    Jeremy Meredith, Fri Oct  4 16:47:47 PDT 2002
//    Added number of current timings.
//
// ****************************************************************************

TimingsManager::TimingsManager()
{
    filename          = ".timings";
    openedFile        = false;
    numTimings        = 0;
    numCurrentTimings = 0;
    enabled           = false;
}


// ****************************************************************************
//  Method: TimingsManager::Initialize
//
//  Purpose:
//      A static method that initializes the visitTimer global object.  It
//      determines if the object should be of the MPI type of the UNIX type.
//
//  Returns:      A pointer to the visitTimer object.  This is returned
//                primarily so the global reference can be initialized.
//
//  Arguments:
//      fname     The file name to use.
//
//  Programmer:   Hank Childs
//  Creation:     March 10, 2001
//
//  Modifications:
//    Brad Whitlock, Thu Mar 14 12:24:34 PDT 2002
//    Renamed UnixTimingsManager to SystemTimingsManager.
//
// ****************************************************************************

TimingsManager *
TimingsManager::Initialize(const char *fname)
{
#ifdef PARALLEL
    visitTimer = new MPITimingsManager;
#else
    visitTimer = new SystemTimingsManager;
#endif

    visitTimer->SetFilename(fname);

    return visitTimer;
}


// ****************************************************************************
//  Method: TimingsManager::SetFilename
//
//  Purpose:
//      Sets the filename that the timings manager should dump out to.
//
//  Arguments:
//      fname   The new filename.
//
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
// ****************************************************************************

void
TimingsManager::SetFilename(const std::string &fname)
{
    filename = fname + ".timings";
}


// ****************************************************************************
//  Method: TimingsManager::Enable
//
//  Purpose:
//      Enables the timing manager.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
TimingsManager::Enable(void)
{
    enabled = true;
}


// ****************************************************************************
//  Method: TimingsManager::Disable
//
//  Purpose:
//      Disables the timing manager.
//
//  Programmer: Eric Brugger
//  Creation:   November 5, 2001
//
// ****************************************************************************

void
TimingsManager::Disable(void)
{
    enabled = false;
}


// ****************************************************************************
//  Method: TimingsManager::StartTimer
//
//  Purpose:
//      Starts the timer for a specific symbol.
//
//  Returns:     The index of the timing started by this function call.
//               It returns -1 if timings are disabled.
//
//  Programmer:  Hank Childs
//  Creation:    March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:17:24 PST 2001
//    I added the ability to enable and disable timings.
//
//    Jeremy Meredith, Fri Oct  4 16:47:47 PDT 2002
//    Added number of current timings.
//
// ****************************************************************************

int
TimingsManager::StartTimer(void)
{
    //
    // Return if timings disabled.
    //
    if (!enabled)
    {
        return -1;
    }

    PlatformStartTimer();
    numTimings        += 1;
    numCurrentTimings += 1;
    return numTimings-1;
}


// ****************************************************************************
//  Method: TimingsManager::StopTimer
//
//  Purpose:
//      Stops the timer for the indicated index and dumps it to a file.
//
//  Arguments:
//      index    The index of the timer.
//      summary  A string that summarizes what we are timing.
//
//  Programmer:  Hank Childs
//  Creation:    March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:17:24 PST 2001
//    I added the ability to enable and disable timings.
//
//    Brad Whitlock, Tue Apr 16 16:56:48 PST 2002
//    Changed code so it behaves using MSVC++.
//
//    Brad Whitlock, Fri Sep 20 16:13:51 PST 2002
//    I made it return the time.
//
//    Jeremy Meredith, Fri Oct  4 16:47:47 PDT 2002
//    Added number of current timings and indentation based on it.
//
// ****************************************************************************

double
TimingsManager::StopTimer(int index, const std::string &summary)
{
    double t = 0.;

    if (enabled)
    {
        if (index < 0 || index >= numTimings)
        {
            debug1 << "Invalid timing index (" << index << ") was specified."
                   << endl;
        }
        else
        {
            numCurrentTimings -= 1;

            t = PlatformStopTimer(index);
            times.push_back(t);
            char indented[1000];
            sprintf(indented, "%*s%s", 3*numCurrentTimings, " ", summary.c_str());
            summaries.push_back(indented);
        }
    }

    return t;
}


// ****************************************************************************
//  Method: TimingsManager::DumpTimings
//
//  Purpose:
//      Dumps out all of the completed timings since the last dump.
//
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:17:24 PST 2001
//    I added the ability to enable and disable timings.
//
// ****************************************************************************

void
TimingsManager::DumpTimings(void)
{
    //
    // Return if timings disabled.
    //
    if (!enabled)
    {
        return;
    }

    ofstream ofile;

    if (!openedFile)
    {
        //
        // We haven't opened up the file before, so blow it away from previous
        // times the program was run.
        //
        ofile.open(filename.c_str());
        openedFile = true;
    }
    else
    {
        //
        // We have already dumped the timings once while running this program,
        // so just append to that file.
        //
        ofile.open(filename.c_str(), ios::app);
    }

    if (ofile.fail())
    {
        debug1 << "Unable to open file " << filename.c_str()
               << " to dump timings information." << endl;
    }
    else
    {
        DumpTimings(ofile);
    }
}


// ****************************************************************************
//  Method: TimingsManager::DumpTimings
//
//  Purpose:
//      Dumps the timings to an ostream.
//
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
//  Modifications:
//    Eric Brugger, Mon Nov  5 14:17:24 PST 2001
//    I added the ability to enable and disable timings.
//
// ****************************************************************************

void
TimingsManager::DumpTimings(ostream &out)
{
    //
    // Return if timings disabled.
    //
    if (!enabled)
    {
        return;
    }

    int numT = times.size();
    for (int i = 0 ; i < numT ; i++)
    {
        out << "Timing for " << summaries[i].c_str() << " took " << times[i] << endl;
    }

    //
    // The next time we dump timings, don't use these values.
    //
    times.clear();
    summaries.clear();
}

// ****************************************************************************
//  Method: TimingsManager::DiffTime
//
//  Purpose:
//      Compute the difference in time from platform specific time structures 
//
//  Returns:     The elapsed time in seconds as a double.
//  
//  Programmer:  Hank Childs 
//  Creation:    March 10, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Apr 20 21:12:05 PDT 2004
//    Relocated from PlatformStopTimer
//
// ****************************************************************************
double
TimingsManager::DiffTime(const struct TIMEINFO &startTime,
                         const struct TIMEINFO &endTime)
{
#if defined(__APPLE__)

    double seconds = double(endTime.tv_sec - startTime.tv_sec) + 
                     double(endTime.tv_usec - startTime.tv_usec) / 1000000.;
                     
    return seconds;

#else

    // 
    // Figure out how many milliseconds between start and end times 
    //
    int ms = (int) difftime(endTime.time, startTime.time);
    if (ms == 0)
    {
        ms = endTime.millitm - startTime.millitm;
    }
    else
    {
        ms =  ((ms - 1) * 1000);
        ms += (1000 - startTime.millitm) + endTime.millitm;
    }

    return (ms/1000.);

#endif
}


// ****************************************************************************
//  Method: SystemTimingsManager::PlatformStartTimer
//
//  Purpose:
//      Does a "start timer" for this platform -- unix.
// 
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
//  Modifications:
//    Hank Childs, Tue Apr 10 18:08:54 PDT 2001
//    Re-implemented to get sub-second precision.
//
//    Brad Whitlock, Thu Mar 14 12:24:23 PST 2002
//    Made it work on Windows.
//
//    Brad Whitlock, Mon May 19 12:37:44 PDT 2003
//    I made it work on MacOS X.
//
//    Mark C. Miller Wed Apr 21 12:42:13 PDT 2004
//    I made it use GetCurrentTime
//
// ****************************************************************************

void
SystemTimingsManager::PlatformStartTimer(void)
{
    struct TIMEINFO t;
    GetCurrentTimeInfo(t);
    values.push_back(t);
}

// ****************************************************************************
//  Method: SystemTimingsManager::PlatformStopTimer
//
//  Purpose:
//      Does a "stop timer" for this platform -- unix.  Also determines what
//      the elapsed time is in seconds and returns that as a double.
//
//  Arguments:
//      index    The index of the timing.  This has already been checked
//               for validity.
//
//  Returns:     The elapsed time in seconds as a double.
//  
//  Programmer:  Hank Childs
//  Creation:    March 10, 2001
//
//  Modifications:
//    Hank Childs, Tue Apr 10 18:08:54 PDT 2001
//    Re-implemented to get sub-second precision.
//
//    Brad Whitlock, Thu Mar 14 12:24:23 PST 2002
//    Added a Windows implementation.
//
//    Brad Whitlock, Mon May 19 12:37:58 PDT 2003
//    I made it work on MacOS X.
//
//    Mark C. Miller, Tue Apr 20 21:12:05 PDT 2004
//    Moved bulk of implementation to DiffTime so code could be shared
//    with TOATimer
//
// ****************************************************************************

double
SystemTimingsManager::PlatformStopTimer(int index)
{
    struct TIMEINFO endTime;
    GetCurrentTimeInfo(endTime);
    return DiffTime(values[index], endTime);
}

// ****************************************************************************
//  Method: MPITimingsManager::PlatformStartTimer
//
//  Purpose:
//      Does a "start timer" for a platform that supports MPI.
// 
//  Programmer: Hank Childs
//  Creation:   March 10, 2001
//
// ****************************************************************************

void
MPITimingsManager::PlatformStartTimer(void)
{
    double t = 0.;
#ifdef PARALLEL
    t = MPI_Wtime();
#endif
    values.push_back(t);
}


// ****************************************************************************
//  Method: MPITimingsManager::PlatformStopTimer
//
//  Purpose:
//      Does a "stop timer" for this platforms that support MPI.  Also
//      determines what the elapsed time is in seconds and returns that as a
//      double.
//
//  Arguments:
//      index    The index of the timing.  This has already been checked
//               for validity.
//
//  Returns:     The elapsed time in seconds as a double.
//  
//  Programmer:  Hank Childs
//  Creation:    March 10, 2001
//
// ****************************************************************************

double
MPITimingsManager::PlatformStopTimer(int index)
{
    double t = 0.;
    double tick = 1.;
#ifdef PARALLEL
    t = MPI_Wtime();
    // It is unclear to me why a tick is off by a factor of 10^6.
    tick = MPI_Wtick()*10e5;
#endif
    return (t - values[index])*tick;
}


