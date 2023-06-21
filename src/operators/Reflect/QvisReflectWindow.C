// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisReflectWindow.h"

#include <ReflectAttributes.h>

#include <QvisReflectWidget.h>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QComboBox>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QTabWidget>
#include <QPalette>

static const int widget2OperatorOctants[] = {0, 1, 3, 2, 4, 5, 7, 6};
static const int operator2WidgetOctants[] = {0, 1, 3, 2, 4, 5, 7, 6};


// ****************************************************************************
//  Constructor:  QvisReflectWindow::QvisReflectWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Fri Apr 12 13:13:11 PST 2002
//    Made it inherit from QvisOperatorWindow.
//
//    Brad Whitlock, Thu Mar 6 11:31:12 PDT 2003
//    I added stretch and initialized some widgets that have no parents.
//
//    Brad Whitlock, Wed Jun 25 09:24:38 PDT 2003
//    I initialized modeButtons, userSetMode, and mode2D.
//
// ****************************************************************************

QvisReflectWindow::QvisReflectWindow(const int type,
                         ReflectAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type, subj, caption, shortName, notepad, false)
{
    atts = subj;

    userSetMode = false;
    mode2D = true;

    modeButtons = 0;
    xBound = 0;
    yBound = 0;
    zBound = 0;
}

// ****************************************************************************
//  Destructor:  QvisReflectWindow::~QvisReflectWindow
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Mar 11 11:56:34 PDT 2003
//    I added code to delete certain widgets that have no parents.
//
//    Cyrus Harrison, Tue Aug 26 15:45:00 PDT 2008
//    Set parents for button groups, so we dont have to delete the explicitly.
//
// ****************************************************************************

QvisReflectWindow::~QvisReflectWindow()
{

}

