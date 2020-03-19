// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           avtNodePickQuery.h                              //
// ************************************************************************* //

#ifndef AVT_NODEPICK_QUERY_H
#define AVT_NODEPICK_QUERY_H
#include <query_exports.h>

#include <avtPickQuery.h>

#include <string>
#include <PickAttributes.h>
#include <avtTypes.h>

class avtMatrix;
class vtkDataSet;


// ****************************************************************************
//  Class: avtNodePickQuery
//
//  Purpose:
//    This query gathers information about a node picked by the user.
//
//  Programmer: Kathleen Bonnell
//  Creation:   May 13, 2004
//
//  Modifications:
//    Kathleen Bonnell, Mon Dec  6 14:30:39 PST 2004
//    Added PreExecute and PostExecute methods, minDist, foundNode and
//    foundDomain members.
//
//    Kathleen Bonnell, Tue Nov  8 10:45:43 PST 2005
//    Added avtDataAttributes arg to Preparation.
//
//    Kathleen Biagas, Wed Jun 28 15:25:16 PDT 2017
//    Added SetTransform. InvTransform is used to transform the point used for
//    identifying nodes back to the original data space.  Transform is used to
//    transform the location for the Pick letter back into the plot space.
//
// ****************************************************************************

class QUERY_API avtNodePickQuery : public avtPickQuery
{
  public:
                                    avtNodePickQuery();
    virtual                        ~avtNodePickQuery();

    virtual const char             *GetType(void)
                                             { return "avtNodePickQuery"; };

    virtual void                    SetInvTransform(const avtMatrix *);
    virtual void                    SetTransform(const avtMatrix *);

  protected:
    virtual void                    Execute(vtkDataSet *, const int);
    virtual void                    PostExecute(void);
    virtual void                    PreExecute(void);
    virtual void                    Preparation(const avtDataAttributes &);
    int                             DeterminePickedNode(vtkDataSet *);

  private:
    double                          minDist;
    int                             foundNode;
    int                             foundDomain;
};


#endif


