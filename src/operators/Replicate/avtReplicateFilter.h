// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//  File: avtReplicateFilter.h
// ************************************************************************* //

#ifndef AVT_Replicate_FILTER_H
#define AVT_Replicate_FILTER_H

#include <avtSIMODataTreeIterator.h>
#include <avtPluginFilter.h>

#include <ReplicateAttributes.h>

class vtkDataArray;
class vtkDataSet;
class vtkPointSet;
class vtkPolyData;
class vtkRectilinearGrid;


// ****************************************************************************
//  Class: avtReplicateFilter
//
//  Purpose:
//      A plugin operator for Replicate.
//
//  Programmer: Jeremy Meredith
//  Creation:   August 29, 2006
//
//  Modifications:
//    Jeremy Meredith, Fri Feb 23 12:27:29 EST 2007
//    Added support for transformed rectilinear grids.  Also fixed a typo.
//
//    Jeremy Meredith, Thu Mar 22 15:22:28 EDT 2007
//    Added support for replicating individual atoms in molecular data
//    that lie exactly on a unit cell boundary.
//
//    Jeremy Meredith, Tue Jun  2 16:25:01 EDT 2009
//    Added support for unit cell origin (previously assumed to be 0,0,0);
//    Allowed periodic boundary atom replication to use the specified
//    replication vectors as the unit cell (though it still assumes 0,0,0
//    origin in this case).  Add support for shifting atoms to a new
//    unit cell origin.
//
//    Eric Brugger, Thu Aug 14 08:53:21 PDT 2014
//    Modified the class to work with avtDataRepresentation.
//
// ****************************************************************************

class avtReplicateFilter : public virtual avtSIMODataTreeIterator,
                         public virtual avtPluginFilter
{
  public:
                         avtReplicateFilter();
    virtual             ~avtReplicateFilter();

    static avtFilter    *Create();

    virtual const char  *GetType(void)  { return "avtReplicateFilter"; };
    virtual const char  *GetDescription(void)
                             { return "Replicating the data"; };

    virtual void         SetAtts(const AttributeGroup*);
    virtual bool         Equivalent(const AttributeGroup*);

  protected:
    ReplicateAttributes   atts;

    virtual void          PostExecute(void);
    virtual avtDataTree_p ExecuteDataTree(avtDataRepresentation *);
    virtual avtContract_p
                          ModifyContract(avtContract_p);
    virtual void          UpdateDataObjectInfo(void);

    vtkDataSet           *Replicate(vtkDataSet *, double[3]);
    vtkDataSet           *ReplicateRectilinear(vtkRectilinearGrid *, double[3]);
    vtkDataSet           *ReplicatePointSet(vtkPointSet *, double[3]);
    vtkDataArray         *OffsetDataArray(vtkDataArray *, double);
    vtkPolyData          *ReplicateAndShiftUnitCellAtoms(vtkPolyData *in);

    virtual bool          FilterUnderstandsTransformedRectMesh();
};


#endif
