#include <QvisAnnotationObjectInterfaceFactory.h>
#include <QvisImageAnnotationInterface.h>
#include <QvisLine2DInterface.h>
#include <QvisText2DInterface.h>
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
// ****************************************************************************

int
QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces() const
{
    return 8;
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
// ****************************************************************************

QvisAnnotationObjectInterface *
QvisAnnotationObjectInterfaceFactory::CreateInterface(int i, QWidget *parent) const
{
    QvisAnnotationObjectInterface *retval = 0;

    switch(i)
    {
    case 0: // Text2D
        retval = new QvisText2DInterface(parent, "text2DInterface");
        break;
    case 2: // TimeSlider;
        retval = new QvisTimeSliderInterface(parent, "timeSliderInterface");
        break;
    case 3: // Line2D
        retval = new QvisLine2DInterface(parent, "line2DInterface");
        break;
    // Arrow2D
    // Arrow3D
    // Box
    case 7: // Image
        retval = new QvisImageAnnotationInterface(parent, "imageAnnotationInterface");
        break;
    default:
        debug1 << "QvisAnnotationObjectInterfaceFactory::CreateInterface:"
               << " invalid index=" << i << endl;
        break;
    }

    return retval;
}