// ****************************************************************************
//  Method:  QvisReflectWindow::CreateWindowContents
//
//  Purpose:
//    Sets up widgets and signal connections.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Mon Mar 3 11:42:37 PDT 2003
//    I made it use a QvisReflectWidget.
//
//    Brad Whitlock, Wed Jun 25 09:25:32 PDT 2003
//    I added code that lets us have a 2D input mode.
//
//    Jeremy Meredith, Wed Mar  3 16:17:01 PST 2004
//    I had it update the octant menu since we only
//    update it when it changes now.
//
//    Brad Whitlock, Thu Apr 24 16:54:17 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
//    Alister Maguire, Wed Apr 11 09:29:49 PDT 2018
//    Added option for user to reflect by an arbitrary plane,
//    and added tabs to distinguish options.
//
//    Kathleen Biagas, Wed Apr  5 15:11:57 PDT 2023
//    Replace obsolete QPalette::Background with QPalette::Window.
//
//    Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//    Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisReflectWindow::CreateWindowContents()
{
    QVBoxLayout *mainLayout = new QVBoxLayout();
    topLayout->addLayout(mainLayout);

    // Create the reflection tabs.
    reflectTabs = new QTabWidget();
    axisTab = new QWidget(central);
    planeTab = new QWidget(central);
    connect(reflectTabs, SIGNAL(currentChanged(int)),
        this, SLOT(reflectTabsChangedIndex(int)));
    reflectTabs->addTab(axisTab, tr("Reflect across axis"));
    reflectTabs->addTab(planeTab, tr("Reflect across plane"));
    mainLayout->addWidget(reflectTabs);

    // The layout of the axis reflect.
    QVBoxLayout *axisLayout = new QVBoxLayout(axisTab);

    // Add the controls to select the input mode.
    QGridLayout *octantLayout = new QGridLayout();
    axisLayout->addLayout(octantLayout);

    modeButtons = new QButtonGroup(central);
    octantLayout->addWidget(new QLabel(tr("Input mode"), central),0, 0);
    QRadioButton *rb = new QRadioButton(tr("2D"), central);
    modeButtons->addButton(rb,0);
    octantLayout->addWidget(rb, 0, 1);
    rb = new QRadioButton(tr("3D"), central);
    modeButtons->addButton(rb,1);
    modeButtons->button(0)->setChecked(true);
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(modeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(selectMode(int)));
#else
    connect(modeButtons, SIGNAL(idClicked(int)),
            this, SLOT(selectMode(int)));
#endif
    octantLayout->addWidget(rb, 0, 2);

    // Octant
    originalDataLabel = new QLabel(tr("Original data quadrant"), central);
    octantLayout->addWidget(originalDataLabel, 1, 0);
    octant = new QComboBox(central);
    octantLayout->addWidget(octant, 1, 1, 1, 2);

    // Reflection widget
    reflect = new QvisReflectWidget(central);
    reflect->setMode2D(mode2D);
    connect(reflect, SIGNAL(valueChanged(bool*)),
            this, SLOT(selectOctants(bool*)));
    reflectionLabel = new QLabel(tr("Reflection quadrants"), central);
    // Change the background color to match the tab space.
    QPalette pal = QPalette();
    pal.setColor(QPalette::Window, Qt::white);
    reflect->setAutoFillBackground(true);
    reflect->setPalette(pal);
    axisLayout->addWidget(reflectionLabel);
    axisLayout->addWidget(reflect, 100);

    // Limits
    axisLayout->addWidget(new QLabel(tr("Reflection Limits:"), central));
    QGridLayout *limitsLayout = new QGridLayout();
    axisLayout->addLayout(limitsLayout);
    //limitsLayout->addColumnSpacing(0, 20);

    xBound = new QButtonGroup(central);
    xUseData = new QRadioButton(tr("Use X Min"),   central);
    xSpecify = new QRadioButton(tr("Specify X ="), central);
    xBound->addButton(xUseData,0);
    xBound->addButton(xSpecify,1);
    specifiedX = new QLineEdit("0", central);
    limitsLayout->addWidget(xUseData,   0, 1);
    limitsLayout->addWidget(xSpecify,   0, 2);
    limitsLayout->addWidget(specifiedX, 0, 3);

    yBound = new QButtonGroup(central);
    yUseData = new QRadioButton(tr("Use Y Min"),   central);
    ySpecify = new QRadioButton(tr("Specify Y ="), central);
    yBound->addButton(yUseData,0);
    yBound->addButton(ySpecify,1);
    specifiedY = new QLineEdit("0", central);
    limitsLayout->addWidget(yUseData,   1, 1);
    limitsLayout->addWidget(ySpecify,   1, 2);
    limitsLayout->addWidget(specifiedY, 1, 3);

    zBound = new QButtonGroup(central);
    zUseData = new QRadioButton(tr("Use Z Min"),   central);
    zSpecify = new QRadioButton(tr("Specify Z ="), central);
    zBound->addButton(zUseData,0);
    zBound->addButton(zSpecify,1);
    specifiedZ = new QLineEdit("0", central);
    limitsLayout->addWidget(zUseData,   2, 1);
    limitsLayout->addWidget(zSpecify,   2, 2);
    limitsLayout->addWidget(specifiedZ, 2, 3);

    UpdateOctantMenuContents();

    connect(octant,        SIGNAL(activated(int)),
            this,          SLOT(octantChanged(int)));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(xBound,        SIGNAL(buttonClicked(int)),
            this,          SLOT(xBoundaryChanged(int)));
#else
    connect(xBound,        SIGNAL(idClicked(int)),
            this,          SLOT(xBoundaryChanged(int)));
#endif
    connect(specifiedX,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedXProcessText()));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(yBound,        SIGNAL(buttonClicked(int)),
            this,          SLOT(yBoundaryChanged(int)));
#else
    connect(yBound,        SIGNAL(idClicked(int)),
            this,          SLOT(yBoundaryChanged(int)));
#endif
    connect(specifiedY,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedYProcessText()));
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(zBound,        SIGNAL(buttonClicked(int)),
            this,          SLOT(zBoundaryChanged(int)));
#else
    connect(zBound,        SIGNAL(idClicked(int)),
            this,          SLOT(zBoundaryChanged(int)));
