/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include <QvisLightingWindow.h>
#include <stdio.h>

#include <QLabel>
#include <QLayout>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QPixmap>
#include <QRadioButton>
#include <QvisColorButton.h>
#include <QvisLightingWidget.h>
#include <QvisOpacitySlider.h>
#include <QLineEdit>
#include <QSpinBox>

#include <ColorAttribute.h>
#include <LightAttributes.h>
#include <LightList.h>
#include <ViewerProxy.h>

#include <icons/light2.xpm>
#include <icons/light3.xpm>

// ****************************************************************************
// Method: QvisLightingWindow::QvisLightingWindow
//
// Purpose: 
//   This is the constructor for the QvisLightingWindow class.
//
// Arguments:
//   subj      : The light list to be observed.
//   caption   : The caption of the window.
//   shortName : The name the window uses when posted.
//   notepad   : The notepad area to which the window will post.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:08:39 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:34:46 PST 2002
//   Initialized parentless widgets.
//
//   Brad Whitlock, Mon Aug 26 14:40:28 PST 2002
//   I initialized the mode member.
//
//   Brad Whitlock, Wed Apr  9 11:08:16 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisLightingWindow::QvisLightingWindow(LightList *subj, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::AllExtraButtons,
                               false)
{
    lights = subj;
    activeLight = 0;
    mode = 0;

    // Initialize parentless widgets.
    modeButtons = 0;
}

// ****************************************************************************
// Method: QvisLightingWindow::~QvisLightingWindow
//
// Purpose: 
//   This is the destructor for the QvisLightingWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:10:55 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Feb 15 15:34:19 PST 2002
//   Deleted parentless widgets.
//
//   Cyrus Harrison, Wed Aug 27 08:28:22 PDT 2008
//   Set parent for button groups so we dont have to explicitly clean them up.
//
// ****************************************************************************

QvisLightingWindow::~QvisLightingWindow()
{
}

