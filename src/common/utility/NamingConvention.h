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