#endif
    connect(specifiedZ,    SIGNAL(returnPressed()),
            this,          SLOT(specifiedZProcessText()));

    // The layout of the plane reflect.
    QGridLayout *planeLayout = new QGridLayout(planeTab);

    // Create the plane point editor.
    QLabel *planePointLabel = new QLabel(tr("Plane Point"), central);
    planeLayout->addWidget(planePointLabel, 0, 0, 1, 1);
    planePoint = new QLineEdit(central);
    planePoint->setPlaceholderText("0.0 0.0 0.0");
    connect(planePoint, SIGNAL(returnPressed()),
            this, SLOT(planePointProcessText()));
    planeLayout->addWidget(planePoint, 0, 1, 1, 1);

    // Create the plane normal editor.
    QLabel *planeNormLabel = new QLabel(tr("Plane Normal"), central);
    planeLayout->addWidget(planeNormLabel, 2, 0, 1, 1);
    planeNorm = new QLineEdit(central);
    planeNorm->setPlaceholderText("0.0 0.0 0.0");
    connect(planeNorm, SIGNAL(returnPressed()),
            this, SLOT(planeNormProcessText()));
    planeLayout->addWidget(planeNorm, 2, 1, 1, 1);

    planeLayout->setRowStretch(3, 100);
}

// ****************************************************************************
//  Method:  QvisReflectWindow::UpdateWindow
//
//  Purpose:
//    Updates portions of the window based on the current attributes.
//
//  Arguments:
//    doAll      update the whole window
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Wed Mar 5 15:49:22 PST 2003
//    I made it use a QvisReflectWidget widget.
//
//    Brad Whitlock, Wed Jun 25 10:11:14 PDT 2003
//    I added a 2D input mode.
//
//    Jeremy Meredith, Wed Mar  3 16:18:08 PST 2004
//    Only update the octant menu automatically if it has
//    changed dimensionality.
//
//    Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//    Replaced simple QString::sprintf's with a setNum because there seems
//    to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//    Brad Whitlock, Thu Apr 24 16:55:18 PDT 2008
//    Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
//    Kathleen Biagas, Thu Apr 9 07:17:44 MST 2015
//    Use helper functions DoubleToQString for consistency in formatting across
//    all windows.
//
//    Alister Maguire, Wed Apr 11 09:29:49 PDT 2018
//    Added updates for the plane point, plane normal, and reflect tabs.
//
// ****************************************************************************

void
QvisReflectWindow::UpdateWindow(bool doAll)
{
    bool    setOctant  = false;
    bool    originIs3D = (operator2WidgetOctants[atts->GetOctant()] > 3);
    bool    reflection3D = false;

    reflect->blockSignals(true);

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
            {
                continue;
            }
        }

        switch(i)
        {
          case 0: // octant
            setOctant = true;
            break;
          case 1: //xBound
            xUseData->setText((atts->GetOctant()&0x01) ? tr("Use dataset max") : tr("Use dataset min"));
            xBound->button(atts->GetUseXBoundary() ? 0 : 1)->setChecked(true);
            specifiedX->setEnabled(atts->GetUseXBoundary() ? false : true);
            break;
          case 2: //specifiedX
            specifiedX->setText(DoubleToQString(atts->GetSpecifiedX()));
            break;
          case 3: //yBound
            yUseData->setText((atts->GetOctant()&0x02) ? tr("Use dataset max") : tr("Use dataset min"));
            yBound->button(atts->GetUseYBoundary() ? 0 : 1)->setChecked(true);
            specifiedY->setEnabled(atts->GetUseYBoundary() ? false : true);
            break;
          case 4: //specifiedY
            specifiedY->setText(DoubleToQString(atts->GetSpecifiedY()));
            break;
          case 5: //zBound
            zUseData->setText((atts->GetOctant()&0x04) ? tr("Use dataset max") : tr("Use dataset min"));
            zBound->button(atts->GetUseZBoundary() ? 0 : 1)->setChecked(true);
            specifiedZ->setEnabled(atts->GetUseZBoundary() ? false : true);
            break;
          case 6: //specifiedZ
            specifiedZ->setText(DoubleToQString(atts->GetSpecifiedZ()));
            break;
          case 7: //reflections
            {
                int *r = atts->GetReflections();
                bool octants[8];
                for (int i=0; i<8; i++)
                {
                    int b = i ^ atts->GetOctant();
                    octants[operator2WidgetOctants[i]] = (r[b] > 0);
                    reflection3D |= (octants[operator2WidgetOctants[i]] && operator2WidgetOctants[i] > 3);
                }

                reflect->setValues(octants);
            }
            break;
          case ReflectAttributes::ID_planePoint:
            planePoint->setText(DoublesToQString(atts->GetPlanePoint(), 3));
            break;
          case ReflectAttributes::ID_planeNormal:
            planeNorm->setText(DoublesToQString(atts->GetPlaneNormal(), 3));
            break;
          case ReflectAttributes::ID_reflectType:
            {
                if (atts->GetReflectType() == ReflectAttributes::Axis)
                {
                    reflectTabs->setCurrentIndex(0);
                }
                else if (atts->GetReflectType() == ReflectAttributes::Plane)
                {
                    reflectTabs->setCurrentIndex(1);
                }
            }
            break;
        }
    }

    if(originIs3D || reflection3D)
    {
        bool newMode2D = !originIs3D && !reflection3D;
        if(newMode2D != mode2D)
        {
            mode2D = newMode2D;
            modeButtons->blockSignals(true);
            modeButtons->button(mode2D ? 0 : 1)->setChecked(true);
            modeButtons->blockSignals(false);
            reflect->setMode2D(mode2D);
            UpdateOctantMenuContents();
        }
    }
    else if(!userSetMode)
    {
        bool oldMode2D = mode2D;
        mode2D = true;
        modeButtons->blockSignals(true);
        modeButtons->button(0)->setChecked(true);
        modeButtons->blockSignals(false);
        reflect->setMode2D(mode2D);
        if (oldMode2D != mode2D)
            UpdateOctantMenuContents();
    }

    if(setOctant)
    {
        octant->blockSignals(true);
        octant->setCurrentIndex(atts->GetOctant());
        octant->blockSignals(false);
        reflect->setOriginalOctant(operator2WidgetOctants[atts->GetOctant()]);
    }

    reflect->blockSignals(false);
}

