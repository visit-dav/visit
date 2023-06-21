// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "QvisThresholdWindow.h"

#include <ThresholdAttributes.h>

#include <QTableWidget>
#include <QTableWidgetItem>
#include <QHeaderView>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLayout>
#include <QWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QRadioButton>
#include <QvisVariableButton.h>


// ****************************************************************************
// Method: QvisThresholdWindow::QvisThresholdWindow
//
// Purpose:
//   Constructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Certain Threshold changes are
//   preserved when viewer sends attributes that do not know about the changes.
//   (See full explanation in header of RestoreAppropriateUnappliedAttributes.)
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
// ****************************************************************************

QvisThresholdWindow::QvisThresholdWindow(const int type,
                         ThresholdAttributes *subj,
                         const QString &caption,
                         const QString &shortName,
                         QvisNotepadArea *notepad)
    : QvisOperatorWindow(type,subj, caption, shortName, notepad)
{
    atts = subj;
}


// ****************************************************************************
// Method: QvisThresholdWindow::~QvisThresholdWindow
//
// Purpose:
//   Destructor
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Need to delete GUI variable list object.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.
//
// ****************************************************************************

QvisThresholdWindow::~QvisThresholdWindow()
{
    // Nothing here.
}


// ****************************************************************************
// Method: QvisThresholdWindow::CreateWindow
//
// Purpose:
//   Creates the widgets for the window.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Brad Whitlock, Fri Dec 10 09:40:53 PDT 2004
//   Changed so it uses a variable button. I also improved the widget spacing.
//
//   Hank Childs, Tue Sep 13 09:25:35 PDT 2005
//   Add support for "PointsOnly".
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
//   Brad Whitlock, Thu Apr 24 15:58:32 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
//   Kevin Griffin, Thu Feb  9 11:33:18 PST 2017
//   Added 'Bounds Input' label and associated radio buttons (Default, Custom).
//
//   Kathleen Biagas, Tue Apr 18 16:34:41 PDT 2023
//   Support Qt6: buttonClicked -> idClicked.
//
// ****************************************************************************

