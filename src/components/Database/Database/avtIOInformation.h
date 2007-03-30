// ************************************************************************* //
//                           avtIOInformation.h                              //
// ************************************************************************* //

#ifndef AVT_IO_INFORMATION_H
#define AVT_IO_INFORMATION_H
#include <database_exports.h>


#include <vector>


typedef std::vector<std::vector<int> > HintList;


// ****************************************************************************
//  Class: avtIOInformation
//
//  Purpose:
//      Contains information needed for I/O, including hints that will give
//      better performance and restrictions for non-global filesystems (the
//      latter is currently unimplemented).
//
//  Programmer: Hank Childs
//  Creation:   May 11, 2001
//
//  Modifications:
//    Jeremy Meredith, Tue Sep 18 15:36:47 PDT 2001
//    Added nDomains.
//
// ****************************************************************************

class DATABASE_API avtIOInformation
{
  public:
                               avtIOInformation();

    void                       AddHints(HintList &);
    const HintList            &GetHints(void) const  { return hints; };
    void                       SetNDomains(int);
    int                        GetNDomains()  const  { return nDomains; }

  protected:
    int                        nDomains;
    HintList                   hints;
};


#endif


