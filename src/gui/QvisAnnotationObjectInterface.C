#include <stdio.h>
#include <QvisAnnotationObjectInterface.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qspinbox.h>
#include <qtimer.h>
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
//   
// ****************************************************************************

QvisAnnotationObjectInterface::QvisAnnotationObjectInterface(QWidget *parent,
    const char *name) : QGroupBox(parent, name), GUIBase()
{
    annot = 0;

    topLayout = new QVBoxLayout(this);
    topLayout->setMargin(10);
    topLayout->addSpacing(10);
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
//   
// ****************************************************************************

bool
QvisAnnotationObjectInterface::GetCoordinate(QLineEdit *le, double c[3],
    bool force2D)
{
    QString temp(le->displayText().stripWhiteSpace());

    bool okay = !temp.isEmpty();
    if (okay)
    {
        okay = (sscanf(temp.latin1(), "%lg %lg %lg", &c[0], &c[1], &c[2]) == 3);
        if(!okay)
            okay = (sscanf(temp.latin1(), "%lg %lg", &c[0], &c[1]) == 2);
        if(force2D)
            c[2] = 0.;
    }

    return okay;
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
            QString msg; msg.sprintf("The %s value was invalid. "
                "Resetting to the last good value.", name.latin1());
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
            QString msg; msg.sprintf("The %s value was invalid. "
                "Resetting to the last good value.", name.latin1());
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
            QString msg; msg.sprintf("The %s value was invalid. "
                "Resetting to the last good value.", name.latin1());
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
            QString msg; msg.sprintf("The %s value was invalid. "
                "Resetting to the last good value.", name.latin1());
            Error(msg);
            annot->SetPosition2(annot->GetPosition2());
        }
    }
}

// ****************************************************************************
// Method: QvisAnnotationObjectInterface::ForceSpinBoxUpdate
//
// Purpose: 
//   Utility method that lets us trick a spin box into thinking it has new
//   text.
//
// Arguments:
//   sb : The spin box to update.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 14:21:40 PST 2006
//
// Modifications:
//   
// ****************************************************************************

void
QvisAnnotationObjectInterface::ForceSpinBoxUpdate(QSpinBox *sb)
{
    // Block signals.
    sb->blockSignals(true);

    // textChanged is protected and the compiler does not let us call
    // it directly so call it as a slot. We do this to make the spin box
    // think that it has new text input so it will parse it again when
    // we call value().
    QTimer::singleShot(0, sb, SLOT(textChanged()));

    // Call the value function to make the spin box parse the new value.
    sb->value();

    // Let the spinbox emit signals again.
    sb->blockSignals(false);
}