void
QvisThresholdWindow::CreateWindowContents()
{
    // For individual threshold variables
    QGroupBox *threshVarsBox = new QGroupBox(central);
    threshVarsBox->setTitle(tr("For individual threshold variables"));
    topLayout->addWidget(threshVarsBox);

    QGridLayout *threshVarsLayout = new QGridLayout(threshVarsBox);
    threshVars = new QTableWidget(threshVarsBox);

    threshVarsLayout->addWidget(threshVars, 0, 0, 1, 3); // row, col, rowspan, colspan

    threshVars->setSelectionMode(QAbstractItemView::SingleSelection);
    threshVars->setColumnCount(4);

    QStringList hzHeaderLbls;
    hzHeaderLbls << tr("Variable")
                 << tr("Lower bound")
                 << tr("Upper bound")
                 << tr("Show zone if");
    threshVars->setHorizontalHeaderLabels(hzHeaderLbls);
    threshVars->verticalHeader()->hide();
    threshVars->setColumnWidth(3, threshVars->columnWidth(3)+15);

    QvisVariableButton *addVarToList = new QvisVariableButton(false,
                                                              true,
                                                              true,
                                   QvisVariableButton::Scalars, threshVarsBox);
    addVarToList->setText(tr("Add variable"));
    addVarToList->setChangeTextOnVariableChange(false);
    connect(addVarToList, SIGNAL(activated(const QString &)), this, SLOT(variableAddedToList(const QString &)));
    threshVarsLayout->addWidget(addVarToList, 3, 0);

    QPushButton *deleteSelectedVar = new QPushButton(
        QString(tr("Delete selected variable")), threshVarsBox);
    connect(deleteSelectedVar, SIGNAL(clicked()),
        this, SLOT(selectedVariableDeleted()));
    threshVarsLayout->addWidget(deleteSelectedVar, 3, 1);

    threshVarsLayout->addWidget(new QLabel(tr("Bounds Input:"), threshVarsBox), 5, 0, 1, 1);

    QWidget *boundsInputWidget = new QWidget(threshVarsBox);
    QHBoxLayout *boundsInputWidgetLayout = new QHBoxLayout(boundsInputWidget);
    boundsInputWidgetLayout->setAlignment(Qt::AlignLeft);

    boundsInputType = new QButtonGroup(threshVarsBox);

    QRadioButton *defaulRadioButton = new QRadioButton(tr("Default"), boundsInputWidget);
    boundsInputType->addButton(defaulRadioButton,0);
    boundsInputWidgetLayout->addWidget(defaulRadioButton);

    QRadioButton *customRadioButton = new QRadioButton(tr("Custom"), boundsInputWidget);
    boundsInputType->addButton(customRadioButton,1);
    boundsInputWidgetLayout->addWidget(customRadioButton);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(boundsInputType, SIGNAL(buttonClicked(int)), this, SLOT(boundsInputTypeChanged(int)));
#else
    connect(boundsInputType, SIGNAL(idClicked(int)), this, SLOT(boundsInputTypeChanged(int)));
#endif
    threshVarsLayout->addWidget(boundsInputWidget, 5, 1, 1, 2);


    // For all threshold variables
    QGroupBox *forAllVarsBox = new QGroupBox(tr("For all threshold variables"),central);
    topLayout->addWidget(forAllVarsBox);

    QGridLayout *forAllVarsLayout = new QGridLayout(forAllVarsBox);

    forAllVarsLayout->addWidget(new QLabel(tr("Output Mesh Is:"),
                                forAllVarsBox), 1, 0, 1, 1);

    outputMeshType = new QButtonGroup(forAllVarsBox);
    QWidget *outputMeshWidget = new QWidget(forAllVarsBox);

    QHBoxLayout *outputMeshWidgetLayout = new QHBoxLayout(outputMeshWidget);

    QRadioButton *zonesFromInput =
        new QRadioButton(tr("Zones from input"), outputMeshWidget);
    outputMeshType->addButton(zonesFromInput,0);
    outputMeshWidgetLayout->addWidget(zonesFromInput);
    QRadioButton *pointMesh = new QRadioButton(tr("Point mesh"), outputMeshWidget);
    outputMeshType->addButton(pointMesh,1);
    outputMeshWidgetLayout->addWidget(pointMesh);

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    connect(outputMeshType, SIGNAL(buttonClicked(int)),
            this, SLOT(outputMeshTypeChanged(int)));
#else
    connect(outputMeshType, SIGNAL(idClicked(int)),
            this, SLOT(outputMeshTypeChanged(int)));
#endif
    forAllVarsLayout->addWidget(outputMeshWidget, 1, 1, 1, 2);
}

// ****************************************************************************
// Method: QvisThresholdWindow::SetLowerUpper
//
// Purpose:
//   Set lower/upper values at the same time so we can check for sameness and
//   use more precision if needed.
//
// Arguments:
//   idx : The index of the lower/upper pair.
//   lower : The lower value.
//   upper : The upper value.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Aug 18 12:33:04 PDT 2014
//
// Modifications:
//
//   Kevin Griffin, Wed Apr 1 16:58:38 PDT 2015
//   Changed the argument format in the setNum() to 'g' so that it can use
//   e or f format to display numbers in scientific notation when appropriate.
//   Removed call to TrimTrailing() since it truncatted whole numbers. (i.e.
//   600 would be changed to 6).
//
//   Kathleen Biagas, Thu Apr 9 07:19:54 MST 2015
//   Use helper function DoubleToQString for consistency in formatting across
//   all windows.
//
// ****************************************************************************

void
QvisThresholdWindow::SetLowerUpper(int idx, double lower, double upper)
{
    QString lStr, uStr;
    if (lower < -9e+36)
        lStr = "min";
    else
        lStr = DoubleToQString(lower);

    if (upper > 9e+36)
        uStr = "max";
    else
        uStr = DoubleToQString(upper);

    threshVars->item(idx, 1)->setText(lStr);
    threshVars->item(idx, 2)->setText(uStr);
}

