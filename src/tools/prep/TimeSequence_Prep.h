// ************************************************************************* //
//                          TimeSequence_Prep.h                              //
// ************************************************************************* //

#ifndef TIME_SEQUENCE_PREP_H
#define TIME_SEQUENCE_PREP_H


#include <visitstream.h>
#include <silo.h>

#include <NamingConvention.h>
#include <TimeSequence.h>


// ****************************************************************************
//  Class: TimeSequence_Prep
//
//  Purpose:
//      A derived type of TimeSequence that is meant exclusively for 
//      meshtvprep.
// 
//  Programmer: Hank Childs
//  Creation:   January 28, 2000
//
// ****************************************************************************

class TimeSequence_Prep : public TimeSequence
{
  public:
                      TimeSequence_Prep();
                     ~TimeSequence_Prep();

    void              ReadFromRootFile(DBfile *, DBtoc *);
    void              ReadVar(char *, DBmeshvar *);
    void              ReadVar(char *, DBquadvar *);
    void              ReadVar(char *, DBucdvar *);

    virtual void      Write(DBfile *);

    void              SetStates(NamingConvention *);

  protected:
    int               iStates;
    bool             *readInDimension;
    bool              readInFields;

    // Protected methods
    void              AddState(int, double);
    void              ReadCycleAndTime(DBfile *);
    void              ReadDimension(char *, int);
    void              ReadFields(DBfile *, DBtoc *);
    void              SetFields(int);
    void              EliminateUnusedFields(void);

    // Class-scoped constants.
    static char * const  SILO_LOCATION;
    static char * const  SILO_IN_CYCLE_NAME;
    static char * const  SILO_IN_TIME_NAME;
    static char * const  SILO_IN_TIME_NAME_ALT;
};


#endif


