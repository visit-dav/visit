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
//                            TimeSequence.h                                 //
// ************************************************************************* //

#ifndef TIME_SEQUENCE_H
#define TIME_SEQUENCE_H
#include <siloobj_exports.h>


#include <visitstream.h>
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


