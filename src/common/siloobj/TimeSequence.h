// ************************************************************************* //
//                            TimeSequence.h                                 //
// ************************************************************************* //

#ifndef TIME_SEQUENCE_H
#define TIME_SEQUENCE_H
#include <siloobj_exports.h>


#include <iostream.h>
#include <silo.h>


// ****************************************************************************
//  Class: TimeSequence
//
//  Purpose:
//      Keeps all information about the time sequence.
//      This object also contains information about the fields associated
//      with each state.
// 
//  Data Members:
//      nStates            -  The number of times states in the sequence.
//      times              -  The time associated with each state.  The time 
//                            need to be organized in a monotonically 
//                            increasing fashion.  The size of this array will 
//                            be nStates.
//      cycles             -  The cycle associated with each state.  The 
//                            cycles need to be organized in a monotonically 
//                            increasing fashion.  The size of this array will 
//                            be nStates.
//      states             -  The name of the file containing the fields for 
//                            each state. The size of this array wil be 
//                            nStates.
//      nFields            -  The number of fields associated with each state.
//      fieldNames         -  The name of each field in a state.  The size of
//                            this array will be nFields.
//      fieldTypes         -  The type of each field in a state.  The size of
//                            this array will be nFields.
//      fieldDimension     -  The dimension of each field in the state.  The
//                            size of this array will be nFields.
//      fieldIntervalTree  -  The name of the interval tree of each field in
//                            the state.  The size of this array will be 
//                            nFields.
//
//  Programmer: Hank Childs
//  Creation:   December 2, 1999
//
// ****************************************************************************

class SILOOBJ_API TimeSequence
{
  public:
                      TimeSequence();
    virtual          ~TimeSequence();

    void              Read(DBobject *, DBfile *);
    virtual void      Write(DBfile *);

    void              PrintSelf(ostream &);

  protected:
    int               nStates;
    double           *times;
    int              *cycles;
    char            **states;
    
    int               nFields;
    char            **fieldNames;
    int              *fieldTypes;
    int              *fieldDimension;
    char            **fieldIntervalTree;

    // Class-scoped constants.
  public:
    static char * const  SILO_TYPE;
  protected:
    static char * const  SILO_OBJ_NAME;
    static char * const  SILO_N_STATES_NAME;
    static char * const  SILO_TIMES_NAME;
    static char * const  SILO_CYCLES_NAME;
    static char * const  SILO_STATES_NAME;
    static char * const  SILO_N_FIELDS_NAME;
    static char * const  SILO_FIELD_NAMES_NAME;
    static char * const  SILO_FIELD_TYPES_NAME;
    static char * const  SILO_FIELD_DIMENSION_NAME;
    static char * const  SILO_FIELD_INTERVAL_TREE_NAME;
    static int    const  SILO_NUM_COMPONENTS;
};


#endif


