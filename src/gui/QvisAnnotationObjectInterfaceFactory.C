#include <QvisAnnotationObjectInterfaceFactory.h>
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
//   
// ****************************************************************************

int
QvisAnnotationObjectInterfaceFactory::GetMaxInterfaces() const
{
    return 7;
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
    default:
        debug1 << "QvisAnnotationObjectInterfaceFactory::CreateInterface:"
               << " invalid index=" << i << endl;
        break;
    }

    return retval;
}