// ****************************************************************************
// Method: QvisThresholdWindow::UpdateWindow
//
// Purpose:
//   Updates the widgets in the window when the subject changes.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//
//   Jeremy Meredith, Tue Nov 16 11:39:53 PST 2004
//   Replaced simple QString::sprintf's with a setNum because there seems
//   to be a bug causing numbers to be incremented by .00001.  See '5263.
//
//   Hank Childs, Thu Sep 15 15:31:34 PDT 2005
//   Add support for meshType.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Upgraded to support multiple threshold variables.
//
//   Mark Blair, Tue Aug  8 17:47:00 PDT 2006
//   Now accommodates an empty list of threshold variables.
//
//   Eric Brugger, Fri Sep  8 11:10:09 PDT 2006
//   Changed the way a conversion between std::string and QString was done
//   to eliminate a runtime link error on the ibm.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Restore appropriate unapplied changes when Threshold attributes are
//   received from the viewer that do not yet know about these changes.
//
//   Mark Blair, Tue Oct  3 13:19:11 PDT 2006
//   Handle default scalar variable flag, display default variable as "default"
//   once again.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Second change back undone.  Too much conflict when multiple vis windows in
//   use.  All unapplied Threshold GUI changes are now lost if user dismisses
//   then reopens GUI, or if user moves an arrowhead in Extents tool of a second
//   vis window tool-locked to the Threshold operator's vis window.  Too bad.
//
//   Mark Blair, Tue Mar 13 19:51:29 PDT 2007
//   Now forces attribute consistency if inconsistent, which can occur, for
//   instance, if user specifies an invalid attribute combination in the CLI.
//
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
//   Brad Whitlock, Mon May 14 16:32:55 PDT 2012
//   Select the first row if no row is selected.
//
//   Brad Whitlock, Mon Aug 18 12:32:35 PDT 2014
//   Change code to set upper / lower bounds.
//
//   Kevin Griffin, Wed Feb 15 11:50:13 PDT 2017
//   Added support for the custom bounds input capability (Feature #2646).
//
// ****************************************************************************

