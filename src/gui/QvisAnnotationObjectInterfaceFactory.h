#ifndef QVIS_ANNOTATION_OBJECT_INTERFACE_FACTORY_H
#define QVIS_ANNOTATION_OBJECT_INTERFACE_FACTORY_H
#include <gui_exports.h>

class QvisAnnotationObjectInterface;
class QWidget;

// ****************************************************************************
// Class: QvisAnnotationObjectInterfaceFactory
//
// Purpose:
//   Factory class for all annotation interfaces.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 09:19:35 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisAnnotationObjectInterfaceFactory
{
public:
    QvisAnnotationObjectInterfaceFactory();
    virtual ~QvisAnnotationObjectInterfaceFactory();

    int GetMaxInterfaces() const;
    QvisAnnotationObjectInterface *CreateInterface(int i, QWidget *parent) const;
};

#endif
