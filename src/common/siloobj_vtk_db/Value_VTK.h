// ************************************************************************* //
//                               Value_VTK.h                                 //
// ************************************************************************* //

#ifndef VALUE_VTK_H
#define VALUE_VTK_H
#include <siloobj_vtk_exports.h>

#include <Value.h>


//
// Forward declarations of classes.
//

class TableOfContents;


// ****************************************************************************
//  Class: Value_VTK
// 
//  Purpose:
//      A derived type of Value that can get the arrays for certain domains.
//
//  Programmer: Hank Childs
//  Creation:   February 2, 2000
//
// ****************************************************************************

class SILOOBJ_VTK_API Value_VTK : public Value
{
  public:
                        Value_VTK();
    virtual            ~Value_VTK();
    
    void                GetDomains(const int *, int, float ***, int **);

    void                UpdateReferences(TableOfContents *);

  protected:
    // The table of contents object that contains this.
    TableOfContents    *toc;

    // An array of the arrays read in.  This is an array of size nDomains.
    float             **cache;

    // Protected Methods
    void                CreateCache(void);
};


#endif


