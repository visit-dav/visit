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

#include "QvisSpreadsheetPlotWindow.h"

#include <SpreadsheetAttributes.h>
#include <ViewerProxy.h>

#include <QCheckBox>
#include <QComboBox>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QSpinBox>
#include <QWidget>
#include <QButtonGroup>
#include <QRadioButton>
#include <QvisColorTableButton.h>
#include <QvisOpacitySlider.h>
#include <QvisColorButton.h>
#include <QvisLineStyleWidget.h>
#include <QvisLineWidthWidget.h>
#include <QvisVariableButton.h>
#include <QvisDialogLineEdit.h>

#include <stdio.h>
#include <string>

#include <BadIndexException.h>

#include <SILRestrictionAttributes.h>
#include <avtSILRestriction.h>
#include <avtSILRestrictionTraverser.h>

const char *QvisSpreadsheetPlotWindow::defaultItem = "Whole";

// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::QvisSpreadsheetPlotWindow
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   Brad Whitlock, Mon Aug 11 16:04:41 PDT 2008
//   Initialize normal.
//
// ****************************************************************************

QvisSpreadsheetPlotWindow::QvisSpreadsheetPlotWindow(const int type,
                         SpreadsheetAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisPostableWindowObserver(subj, caption, shortName, notepad)
{
    plotType = type;
    atts = subj;

    normal = 0;

    // We will observe the SILRestriction attributes too so we can display
    // the available subsets in the window.
    silTopSet = -1;
    silNumSets = -1;
    silNumCollections = -1;
    GetViewerState()->GetSILRestrictionAttributes()->Attach(this);
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::~QvisSpreadsheetPlotWindow
//
// Purpose: 
//   Destructor
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   Brad Whitlock, Mon Aug 11 16:04:33 PDT 2008
//   deleted normal.
//
// ****************************************************************************

QvisSpreadsheetPlotWindow::~QvisSpreadsheetPlotWindow()
{
    GetViewerState()->GetSILRestrictionAttributes()->Detach(this);

    delete normal;
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::CreateWindowContents
//
// Purpose: 
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   Gunther H. Weber, Thu Sep 27 12:05:14 PDT 2007
//   Added font selection for spreadsheet
//   
//   Gunther H. Weber, Wed Oct 17 14:48:16 PDT 2007
//   Support toggling patch outline and tracer plane separately
//
//   Gunther H. Weber, Wed Nov 28 15:37:17 PST 2007
//   Support toggeling the current cell outline
//
//   Brad Whitlock, Wed Apr 23 11:36:41 PDT 2008
//   Added tr()'s
//
//   Brad Whitlock, Mon Aug 11 16:03:49 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::CreateWindowContents()
{
    QGridLayout *mainLayout = new QGridLayout(0);
    topLayout->addLayout(mainLayout);

    subsetNameLabel = new QLabel(tr("Subset name"), central);
    mainLayout->addWidget(subsetNameLabel,0,0);
    subsetName = new QComboBox(central);
    subsetName->setEditable(false);
    subsetName->addItem(defaultItem);
    connect(subsetName, SIGNAL(activated(const QString &)),
            this, SLOT(subsetNameChanged(const QString &)));
    mainLayout->addWidget(subsetName, 0,1);

    QFrame *splitter = new QFrame(central);
    splitter->setFrameStyle(QFrame::HLine + QFrame::Raised);
    mainLayout->addWidget(splitter, 1, 0, 1, 2);

    normalLabel = new QLabel(tr("Normal"), central);
    mainLayout->addWidget(normalLabel,2,0);
    normal = new QButtonGroup(0);
    QWidget *normalWidget = new QWidget(central);
    QHBoxLayout *normalLayout = new QHBoxLayout(normalWidget);
    normalLayout->setMargin(0);
    normalLayout->setSpacing(10);
    QRadioButton *normalNormalAxisX = new QRadioButton(tr("X"), normalWidget);
    normal->addButton(normalNormalAxisX, 0);
    normalLayout->addWidget(normalNormalAxisX);
    QRadioButton *normalNormalAxisY = new QRadioButton(tr("Y"), normalWidget);
    normal->addButton(normalNormalAxisY, 1);
    normalLayout->addWidget(normalNormalAxisY);
    QRadioButton *normalNormalAxisZ = new QRadioButton(tr("Z"), normalWidget);
    normal->addButton(normalNormalAxisZ, 2);
    normalLayout->addWidget(normalNormalAxisZ);
    connect(normal, SIGNAL(buttonClicked(int)),
            this, SLOT(normalChanged(int)));
    mainLayout->addWidget(normalWidget, 2,1);

    formatStringLabel = new QLabel(tr("Format string"), central);
    mainLayout->addWidget(formatStringLabel,3,0);
    formatString = new QLineEdit(central);
    connect(formatString, SIGNAL(returnPressed()),
            this, SLOT(formatStringProcessText()));
    mainLayout->addWidget(formatString, 3,1);

    fontName = new QvisDialogLineEdit(central);
    fontName->setDialogMode(QvisDialogLineEdit::ChooseFont);
    connect(fontName, SIGNAL(textChanged(const QString &)),
            this, SLOT(fontNameChanged(const QString &)));
    mainLayout->addWidget(fontName, 4, 1);
    mainLayout->addWidget(new QLabel(tr("Spreadsheet font"), central), 4, 0);

    useColorTable = new QCheckBox(tr("Use color table"), central);
    connect(useColorTable, SIGNAL(toggled(bool)),
            this, SLOT(useColorTableChanged(bool)));
    mainLayout->addWidget(useColorTable, 5,0);

    colorTableName = new QvisColorTableButton(central);
    connect(colorTableName, SIGNAL(selectedColorTable(bool, const QString&)),
            this, SLOT(colorTableNameChanged(bool, const QString&)));
    mainLayout->addWidget(colorTableName, 5,1);

    showPatchOutline = new QCheckBox(tr("Show patch outline"), central);
    connect(showPatchOutline, SIGNAL(toggled(bool)),
            this, SLOT(showPatchOutlineChanged(bool)));
    mainLayout->addWidget(showPatchOutline, 6,0);

    showCurrentCellOutline = new QCheckBox(tr("Show current cell outline"), central);
    connect(showCurrentCellOutline, SIGNAL(toggled(bool)),
            this, SLOT(showCurrentCellOutlineChanged(bool)));
    mainLayout->addWidget(showCurrentCellOutline, 7,0);
 
    showTracerPlane = new QCheckBox(tr("Show tracer plane"), central);
    connect(showTracerPlane, SIGNAL(toggled(bool)),
            this, SLOT(showTracerPlaneChanged(bool)));
    mainLayout->addWidget(showTracerPlane, 8,0);

    tracerColorLabel = new QLabel(tr("Tracer color"), central);
    mainLayout->addWidget(tracerColorLabel,9,0);
    tracerColor = new QvisColorButton(central);
    connect(tracerColor, SIGNAL(selectedColor(const QColor&)),
            this, SLOT(tracerColorChanged(const QColor&)));
    mainLayout->addWidget(tracerColor, 9,1);

    tracerOpacity = new QvisOpacitySlider(0, 255, 10, 0, central);
    connect(tracerOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(tracerOpacityChanged(int)));
    mainLayout->addWidget(tracerOpacity, 10,1);
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::UpdateWindow
//
// Purpose: 
//   Updates the widgets in the window when the subject changes.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 18:16:03 PST 2007
//   Made UpdateSubsetNames take place more often.
//
//   Gunther H. Weber, Thu Sep 27 12:05:14 PDT 2007
//   Added font selection for spreadsheet
//
//   Brad Whitlock, Mon Aug 11 16:09:59 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::UpdateWindow(bool doAll)
{
    QString temp;
    QColor tempcolor;

    if (selectedSubject == GetViewerState()->GetSILRestrictionAttributes())
    {
        UpdateSubsetNames();
        subsetName->blockSignals(true);
        subsetName->setCurrentIndex(subsetName->findText(atts->GetSubsetName().c_str()));
        subsetName->blockSignals(false);
        subsetName->setEnabled(atts->GetSubsetName() != defaultItem);
        return;
    }

    for(int i = 0; i < atts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!atts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case SpreadsheetAttributes::ID_subsetName:
            UpdateSubsetNames();
            subsetName->blockSignals(true);
            subsetName->setCurrentIndex(subsetName->findText(atts->GetSubsetName().c_str()));
            subsetName->blockSignals(false);
            subsetName->setEnabled(atts->GetSubsetName() != defaultItem);
            break;
        case SpreadsheetAttributes::ID_formatString:
            temp = atts->GetFormatString().c_str();
            formatString->blockSignals(true);
            formatString->setText(temp);
            formatString->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_useColorTable:
            if (atts->GetUseColorTable())
                colorTableName->setEnabled(true);
            else
                colorTableName->setEnabled(false);
            useColorTable->blockSignals(true);
            useColorTable->setChecked(atts->GetUseColorTable());
            useColorTable->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_colorTableName:
            colorTableName->blockSignals(true);
            colorTableName->setColorTable(atts->GetColorTableName().c_str());
            colorTableName->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_showTracerPlane:
            if (atts->GetShowTracerPlane())
            {
                tracerColor->setEnabled(true);
                tracerColorLabel->setEnabled(true);
                tracerOpacity->setEnabled(true);
            }
            else
            {
                tracerColor->setEnabled(false);
                tracerColorLabel->setEnabled(false);
                tracerOpacity->setEnabled(false);
            }
            showTracerPlane->blockSignals(true);
            showTracerPlane->setChecked(atts->GetShowTracerPlane());
            showTracerPlane->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_tracerColor:
            tempcolor = QColor(atts->GetTracerColor().Red(),
                               atts->GetTracerColor().Green(),
                               atts->GetTracerColor().Blue());
            tracerColor->blockSignals(true);
            tracerColor->setButtonColor(tempcolor);
            tracerColor->blockSignals(false);
            tracerOpacity->blockSignals(true);
            tracerOpacity->setValue(atts->GetTracerColor().Alpha());
            tracerOpacity->setGradientColor(tempcolor);
            tracerOpacity->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_normal:
            normal->blockSignals(true);
            normal->button(atts->GetNormal())->setChecked(true);
            normal->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_sliceIndex:
            break;
        case SpreadsheetAttributes::ID_spreadsheetFont:
            fontName->blockSignals(true);
            fontName->setText(atts->GetSpreadsheetFont().c_str());
            fontName->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_showPatchOutline:
            showPatchOutline->blockSignals(true);
            showPatchOutline->setChecked(atts->GetShowPatchOutline());
            showPatchOutline->blockSignals(false);
            break;
        case SpreadsheetAttributes::ID_showCurrentCellOutline:
            showCurrentCellOutline->blockSignals(true);
            showCurrentCellOutline->setChecked(atts->GetShowCurrentCellOutline());
            showCurrentCellOutline->blockSignals(false);
            break;
        }
    }
}

// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::UpdateSubsetNames
//
// Purpose: 
//   Update the subset names from the SIL.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 12:47:36 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Mar 28 18:20:12 PST 2007
//   Made the subset names update more often. If the subset happens to have the
//   same name as the subset name in the Spreadsheet attributes, add it 
//   temporarily regardless of whether the subset is selected in the SIL
//   restriction.
//
//   Brad Whitlock, Mon Aug 11 16:19:39 PDT 2008
//   Qt 4.
//
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::UpdateSubsetNames()
{
    avtSILRestriction_p restriction = GetViewerProxy()->GetPlotSILRestriction();

    // If the SIL looks different from what we've displayed already, redo
    // the menu.
    {
        silTopSet = restriction->GetTopSet();
        silNumCollections = restriction->GetNumCollections();
        silNumSets = restriction->GetNumSets();

        // The SIL looks different. Update the subset names.
        subsetName->blockSignals(true);
        subsetName->clear();

        TRY
        {
            avtSILRestrictionTraverser trav(restriction);
            // Search for a SIL_DOMAIN category under the current silTopSet and add
            // sets that are used.
            avtSILSet_p current = restriction->GetSILSet(silTopSet);
            const std::vector<int> &mapsOut = current->GetMapsOut();
            for(int j = 0; j < mapsOut.size(); ++j)
            {
                int cIndex = mapsOut[j];
                avtSILCollection_p collection =restriction->GetSILCollection(cIndex);
                if(*collection != NULL && collection->GetRole() == SIL_DOMAIN) 
                {
                    const std::vector<int> &setIds = collection->GetSubsetList();
                    for(int si = 0; si < setIds.size(); ++si)
                    {
                        if(trav.UsesData(setIds[si]) ||
                           restriction->GetSILSet(setIds[si])->GetName() == atts->GetSubsetName())
                        {
                            subsetName->addItem(restriction->
                                GetSILSet(setIds[si])->GetName().c_str()); 
                        }
                    }
                    break;
                }
            }
        }
        CATCH(BadIndexException)
        {
            // There was an error displaying the domains from the SIL
            // so clear out the list and reset some variables so we'll
            // try updating the next time the SILAttributes are sent. In 
            // the meantime, display "Whole".
            subsetName->clear();
            silTopSet = -1;
            silNumCollections = -1;
            silNumSets = -1;
        }
        ENDTRY

        // Set the enabled state.
        if(subsetName->count() == 0)
        {
            subsetName->addItem(defaultItem);
            subsetName->setEnabled(false);
        }
        else if(subsetName->count() == 0)
            subsetName->setEnabled(false);
        else
            subsetName->setEnabled(true);
        subsetName->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::GetCurrentValues
//
// Purpose: 
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   Brad Whitlock, Wed Apr 23 11:38:00 PDT 2008
//   Added tr().
//
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::GetCurrentValues(int which_widget)
{
    bool okay, doAll = (which_widget == -1);
    QString msg, temp;

    // Do formatString
    if(which_widget == 1 || doAll)
    {
        temp = formatString->displayText();
        okay = !temp.isEmpty();
        if(okay)
        {
            atts->SetFormatString(temp.toStdString());
        }

        if(!okay)
        {
            msg = tr("The value of formatString was invalid. "
                     "Resetting to the last good value of %1.").
                  arg(atts->GetFormatString().c_str());
            Message(msg);
            atts->SetFormatString(atts->GetFormatString());
        }
    }
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::Apply
//
// Purpose: 
//   Called to apply changes in the subject.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        GetCurrentValues(-1);
        atts->Notify();

        GetViewerMethods()->SetPlotOptions(plotType);
    }
    else
        atts->Notify();
}


//
// Qt Slot functions
//


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::apply
//
// Purpose: 
//   Qt slot function called when apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::apply()
{
    Apply(true);
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::makeDefault
//
// Purpose: 
//   Qt slot function called when "Make default" button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::makeDefault()
{
    GetCurrentValues(-1);
    atts->Notify();
    GetViewerMethods()->SetDefaultPlotOptions(plotType);
}


// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::reset
//
// Purpose: 
//   Qt slot function called when reset button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Feb 15 11:37:49 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::reset()
{
    GetViewerMethods()->ResetPlotOptions(plotType);
}


void
QvisSpreadsheetPlotWindow::subsetNameChanged(const QString &name)
{
    atts->SetSubsetName(name.toStdString());
    SetUpdate(false);
    Apply();
}

void
QvisSpreadsheetPlotWindow::formatStringProcessText()
{
    GetCurrentValues(1);
    Apply();
}


void
QvisSpreadsheetPlotWindow::useColorTableChanged(bool val)
{
    atts->SetUseColorTable(val);
    Apply();
}


void
QvisSpreadsheetPlotWindow::colorTableNameChanged(bool useDefault, const QString &ctName)
{
    atts->SetColorTableName(ctName.toStdString());
    SetUpdate(false);
    Apply();
}

void
QvisSpreadsheetPlotWindow::showPatchOutlineChanged(bool val)
{
    atts->SetShowPatchOutline(val);
    Apply();
}

void
QvisSpreadsheetPlotWindow::showCurrentCellOutlineChanged(bool val)
{
    atts->SetShowCurrentCellOutline(val);
    Apply();
}

void
QvisSpreadsheetPlotWindow::showTracerPlaneChanged(bool val)
{
    atts->SetShowTracerPlane(val);
    Apply();
}


void
QvisSpreadsheetPlotWindow::tracerColorChanged(const QColor &color)
{
    ColorAttribute temp(color.red(), color.green(), color.blue());
    temp.SetAlpha(atts->GetTracerColor().Alpha());
    atts->SetTracerColor(temp);
    Apply();
}

void
QvisSpreadsheetPlotWindow::tracerOpacityChanged(int alpha)
{
    ColorAttribute temp(atts->GetTracerColor());
    temp.SetAlpha(alpha);
    atts->SetTracerColor(temp);
    SetUpdate(false);
    Apply();
}

void
QvisSpreadsheetPlotWindow::normalChanged(int val)
{
    atts->SetNormal(SpreadsheetAttributes::NormalAxis(val));
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisSpreadsheetPlotWindow::fontNameChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the Font... button is
//   clicked.
//
// Programmer: Gunther H. Weber
// Creation:   Thu Sep 27 12:15:37 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisSpreadsheetPlotWindow::fontNameChanged(const QString &newFont)
{
    atts->SetSpreadsheetFont(newFont.toStdString());
    SetUpdate(false);
    Apply();
}
