// ************************************************************************* //
//                              avtDataSelection.h                           //
// ************************************************************************* //

#ifndef AVT_DATA_SELECTION_H
#define AVT_DATA_SELECTION_H 
#include <pipeline_exports.h>

#include <ImproperUseException.h>

#include <ref_ptr.h>

// ****************************************************************************
//  Class: avtDataSelection 
//
//  Purpose:
//      Manage data selection information associated with operators that
//      some kinds of database plugins might like to know about.
//
//      In many instances, a given operator's (or filter's) effect is primarily
//      to select a portion of a mesh to be processed. For example, 
//      an IndexSelect filter selects zones between logical min and max bounds
//      as well as only every Nth where N is a step size. A Box operator
//      selects zones within some axis-oriented, spatial bounding box.
//
//      It turns out that some database formats have an inherent capability for
//      doing data selection during the process of reading data from
//      the format. For example, the ViSUS format has the ability to
//      efficiently read arbitrary bounding boxes with power-of-2 
//      down-samplings. It may even be enhanced to read along arbitrary slice
//      planes. So, the ViSUS format could benefit from knowing about 
//      various operators and then performing the selections on their behalf.
//
//      For other databases, it may not be that the format has an inherent
//      capability to perform data selections. However, it may be that there is
//      still a very good reason for the database plugin that implements
//      the format to perform data selection operations. An example would be a
//      format that stores a continuous, CSG representation of a mesh on disk
//      while the plugin re-samples this to produce a, discrete, vtkDataSet
//      to serve up to VisIt. In this case, it would be very useful for things
//      like slice or box information to make it all the way to the plugin so
//      that the discretization could be performed on a smaller domain. 
//
//      VisIt already has a well-defined architecture for how operations like
//      those mentioned above are performed. They are simply filters in an
//      avtPipeline. Why would we want to circumvent this? A key concept in
//      the various scenarios mentioned above is that there is an opportunity
//      to reduce the amount of data read from disk and processed by the
//      avtPipeline. The potential for reducing the amount of data is the
//      primary driver for enabling this kind of behavior.
//
//      For some operators, data selection is the sole reason the operator
//      exists. When the database is able to do the data selection the
//      operator would have, that stage in the pipeline can become a no-op.
//      In other cases, data selection is only a part of the effect of an
//      operator. So, even if the database does the selection, the operator
//      may still have work to do, though maybe less than it would have if
//      the database did not perform the selection.
//
//      How does it all work? Any filter that has data selection consequences,
//      sets the relevant parameters for an appropriate DataSelection object
//      which is then added to the avtDataSpecification object. This should
//      happen in each filter's PerformRestriction (legacy name) method.
//      Eventually, data selection information for all the operators that
//      advertise it arrives at avtGenericDatabase in the avtDataSpecfication
//      object. Any database that wants this information must implement the
//      RegisterDataSelections() method on the avtFileFormat. avtGenericDatabase
//      calls this method to push data selection information to the database
//      plugins just prior to requesting data via a GetMesh() or GetVar() call.
//
//      A database plugin that requests selection information can then use
//      it to effect the behavior of succeeding GetMesh() or GetVar() calls
//      from avtGenericDatabase. If, in the process of fullfilling a GetMesh()
//      or GetVar() call, the plugin succeeds in satisfying the requirements
//      of a given data selection, it must report its success back to
//      avtGenericDatabase.  When pipeline execution arrives in the filter
//      for which the given data selection is associated, the filter can ask
//      if the selection has already been satisifed and act accordingly.
//
//      This is a base class for several classes of data selection
//      information.
//
//  Programmer: Mark C. Miller 
//  Creation:   September 22, 2004 
//
// ****************************************************************************

class PIPELINE_API avtDataSelection 
{
  public:
                            avtDataSelection() {} ;
    virtual                ~avtDataSelection() {} ;

    virtual const char *    GetType() const = 0;
    virtual bool            operator==(const avtDataSelection &) const
                                { EXCEPTION0(ImproperUseException); };

  private:
};


typedef ref_ptr<avtDataSelection> avtDataSelection_p;

#endif


