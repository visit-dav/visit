// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisAnnotationObjectInterfaceFactory.h>

#include <QvisImageAnnotationInterface.h>
#include <QvisLegendAttributesInterface.h>
#include <QvisLine2DInterface.h>
#include <QvisLine3DInterface.h>
#include <QvisText2DInterface.h>
#include <QvisText3DInterface.h>
#include <QvisTimeSliderInterface.h>

#include <DebugStream.h>

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::QvisAnnotationObjectInterfaceFactory
//
// Purpose: 
//   Constructor for the QvisAnnotationObjectInterfaceFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationObjectInterfaceFactory::QvisAnnotationObjectInterfaceFactory()
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::~QvisAnnotationObjectInterfaceFactory
//
// Purpose: 
//   Destructor for the QvisAnnotationObjectInterfaceFactory class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationObjectInterfaceFactory::~QvisAnnotationObjectInterfaceFactory()
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces
//
// Purpose: 
//   Returns the maximum number of interfaces that the factory creates.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:31:50 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 13:32:58 PST 2005
//   Made it return 8.
//
//   Brad Whitlock, Tue Mar 20 14:38:57 PST 2007
//   Made it return MaxAnnotationType.
//
// ****************************************************************************

int
QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces() const
{
    return (int)AnnotationObject::MaxAnnotationType;
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterfaceFactory::CreateInterface
//
// Purpose: 
//   Creates an interface and returns it.
//
// Arguments:
//   i      : The index of the interface to create.
//   parent : The parent widget for the interface.
//
// Returns:    A pointer to the new interface or 0 if no interface was created.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:32:56 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Jun 28 12:09:55 PDT 2005
//   Added John Anderson's image and line2d annotation interfaces.
//
//   Brad Whitlock, Tue Mar 20 14:13:00 PST 2007
//   Added legend attributes interface.
//
//   Brad Whitlock, Wed Nov 7 17:09:34 PST 2007
//   Added text 3d.
//
// ****************************************************************************

QvisAnnotationObjectInterface *
QvisAnnotationObjectInterfaceFactory::CreateInterface(
    AnnotationObject::AnnotationType i, QWidget *parent) const
{
    QvisAnnotationObjectInterface *retval = 0;

    switch(i)
    {
    case AnnotationObject::Text2D:
        retval = new QvisText2DInterface(parent);
        break;
    case AnnotationObject::Text3D:
        retval = new QvisText3DInterface(parent);
        break;
    case AnnotationObject::TimeSlider:
        retval = new QvisTimeSliderInterface(parent);
        break;
    case AnnotationObject::Line2D:
        retval = new QvisLine2DInterface(parent);
        break;
    case AnnotationObject::Line3D:
        retval = new QvisLine3DInterface(parent);
        break;
    case AnnotationObject::Arrow2D:
        // Nothing yet
        break;
    case AnnotationObject::Arrow3D:
        // Nothing yet
        break;
    case AnnotationObject::Box:
        // Nothing yet
        break;
    case AnnotationObject::Image:
        retval = new QvisImageAnnotationInterface(parent);
        break;
    case AnnotationObject::LegendAttributes:
        retval = new QvisLegendAttributesInterface(parent);
        break;
    default:
        debug1 << "QvisAnnotationObjectInterfaceFactory::CreateInterface:"
               << " invalid index=" << (int)i << endl;
        break;
    }

    return retval;
}