// ****************************************************************************
// Method: QvisLightingWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the window's widgets.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:11:24 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 15:21:51 PST 2002
//   I renamed a couple light types and added a light icon.
//
//   Brad Whitlock, Wed Mar 26 08:12:00 PDT 2003
//   I added a brightness spin box.
//
//   Brad Whitlock, Wed Feb 23 18:01:42 PST 2005
//   I made the brightness spin box use a different slot.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisLightingWindow::CreateWindowContents()
{
    QGridLayout *gLayout = new QGridLayout();
    topLayout->addLayout(gLayout);

    //
    // Create light icons.
    //
    onLightIcon = new QPixmap(light2_xpm);
    offLightIcon = new QPixmap(light3_xpm);

    //
    // Create the mode setting radio buttons.
    //
    QLabel *modeLabel = new QLabel(tr("Mode"), central);
    gLayout->addWidget(modeLabel, 0, 0);

    modeButtons = new QButtonGroup(central);
    connect(modeButtons, SIGNAL(buttonClicked(int)),
            this, SLOT(modeClicked(int)));
    QRadioButton *rb = new QRadioButton(tr("Edit"), central);
    modeButtons->addButton(rb, 0);
    gLayout->addWidget(rb, 0, 1, Qt::AlignLeft);
    rb = new QRadioButton(tr("Preview"), central);
    modeButtons->addButton(rb, 1);
    gLayout->addWidget(rb, 0, 2, Qt::AlignLeft);

    //
    // Create the active light combo box.
    //
    activeLightComboBox = new QComboBox(central);
    activeLightComboBox->addItem("1");
    activeLightComboBox->addItem("2");
    activeLightComboBox->addItem("3");
    activeLightComboBox->addItem("4");
    activeLightComboBox->addItem("5");
    activeLightComboBox->addItem("6");
    activeLightComboBox->addItem("7");
    activeLightComboBox->addItem("8");
    connect(activeLightComboBox, SIGNAL(activated(int)),
            this, SLOT(activeLightComboBoxChanged(int)));
    activeLightLabel = new QLabel(tr("Active light"),central);
    gLayout->addWidget(activeLightLabel, 0, 3, Qt::AlignRight);
    gLayout->addWidget(activeLightComboBox, 0, 4, Qt::AlignLeft);

    //
    // Create the 3D area that lets users move the light sources.
    //
    lightWidget = new QvisLightingWidget(central);
    connect(lightWidget, SIGNAL(lightMoved(double,double,double)),
            this, SLOT(lightMoved(double,double,double)));
    gLayout->addWidget(lightWidget, 1, 0, 1, 3);

    //
    // Create the light properties group box.
    //
    lightGroupBox = new QGroupBox(central);
    lightGroupBox->setTitle(tr("Properties"));
    gLayout->addWidget(lightGroupBox, 1, 3, 1, 2);
    gLayout->setMargin(10);
    QGridLayout *sLayout = new QGridLayout(lightGroupBox);
    sLayout->setMargin(10);
    sLayout->setSpacing(10);
    sLayout->setRowMinimumHeight(0, 15);

    lightTypeComboBox = new QComboBox(lightGroupBox);
    lightTypeComboBox->addItem(tr("Ambient"));
    lightTypeComboBox->addItem(tr("Object"));
    lightTypeComboBox->addItem(tr("Camera"));
    connect(lightTypeComboBox, SIGNAL(activated(int)),
            this, SLOT(lightTypeComboBoxChanged(int)));
    sLayout->addWidget(lightTypeComboBox, 1, 1);
    QLabel *typeLabel = new QLabel(tr("Light type"),lightGroupBox);
    sLayout->addWidget(typeLabel, 1, 0);

    lightDirectionLineEdit = new QLineEdit(lightGroupBox);
    connect(lightDirectionLineEdit, SIGNAL(returnPressed()),
            this, SLOT(processLineDirectionText()));
    sLayout->addWidget(lightDirectionLineEdit, 2, 1);
    lightDirectionLabel = new QLabel(tr("Direction"), lightGroupBox);
    sLayout->addWidget(lightDirectionLabel, 2, 0);

    lightColorButton = new QvisColorButton(lightGroupBox);
    connect(lightColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedLightColor(const QColor &)));
    sLayout->addWidget(lightColorButton, 3, 1, Qt::AlignLeft);
    QLabel *colorLabel = new QLabel(tr("Color"),lightGroupBox);
    sLayout->addWidget(colorLabel, 3, 0);

    lightBrightness = new QvisOpacitySlider(lightGroupBox);
    lightBrightness->setMinimum(0);
    lightBrightness->setMaximum(100);
    connect(lightBrightness, SIGNAL(valueChanged(int)),
            this, SLOT(brightnessChanged(int)));
    QLabel *brightnessLabel = new QLabel(tr("Brightness"),lightGroupBox);
    sLayout->addWidget(brightnessLabel, 4, 0);
    sLayout->addWidget(lightBrightness, 4, 1);

    lightBrightnessSpinBox = new QSpinBox(lightGroupBox);
    lightBrightnessSpinBox->setRange(0,100);
    lightBrightnessSpinBox->setSingleStep(1);
    lightBrightnessSpinBox->setSuffix("%");
    connect(lightBrightnessSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(brightnessChanged2(int)));
    sLayout->addWidget(lightBrightnessSpinBox, 5, 1);

    lightEnabledCheckBox = new QCheckBox(tr("Enabled"), lightGroupBox);
    connect(lightEnabledCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(enableToggled(bool)));
    sLayout->addWidget(lightEnabledCheckBox, 6, 1, Qt::AlignLeft);

    sLayout->setRowStretch(7, 5);

    // Set the enabled state of certain widgets based on state that is not
    // kept in the state object.
    modeButtons->blockSignals(true);
    modeButtons->button(0)->setChecked(true);
    modeButtons->blockSignals(false);
}

