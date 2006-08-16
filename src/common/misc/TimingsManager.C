/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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
//                               TimingsManager.C                            //
// ************************************************************************* //

#include <TimingsManager.h>
#include <snprintf.h>
#include <visit-config.h>

#include <map>
#include <float.h>
#include <visitstream.h>
#include <time.h>
#include <stdio.h>

#ifdef PARALLEL
  #include <mpi.h>
#endif

#include <DebugStream.h>


#if defined(_WIN32)
#include <direct.h>
#else
#include <unistd.h>
#ifndef HAVE_FTIME_PROTOTYPE
extern "C" {
int ftime(struct timeb *);
}
#endif
#endif


TimingsManager   *visitTimer = NULL;

static struct TIMEINFO initTimeInfo;

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
//  Function: TimingsManager::TimeSinceLine
//
//  Purpose: Computes time since last called with same key. When the key is
//  specified as __FILE__ and __LINE__, this function is useful in computing
//  the time since last arrival at a given source code line.
// 
//  Programmer: Mark C. Miller 
//  Creation:   April 20, 2004 
//
//  Modifications:
//
//    Mark C. Miller, Thu Nov 10 18:46:12 PST 2005
//    Changed the name to better represent meaning of function.
//
// ****************************************************************************
double
TimingsManager::TimeSinceLine(const char *file, int line)
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
//  Function: TimingsManager::TimeSinceInit
//
//  Purpose: Computes time since initialize was called. 
// 
//  Programmer: Mark C. Miller 
//  Creation:   November 11, 2005
//
// ****************************************************************************