// ****************************************************************************
// Method: QvisReflectWindow::UpdateOctantMenuContents
//
// Purpose:
//   Updates the octant menu's contents so it shows the appropriate choices.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 25 09:34:50 PDT 2003
//
// Modifications:
//   Jeremy Meredith, Mon Aug 18 11:38:56 PDT 2003
//   Removed Z axis from 2D labels.
//
//   Brad Whitlock, Thu Apr 24 16:55:58 PDT 2008
//   Added tr()'s
//
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::UpdateOctantMenuContents()
{
    octant->clear();
    if (mode2D)
    {
        octant->addItem(tr("+X  +Y"));
        octant->addItem(tr("-X  +Y"));
        octant->addItem(tr("+X  -Y"));
        octant->addItem(tr("-X  -Y"));
        reflectionLabel->setText(tr("Reflection quadrants"));
        originalDataLabel->setText(tr("Original data quadrant"));
    }
    else
    {
        octant->addItem(tr("+X  +Y  +Z"));
        octant->addItem(tr("-X  +Y  +Z"));
        octant->addItem(tr("+X  -Y  +Z"));
        octant->addItem(tr("-X  -Y  +Z"));
        octant->addItem(tr("+X  +Y  -Z"));
        octant->addItem(tr("-X  +Y  -Z"));
        octant->addItem(tr("+X  -Y  -Z"));
        octant->addItem(tr("-X  -Y  -Z"));
        reflectionLabel->setText(tr("Reflection octants"));
        originalDataLabel->setText(tr("Original data octant"));
    }
}