// ****************************************************************************
// Method: QvisLightingWindow::UpdateWindow
//
// Purpose: 
//   This method updates the window's widgets when the light list changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:12:21 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 15:18:22 PST 2002
//   I made the direction text field enabled for camera lights. I also added
//   icons in the active light combobox that show which lights are enabled.
//
//   Brad Whitlock, Wed Mar 26 09:22:15 PDT 2003
//   I added a spinbox for light brightness.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisLightingWindow::UpdateWindow(bool)
{
    // Make all enabled lights have a light icon.
    activeLightComboBox->blockSignals(true);
    if (activeLight == 0)
    {
        lightEnabledCheckBox->setEnabled(false);
    }
    else
    {
        lightEnabledCheckBox->setEnabled(true);
    }
    for(int i = 0; i < activeLightComboBox->count(); ++i)
    {
        bool enabled = lights->GetLight(i).GetEnabledFlag();
        QString num;
        num.sprintf("%d", i + 1);
        if(enabled)
        {
            activeLightComboBox->setItemText(i,num);
            activeLightComboBox->setItemIcon(i,*onLightIcon);
        }
        else
        {
            activeLightComboBox->setItemText(i,num);
            activeLightComboBox->setItemIcon(i,*offLightIcon);
        }
    }
    activeLightComboBox->blockSignals(false);

    const LightAttributes &light = lights->GetLight(activeLight);

    // Update the light type combobox.
    lightTypeComboBox->blockSignals(true);
    lightTypeComboBox->setCurrentIndex(light.GetType());
    lightTypeComboBox->blockSignals(false);

    // Update the enabled toggle.
    lightEnabledCheckBox->blockSignals(true);
    lightEnabledCheckBox->setChecked(light.GetEnabledFlag());
    lightEnabledCheckBox->blockSignals(false);

    // Update the light direction line edit.
    QString tmp;
    tmp.sprintf("%1.3g %1.3g %1.3g", light.GetDirection()[0],
                                     light.GetDirection()[1],
                                     light.GetDirection()[2]);
    lightDirectionLineEdit->setText(tmp);
    bool val = (light.GetType() != LightAttributes::Ambient);
    lightDirectionLineEdit->setEnabled(val);
    lightDirectionLabel->setEnabled(val);

    // Update the color button.
    lightColorButton->blockSignals(true);
    QColor c(light.GetColor().Red(), light.GetColor().Green(),
             light.GetColor().Blue());
    lightColorButton->setButtonColor(c);
    lightColorButton->blockSignals(false);

    // Update the slider.
    lightBrightness->blockSignals(true);
    lightBrightness->setGradientColor(c);
    lightBrightness->setValue(int(light.GetBrightness() * 100));
    lightBrightness->blockSignals(false);

    // Update the brightness spin box.
    lightBrightnessSpinBox->blockSignals(true);
    lightBrightnessSpinBox->setValue(int(light.GetBrightness() * 100));
    lightBrightnessSpinBox->blockSignals(false);

    // Update the light displayed in the light widget.
    UpdateLightWidget();
}

// ****************************************************************************
// Method: QvisLightingWindow::UpdateLightWidget
//
// Purpose: 
//   Updates the light widget using the state in the light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:14:22 PST 2001
//
// Modifications:
//   Kathleen Bonnell, Tue Dec 28 16:20:47 PST 2004
//   Cast args for QColor constructor to int to prevent comiler warnings.
//   
//   Kathleen Bonnell, Mon Feb  6 16:58:40 PST 2006 
//   Removed unnecessary code that counted 'numEnabled' as it was not used. 
//   
// ****************************************************************************

void
QvisLightingWindow::UpdateLightWidget()
{
    // Update the light displayed in the light widget.
    if(mode == 1)
    {
        lightWidget->setUpdatesEnabled(false);
        lightWidget->clearLights();

        for(int i = 0; i < lights->NumLights(); ++i)
        {
            const LightAttributes &light = lights->GetLight(i);
           
            if(light.GetEnabledFlag())
            {
                QColor c2((int)(light.GetColor().Red() * light.GetBrightness()),
                          (int)(light.GetColor().Green() * light.GetBrightness()),
                          (int)(light.GetColor().Blue() * light.GetBrightness()));

                lightWidget->addLight(light.GetDirection(),
                                      c2, light.GetType());
            }
        }

        // Take care of the case when there are no active lights.
        lightWidget->setUpdatesEnabled(true);
        lightWidget->setPreviewMode(true);
    }
    else // mode == edit. Add all the enabled lights.
    {
        lightWidget->setUpdatesEnabled(false);
        lightWidget->clearLights();
        lightWidget->setPreviewMode(false);
        lightWidget->setUpdatesEnabled(true);

        const LightAttributes &light = lights->GetLight(activeLight);
        QColor c2((int)(light.GetColor().Red() * light.GetBrightness()),
                  (int)(light.GetColor().Green() * light.GetBrightness()),
                  (int)(light.GetColor().Blue() * light.GetBrightness()));

        lightWidget->addLight(light.GetDirection(),
                              c2, light.GetType());
    }
}

