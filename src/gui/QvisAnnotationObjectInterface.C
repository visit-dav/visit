// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisAnnotationObjectInterface.h>

#include <QApplication>
#include <QLayout>
#include <QLineEdit>
#include <QKeyEvent>
#include <QSpinBox>
#include <QTimer>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::QvisAnnotationObjectInterface
//
// Purpose: 
//   Constructor for the QvisAnnotationObjectInterface class, which is the
//   base class for all annotation object interfaces.
//
// Arguments:
//   parent : The parent widget for the interface.
//   name   : The name of the interface.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:00:18 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jun 26 13:51:22 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QvisAnnotationObjectInterface::QvisAnnotationObjectInterface(QWidget *parent) :
    QGroupBox(parent), GUIBase()
{
    annot = 0;

    topLayout = new QVBoxLayout(this);
    topLayout->setContentsMargins(10,10,10,10);
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::~QvisAnnotationObjectInterface
//
// Purpose: 
//   Destructor for the QvisAnnotationObjectInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:01:15 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QvisAnnotationObjectInterface::~QvisAnnotationObjectInterface()
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetMenuText
//
// Purpose: 
//   Returns the menu text string, which is the string that is used to identify
//   the annotation in the annotation list box.
//
// Arguments:
//   annot : The annotation object, which can be used to add more descriptive
//           text to the menu string.
//
// Returns:    A string containing the menu text.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:11:48 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

QString
QvisAnnotationObjectInterface::GetMenuText(const AnnotationObject &) const
{
    return GetName();
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::Update
//
// Purpose: 
//   Updates the interface's controls.
//
// Arguments:
//   annotObj : A pointer to the annotation object.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:13:28 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::Update(AnnotationObject *annotObj)
{
    annot = annotObj;
    UpdateControls();
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for an interface's controls.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:15:10 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::GetCurrentValues(int)
{
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::Apply
//
// Purpose: 
//   Emits a signal that tells the Annotation window to apply the interface's
//   attributes (send them to the viewer and make the viewer use them)
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:15:37 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::Apply()
{
    GetCurrentValues(-1);
    emit applyChanges();
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::SetUpdate
//
// Purpose: 
//   Tells the annotation window not to update when the annotation object list
//   changes. This prevents unwanted updates from interface slot functions.
//
// Arguments:
//   val : Whether or not to update.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:16:24 PDT 2003
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::SetUpdate(bool val)
{
    emit setUpdateForWindow(val);
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetCoordinate
//
// Purpose: 
//   Gets a coordinate from a line edit.
//
// Arguments:
//   le     : The line edit that we're checking.
//   c      : The return coordinate array.
//  force2D : Whether the Z coordinate should always be zero.
//
// Returns:    true if successful; false otherwise.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:17:18 PDT 2003
//
// Modifications:
//   Brad Whitlock, Thu Jun 26 13:53:20 PDT 2008
//   Use base class method to parse the line edit.
//
// ****************************************************************************

bool
QvisAnnotationObjectInterface::GetCoordinate(QLineEdit *le, double c[3],
    bool force2D)
{
    return LineEditGetDoubles(le, c, force2D ? 2 : 3);
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetPosition
//
// Purpose: 
//   Gets the position coordinate from a line edit and sets it into the 
//   annotation object.
//
// Arguments:
//   le   : The line edit that we're checking.
//   name : The name to use in the error message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:18:30 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::GetPosition(QLineEdit *le, const QString &name)
{
    if(annot)
    {
        double coord[3] = {0., 0., 0.};

        if(GetCoordinate(le, coord, true))
        {
            annot->SetPosition(coord);
        }
        else
        {
            QString msg = tr("The %1 value was invalid. "
                             "Resetting to the last good value.").
                          arg(name);
            Error(msg);
            annot->SetPosition(annot->GetPosition());
        }
    }
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetPosition2
//
// Purpose: 
//   Gets the position2 coordinate from a line edit and sets it into the 
//   annotation object.
//
// Arguments:
//   le   : The line edit that we're checking.
//   name : The name to use in the error message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:18:30 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::GetPosition2(QLineEdit *le, const QString &name)
{
    if(annot)
    {
        double coord[3] = {0., 0., 0.};

        if(GetCoordinate(le, coord, true))
        {
            annot->SetPosition2(coord);
        }
        else
        {
            QString msg = tr("The %1 value was invalid. "
                             "Resetting to the last good value.").
                          arg(name);
            Error(msg);
            annot->SetPosition2(annot->GetPosition2());
        }
    }
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetScreenPosition
//
// Purpose: 
//   Gets the position coordinate from a screen position edit and sets it into
//   the annotation object.
//
// Arguments:
//   spe   : The screen position edit that we're checking.
//   name : The name to use in the error message.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 5 12:18:30 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::GetScreenPosition(QvisScreenPositionEdit *spe,
    const QString &name)
{
    if(annot)
    {
        double coord[3] = {0., 0., 0.};

        if(spe->getPosition(coord[0], coord[1]))
        {
            annot->SetPosition(coord);
        }
        else
        {
            QString msg = tr("The %1 value was invalid. "
                             "Resetting to the last good value.").
                          arg(name);
            Error(msg);
            annot->SetPosition(annot->GetPosition());
        }
    }
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::GetScreenPosition2
//
// Purpose: 
//   Gets the position2 coordinate from a screen position edit and sets it into
//   the annotation object.
//
// Arguments:
//   spe   : The screen position edit that we're checking.
//   name : The name to use in the error message.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 11:14:41 PDT 2006
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 16:29:55 PDT 2008
//   Support for internationalization.
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::GetScreenPosition2(QvisScreenPositionEdit *spe,
    const QString &name)
{
    if(annot)
    {
        double coord[3] = {0., 0., 0.};

        if(spe->getPosition(coord[0], coord[1]))
        {
            annot->SetPosition2(coord);
        }
        else
        {
            QString msg = tr("The %1 value was invalid. "
                             "Resetting to the last good value.").
                          arg(name);
            Error(msg);
            annot->SetPosition2(annot->GetPosition2());
        }
    }
}

