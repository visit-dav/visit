// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                             avtMixedVariable.h                            //
// ************************************************************************* //

#ifndef AVT_MIXED_VARIABLE_H
#define AVT_MIXED_VARIABLE_H

#include <string>
#include <vector>

#include <pipeline_exports.h>


// ****************************************************************************
//  Class: avtMixedVariable
//
//  Purpose:
//      Contains a mixed variable.  This can then be used by the MIR to assign
//      the mixed materials in a zone their exact values.
//
//  Programmer: Hank Childs
//  Creation:   September 27, 2001
//
//  Modifications:
//
//    Jeremy Meredith, Tue Dec  4 13:43:54 PST 2001
//    Added const to constructor arguments.
//
//    Hank Childs, Thu Jul  4 13:02:54 PDT 2002
//    Added names for mixed variables.
//
//    Hank Childs, Tue Dec 18 10:04:43 PST 2007
//    Define private copy constructor and assignment operator to prevent
//    accidental use of default, bitwise copy implementations.
//
//    Cyrus Harrison, Wed Jan 30 11:15:25 PST 2008
//    Added helper to obtain the per material values of a given zone.
//
// ****************************************************************************

class avtMaterial;

class PIPELINE_API avtMixedVariable
{
  public:
                        avtMixedVariable(const float *, int, std::string);
    virtual            ~avtMixedVariable();

    static void         Destruct(void *);

    const float        *GetBuffer(void) { return buffer; };
    int                 GetMixlen(void) { return mixlen; };
    const std::string  &GetVarname(void) { return varname; };
    void                GetValuesForZone(int zone_id,
                                         avtMaterial *mat,
                                         std::vector<float> &vals); 
  
  protected:
    std::string         varname;
    float              *buffer;
    int                 mixlen;

  private:
    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                         avtMixedVariable(const avtMixedVariable &) {;};
    avtMixedVariable    &operator=(const avtMixedVariable &) { return *this; };
};


#endif


