// ************************************************************************* //
//                                DebugStream.h                              //
// ************************************************************************* //

#ifndef DEBUG_STREAM_H
#define DEBUG_STREAM_H
#include <misc_exports.h>

#include <visitstream.h>
#include <vector>
#include <signal.h>
#include <stdlib.h>

// ****************************************************************************
//  Class:  DebugStream
//
//  Purpose:
//    An implementation of ostream designed for error and message logging.
//    A DebugStream has a "level" assigned to it as a priority (lower number
//    is higher priority).  Multiple DebugStreams can be created.  When
//    output is written to one DebugStream, it is simultaneously written to
//    all DebugStreams with a higher level.
//
//    In this specific implementation, five debug levels are used.  Only
//    the highest priority messages go to debug1, but every message will go
//    to debug5.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 16, 2000
//
//  Modified:
//    Jeremy Meredith, Fri Jun  1 17:18:03 PDT 2001
//    Added the ability to check if a DebugStream was enabled.
//
//    Brad Whitlock, Thu Mar 14 13:37:34 PST 2002
//    Added exports.
//
//    Jeremy Meredith, Thu Jan 16 11:40:29 PST 2003
//    Made debug streams macros so that we could skip the logic to 
//    do the formatting if the debug output was disabled.  Removed the
//    ability to delete a debug stream automatically on close.
//
//    Eric Brugger, Tue Aug  3 11:00:56 PDT 2004
//    Change the DebugStreamBuf member to be a pointered value instead of
//    a referenced value so that it works with the MIPSpro compiler.
//
// ****************************************************************************

class MISC_API DebugStream : public ostream
{
  public:
                   DebugStream(int level_);
                  ~DebugStream();

    operator       bool()   {return enabled;};
    void           open(const char *progname);
    void           close();
    static void    Initialize(const char *, int);
  private:
    class DebugStreamBuf : public streambuf
    {
      public:
                       DebugStreamBuf();
                      ~DebugStreamBuf();

        void           SetLevel(int level_);

        void           close();
        void           open(const char *filename_);
        int            put(int c);
        virtual int    overflow(int c);
        virtual int    underflow() { return 0; };
      private:
        // the priority of the current messages being logged
        static int                            curLevel;
        // the list of all buffers
        static std::vector<DebugStreamBuf*>   allBuffers;

        // the level of this DebugStreamBuf
        int                                   level;
        // the filename being to which to write the messages
        char                                  filename[256];
        // the ostream used to write the log file
        ofstream                             *out;
    };


    // the streambuf used for this ostream
    DebugStreamBuf  *buf;
    // the level of this DebugStream
    int              level;
    // true if enabled
    bool             enabled;
};

#define debug1 if (!debug1_real) ; else debug1_real
#define debug2 if (!debug2_real) ; else debug2_real
#define debug3 if (!debug3_real) ; else debug3_real
#define debug4 if (!debug4_real) ; else debug4_real
#define debug5 if (!debug5_real) ; else debug5_real

extern MISC_API DebugStream debug1_real;
extern MISC_API DebugStream debug2_real;
extern MISC_API DebugStream debug3_real;
extern MISC_API DebugStream debug4_real;
extern MISC_API DebugStream debug5_real;

#endif


