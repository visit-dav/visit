// ************************************************************************* //
//                            avtDomainNesting.h                             //
// ************************************************************************* //

#ifndef AVT_DOMAIN_NESTING_H
#define AVT_DOMAIN_NESTING_H

#include <database_exports.h>

#include <vector>
using std::vector;

class vtkDataSet;

// ****************************************************************************
//  Class:  avtDomainNesting
//
//  Purpose:
//    Encapsulate domain nesting information. The main purpose of this class
//    is to update the "avtGhostZones" data array for regions where a
//    domain is nested by a finer domain also in the current selection.
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
//  Modifications:
//
//    Hank Childs, Fri Aug 27 16:16:52 PDT 2004
//    Update "Purpose" statement for changes in handling ghost data.
//
//    Hank Childs, Thu Jan  6 16:00:27 PST 2005
//    Added ConfirmMesh.
//
// ****************************************************************************
class DATABASE_API avtDomainNesting
{
  public:
                 avtDomainNesting() {};
    virtual      ~avtDomainNesting() {} ;

    virtual bool ApplyGhost(vector<int> domainList, vector<int> allDomainList,
                            vector<vtkDataSet*> meshes) = 0;

    virtual bool ConfirmMesh(vector<int> &, vector<vtkDataSet*> &) = 0;
};

#endif
