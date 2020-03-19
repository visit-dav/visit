// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ANNOTATION_OBJECT_INTERFACE_FACTORY_H
#define QVIS_ANNOTATION_OBJECT_INTERFACE_FACTORY_H
#include <gui_exports.h>
#include <AnnotationObject.h>

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
    QvisAnnotationObjectInterface *CreateInterface(
        AnnotationObject::AnnotationType i, QWidget *parent) const;
};

#endif
