// ************************************************************************* //
//                                 avtTheater.h                              //
// ************************************************************************* //

#ifndef AVT_THEATER_H
#define AVT_THEATER_H

#include <plotter_exports.h>

#include <avtActor.h>
#include <avtBehavior.h>
#include <avtDrawable.h>

class     avtDataObjectInformation;


// ****************************************************************************
//  Class: avtTheater
//
//  Purpose:
//      The theater adds behavior to a drawable, which makes it an actor.
//
//  Programmer: Hank Childs
//  Creation:   December 22, 2000
//
//  Modifications:
//
//    Kathleen Bonnell, Fri Jul 12 16:10:49 PDT 2002 
//    Add another drawable to the input, for decorations.
//
//    Hank Childs, Thu Feb  5 17:11:06 PST 2004
//    Moved inlined destructor definition to .C file because certain compilers
//    have problems with them.
//
// ****************************************************************************

class PLOTTER_API avtTheater
{
  public:
                                    avtTheater();
    virtual                        ~avtTheater();

    void                            SetInput(avtDrawable_p, 
                                             avtDataObjectInformation &,
                                             avtDrawable_p);
    avtActor_p                      GetActor(void) { return actor; };
    avtBehavior_p                   GetBehavior(void) { return behavior; };

  protected:
    avtActor_p                      actor;
    avtBehavior_p                   behavior;
};


#endif


