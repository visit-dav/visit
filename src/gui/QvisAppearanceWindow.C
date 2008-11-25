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

#include <QvisAppearanceWindow.h>
#include <QApplication>
#include <QCheckBox>
#include <QComboBox>
#include <QLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>

#include <QvisColorButton.h>
#include <QvisDialogLineEdit.h>
#include <AppearanceAttributes.h>
#include <ViewerProxy.h>

//
// Tables of available styles.
//
static const char *styleNamesInMenu[] = {
#ifdef Q_WS_MACX
"Macintosh",
#endif
#ifdef Q_WS_WIN
"Windows XP", "Windows Vista",
#endif
"Windows", "Motif", "CDE", "Plastique", "CleanLooks"
};

static const char *styleNames[] = {
#ifdef Q_WS_MACX
"macintosh",
#endif
#ifdef Q_WS_WIN
"windowsxp", "windowsvista",
#endif
"windows", "motif", "cde", "plastique", "cleanlooks"
};

// Determine the number of styles.
static const int numStyleNames = sizeof(styleNamesInMenu) / sizeof(const char *);

// ****************************************************************************
// Method: QvisAppearanceWindow::QvisAppearanceWindow
//
// Purpose: 
//   This is the constructor for the QvisAppearanceWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:27:16 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Apr  9 11:10:51 PDT 2008
//   QString for caption, shortName.
//
// ****************************************************************************

QvisAppearanceWindow::QvisAppearanceWindow(AppearanceAttributes *subj,
    const QString &caption, const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(subj, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton)
{
    // nothing here
}

// ****************************************************************************
// Method: QvisAppearanceWindow::~QvisAppearanceWindow
//
// Purpose: 
//   This is the destructor for the QvisAppearanceWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:28:27 PDT 2001
//
// Modifications:
//   
// ****************************************************************************

QvisAppearanceWindow::~QvisAppearanceWindow()
{
    // Nothing here
}

// ****************************************************************************
// Method: QvisAppearanceWindow::CreateWindowContents
//
// Purpose: 
//   This method creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:31:03 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:16:20 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Fri Aug 15 13:08:37 PST 2003
//   I changed how the style menu is populated.
//
//   Brad Whitlock, Thu Mar 15 15:25:51 PST 2007
//   Added font support.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Thu Jun 19 11:11:54 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Mon Nov 24 11:57:42 PST 2008
//   Support for default system appearance.
//
// ****************************************************************************

void
QvisAppearanceWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(0);
    mainLayout->setSpacing(10);
    topLayout->addLayout(mainLayout);

    int row = 0;
    
    useSysDefaultCheckBox = new QCheckBox(central);
    connect(useSysDefaultCheckBox , SIGNAL(stateChanged(int)),
            this, SLOT(useSysDefaultChanged(int)));
    useSysDefaultCheckBox->setText(
      tr("Use Default System Appearance (Applied at VisIt startup)"));
    mainLayout->addWidget(useSysDefaultCheckBox,row,0,1,3);
    row++;
    
    // Create the background color button.
    backgroundColorButton = new QvisColorButton(central);
    connect(backgroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(backgroundChanged(const QColor &)));
    mainLayout->addWidget(backgroundColorButton, row, 1);
    backgroundColorLabel = new QLabel(tr("GUI background"), central);
    backgroundColorLabel->setBuddy(backgroundColorButton);
    mainLayout->addWidget(backgroundColorLabel, row, 0);
    row++;

    // Create the background color button.
    foregroundColorButton = new QvisColorButton(central);
    connect(foregroundColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(foregroundChanged(const QColor &)));
    mainLayout->addWidget(foregroundColorButton, row, 1);
    foregroundColorLabel = new QLabel(tr("GUI foreground"), central);
    foregroundColorLabel->setBuddy(foregroundColorButton);
    mainLayout->addWidget(foregroundColorLabel, row, 0);
    row++;
    
    // Create the style combo box.
    styleComboBox = new QComboBox(central);
    for(int i = 0; i < numStyleNames; ++i)
        styleComboBox->addItem(styleNamesInMenu[i]);
    connect(styleComboBox, SIGNAL(activated(int)),
            this, SLOT(styleChanged(int)));
    mainLayout->addWidget(styleComboBox, row, 1);
    styleLabel = new QLabel(tr("GUI style"), central);
    styleLabel->setBuddy(styleComboBox);
    mainLayout->addWidget(styleLabel, row, 0);
    row++;

    // Create the orientation combo box.
    orientationComboBox = new QComboBox(central);
    orientationComboBox->addItem(tr("Vertical"));
    orientationComboBox->addItem(tr("Horizontal"));
    connect(orientationComboBox, SIGNAL(activated(int)),
            this, SLOT(orientationChanged(int)));
    mainLayout->addWidget(orientationComboBox, row, 1);
    orientationLabel = new QLabel(tr("GUI orientation"), central);
    orientationLabel->setBuddy(orientationComboBox);
    mainLayout->addWidget(orientationLabel, row, 0);
    row++;

    // Create the font edit.
    fontName = new QvisDialogLineEdit(central);
    fontName->setDialogMode(QvisDialogLineEdit::ChooseFont);
    connect(fontName, SIGNAL(textChanged(const QString &)),
            this, SLOT(fontNameChanged(const QString &)));
    mainLayout->addWidget(fontName, row, 1, 1, 2);
    fontLabel = new QLabel(tr("GUI font"), central);
    fontLabel->setBuddy(fontName);
    mainLayout->addWidget(fontLabel, row, 0);
    row++;
}

