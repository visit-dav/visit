/*


                          Copyright 2000 - 2004.
               The Regents of the University of California.
                           All rights reserved.

This work was  produced at the  University of California,  Lawrence  Livermore
National Laboratory  (UC LLNL) under contract  no. W-7405-ENG-48 (Contract 48)
between the U.S. Department of Energy (DOE)  and The Regents of the University
of California (University) for the operation of UC LLNL. Copyright is reserved
to the University for purposes of  controlled dissemination, commercialization
through formal licensing, or other disposition under terms of Contract 48; DOE
policies, regulations and orders; and U.S. statutes. The rights of the Federal
Government are reserved  under Contract 48 subject to  the restrictions agreed
upon by the DOE and University as allowed under DOE Acquisition Letter 97-1.


                                DISCLAIMER

This  work was  prepared as an  account of work sponsored  by an agency of the
United  States  Government.  Neither  the  United  States  Government  nor the
University  of California  nor any of  their employees,  makes  any  warranty,
express  or implied,  or  assumes  any liability  or  responsibility  for  the
accuracy,  completeness, or usefulness of any information, apparatus, product,
or process disclosed, or represents that its use would not infringe privately-
owned rights.  Reference herein  to any specific commercial products, process,
or  service by  trade  name, trademark,  manufacturer  or  otherwise  does not
necessarily constitute or  imply its endorsement, recommendation,  or favoring
by the United States Government or the University of California. The views and
opinions of authors expressed herein do not necessarily state or reflect those
of the United States Government or the University of California, and shall not
be used for advertising or product endorsement purposes.


                          LICENSING REQUIREMENTS

Permission is hereby  granted to use and  copy this software and documentation
for internal noncommercial purposes only, provided that 1) the above copyright
notice and disclaimer appear in all  copies of the software and documentation,
and 2) all UC LLNL identification in the user interface remains unchanged. Any
use,  reproduction,   modification,  or  distribution   of  this  software  or
documentation for commercial  purposes requires a license from the University.
Contact:  Lawrence Livermore National Laboratory,  Industrial Partnerships and
Commercialization Office, P.O. Box 808, L-795, Livermore, CA 94551.
*/

// ************************************************************************* //
//                                DebugStream.h                              //
// ************************************************************************* //

#ifndef RCDEBUG_STREAM_H
#define RCDEBUG_STREAM_H


#include <iostream>
#include <fstream>
#include <vector>
#include <signal.h>

namespace rclib {
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
// ****************************************************************************
using namespace std; 
int get_debug_level(void);
class  DebugStream : public std::ostream {
  public:
                   DebugStream(int level_);
                  ~DebugStream();

    operator       bool()   {
      return enabled;
    };
    void           open(const char *progname);
    void           close();
    static void    Initialize(const char *, int);
    bool Enabled(void) { return enabled; }
  private:
    class DebugStreamBuf : public std::streambuf
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
        std::ofstream                             *out;
    };


    // the streambuf used for this ostream
    DebugStreamBuf   buf;
    // the level of this DebugStream
    int              level;
    // true if enabled
    bool             enabled;
};
extern  DebugStream debug1_real;
extern  DebugStream debug2_real;
extern  DebugStream debug3_real;
extern  DebugStream debug4_real;
extern  DebugStream debug5_real;
} // end namespace rclib 

#ifdef DEBUG_TO_STDERR 
#define rcdebug1 if (!rclib::debug1_real.Enabled()) ; else cerr
#define rcdebug2 if (!rclib::debug2_real.Enabled()) ; else cerr
#define rcdebug3 if (!rclib::debug3_real.Enabled()) ; else cerr
#define rcdebug4 if (!rclib::debug4_real.Enabled()) ; else cerr
#define rcdebug5 if (!rclib::debug5_real.Enabled()) ; else cerr
#else
#define rcdebug1 if (!rclib::debug1_real) ; else rclib::debug1_real
#define rcdebug2 if (!rclib::debug2_real) ; else rclib::debug2_real
#define rcdebug3 if (!rclib::debug3_real) ; else rclib::debug3_real
#define rcdebug4 if (!rclib::debug4_real) ; else rclib::debug4_real
#define rcdebug5 if (!rclib::debug5_real) ; else rclib::debug5_real
#endif

#endif


