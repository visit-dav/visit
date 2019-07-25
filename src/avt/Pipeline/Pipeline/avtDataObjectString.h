// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtDataObjectString.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_OBJECT_STRING_H
#define AVT_DATA_OBJECT_STRING_H

#include <pipeline_exports.h>


#include <vector>


// ****************************************************************************
//  Class: avtDataObjectString
//
//  Purpose:
//      A string that a data object can be written to.  This class is not much 
//      more than the STL version (with significantly restricted functionality).
//      It exists because the assembly of the data object string became a
//      serious bottleneck due to the append method.  Re-writing the routines
//      without the append is not worthwhile, so having a class that is less 
//      append-sensitive seems like the best solution.
//
//  Programmer: Hank Childs
//  Creation:   May 25, 2001
//
//  Modifications:
//
//    Hank Childs, Sun Sep 16 16:04:39 PDT 2001
//    Re-worked to prevent copying of strings.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
// ****************************************************************************

class PIPELINE_API avtDataObjectString
{
  public:
                              avtDataObjectString();
    virtual                  ~avtDataObjectString();

    typedef enum
    {
        DATA_OBJECT_STRING_DOES_NOT_OWN_REFERENCE,
        DATA_OBJECT_STRING_OWNS_REFERENCE_AFTER_CALL,
        DATA_OBJECT_STRING_SHOULD_MAKE_COPY
    } APPEND_ACTION_ITEM;
        
    void                      Append(char *, int, APPEND_ACTION_ITEM);

    int                       GetNStrings(void) const;
    void                      GetString(int, char *&, int &);
    int                       GetTotalLength(void) const;

    void                      GetWholeString(char *&, int &len);
    char                     *GetWholeString(int &len) const;

  protected:
    std::vector<char *>       strs;
    std::vector<int>          lens;
    std::vector<bool>         ownString;
    char                     *wholeString;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtDataObjectString(const avtDataObjectString &) {;};
    avtDataObjectString &operator=(const avtDataObjectString &) 
                                                            { return *this; };
};


#endif