double
TimingsManager::TimeSinceInit()
{
    struct TIMEINFO currentTimeInfo;
    GetCurrentTimeInfo(currentTimeInfo);
    return DiffTime(initTimeInfo, currentTimeInfo);
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
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Eliminated numTimings
//
// ****************************************************************************

TimingsManager::TimingsManager()
{
    filename          = ".timings";
    openedFile        = false;
    numCurrentTimings = 0;
    enabled           = false;
    withholdOutput    = false;
    outputAllTimings  = false;
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
//    Hank Childs, Tue Mar 22 16:13:20 PST 2005
//    Fix memory leak.
//
//    Hank Childs, Mon Mar 28 09:42:06 PST 2005
//    Do not initialize the visit timer with a static constructor, because
//    we don't know the name of the process yet.
//
//    Mark C. Miller, Fri Nov 11 09:45:42 PST 2005
//    Added initialization of initTimeInfo
//
//    Brad Whitlock, Thu Dec 15 09:52:05 PDT 2005
//    I made the filename used for the timings include the path.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Moved code to set up filename into SetFilename method
// ****************************************************************************

TimingsManager *
TimingsManager::Initialize(const char *fname)
{
    if (visitTimer != NULL)
        return visitTimer;

#ifdef PARALLEL
    visitTimer = new MPITimingsManager;
#else
    visitTimer = new SystemTimingsManager;
    GetCurrentTimeInfo(initTimeInfo);
#endif

    visitTimer->SetFilename(fname);

    return visitTimer;
}

// ****************************************************************************
//  Method: TimingsManager::Finalize
//
//  Purpose: Terminates the TimingsManager
//
//  Programmer: Mark C. Miller 
//  Creation:   July 26, 2006 
//
// ****************************************************************************

void
TimingsManager::Finalize()
{
    if (visitTimer)
    {
        visitTimer->StopAllUnstoppedTimers();
        visitTimer->DumpTimings();
        delete visitTimer;
        visitTimer = 0;
    }
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
//  Modifications:
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Moved bulk of code setting filename from constructor to here
// ****************************************************************************

void
TimingsManager::SetFilename(const std::string &fname)
{
    if (fname == "")
        return;

    //
    // Make sure that the filename includes the whole path so all .timings
    // files will be written to the right directory.
    //
    if (fname[0] != SLASH_CHAR)
    {
        char currentDir[1024];
#if defined(_WIN32)
        _getcwd(currentDir,1023);
#else
        getcwd(currentDir,1023);
#endif
        currentDir[1023]='\0';
        std::string filenameTmp(currentDir);
        if(filenameTmp[filenameTmp.size()-1] != SLASH_CHAR)
            filenameTmp += SLASH_STRING;
        filename = filenameTmp + fname + ".timings"; 
    }
    else
    {
        filename = fname + ".timings";
    }
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
//  Method: TimingsManager::WithholdOutput
//
//  Purpose:
//      Tells the timings manager not to output the timings until 
//      OutputAllTimings is called.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
TimingsManager::WithholdOutput(bool v)
{
    withholdOutput = v;
}


// ****************************************************************************
//  Method: TimingsManager::OutputAllTimings
//
//  Purpose:
//      Tells the timings manager to truly output all the timings, even if
//      we are withholding them.
//
//  Programmer: Hank Childs
//  Creation:   March 27, 2005
//
// ****************************************************************************

void
TimingsManager::OutputAllTimings(void)
{
    outputAllTimings = true;
    DumpTimings();
    outputAllTimings = false;
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
//    Mark C. Miller, Thu Nov  3 16:59:41 PST 2005
//    Added ability to force acquisition of timing info even if we're not
//    logging timings to files
//
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Eliminated numTimings. Made code a little more robust.
// ****************************************************************************

int
TimingsManager::StartTimer(bool forced)
{
    if (!enabled && !forced)
        return -1;
    numCurrentTimings += 1;
    return PlatformStartTimer();
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
//    Mark C. Miller, Wed Nov  2 09:07:05 PST 2005
//    Added code to force return of timing info even if logging is not enabled
//
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Eliminated numTimings. Moved error message to PlatformStopTimer. Made
//    a little more robust.
// ****************************************************************************

double
TimingsManager::StopTimer(int index, const std::string &summary, bool forced)
{
    double t = 0.;

    if (enabled || forced)
    {
        t = PlatformStopTimer(index);
        times.push_back(t);
        numCurrentTimings -= 1;
        if (enabled)
        {
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
//    Hank Childs, Sun Mar 27 13:38:03 PST 2005
//    Do not output if we are withholding timings.
//
//    Mark C. Miller, Wed Aug  2 19:58:44 PDT 2006
//    Added test for emtpy filename. Added missing call to close ofile
// ****************************************************************************

void
TimingsManager::DumpTimings(void)
{
    //
    // Return if timings disabled.
    //
    if (!enabled)
        return;
    if (withholdOutput && !outputAllTimings)
        return;
    if (filename == "")
    {
        debug1 << "Attempted to DumpTimings without setting name of file" << endl; 
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
        ofile.close();
    }
}


// ****************************************************************************
//  Method: TimingsManager::StopAllUnstoppedTimers
//
//  Purpose: stops all the timers that have not already been stopped
//
//  Programmer: Mark C. Miller 
//  Creation:   August 4, 2006
// ****************************************************************************

void
TimingsManager::StopAllUnstoppedTimers()
{
    //
    // Stop all un-stopped timers
    //
    for (int i = times.size(); i < visitTimer->GetNValues(); i++)
        visitTimer->StopTimer(i, "Unknown");
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
//    Hank Childs, Sun Mar 27 13:38:03 PST 2005
//    Do not output if we are withholding timings.
//
//    Mark C. Miller, Thu Aug  3 13:33:20 PDT 2006
//    Added call to ClearValues()
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
    if (withholdOutput && !outputAllTimings)
        return;

    int numT = times.size();
    for (int i = 0 ; i < numT ; i++)
    {
        out << "Timing for " << summaries[i].c_str() << " took " << times[i] << endl;
    }

    //
    // The next time we dump timings, don't use these values.
    //
    ClearValues();
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
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Made it return current length of values array 
// ****************************************************************************

int
SystemTimingsManager::PlatformStartTimer(void)
{
    struct TIMEINFO t;
    GetCurrentTimeInfo(t);
    values.push_back(t);
    return values.size()-1;
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
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Moved debug message from StopTimer to here 
// ****************************************************************************

double
SystemTimingsManager::PlatformStopTimer(int index)
{
    if (index < 0 || index > values.size()-1)
    {
        debug1 << "Invalid timing index (" << index << ") specified." << endl;
        return 0.0;
    }

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
//  Modifications:
//
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Made it return current length of values array 
// ****************************************************************************

int
MPITimingsManager::PlatformStartTimer(void)
{
    double t = 0.;
#ifdef PARALLEL
    t = MPI_Wtime();
#endif
    values.push_back(t);
    return values.size()-1;
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
//  Modifications:
//
//    Mark C. Miller, Tue Aug 15 20:20:58 PDT 2006
//    Moved debug message from StopTimer to here 
// ****************************************************************************

double
MPITimingsManager::PlatformStopTimer(int index)
{
    if (index < 0 || index > values.size()-1)
    {
        debug1 << "Invalid timing index (" << index << ") specified." << endl;
        return 0.0;
    }

    double t = 0.;
    double tick = 1.;
#ifdef PARALLEL
    t = MPI_Wtime();
    // It is unclear to me why a tick is off by a factor of 10^6.
    tick = MPI_Wtick()*10e5;
#endif
    return (t - values[index])*tick;
}
