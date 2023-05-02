// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                           ViewerOperatorFactory.h                         //
// ************************************************************************* //

#ifndef VIEWER_OPERATOR_FACTORY_H
#define VIEWER_OPERATOR_FACTORY_H
#include <viewercore_exports.h>
#include <ViewerBase.h>

class AttributeSubject;
class ViewerOperator;
class ViewerPlot;
class ViewerOperatorPluginInfo;


// ****************************************************************************
//  Class: ViewerOperatorFactory
//
//  Purpose:
//      ViewerOperatorFactory is a factory for creating operators.  It
//      also has methods for manipulating the operator attributes.  The
//      class is instantiated with a count of the number of operator
//      types.  Functions for copying operator attributes between the
//      client and the default are then registered with the operator type.
//      The class assumes that all the operator types are in the range 0 to
//      the number of operator types minus one.
//
//  Note:
//
//  Programmer: Eric Brugger
//  Creation:   September 15, 2000
//
//  Modifications:
//    Jeremy Meredith, Wed Dec 13 11:23:19 PST 2000
//    Made it also use a plot when creating an operator.
//
//    Jeremy Meredith, Thu Jul 26 03:19:49 PDT 2001
//    Rewrote to support operator plugins.
//
//    Jeremy Meredith, Fri Sep 28 13:47:32 PDT 2001
//    Removed the general plugin info since the viewer info is derived
//    from it now.
//
//    Eric Brugger, Wed Jan 15 15:54:26 PST 2003
//    Modify the CreateOperator method so that the plot argument is not const.
//
//    Kathleen Bonnell, Thu Sep 11 11:37:51 PDT 2003
//    Added 'fromDefault' to CreateOperator method. 
//
//    Brad Whitlock, Tue Jun 24 14:47:35 PDT 2008
//    Inherit from ViewerBase.
//
//    Eric Brugger, Wed Mar 22 16:23:12 PDT 2023
//    Add operator keyframing.
//
// ****************************************************************************

class VIEWERCORE_API ViewerOperatorFactory : private ViewerBase
{
  public:
    ViewerOperatorFactory();
    virtual ~ViewerOperatorFactory();

    int GetNOperatorTypes() const;

    ViewerOperator *CreateOperator(const int type,
                                   const bool keyframeMode,
                                   const int cacheIndex,
                                   const int cacheSize,
                                   ViewerPlot *plot,
                                   const bool fromDefault);

    AttributeSubject *GetDefaultAtts(const int type) const;
    AttributeSubject *GetClientAtts(const int type) const;

    void SetClientAttsFromDefault(const int type);
    void SetDefaultAttsFromClient(const int type);

  private:
    int                          nTypes;
    ViewerOperatorPluginInfo   **viewerPluginInfo;
};

#endif
