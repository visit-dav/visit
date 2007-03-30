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

    int                       GetNStrings(void);
    void                      GetString(int, char *&, int &);
    int                       GetTotalLength(void);

    void                      GetWholeString(char *&, int &);

  protected:
    std::vector<char *>       strs;
    std::vector<int>          lens;
    std::vector<bool>         ownString;
    char                     *wholeString;
};


#endif


