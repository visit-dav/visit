// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                       avtIdentifierSelection.h                            //
// ************************************************************************* //

#ifndef AVT_IDENTIFIER_SELECTION_H
#define AVT_IDENTIFIER_SELECTION_H 

#include <pipeline_exports.h>

#include <ref_ptr.h>

#include <avtDataSelection.h>

#include <vector>
#include <string>

// ****************************************************************************
//  Class: avtIdentifierSelection
//
//  Purpose: 
//      Specify data selection using identifiers.
//
//  Programmer: Hank Childs
//  Creation:   March 5, 2008
//
//  Modifications:
//
//    Hank Childs, Thu Mar  6 09:05:59 PST 2008
//    Add Destruct method.
//
//    Hank Childs, Tue Dec 20 14:43:08 PST 2011
//    Add method DescriptionString.
//
//    Brad Whitlock, Thu Mar 15 14:13:59 PDT 2012
//    Added idVar.
//
// ****************************************************************************

class PIPELINE_API avtIdentifierSelection : public avtDataSelection 
{
  public:
                            avtIdentifierSelection();
    virtual                ~avtIdentifierSelection();

    static void             Destruct(void *);

    virtual const char *    GetType() const
                                { return "Identifier Data Selection"; }; 
    virtual std::string     DescriptionString(void);

    void                    SetIdentifiers(const std::vector<double> &a)
                                { ids = a; };
    const std::vector<double> &GetIdentifiers(void) { return ids; };

    void                    SetIdVariable(const std::string &id) {idVar = id; }
    const std::string      &GetIdVariable() const {return idVar; }

    bool                    operator==(const avtIdentifierSelection &) const;

  private:
    std::vector<double>     ids;
    std::string             idVar;

    // These methods are defined to prevent accidental use of bitwise copy
    // implementations.  If you want to re-define them to do something
    // meaningful, that's fine.
                    avtIdentifierSelection(const avtIdentifierSelection &) {;};
    avtIdentifierSelection &operator=(const avtIdentifierSelection &) 
                                                            { return *this; };
};

typedef ref_ptr<avtIdentifierSelection> avtIdentifierSelection_p;


#endif