// ****************************************************************************
// Method: QvisAppearanceWindow::UpdateWindow
//
// Purpose: 
//   This method is called when the appearance attributes are updated.
//
// Arguments:
//   doAll : Whether or not to update all widgets.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:57:29 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:14:42 PST 2002
//   Added orientation combo box.
//
//   Brad Whitlock, Tue Apr 1 09:15:14 PDT 2003
//   I made it use QColor's parsing instead of sscanf.
//
//   Brad Whitlock, Fri Aug 15 13:10:28 PST 2003
//   I changed how the styles are set.
//
//   Brad Whitlock, Thu Mar 15 15:26:07 PST 2007
//   Added font support.
//
//   Brad Whitlock, Fri Dec 14 16:57:53 PST 2007
//   Made it use ids.
//
//   Brad Whitlock, Thu Jun 19 11:20:42 PDT 2008
//   Qt 4.
//
//   Cyrus Harrison, Mon Nov 24 11:57:42 PST 2008
//   Support for default system appearance.
//
// ****************************************************************************

void
QvisAppearanceWindow::UpdateWindow(bool doAll)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    int  j;

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case AppearanceAttributes::ID_useSystemDefault:
            { // new scope
            bool val = atts->GetUseSystemDefault();
            useSysDefaultCheckBox->blockSignals(true);
            if(val)
                useSysDefaultCheckBox->setCheckState(Qt::Checked);
            else
                useSysDefaultCheckBox->setCheckState(Qt::Unchecked);
            useSysDefaultCheckBox->blockSignals(false);
            }
            break;
        case AppearanceAttributes::ID_background:
            { // new scope
            QColor bg(atts->GetBackground().c_str());
            backgroundColorButton->blockSignals(true);
            backgroundColorButton->setButtonColor(bg);
            backgroundColorButton->blockSignals(false);
            }
            break;
        case AppearanceAttributes::ID_foreground:
            { // new scope
            QColor fg(atts->GetForeground().c_str());
            foregroundColorButton->blockSignals(true);
            foregroundColorButton->setButtonColor(fg);
            foregroundColorButton->blockSignals(false);
            }
            break;
        case AppearanceAttributes::ID_fontName:
            fontName->blockSignals(true);
            fontName->setText(atts->GetFontName().c_str());
            fontName->blockSignals(false);
            break;
        case AppearanceAttributes::ID_style:
            for(j = 0; j < numStyleNames; ++j)
            {
                if(atts->GetStyle() == styleNames[j])
                {
                    styleComboBox->blockSignals(true);
                    styleComboBox->setCurrentIndex(j);
                    styleComboBox->blockSignals(false);
                    break;
                }
            }
            break;
        case AppearanceAttributes::ID_orientation:
            orientationComboBox->blockSignals(true);
            if(atts->GetOrientation() == 0)
                orientationComboBox->setCurrentIndex(0);
            else
                orientationComboBox->setCurrentIndex(1);
            orientationComboBox->blockSignals(false);
            break;
        }
    }
    
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisAppearanceWindow::UpdateWindowSensitivity
//
// Purpose: 
//   This method is called to update window sensitivity
//
//
// Programmer: Cyrus Harrison
// Creation:   Mon Nov 24 10:28:26 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisAppearanceWindow::UpdateWindowSensitivity()
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    bool val = !atts->GetUseSystemDefault();
    backgroundColorButton->setEnabled(val);
    backgroundColorLabel->setEnabled(val);
    foregroundColorButton->setEnabled(val);
    foregroundColorLabel->setEnabled(val);
    fontName->setEnabled(val);
    fontLabel->setEnabled(val);
    styleComboBox->setEnabled(val);
    styleLabel->setEnabled(val);
    orientationComboBox->setEnabled(val);
    orientationLabel->setEnabled(val);
    
}