// ****************************************************************************
// Method: QvisLightingWindow::GetCurrentValues
//
// Purpose: 
//   This method is called in order to get the state for the direction text.
//
// Arguments:
//   which_widget : The index of the widget whose state we're getting.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:13:08 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//   
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
// ****************************************************************************

void
QvisLightingWindow::GetCurrentValues(int which_widget)
{
    bool    okay, doAll = (which_widget == -1);
    double  vals[3];
    QString msg, temp;

    // Do the direction vector
    if(which_widget == 0 || doAll)
    {
        LightAttributes &light = lights->GetLight(activeLight);
        temp = lightDirectionLineEdit->displayText().simplified();
        okay = !temp.isEmpty();
        if(okay)
        {
            okay = (sscanf(temp.toStdString().c_str(), "%lg %lg %lg",
                           &vals[0], &vals[1], &vals[2]) == 3);
            if(okay)
            {
                light.SetDirection(vals);
                lights->SelectLight(activeLight);
            }
        }

        if(!okay)
        {
            const double *d = light.GetDirection();
            QString num; num.sprintf("<%g %g %g>.", d[0], d[1], d[2]);
            msg = tr("The direction vector was invalid. "
                     "Resetting to the last good value %1.").arg(num);
            Message(msg);
            light.SetDirection(d);
            lights->SelectLight(activeLight);
        }
    }
}

// ****************************************************************************
// Method: QvisLightingWindow::Apply
//
// Purpose: 
//   This method applies the light list.
//
// Arguments:
//   ignore : If this is true, make the viewer use the new light list.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:14:50 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 23 17:38:24 PST 2005
//   I made GetCurrentValues be called all the time.
//
// ****************************************************************************

void
QvisLightingWindow::Apply(bool ignore)
{
    // This is a little different from how it is normally done in a window
    // but we're setting attributes for an individual object in a list of 
    // objects and we have to get the current values all the time in case
    // some of them changed because a full update is going to cause all
    // light attributes for the active light to be selected, which could
    // cause text widgets like the light direction to get reset.
    GetCurrentValues(-1);

    if(AutoUpdate() || ignore)
    {
        lights->Notify();

        GetViewerMethods()->SetLightList();
    }
    else
        lights->Notify();
}

//
// Qt slot functions.
//

// ****************************************************************************
// Method: QvisLightingWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the apply button is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:15:41 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisLightingWindow::makeDefault
//
// Purpose: 
//   This is a Qt slot function that is called when the "make default" button
//   is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:16:22 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::makeDefault()
{
    GetCurrentValues(-1);
    lights->Notify();

    GetViewerMethods()->SetDefaultLightList();
}

// ****************************************************************************
// Method: QvisLightingWindow::reset
//
// Purpose: 
//   This is a Qt slot function that is called when the "reset" button is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:17:03 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::reset()
{
    GetViewerMethods()->ResetLightList();
}

// ****************************************************************************
// Method: QvisLightingWindow::activeLightComboBoxChanged
//
// Purpose: 
//   This is a Qt slot function that is called when we change the active light.
//
// Arguments:
//   index : The index of the new active light.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:18:13 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::activeLightComboBoxChanged(int index)
{
    GetCurrentValues(-1);
    activeLight = index;
    UpdateWindow(true);
}

// ****************************************************************************
// Method: QvisLightingWindow::brightnessChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the brightness changes.
//
// Arguments:
//   val : The new brightness value.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:18:56 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 23 18:00:17 PST 2005
//   Made the routines set their partner widget directly so we don't have to
//   use UpdateWindow, called by Apply since that interferes with the slider
//   when you hold down on the +/- arrows.
//
// ****************************************************************************

void
QvisLightingWindow::brightnessChanged(int val)
{
    LightAttributes &light = lights->GetLight(activeLight);
    light.SetBrightness(0.01 * val);
    lights->SelectLight(activeLight);

    // Update the spin box.
    lightBrightnessSpinBox->blockSignals(true);
    lightBrightnessSpinBox->setValue(val);
    lightBrightnessSpinBox->blockSignals(false);

    SetUpdate(false);
    Apply();
}

