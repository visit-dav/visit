// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                                 ZoomAxisArray.h                                  //
// ************************************************************************* //

#ifndef ZOOM_AXISARRAY_H
#define ZOOM_AXISARRAY_H
#include <viswindow_exports.h>


#include <ZoomInteractor.h>


class VisWindowInteractorProxy;


// ****************************************************************************
//  Class: ZoomAxisArray
//
//  Purpose:
//      Defines what Visit's AxisArray Zoom interactions should look like.  
//
//  Programmer: Jeremy Meredith
//  Creation:   February  4, 2008
//
//  Modifications:
//
// ****************************************************************************

class VISWINDOW_API ZoomAxisArray : public ZoomInteractor
{
  public:
                        ZoomAxisArray(VisWindowInteractorProxy &);
 
    virtual void        StartLeftButtonAction();
    virtual void        EndLeftButtonAction();
    virtual void        AbortLeftButtonAction();
    virtual void        StartMiddleButtonAction();
    virtual void        EndMiddleButtonAction();

  protected:
    void                ZoomCamera(void);
    void                ZoomCamera(const int x, const int y);
};


#endif