// ****************************************************************************
//  Method:  QvisReflectWindow::GetCurrentValues
//
//  Purpose:
//    Gets the values from the window text fields for the attributes.
//
//  Arguments:
//    which_field  an integer corresponding to the field to retrieve
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// Modifications:
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
//    Alister Maguire, Wed Apr 11 09:29:49 PDT 2018
//    Added updates for the plane point and plane normal.
//
// ****************************************************************************
void
QvisReflectWindow::GetCurrentValues(int which_field)
{
    bool doAll = (which_field == -1);
    QString msg;

    // Do specifiedX
    if(which_field == ReflectAttributes::ID_specifiedX || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedX, val))
            atts->SetSpecifiedX(val);
        else
        {
            ResettingError(tr("Specified X"),
                DoubleToQString(atts->GetSpecifiedX()));
            atts->SetSpecifiedX(atts->GetSpecifiedX());
        }
    }

    // Do specifiedY
    if(which_field == ReflectAttributes::ID_specifiedY || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedY, val))
            atts->SetSpecifiedY(val);
        else
        {
            ResettingError(tr("Specified Y"),
                DoubleToQString(atts->GetSpecifiedY()));
            atts->SetSpecifiedY(atts->GetSpecifiedY());
        }
    }

    // Do specifiedZ
    if(which_field == ReflectAttributes::ID_specifiedZ || doAll)
    {
        double val;
        if(LineEditGetDouble(specifiedZ, val))
            atts->SetSpecifiedZ(val);
        else
        {
            ResettingError(tr("Specified Z"),
                DoubleToQString(atts->GetSpecifiedZ()));
            atts->SetSpecifiedZ(atts->GetSpecifiedZ());
        }
    }

    // Do plane point
    if(which_field == ReflectAttributes::ID_planePoint || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planePoint, val, 3))
            atts->SetPlanePoint(val);
        else
        {
            ResettingError(tr("Plane Point"),
                DoublesToQString(atts->GetPlanePoint(), 3));
            atts->SetPlanePoint(atts->GetPlanePoint());
        }
    }

    // Do plane normal
    if(which_field == ReflectAttributes::ID_planeNormal || doAll)
    {
        double val[3];
        if(LineEditGetDoubles(planeNorm, val, 3))
            atts->SetPlaneNormal(val);
        else
        {
            ResettingError(tr("Plane Normal"),
                DoublesToQString(atts->GetPlaneNormal(), 3));
            atts->SetPlaneNormal(atts->GetPlaneNormal());
        }
    }
}

//
// Qt Slot functions.
//

// ****************************************************************************
// Method: QvisReflectWindow::selectOctants
//
// Purpose:
//   Slot for changing selected octants.
//
// Arguments:
//   octants : The selected octants.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 11 11:23:25 PDT 2003
//
// Modifications:
//
// ****************************************************************************

void
QvisReflectWindow::selectOctants(bool *octants)
{
    int r[8];
    for(int i = 0; i < 8; ++i)
    {
        int b = i ^ atts->GetOctant();
        r[b] = octants[widget2OperatorOctants[i]] ? 1  : 0;
    }

    atts->SetReflections(r);
    Apply();
}

// ****************************************************************************
//  Method:  QvisReflectWindow::octantChanged
//
//  Purpose:
//    Slot for the original octant changing.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
//  Modifications:
//    Brad Whitlock, Tue Mar 11 11:05:03 PDT 2003
//    I made it work with the new widget.
//
// ****************************************************************************

void
QvisReflectWindow::octantChanged(int val)
{
    ReflectAttributes::Octant Old = atts->GetOctant();
    ReflectAttributes::Octant New = ReflectAttributes::Octant(val);

    if(Old != New)
    {
        // must swap reflections to new match
        int r[8];
        bool b[8];
        reflect->blockSignals(true);
        int i;
        for (i=0; i<8; i++)
        {
            r[i ^ Old] = atts->GetReflections()[i];
        }
        for (i=0; i<8; i++)
        {
            b[widget2OperatorOctants[i]] = (r[i ^ New] != 0);
        }
        reflect->setValues(b);
        reflect->blockSignals(false);

        xUseData->setText((New&0x01) ? tr("Use dataset max") : tr("Use dataset min"));
        yUseData->setText((New&0x02) ? tr("Use dataset max") : tr("Use dataset min"));
        zUseData->setText((New&0x04) ? tr("Use dataset max") : tr("Use dataset min"));

        atts->SetOctant(New);
        Apply();
    }
}

// ****************************************************************************
//  Method:  QvisReflectWindow::?BoundaryChanged
//
//  Purpose:
//    Slot function to set the attributes for the type of boundary limits.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// ****************************************************************************
void
QvisReflectWindow::xBoundaryChanged(int val)
{
    atts->SetUseXBoundary(val==0 ? true : false);
    Apply();
}