void
QvisLightingWindow::brightnessChanged2(int val)
{
    LightAttributes &light = lights->GetLight(activeLight);
    light.SetBrightness(0.01 * val);
    lights->SelectLight(activeLight);

    // Update the slider
    lightBrightness->blockSignals(true);
    lightBrightness->setValue(val);
    lightBrightness->blockSignals(false);

    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLightingWindow::enableToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the light is enabled or
//   disabled.
//
// Arguments:
//   val : The new state of the light.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:19:27 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::enableToggled(bool val)
{
    LightAttributes &light = lights->GetLight(activeLight);
    light.SetEnabledFlag(val);
    lights->SelectLight(activeLight);
    Apply();
}

// ****************************************************************************
// Method: QvisLightingWindow::lightMoved
//
// Purpose: 
//   This is a Qt slot function that is called when the active light is
//   moved interactively.
//
// Arguments:
//   x : The new x component of the light direction vector.
//   y : The new y component of the light direction vector.
//   z : The new z component of the light direction vector.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:20:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Aug 19 15:24:06 PST 2002
//   I prevented the light's direction from changing if it is an ambient
//   light and changed the code so that it will accept new light directions
//   when autoupdate mode is on.
//
//   Brad Whitlock, Mon Aug 26 14:24:40 PST 2002
//   I added some code back in to set the light's direction directly so it
//   works when autoupdate is not on.
//
//   Brad Whitlock, Tue May 20 15:03:27 PST 2003
//   Made it work with updated LightAttributes.
//
// ****************************************************************************

void
QvisLightingWindow::lightMoved(double x, double y, double z)
{
    if(mode == 0)
    {
        LightAttributes &light = lights->GetLight(activeLight);
        if(light.GetType() != LightAttributes::Ambient)
        {
            // Set the light's new direction.
            double d[3] = {x, y, z};
            light.SetDirection(d);
            lights->SelectLight(activeLight);
            // Set the light direction into the direction line edit so
            // it works when autoupdate is on.
            QString direction;
            direction.sprintf("%1.3g %1.3g %1.3g", x, y, z);
            lightDirectionLineEdit->setText(direction);
            SetUpdate(false);
            Apply();
        }
    }
}

// ****************************************************************************
// Method: QvisLightingWindow::lightTypeComboBoxChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the light type is changed.
//
// Arguments:
//   newType : The new light type.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:21:16 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::lightTypeComboBoxChanged(int newType)
{
    LightAttributes &light = lights->GetLight(activeLight);
    light.SetType(LightAttributes::LightType(newType));
    lights->SelectLight(activeLight);
    Apply();
}

// ****************************************************************************
// Method: QvisLightingWindow::modeClicked
//
// Purpose: 
//   This is a Qt slot function that is called when we want to change the
//   window's mode from edit<->preview.
//
// Arguments:
//   index : The new mode.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:21:52 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::modeClicked(int index)
{
    mode = index;

    // Disable the group box and add all lights to the light widget.
    lightGroupBox->setEnabled(mode == 0);
    activeLightLabel->setEnabled(mode == 0);
    activeLightComboBox->setEnabled(mode == 0);

    // Update the light widget for the new mode.
    UpdateLightWidget();
}

// ****************************************************************************
// Method: QvisLightingWindow::processLineDirectionText
//
// Purpose: 
//   This is a Qt slot function that is called when the direction text changes.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:22:40 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::processLineDirectionText()
{
    GetCurrentValues(0);
    Apply();
}

// ****************************************************************************
// Method: QvisLightingWindow::selectedLightColor
//
// Purpose: 
//   This is a Qt slot function that is called when the light color changes.
//
// Arguments:
//   c : The new light color.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 19 16:23:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisLightingWindow::selectedLightColor(const QColor &c)
{
    LightAttributes &light = lights->GetLight(activeLight);
    ColorAttribute C(c.red(), c.green(), c.blue());
    light.SetColor(C);
    lights->SelectLight(activeLight);
    Apply();
}
