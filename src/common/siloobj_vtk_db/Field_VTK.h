// ************************************************************************* //
//                              Field_VTK.h                                  //
// ************************************************************************* //

#ifndef FIELD_VTK_H
#define FIELD_VTK_H
#include <siloobj_vtk_exports.h>

#include <vtkScalars.h>

#include <Field.h>
#include <IntervalTree_VTK.h>
#include <Value_VTK.h>


//
// Forward declaration of classes.
//

class  TableOfContents;


// ****************************************************************************
//  Class: Field_VTK
// 
//  Purpose:
//      A derived class of Field that has an understanding of VTK.
//
//  Programmer: Hank Childs
//  Creation:   February 3, 2000
//
//  Modifications:
//  
//     Hank Childs, Wed Apr 12 21:45:33 PDT 2000
//     Removed methods GetDomainList and GetRange, added method GetMetaData.
//
// ****************************************************************************

class SILOOBJ_VTK_API Field_VTK : public Field
{
    friend                    class Mesh_VTK;

  public:
                              Field_VTK();
    virtual                  ~Field_VTK();

    const IntervalTree_VTK   *GetMetaData(void);
    const char               *GetMeshName(void);
    void                      GetVar(const int *, int, vtkScalars **);

    void                      UpdateReferences(TableOfContents *);

  protected:
    // A reference to the TOC that holds it.
    TableOfContents          *toc;

    // A properly typed alias to the base class' mixed value object that allows
    // it to act as a _VTK object. 
    Value_VTK                *aliasedMixedValueObject;

    // A properly typed alias to the base class' value object that allows it 
    // to act as a _VTK object. 
    Value_VTK                *aliasedValueObject;

    // A properly typed alias to the base class' value object that allows it 
    // to act as a _VTK object. 
    Value_VTK                *aliasedDimsObject;

    // A properly typed alias to the base class' interval tree object that 
    // allows it to act as a _VTK object. 
    IntervalTree_VTK         *aliasedIntervalTreeObject;

    void                      CreateValues(Value_VTK *, Value_VTK *,Value_VTK *,
                                           IntervalTree_VTK *);
    void                      GetValueDomains(const int *, int, float ***, 
                                              int **);
    void                      GetDimsDomains(const int *, int, float ***, 
                                              int **);
};


#endif


