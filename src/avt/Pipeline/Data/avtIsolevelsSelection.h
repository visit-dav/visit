// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                         avtIsolevelsSelection.h                           //
// ************************************************************************* //

#ifndef AVT_ISOLEVELS_SELECTION_H
#define AVT_ISOLEVELS_SELECTION_H

#include <pipeline_exports.h>

#include <float.h>
#include <string>
#include <vector>
 
#include <ref_ptr.h>

#include <avtDataSelection.h>


// ****************************************************************************
//  Class: avtIsolevelsSelection
//
//  Purpose: 
//      Specify a data selection consisting of multiple isolevels.
// 
//  Programmer: Hank Childs
//  Creation:   January 5, 2008
//
//  Modifications:
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
// ****************************************************************************

class PIPELINE_API avtIsolevelsSelection : public avtDataSelection 
{
  public:
                            avtIsolevelsSelection();
                            avtIsolevelsSelection(const std::string _var, 
                                            const std::vector<double> &values);
    virtual                ~avtIsolevelsSelection();

    virtual const char *    GetType() const { return "Isolevels Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetVariable(const std::string _var)
                                { var = _var; };
    void                    SetIsolevels(const std::vector<double> &_isolevels)
                                { isolevels = _isolevels; };

    std::string             GetVariable() const
                                { return var; };
    const std::vector<double> &GetIsolevels() const
                                { return isolevels; };

  private:
    std::string var;
    std::vector<double> isolevels;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                       avtIsolevelsSelection(const avtIsolevelsSelection&) {;};
    avtIsolevelsSelection  &operator=(const avtIsolevelsSelection &) 
                                                             { return *this; };
};

typedef ref_ptr<avtIsolevelsSelection> avtIsolevelsSelection_p;


#endif


