/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
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
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

// ************************************************************************* //
//                            NamingConvention.h                             //
// ************************************************************************* //

#ifndef NAMING_CONVENTION_H
#define NAMING_CONVENTION_H
#include <utility_exports.h>


// ****************************************************************************
//  Class: NamingConvention
//
//  Purpose:
//      Handles the details of parsing file naming conventions in an
//      encapsulated way for the preprocessor.
//
//  Note:       Each concrete subtype should define the static method
//              IsFormat and be reflected in 
//              NamingConvention::DetermineNamingConvention.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 13:39:55 PDT 2000
//    Rewrote to reduce coding for derived types.  Added static method
//    DetermineNamingConvention.
//
//    Hank Childs, Wed Sep 13 09:23:28 PDT 2000
//    Renamed NamingConvention from Code.
//
// ****************************************************************************

class UTILITY_API NamingConvention
{
  public:
    virtual                  ~NamingConvention();

    char                     *GetFile(int, int);
    char                     *GetRootFile(int);
    int                       GetStatesN() { return stateTypeN; };
    void                      GetState(char *, int);
   
    static NamingConvention  *DetermineNamingConvention(const char * const *, 
                                                        int);

    void                      PrintSelf();

  protected:
                              NamingConvention(const char * const *, int);

    void                      AssignState(int, const char *, bool = false);

    // A copy of the input files.
    char                    **fileList;
 
    // The number of files in fileList.
    int                       fileListN;

    // An array of size totalFiles that identifies which state each file 
    // falls in.
    int                      *stateType;

    // A list of all of the state names
    char                    **stateName;
  
    // The number of different kinds of state types.  This is NOT the size 
    // of the array stateType.
    int                       stateTypeN;

    // The portion common to all file names
    char                     *common;

    // An array that indicates whether that file is a root file.
    bool                     *rootFile;

    // An array that indicates whether the file is a valid file (some root
    // files are not).
    bool                     *validFile;
};
 

// ****************************************************************************
//  Class: Ares
//
//  Purpose:
//      A concrete type derived from NamingConvention, this will take care of 
//      file processing from the Ares code.
//
//  Programmer: Hank Childs
//  Creation:   December 6, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 15:47:57 PDT 2000
//    Removed all methods but IsFormat and added constructor to take place
//    of IdentifyStates.  Base type now does all work previously captured
//    in virtual functions.
//
// ****************************************************************************

class UTILITY_API Ares : public NamingConvention
{
  public:
                           Ares(const char * const *,const char * const *,int);
    static bool            IsFormat(const char * const *, int);
};


// ****************************************************************************
//  Class: Ale3D
//
//  Purpose:
//      A concrete type derived from NamingConvention, this will take care of 
//      parsing the file names into states and files within a state if the 
//      files follow the Ale3D naming convention.
//
//  Programmer: Hank Childs
//  Creation:   June 15, 2000
//
// ****************************************************************************

class UTILITY_API Ale3D : public NamingConvention
{
  public:
                        Ale3D(const char * const *, const char * const *, int);
    static bool         IsFormat(const char * const *, int);
};


// ****************************************************************************
//  Class: SingleFile
//
//  Purpose:
//      A concrete type derived from NamingConvention, this will take care of 
//      processing a single, stand-alone file.
//
//  Programmer: Hank Childs
//  Creation:   December 19, 1999
//
//  Modifications:
//
//    Hank Childs, Thu Jun 15 15:47:57 PDT 2000
//    Removed all methods but IsFormat and added constructor to take place
//    of IdentifyStates.  Base type now does all work previously captured
//    in virtual functions.
//
// ****************************************************************************

class UTILITY_API SingleFile : public NamingConvention
{
  public:
                   SingleFile(const char * const *, const char * const *, int);
    static bool    IsFormat(const char * const *, int);
};


// ****************************************************************************
//  Class: FileSequence
//
//  Purpose:
//      A concrete type derived from NamingConvention, this will take care of
//      processing a sequence of files where each file represents one state.
//
//  Programmer: Hank Childs
//  Creation:   August 30, 2000
//
// ****************************************************************************

class UTILITY_API FileSequence : public NamingConvention
{
  public:
                 FileSequence(const char * const *, const char * const *, int);
    static bool  IsFormat(const char * const *, int);
};


// ****************************************************************************
//  Class: Exodus
//
//  Purpose:
//      A concrete type derived from NamingConvention that handles a set of 
//      (Silo) Exodus files.
//
//  Programmer: Hank Childs
//  Creation:   September 6, 2000
//
// ****************************************************************************

class UTILITY_API Exodus : public NamingConvention
{
  public:
                       Exodus(const char * const *, const char * const *, int);
    static bool        IsFormat(const char * const *, int);
};


// ****************************************************************************
//  Class: SiloObj
//
//  Purpose:
//      A concrete type derived from NamingConvention that handles SiloObj 
//      files.
//
//  Programmer: Hank Childs
//  Creation:   September 13, 2000
//
// ****************************************************************************

class UTILITY_API SiloObj : public NamingConvention
{
  public:
                      SiloObj(const char * const *, const char * const *, int);
    static bool       IsFormat(const char * const *, int);
};


#endif