// ****************************************************************************
// Method: QvisAppearanceWindow::GetCurrentValues
//
// Purpose: 
//   This method gets the current values from line edits.
//
// Arguments:
//   which : The index of the widget that we want to get.
//
// Programmer: Brad Whitlock
// Creation:   Thu Mar 15 15:45:17 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::GetCurrentValues(int which)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;

    if(which == 0 || which == -1)
    {
        std::string newFontName(fontName->text().toStdString());
        if(QFont().fromString(newFontName.c_str()))
            atts->SetFontName(newFontName);
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::Apply
//
// Purpose: 
//   This method applies the changes to the appearance.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:15:11 PST 2001
//
// Modifications:
//   Brad Whitlock, Tue Sep 18 13:58:39 PST 2001
//   Removed an unused variable.
//
//   Brad Whitlock, Thu Mar 15 15:51:22 PST 2007
//   Call GetCurrentValues.
//
// ****************************************************************************

void
QvisAppearanceWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);

        emit changeAppearance(true);
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::apply
//
// Purpose: 
//   This is a Qt slot function that is called when the apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:09 PST 2001
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisAppearanceWindow::ColorsNotTooClose
//
// Purpose: 
//   Prevents bad colors from being used together.
//
// Arguments:
//   c0    : The first color to check.
//   c1str : The string representation of the second color to check.
//
// Returns:    True if the colors can be used together; false otherwise.
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 3 10:02:10 PDT 2003
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
// 
// ****************************************************************************

bool
QvisAppearanceWindow::ColorsNotTooClose(const QColor &c0, const char *c1str)
{
    QColor c1(c1str);

    int dR = int(c0.red()) - int(c1.red());
    int dG = int(c0.green()) - int(c1.green());
    int dB = int(c0.blue()) - int(c1.blue());

    const int threshold = 10;
    bool rClose = dR > -threshold && dR < threshold;
    bool gClose = dG > -threshold && dG < threshold;
    bool bClose = dB > -threshold && dB < threshold;

    bool retval = true;
    if(rClose && gClose && bClose)
    {
        // Update the window so the color buttons get set back properly.
        UpdateWindow(true);

        // Tell the user that it was a bad idea.
        Warning(tr("The background color and foreground color will not be "
                   "changed because the selected colors are too similar and "
                   "using them would make it too difficult to use VisIt."));
        retval = false;
    }

    return retval;
}

// ****************************************************************************
// Method: QvisAppearanceWindow::useSysDefaultChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the 
//   the "Use System Default Appearance" Check Box.
//
// Arguments:
//   state : The check state.
//
// Programmer: Cyrus Harrison
// Creation:   Mon Nov 24 10:20:10 PST 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisAppearanceWindow::useSysDefaultChanged(int state)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetUseSystemDefault(state == Qt::Checked);
    SetUpdate(false);
    Apply();
    UpdateWindowSensitivity();
}

// ****************************************************************************
// Method: QvisAppearanceWindow::backgroundChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the GUI
//   background color via the color button.
//
// Arguments:
//   bg : The new background color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:36 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 10:03:19 PDT 2003
//   Prevented bad colors from being used together.
//
// ****************************************************************************

void
QvisAppearanceWindow::backgroundChanged(const QColor &bg)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;

    if(ColorsNotTooClose(bg, atts->GetForeground().c_str()))
    {
        QString tmp;
        tmp.sprintf("#%02x%02x%02x", bg.red(), bg.green(), bg.blue());
        atts->SetBackground(tmp.toStdString());
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::foregroundChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the GUI
//   foreground color via the color button.
//
// Arguments:
//   fg : The new foreground color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:16:36 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Oct 3 10:03:19 PDT 2003
//   Prevented bad colors from being used together.
//   
// ****************************************************************************

void
QvisAppearanceWindow::foregroundChanged(const QColor &fg)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;

    if(ColorsNotTooClose(fg, atts->GetBackground().c_str()))
    {
        QString tmp;
        tmp.sprintf("#%02x%02x%02x", fg.red(), fg.green(), fg.blue());
        atts->SetForeground(tmp.toStdString());
        SetUpdate(false);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisAppearanceWindow::styleChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the user changes the style.
//
// Arguments:
//   index : The index of the new style.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 13:17:42 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Aug 15 13:11:13 PST 2003
//   I made it use a table lookup.
//
// ****************************************************************************

void
QvisAppearanceWindow::styleChanged(int index)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetStyle(styleNames[index]);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAppearanceWindow::fontNameChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the Font... button is
//   clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 19:38:04 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Mar 15 15:31:57 PST 2007
//   Rewrote.
//
// ****************************************************************************

void
QvisAppearanceWindow::fontNameChanged(const QString &newFont)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetFontName(newFont.toStdString());
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisAppearanceWindow::orientationChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the orientation is changed.
//
// Arguments:
//   index : The new orientation.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jan 29 13:21:26 PST 2002
//
// Modifications:
//   
// ****************************************************************************

void
QvisAppearanceWindow::orientationChanged(int index)
{
    AppearanceAttributes *atts = (AppearanceAttributes *)subject;
    atts->SetOrientation((index == 0) ? 0 : 2);
    SetUpdate(false);
    Apply();
}