void
QvisReflectWindow::yBoundaryChanged(int val)
{
    atts->SetUseYBoundary(val==0 ? true : false);
    Apply();
}

void
QvisReflectWindow::zBoundaryChanged(int val)
{
    atts->SetUseZBoundary(val==0 ? true : false);
    Apply();
}

// ****************************************************************************
//  Method:  QvisReflectWindow::specified?ProcessText
//
//  Purpose:
//    Slot that gets the current values for the specified reflection limits.
//
//  Programmer:  Jeremy Meredith
//  Creation:    March 11, 2002
//
// ****************************************************************************

void
QvisReflectWindow::specifiedXProcessText()
{
    GetCurrentValues(2);
    Apply();
}

void
QvisReflectWindow::specifiedYProcessText()
{
    GetCurrentValues(4);
    Apply();
}

void
QvisReflectWindow::specifiedZProcessText()
{
    GetCurrentValues(6);
    Apply();
}

// ****************************************************************************
//  Method:  QvisReflectWindow::planePointProcessText
//
//  Purpose:
//      Retrieve the user-input plane point.
//
//  Programmer:  Alister Maguire
//  Creation:    Apr 11, 2018
//
// ****************************************************************************
void
QvisReflectWindow::planePointProcessText()
{
    GetCurrentValues(ReflectAttributes::ID_planePoint);
}

// ****************************************************************************
//  Method:  QvisReflectWindow::planeNormProcessText
//
//  Purpose:
//      Retrieve the user-input plane normal.
//
//  Programmer:  Alister Maguire
//  Creation:    Apr 11, 2018
//
// ****************************************************************************
void
QvisReflectWindow::planeNormProcessText()
{
    GetCurrentValues(ReflectAttributes::ID_planeNormal);
}

// ****************************************************************************
// Method: QvisReflectWindow::selectMode
//
// Purpose:
//   Selects the input mode.
//
// Arguments:
//   mode : The new input mode.
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 25 12:44:20 PDT 2003
//
// Modifications:
//    Cyrus Harrison, Tue Aug 19 08:52:09 PDT 2008
//    Qt4 Port.
//
// ****************************************************************************

void
QvisReflectWindow::selectMode(int mode)
{
    if(mode == 0)
    {
        bool originIs3D = (operator2WidgetOctants[atts->GetOctant()] > 3);
        bool reflection3D = false;
        int *r = atts->GetReflections();
        bool octants[8];
        for (int i=0; i<8; i++)
        {
            int b = i ^ atts->GetOctant();
            octants[operator2WidgetOctants[i]] = (r[b] > 0);
            reflection3D |= (octants[operator2WidgetOctants[i]] && operator2WidgetOctants[i] > 3);
        }

        if(originIs3D || reflection3D)
        {
            Error(tr("The reflection attributes require the 3D input mode because "
                     "the original data octant or one or more of the reflection "
                     "octants has a negative Z value. The input mode will remain 3D."));
            mode2D = false;
            modeButtons->blockSignals(true);
            modeButtons->button(1)->setChecked(true);
            modeButtons->blockSignals(false);
        }
        else
        {
            if(!mode2D)
            {
                mode2D = true;
                reflect->setMode2D(mode2D);
                UpdateOctantMenuContents();
            }
        }
    }
    else
    {
        if(mode2D)
        {
            mode2D = false;
            reflect->setMode2D(mode2D);
            UpdateOctantMenuContents();
        }

        userSetMode = true;
    }
}

// ****************************************************************************
// Method: QvisReflectWindow::reflectTabsChangedIndex
//
// Purpose:
//     Set the reflect type based on the current tab selected.
//
// Arguments:
//     index    The index of the selected tab
//
// Programmer:  Alister Maguire
// Creation:    Apr 11, 2018
//
// Modifications:
//
// ****************************************************************************

void
QvisReflectWindow::reflectTabsChangedIndex(int index)
{
    switch(index)
    {
        case 0:
        {
            atts->SetReflectType(ReflectAttributes::Axis);
        }
        break;
        case 1:
        {
            atts->SetReflectType(ReflectAttributes::Plane);
        }
        break;
    }
}
