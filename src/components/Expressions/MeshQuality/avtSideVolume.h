// ************************************************************************* //
//                             avtSideVolume.h                               //
// ************************************************************************* //

#ifndef AVT_SIDE_VOLUME_H
#define AVT_SIDE_VOLUME_H


#include <avtSingleInputExpressionFilter.h>

class     vtkCell;


// ****************************************************************************
//  Class: avtSideVolume
//
//  Purpose:
//      Calculates all of the side volumes of the sides of a cell and then
//      assigns the side volume for the cell to be smallest.
//
//      A side volume is defined as the following:
//      A zone Z is comprised of edges, faces, and nodes.
//      Each face F has a center, C, where C is the linear average
//      of the nodes on F.
//      The center of Z is the linear average of the center of its
//      faces.
//      
//      Then there are two sides for each edge, E.
//      If edge E is made up of points V1 and V2 and E is incident
//      to faces F1 and F2, then one side is a tet consisting of
//      V1, V2, center(F1) and center(Z), where the other tet
//      consists of V1, V2, center(F2) and center(Z).
//      
//      The side volume expression returns the smallest volume of the sides of
//      a zone.  This is a little bit of laziness on our part, since the
//      right thing to do would probably be to subdivide the mesh into tets
//      and go from there.  But then there would be additional issues, and
//      all of the customers agree this is a reasonable thing to do.
//
//  Programmer: Hank Childs
//  Creation:   January 20, 2005
//
// ****************************************************************************

class EXPRESSION_API avtSideVolume : public avtSingleInputExpressionFilter
{
  public:
                                avtSideVolume();

    virtual const char         *GetType(void) { return "avtSideVolume"; };
    virtual const char         *GetDescription(void)
                                    { return "Calculating side volume"; };
    
  protected:
    bool                        haveIssuedWarning;

    virtual vtkDataArray       *DeriveVariable(vtkDataSet *);
    virtual void                PreExecute(void);

    virtual bool                IsPointVariable(void)  { return false; };

    double                      GetZoneVolume(vtkCell *);
};


#endif