void
QvisThresholdWindow::UpdateWindow(bool doAll)
{
    bool isDefault = threshVars->columnCount() == 4;
    int biType = isDefault ? ThresholdAttributes::Default : ThresholdAttributes::Custom;
    intVector curZonePortions;

    atts->ForceAttributeConsistency();

    for (int attIndex = 0; attIndex < atts->NumAttributes(); attIndex++)
    {
        switch (attIndex)
        {
            case ThresholdAttributes::ID_outputMeshType:
                outputMeshType->button((int)atts->GetOutputMeshType())->setChecked(true);

                break;
            case ThresholdAttributes::ID_boundsInputType:
                boundsInputType->button(biType)->setChecked(true);
                atts->SetBoundsInputType(biType);

                break;

            case ThresholdAttributes::ID_listedVarNames:
                PopulateThresholdVariablesList();

                break;
            case ThresholdAttributes::ID_zonePortions:
                curZonePortions = atts->GetZonePortions();
                QComboBox *cbox;
                for (size_t varNum = 0; varNum < curZonePortions.size(); varNum++ )
                {
                    cbox=(QComboBox*)threshVars->cellWidget((int)varNum, threshVars->columnCount()-1);
                    cbox->setCurrentIndex(curZonePortions[varNum]);
                }

                SetZoneIncludeSelectEnabled(atts->GetOutputMeshType() == ThresholdAttributes::InputZones);
                break;

            case ThresholdAttributes::ID_lowerBounds:
            case ThresholdAttributes::ID_upperBounds:
                if(isDefault)
                {
                    if(atts->GetLowerBounds().size() == atts->GetUpperBounds().size())
                    {
                        for (size_t varNum = 0; varNum < atts->GetLowerBounds().size(); varNum++ )
                        {
                            double lower = atts->GetLowerBounds()[varNum];
                            double upper = atts->GetUpperBounds()[varNum];
                            SetLowerUpper((int)varNum, lower, upper);
                        }
                    }
                }

                break;
            case ThresholdAttributes::ID_boundsRange:
                if(!isDefault)
                {
                    for (size_t varNum = 0; varNum < atts->GetBoundsRange().size(); varNum++ )
                    {
                        std::string range = atts->GetBoundsRange()[varNum];
                        threshVars->item((int)varNum, 1)->setText(QString(range.c_str()));
                    }
                }
        }
    }

    // Select the first row if no row is selected.
    if(threshVars->rowCount() > 0)
    {
        if(!threshVars->selectionModel()->hasSelection())
        {
            QModelIndex index(threshVars->model()->index(0, 1));
            threshVars->selectionModel()->select(index, QItemSelectionModel::ClearAndSelect);
            threshVars->setCurrentIndex(index);
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::GetCurrentValues
//
// Purpose:
//   Gets values from certain widgets and stores them in the subject.
//
// Programmer: xml2window
// Creation:   Fri Apr 12 14:41:06 PST 2002
//
// Modifications:
//   Hank Childs, Thu Sep 25 09:16:09 PDT 2003
//   Allow for "min" and "max" to be valid values in the lbound and ubound
//   windows.
//
//   Jeremy Meredith, Wed Mar  3 16:02:43 PST 2004
//   Fixed a type with using "min".
//
//   Brad Whitlock, Fri Dec 10 09:43:19 PDT 2004
//   Removed code to get the variable.
//
//   Mark Blair, Tue Mar  7 13:25:00 PST 2006
//   Added support for multiple threshold variables.
//
//   Mark Blair, Wed Sep  6 19:33:00 PDT 2006
//   Removed problematic mechanism for accommodating ExtentsAttributes from
//   extents tool.
//
//   Mark Blair, Thu Sep 21 15:16:27 PDT 2006
//   Added support for input from Extents tool.  Save pending GUI changes.
//
//   Mark Blair, Tue Oct 31 20:18:10 PST 2006
//   Previous change undone.
//
//   Mark Blair, Tue Apr 17 16:24:42 PDT 2007
//   Rewritten to support new Threshold GUI.
//
//   Brad Whitlock, Thu Apr 24 16:00:33 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
//   Kevin Griffin, Wed Feb 15 11:50:13 PDT 2017
//   Added support for the custom bounds input capability (Feature #2646).
//
//   Kathleen Biagas, Wed Apr 12, 2023
//   QString::SkipEmptyParts => Qt::SkipEmptyParts for Qt >= 6.
//
// ****************************************************************************

void
QvisThresholdWindow::GetCurrentValues(int which_widget)
{
    if (which_widget != -1) return;

    bool isDefault = threshVars->columnCount() == 4;
    stringVector curVarNames;
    doubleVector curLowerBounds;
    doubleVector curUpperBounds;
    stringVector curBoundsRange;
    intVector    curZonePortions;

    if (threshVars->rowCount() != int(guiFullVarNames.size())) // Just in case
    {
        debug3 << "QTW/GCV/1: Threshold GUI out of sync with internal data."
               << endl;

        curVarNames.push_back(std::string("default"));
        curLowerBounds.push_back(-1e+37);
        curUpperBounds.push_back(+1e+37);
        curBoundsRange.push_back("-1e+37:1e+37");
        curZonePortions.push_back((int)ThresholdAttributes::PartOfZone);

        atts->SetListedVarNames(curVarNames);
        atts->SetLowerBounds(curLowerBounds);
        atts->SetUpperBounds(curUpperBounds);
        atts->SetBoundsRange(curBoundsRange);
        atts->SetZonePortions(curZonePortions);

        threshVars->setRowCount(0);
        AddNewRowToVariablesList(QString("default"));

        guiFullVarNames.clear();
        guiFullVarNames.push_back(std::string("default"));

        return;
    }

    int listRowCount = threshVars->rowCount();
    bool valueIsValid;
    double lowerBound, upperBound, bound;
    QString lowerBoundText, upperBoundText, boundsText, errMsg;

    for (int rowNum = 0; rowNum < listRowCount; rowNum++ )
    {
        int col = 1;
        curVarNames.push_back(guiFullVarNames[rowNum]);

        if(isDefault)
        {
            lowerBoundText = threshVars->item(rowNum,col++)->text().trimmed();
            upperBoundText = threshVars->item(rowNum,col++)->text().trimmed();
        }
        else
        {
            boundsText = threshVars->item(rowNum, col++)->text().trimmed();
            curBoundsRange.push_back(boundsText.toStdString());

            if(!boundsText.trimmed().isEmpty() && IsSimpleBounds(boundsText))
            {
#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
                QStringList minmaxTokens = boundsText.split(":", QString::SkipEmptyParts);
#else
                QStringList minmaxTokens = boundsText.split(":", Qt::SkipEmptyParts);
#endif

                if(minmaxTokens.length() == 1)
                {
                    lowerBoundText = minmaxTokens[0].trimmed();
                    upperBoundText = lowerBoundText;
                }
                else
                {
                    lowerBoundText = minmaxTokens[0].trimmed();
                    upperBoundText = minmaxTokens[1].trimmed();
                }
            }
            else    // Custom range
            {
                lowerBoundText = "min";
                upperBoundText = "max";
            }
        }

        if (lowerBoundText == QString(tr("min")))
        {
            lowerBound = -1e+37;
        }
        else
        {
            valueIsValid = !lowerBoundText.isEmpty();
            if (valueIsValid) lowerBound = lowerBoundText.toDouble(&valueIsValid);

            if (!valueIsValid)
            {
                Message(tr("Invalid lower bound; will reset to min."));

                lowerBound = -1e+37;
            }
        }

        if (upperBoundText == QString(tr("max")))
        {
            upperBound = +1e+37;
        }
        else
        {
            valueIsValid = !upperBoundText.isEmpty();
            if (valueIsValid) upperBound = upperBoundText.toDouble(&valueIsValid);

            if (!valueIsValid)
            {
                Message(tr("Invalid upper bound; will reset to max."));

                upperBound = +1e+37;
            }
        }

        if (lowerBound > upperBound)
        {
            Message(tr("Lower bound exceeds upper bound; will reverse them."));

            bound = lowerBound; lowerBound = upperBound; upperBound = bound;
        }

        curLowerBounds.push_back(lowerBound);
        curUpperBounds.push_back(upperBound);

        if(isDefault)
        {
            boundsText = DoubleToQString(lowerBound).append(":").append(DoubleToQString(upperBound));
            curBoundsRange.push_back(boundsText.toStdString());
        }

        QComboBox *cbox = (QComboBox*)threshVars->cellWidget(rowNum, col++);
        curZonePortions.push_back(cbox->currentIndex());
    }

    atts->SetListedVarNames(curVarNames);
    atts->SetLowerBounds(curLowerBounds);
    atts->SetUpperBounds(curUpperBounds);
    atts->SetBoundsRange(curBoundsRange);
    atts->SetZonePortions(curZonePortions);
}

// ****************************************************************************
//  Method: avtThresholdFilter::IsSimpleBounds
//
//  Purpose: Determine if a range string consists of only one range or is
//           empty.
//
//  Arguments:
//      boundsText  The range string to examine.
//
//  Returns: true if there is only one or no range in boundsText, otherwise
//           false
//
//  Programmer: Kevin Griffin
//  Creation:   Thu Mar 23 08:25:21 PDT 2017
//
//  Modifications:
//   Kathleen Biagas, Wed Apr 12, 2023
//   QString::SkipEmptyParts => Qt::SkipEmptyParts for Qt >= 6.
//
// ****************************************************************************

bool
QvisThresholdWindow::IsSimpleBounds(const QString &boundsText)
{
    if(boundsText.trimmed().isEmpty())
    {
        return true;
    }

#if QT_VERSION < QT_VERSION_CHECK(6,0,0)
    QStringList tokens = boundsText.split(":", QString::SkipEmptyParts);
#else
    QStringList tokens = boundsText.split(":", Qt::SkipEmptyParts);
#endif
    return (tokens.length() <= 2);
}


// ****************************************************************************
//
// Qt Slot functions
//
//  Modifications:
//   Cyrus Harrison, Tue Jun 12 20:16:59 PDT 2007
//   Fixed comparison with QString and std::string that causes a runtime
//   linking error on AIX. Note: to avoid this in the future do not directly
//   assign or compare a std::string instance to a QString object, use c_str().
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
// ****************************************************************************

void
QvisThresholdWindow::variableAddedToList(const QString &variableToAdd)
{
    if (threshVars->rowCount() != int(guiFullVarNames.size()))
    {
        debug3 << "QTW/vATL/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    for (size_t varNum = 0; varNum < guiFullVarNames.size(); varNum++ )
    {
        if (QString(guiFullVarNames[varNum].c_str()) == variableToAdd) return;
    }

    QString vname = PrepareVariableNameText(variableToAdd, 20);
    AddNewRowToVariablesList(QString(vname));

    guiFullVarNames.push_back(variableToAdd.toStdString());
}


// ****************************************************************************
//  Modifications:
//
//    Hank Childs, Thu May 20 10:09:27 PDT 2010
//    Fix problem uncovered by Cihan Altinay with deleting the last variable.
//
// ****************************************************************************

void
QvisThresholdWindow::selectedVariableDeleted()
{
    if (threshVars->rowCount() != int(guiFullVarNames.size()))
    {
        debug3 << "QTW/sVD/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    if (guiFullVarNames.empty())
        return;

    int selectedVarNum = threshVars->currentRow();

    if (selectedVarNum >= 0)
    {
        threshVars->removeRow(selectedVarNum);
        guiFullVarNames.erase(guiFullVarNames.begin() + selectedVarNum);
    }
}

// ****************************************************************************
// Method: QvisThresholdWindow::boundsInputTypeChanged
//
// Purpose:
//   Slot for the bounds input radio buttons
//
// Arguments:
//   buttonID the ID of the radio button selected
//
// Programmer: Kevin Griffin
// Creation:   Tue Feb 14 09:37:50 PST 2017
//
// Modifications:
//
// ****************************************************************************
void
QvisThresholdWindow::boundsInputTypeChanged(int buttonID)
{
    ThresholdAttributes::BoundsInputType newBoundsInputType = ThresholdAttributes::BoundsInputType(buttonID);

    if(newBoundsInputType != atts->GetBoundsInputType())
    {
        QStringList headerLabels;

        // Change input table
        if(newBoundsInputType == ThresholdAttributes::Custom)
        {
            int colSize = threshVars->columnWidth(1) + threshVars->columnWidth(2);

            // Delete upper bound column
            threshVars->removeColumn(2);

            // Add new header
            headerLabels << tr("Variable") << tr("Range") << tr("Show zone if");
            threshVars->setHorizontalHeaderLabels(headerLabels);

            threshVars->setColumnWidth(1, colSize);

            // Update range data
            if(atts->GetBoundsRange().size() > 0)
            {
                for (size_t varNum = 0; varNum < atts->GetBoundsRange().size(); varNum++ )
                {
                    std::string range = atts->GetBoundsRange()[varNum]; // lowerbound : upperbound
                    threshVars->setItem((int)varNum, 1, new QTableWidgetItem(QString(range.c_str())));
                }
            }
            else
            {
                threshVars->setItem(0, 1, new QTableWidgetItem(QString("min:max")));

                stringVector boundsRange;
                boundsRange.push_back("min:max");
                atts->SetBoundsRange(boundsRange);
            }
        }
        else    // Bounds input == Default
        {
            int colSize = threshVars->columnWidth(1) / 2;

            // Add column
            threshVars->insertColumn(2);
            threshVars->setColumnWidth(1, colSize);
            threshVars->setColumnWidth(2, colSize);

            // Add new header
            headerLabels << tr("Variable") << tr("Lower bound") << tr("Upper bound") << tr("Show zone if");
            threshVars->setHorizontalHeaderLabels(headerLabels);

            // Update lower/upper bounds data
            if(atts->GetLowerBounds().size() == atts->GetUpperBounds().size())
            {
                for (size_t varNum = 0; varNum < atts->GetLowerBounds().size(); varNum++ )
                {
                    double lower = atts->GetLowerBounds()[varNum];
                    double upper = atts->GetUpperBounds()[varNum];

                    // Set lower/upper bounds values
                    QString lStr, uStr;
                    if (lower < -9e+36)
                        lStr = "min";
                    else
                        lStr = DoubleToQString(lower);

                    if (upper > 9e+36)
                        uStr = "max";
                    else
                        uStr = DoubleToQString(upper);

                    threshVars->setItem((int)varNum, 1, new QTableWidgetItem(lStr));
                    threshVars->setItem((int)varNum, 2, new QTableWidgetItem(uStr));
                }
            }
        }

        atts->SetBoundsInputType(newBoundsInputType);
    }
}


void
QvisThresholdWindow::outputMeshTypeChanged(int buttonID)
{
    ThresholdAttributes::OutputMeshType newOutputMeshType =
        ThresholdAttributes::OutputMeshType(buttonID);

    if (newOutputMeshType != atts->GetOutputMeshType())
    {
        SetZoneIncludeSelectEnabled(newOutputMeshType ==
                                       ThresholdAttributes::InputZones);
        atts->SetOutputMeshType(newOutputMeshType);
    }
}


// ****************************************************************************
//  Modified:
//
//    Hank Childs, Fri Oct 29 20:26:17 PDT 2010
//    Work around Qt bug on Mac where reading the text in a QTableWidgetItem
//    cell does not match what the user has typed until you change focus to
//    the next cell.  The work around is to manually set the focus to another
//    cell and back.
//
// ****************************************************************************

void
QvisThresholdWindow::apply()
{
    int currentRow = threshVars->currentRow();
    int currentCol = threshVars->currentColumn();
    threshVars->setCurrentCell(currentRow, currentCol+1);
    threshVars->setCurrentCell(currentRow, currentCol);
    QvisOperatorWindow::apply();
}


// ****************************************************************************
// Method: QvisThresholdWindow::PopulateThresholdVariableTable
//
// Purpose: Adds a new row to the table of current threshold variables for each
//          variable not already in the table and deletes any row corresponding
//          to a variable no longer in effect.
//
// Programmer: Mark Blair
// Creation:   Tue Apr 10 17:59:47 PDT 2007
//
// Modifications:
//
// ****************************************************************************

void
QvisThresholdWindow::PopulateThresholdVariablesList()
{
    stringVector curVarNames = atts->GetListedVarNames();
    std::string curVarName, guiVarName;
    int curVarCount = (int)curVarNames.size();
    int guiVarCount = (int)guiFullVarNames.size();
    int guiVarNum = 0;
    int curVarNum;

    if (threshVars->rowCount() != guiVarCount)
    {
        debug3 << "QTW/PTVL/1: Threshold GUI out of sync with internal data."
               << endl;
        return;
    }

    while (guiVarNum < guiVarCount)
    {
        guiVarName = guiFullVarNames[guiVarNum];

        for (curVarNum = 0; curVarNum < curVarCount; curVarNum++ )
        {
            if (curVarNames[curVarNum] == guiVarName) break;
        }

        if (curVarNum < curVarCount)
            guiVarNum++;
        else
        {
            threshVars->removeRow(guiVarNum);
            guiFullVarNames.erase(guiFullVarNames.begin() + guiVarNum);

            guiVarCount--;
        }
    }

    for (curVarNum = 0; curVarNum < curVarCount; curVarNum++ )
    {
        curVarName = curVarNames[curVarNum];

        for (guiVarNum = 0; guiVarNum < guiVarCount; guiVarNum++ )
        {
            if (guiFullVarNames[guiVarNum] == curVarName) break;
        }

        if (guiVarNum >= guiVarCount)   // guiVarCount is NOT incremented.
        {
            QString vname = PrepareVariableNameText(QString(curVarName.c_str()), 20);
            AddNewRowToVariablesList(vname);

            guiFullVarNames.push_back(curVarName);
        }
    }
}


// ****************************************************************************
// Method: QvisThresholdWindow::AddNewRowToVariablesList
//
// Purpose: Adds a new row for a selected variable, or for no variable, to the
//          list of current threshold variables and their attributes.
//
// Programmer: Mark Blair
// Creation:   Tue Apr 10 17:59:47 PDT 2007
//
// Modifications:
//   Brad Whitlock, Thu Apr 24 16:01:11 PDT 2008
//   Added tr()'s
//
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
//   Kevin Griffin, Wed Feb 15 11:50:13 PDT 2017
//   Added support for a 3 column table that takes a range as input instead
//   of the min and max (Feature #2646).
//
// ****************************************************************************

void
QvisThresholdWindow::AddNewRowToVariablesList(const QString &variableName)
{
    bool isDefault = threshVars->columnCount() == 4;
    int col = 0;

    int nrows = threshVars->rowCount();
    threshVars->setRowCount(nrows + 1);

    QTableWidgetItem *name_item = new QTableWidgetItem(variableName);
    name_item->setFlags(Qt::ItemIsSelectable);
    threshVars->setItem(nrows,col++,name_item);

    if(isDefault)
    {
        threshVars->setItem(nrows,col++,new QTableWidgetItem(tr("min")));
        threshVars->setItem(nrows,col++,new QTableWidgetItem(tr("max")));
    }
    else
    {
        threshVars->setItem(nrows, col++, new QTableWidgetItem(tr("min:max")));
    }

    QComboBox *cbox = new QComboBox();
    cbox->addItem(tr("All in range"));
    cbox->addItem(tr("Part in range"));
    cbox->setCurrentIndex(1);
    threshVars->setCellWidget(nrows,col++,cbox);

    threshVars->resizeColumnToContents(0);
    threshVars->resizeColumnToContents(1);
    if(isDefault)
    {
        threshVars->resizeColumnToContents(2);
    }
    SetTableWidth();
}


// *****************************************************************************
//  Method: QvisThresholdWindow::MakeDisplayableVariableNameText
//
//  Purpose: Creates a displayable version of a threshold variable name.  Long
//           variable names and compound names are shortened in a meaningful way.
//
//  Programmer: Mark Blair
//  Creation:   Tue Apr 10 17:59:47 PDT 2007
//
//  Modifications:
//   Cyrus Harrison, Thu Aug 21 08:45:29 PDT 2008
//   Qt4 Port.
//
// *****************************************************************************

QString
QvisThresholdWindow::PrepareVariableNameText(const QString &variableName,
                                             int maxDisplayChars)
{
    QString res(variableName);
    if(res.size() > maxDisplayChars)
        res = res.left(maxDisplayChars-4) + ".." +  res.right(2);
    return res;
}

// *****************************************************************************
//  Method: QvisThresholdWindow::SetZoneIncludeSelectEnabled
//
//  Purpose: Helper to enable/disable editing of the zone inclusion combo boxes.
//
//  Programmer: Cyrus Harrison
//  Creation:   Wed Aug 20 16:04:36 PDT 2008
//
//  Modifications:
//
// *****************************************************************************

void
QvisThresholdWindow::SetZoneIncludeSelectEnabled(bool val)
{
    int nrows = threshVars->rowCount();

    for(int i=0; i<nrows;i++)
        threshVars->cellWidget(i,threshVars->columnCount()-1)->setEnabled(val);
}

// *****************************************************************************
//  Method: QvisThresholdWindow::SetTableWidth
//
//  Purpose: Helper to enable/disable editing of the zone inclusion combo boxes.
//
//  Programmer: Cyrus Harrison
//  Creation:   Thu Aug 21 09:26:06 PDT 2008
//
//  Modifications:
//
//    Kevin Griffin, Wed Feb 15 11:50:13 PDT 2017
//    Allowed the column count to be queried from the table and
//    not hard-coded.
//
// *****************************************************************************

void
QvisThresholdWindow::SetTableWidth()
{
    int cols = threshVars->columnCount();
    int width = 4;

    for(int i=0; i<cols; i++)
        width += threshVars->columnWidth(i);
    threshVars->setMinimumWidth(width);
}
