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
//    is to add the "vtkGhostLevels" data array that is '1' in coarser
//    domains where there is a finer (nested) domain also in the current
//    selection.
//
//  Programmer:  Mark C. Miller 
//  Creation:    October 13, 2003
//
// ****************************************************************************
class DATABASE_API avtDomainNesting
{
  public:
                 avtDomainNesting() {};
    virtual      ~avtDomainNesting() {} ;

    virtual bool ApplyGhost(vector<int> domainList, vector<int> allDomainList,
                            vector<vtkDataSet*> meshes) = 0;

};

#endif
