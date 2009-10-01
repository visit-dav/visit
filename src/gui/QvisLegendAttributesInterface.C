/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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

#include <QvisLegendAttributesInterface.h>
#include <QButtonGroup>
#include <QCheckBox>
#include <QComboBox>
#include <QFrame>
#include <QHeaderView>
#include <QLabel>
#include <QLayout>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSpinBox>
#include <QStringList>
#include <QTabWidget>
#include <QTableWidget>
#include <QNarrowLineEdit.h>
#include <QvisColorButton.h>
#include <QvisOpacitySlider.h>
#include <QvisScreenPositionEdit.h>

#include <AnnotationObject.h>
#include <Plot.h>
#include <PlotList.h>
#include <ViewerState.h>
#include <PlotPluginInfo.h>
#include <OperatorPluginInfo.h>
#include <ViewerProxy.h>
#include <legend_defines.h>

#define WIDTH_HEIGHT_PRECISION 100.

// Allow this when text opacity is supported.
//#define TEXT_OPACITY_SUPPORTED


// ****************************************************************************
// Method: QvisLegendAttributesInterface::QvisLegendAttributesInterface
//
// Purpose: 
//   Constructor for the QvisLegendAttributesInterface class.
//
// Arguments:
//   parent : This widget's parent widget.
//   name   : The name of this widget.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 15:31:44 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Mar 26 11:41:10 PDT 2007
//   Added drawTitlesCheckBox.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Mon Jul 21 10:21:37 PDT 2008
//   Qt 4.
//
//   Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//   Added orientationComboBox
//
//   Hank Childs, Fri Jan 23 15:19:56 PST 2009
//   Add support for drawMinmax.
//
//   Kathleen Bonnell, Thu Oct  1 14:46:14 PDT 2009
//   Added controls that allow users to modifiy tick values and labels.
//   Tabified the controls to keep the window from becoming too long.
//
// ****************************************************************************

QvisLegendAttributesInterface::QvisLegendAttributesInterface(QWidget *parent) :
    QvisAnnotationObjectInterface(parent)
{
    // Set the title of the group box.
    this->setTitle(GetName());

    tabs = new QTabWidget(0);
    topLayout->addWidget(tabs);
    
    int row = 0;

    // Position tab
    QWidget *position = new QWidget(tabs);
    QVBoxLayout *vLayout = new QVBoxLayout(position);
    vLayout->setMargin(10);
    QGridLayout *pLayout = new QGridLayout(0);
    vLayout->addLayout(pLayout);
    vLayout->addStretch(100);
    //topLayout->addLayout(pLayout);
    pLayout->setSpacing(10);

    // Add controls for the layout management.
    manageLayout = new QCheckBox(tr("Let VisIt manage legend position"), this);
    connect(manageLayout, SIGNAL(toggled(bool)),
            this, SLOT(layoutChanged(bool)));
    pLayout->addWidget(manageLayout, row, 0, 1, 4);
    ++row;

    // Add controls for the position
    positionEdit = new QvisScreenPositionEdit(this);
    connect(positionEdit, SIGNAL(screenPositionChanged(double, double)),
            this, SLOT(positionChanged(double, double)));
    pLayout->addWidget(positionEdit, row, 1, 1, 3);
    positionLabel = new QLabel(tr("Legend position"), this);
    pLayout->addWidget(positionLabel, row, 0);
    ++row;

    // Add controls for position2
    widthSpinBox = new QSpinBox(this);
    widthSpinBox->setMinimum(1);
    widthSpinBox->setMaximum(int(WIDTH_HEIGHT_PRECISION * 5));
    widthSpinBox->setSuffix("%");
    widthSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(widthSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(widthChanged(int)));
    pLayout->addWidget(widthSpinBox, row, 1);
    pLayout->addWidget(new QLabel(tr("X-scale"), this), row, 0);

    heightSpinBox = new QSpinBox(this);
    heightSpinBox->setMinimum(1);
    heightSpinBox->setMaximum(int(WIDTH_HEIGHT_PRECISION * 5));
    heightSpinBox->setSuffix("%");
    heightSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(heightSpinBox, SIGNAL(valueChanged(int)),
            this, SLOT(heightChanged(int)));
    pLayout->addWidget(heightSpinBox, row, 3);
    pLayout->addWidget(new QLabel(tr("Y-scale"), this), row, 2);
    ++row;

    // Add controls to set the orientation
    orientationComboBox = new QComboBox(this);
    orientationComboBox->addItem(tr("Vertical, Text on Right"));
    orientationComboBox->addItem(tr("Vertical, Text on Left"));
    orientationComboBox->addItem(tr("Horizontal, Text on Top"));
    orientationComboBox->addItem(tr("Horizontal, Text on Bottom"));
    orientationComboBox->setEditable(false);
    connect(orientationComboBox, SIGNAL(activated(int)),
            this, SLOT(orientationChanged(int)));
    pLayout->addWidget(orientationComboBox, row, 1, 1, 3);
    pLayout->addWidget(new QLabel(tr("Orientation"), this), row, 0);
  
    tabs->addTab(position, tr("Position")); 

    row = 0;
    // Tick Marks tab
    QWidget *tickMarks = new QWidget(tabs);
    QGridLayout *tLayout = new QGridLayout(tickMarks);
    //topLayout->addLayout(tLayout);
    tLayout->setSpacing(10);

    // controls for tick values and labels
    tickControl = new QCheckBox(tr("Let VisIt determine tick labels"), this);
    connect(tickControl, SIGNAL(toggled(bool)), this, 
            SLOT(tickControlToggled(bool)));
    tLayout->addWidget(tickControl, row, 0, 1, 4);
    ++row;

    numTicksSpinBox = new QSpinBox(this);
    numTicksSpinBox->setMinimum(1);
    numTicksSpinBox->setMaximum(100);
    numTicksSpinBox->setButtonSymbols(QSpinBox::PlusMinus);
    connect(numTicksSpinBox, SIGNAL(valueChanged(int)), this,
            SLOT(numTicksChanged(int)));
    tLayout->addWidget(numTicksSpinBox, row, 1);

    numTicksLabel = new QLabel(tr("Num ticks"), this);
    tLayout->addWidget(numTicksLabel, row, 0);
    numTicksLabel->setBuddy(numTicksSpinBox); 

    minMaxCheckBox = new QCheckBox(tr("Inclusive of min and max"), this);
    connect(minMaxCheckBox, SIGNAL(toggled(bool)), this,
            SLOT(minMaxToggled(bool)));
    tLayout->addWidget(minMaxCheckBox, row, 2, 1, 2);
    ++row;

    drawLabelsComboBox = new QComboBox(this);
    drawLabelsComboBox->addItem(tr("None"));
    drawLabelsComboBox->addItem(tr("Values"));
    drawLabelsComboBox->addItem(tr("Labels"));
    drawLabelsComboBox->addItem(tr("Values and Labels"));
    connect(drawLabelsComboBox, SIGNAL(activated(int)), this,
            SLOT(drawLabelsChanged(int)));
    QLabel *dlLabel = new QLabel(tr("Draw: "), this);
    dlLabel->setBuddy(drawLabelsComboBox);
    tLayout->addWidget(dlLabel, row, 0);
    tLayout->addWidget(drawLabelsComboBox, row, 1, 1, 2);
    ++row;

    QGroupBox *suppliedLabelsBox = new QGroupBox(this);
    suppliedLabelsBox->setTitle(tr("Specify tick values and labels"));

    // play with row spans to see if can get the table to display more rows.
    tLayout->addWidget(suppliedLabelsBox, row, 0, 1, 4); 
    QGridLayout *suppliedLabelsLayout = new QGridLayout(suppliedLabelsBox);
    suppliedLabels = new QTableWidget(this);

    // play with row spans to see if can get the table to display more rows.
    suppliedLabelsLayout->addWidget(suppliedLabels, 0, 0, 4, 2);

    suppliedLabels->setSelectionMode(QAbstractItemView::SingleSelection);
    suppliedLabels->setColumnCount(2);

    QStringList headerLabels;
    headerLabels << tr("Values")
                 << tr("Labels");
    suppliedLabels->setHorizontalHeaderLabels(headerLabels);
    suppliedLabels->verticalHeader()->hide();
    suppliedLabels->horizontalHeader()->setStretchLastSection(true);

    addRowButton = new QPushButton(
        QString(tr("Add tick value")), this);
    connect(addRowButton, SIGNAL(clicked()), this,
        SLOT(addSuppliedLabelsRow()));
    suppliedLabelsLayout->addWidget(addRowButton, 0, 2);

    deleteRowButton = new QPushButton(
        QString(tr("Delete selected value")), this);
    connect(deleteRowButton, SIGNAL(clicked()), this,
        SLOT(deleteSelectedRow()));
    suppliedLabelsLayout->addWidget(deleteRowButton, 1, 2);

    tabs->addTab(tickMarks, tr("Tick Marks"));

    row = 0;   
    // Tick Marks tab
    QWidget *appearance = new QWidget(tabs);
    QVBoxLayout *vLayout2 = new QVBoxLayout(appearance);
    vLayout2->setMargin(10);
    QGridLayout *aLayout = new QGridLayout(0);
    vLayout2->addLayout(aLayout);
    vLayout2->addStretch(100);
    aLayout->setSpacing(10);

    // Add controls for the text color.
    drawBoundingBoxCheckBox = new QCheckBox(tr("Bounding box"), this);
    connect(drawBoundingBoxCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawBoundingBoxToggled(bool)));
    aLayout->addWidget(drawBoundingBoxCheckBox, row, 0);

    boundingBoxColorButton = new QvisColorButton(this);
    connect(boundingBoxColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(boundingBoxColorChanged(const QColor &)));
    aLayout->addWidget(boundingBoxColorButton, row, 1);
    boundingBoxOpacity = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(boundingBoxOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(boundingBoxOpacityChanged(int)));
    aLayout->addWidget(boundingBoxOpacity, row, 2, 1, 2);
    ++row;

    // Turn off pieces of the legend.
    drawTitleCheckBox = new QCheckBox(tr("Draw title"), this);
    connect(drawTitleCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawTitleToggled(bool)));
    aLayout->addWidget(drawTitleCheckBox, row, 0);
    drawMinmaxCheckBox = new QCheckBox(tr("Draw min/max"), this);
    connect(drawMinmaxCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(drawMinmaxToggled(bool)));
    aLayout->addWidget(drawMinmaxCheckBox, row, 1, 1, 2);
    ++row;

    QFrame *splitter3 = new QFrame(this);
    splitter3->setFrameStyle(QFrame::HLine + QFrame::Raised);
    aLayout->addWidget(splitter3, row, 0, 1, 4);
    ++row;

    // Add controls for the text color.
    textColorButton = new QvisColorButton(this);
    connect(textColorButton, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(textColorChanged(const QColor &)));
    textColorLabel = new QLabel(tr("Text color"), this);
    aLayout->addWidget(textColorLabel, row, 2, Qt::AlignLeft);
    aLayout->addWidget(textColorButton, row, 3);
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity = new QvisOpacitySlider(0, 255, 10, 0, this);
    connect(textColorOpacity, SIGNAL(valueChanged(int)),
            this, SLOT(textOpacityChanged(int)));
    aLayout->addWidget(textColorOpacity, row, 2, 1, 2);
    ++row;
#endif

    // Added a use foreground toggle
    useForegroundColorCheckBox = new QCheckBox(tr("Use foreground color"), this);
    connect(useForegroundColorCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(useForegroundColorToggled(bool)));
    aLayout->addWidget(useForegroundColorCheckBox, row, 0, 1, 2);
    ++row;

    // Add controls for text format string.
    formatString = new QNarrowLineEdit(this);
    connect(formatString, SIGNAL(returnPressed()),
            this, SLOT(textChanged()));
    aLayout->addWidget(formatString, row, 3);
    aLayout->addWidget(new QLabel(tr("Number format"), this), row, 2);
    // Add control for text font height
    fontHeight = new QNarrowLineEdit(this);
    connect(fontHeight, SIGNAL(returnPressed()),
            this, SLOT(fontHeightChanged()));
    aLayout->addWidget(fontHeight, row, 1);
    aLayout->addWidget(new QLabel(tr("Font height"), this), row, 0);
    ++row;

    // Add controls to set the font family.
    fontFamilyComboBox = new QComboBox(this);
    fontFamilyComboBox->addItem(tr("Arial"));
    fontFamilyComboBox->addItem(tr("Courier"));
    fontFamilyComboBox->addItem(tr("Times"));
    fontFamilyComboBox->setEditable(false);
    connect(fontFamilyComboBox, SIGNAL(activated(int)),
            this, SLOT(fontFamilyChanged(int)));
    aLayout->addWidget(fontFamilyComboBox, row, 1, 1, 3);
    aLayout->addWidget(new QLabel(tr("Font family"), this), row, 0);
    ++row;

    // Add controls for font properties.
    boldCheckBox = new QCheckBox(tr("Bold"), this);
    connect(boldCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(boldToggled(bool)));
    aLayout->addWidget(boldCheckBox, row, 0);

    italicCheckBox = new QCheckBox(tr("Italic"), this);
    connect(italicCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(italicToggled(bool)));
    aLayout->addWidget(italicCheckBox, row, 1);

    shadowCheckBox = new QCheckBox(tr("Shadow"), this);
    connect(shadowCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(shadowToggled(bool)));
    aLayout->addWidget(shadowCheckBox, row, 2);
    shadowCheckBox->setEnabled(false); // Until this works in the legend.
    ++row;

    tabs->addTab(appearance, tr("Appearance"));
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::~QvisLegendAttributesInterface
//
// Purpose: 
//   Destructor for the QvisLegendAttributesInterface class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

QvisLegendAttributesInterface::~QvisLegendAttributesInterface()
{
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::GetMenuText
//
// Purpose: 
//   Returns the text to use in the annotation list box.
//
// Arguments:
//   annot : The annotation object to use for extra information.
//
// Returns:    The text to use in the annotation list box.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 14:16:31 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Jun 24 12:05:53 PDT 2008
//   Get the plugin managers from the viewer proxy.
//
//   Brad Whitlock, Mon Jul 21 10:29:13 PDT 2008
//   Qt 4.
//
// ****************************************************************************

QString
QvisLegendAttributesInterface::GetMenuText(const AnnotationObject &annot) const
{
    // Look for the name of a plot in the plot list that has the same name as 
    // the annotation object. There should be a match because of how plots are
    // created.
    QString retval;
    bool match = false;
    PlotPluginManager *pMgr = GetViewerProxy()->GetPlotPluginManager();
    OperatorPluginManager *oMgr = GetViewerProxy()->GetOperatorPluginManager();
    for(int i = 0; i < GetViewerState()->GetPlotList()->GetNumPlots(); ++i)
    {
        const Plot &plot = GetViewerState()->GetPlotList()->GetPlots(i);
        if(plot.GetPlotName() == annot.GetObjectName())
        {
            match = true;
            retval = GetName();
            retval += ":";

            CommonPlotPluginInfo *plotInfo = pMgr->GetCommonPluginInfo(
                pMgr->GetEnabledID(plot.GetPlotType()));
            retval += QString(plotInfo->GetName());
            retval += " - ";
            for(int j = 0; j < plot.GetNumOperators(); ++j)
            {
                CommonOperatorPluginInfo *info = oMgr->GetCommonPluginInfo(
                oMgr->GetEnabledID(plot.GetOperator(plot.GetNumOperators() - 1 - j)));
                retval += info->GetName();
                retval += "(";
            }
            retval += plot.GetPlotVar().c_str();
            for(int j = 0; j < plot.GetNumOperators(); ++j)
                retval += ")";

            match = true;
            break;
        }
    }

    if(!match)
        retval = QString("%1 - %2").arg(GetName()).arg(annot.GetObjectName().c_str());

    return retval;
}


bool
QvisLegendAttributesInterface::GetBool(int bit) const
{
    return (annot->GetIntAttribute1() & (1 << bit)) != 0;
}

void
QvisLegendAttributesInterface::SetBool(int bit, bool val)
{
    int shifted = (1 << bit);
    int mask = ~shifted;
    int data = annot->GetIntAttribute1();
    annot->SetIntAttribute1((data & mask) | (val ? shifted : 0));
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::UpdateControls
//
// Purpose: 
//   Updates the controls in the interface using the data in the Annotation
//   object pointed to by the annot pointer.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 15:58:02 PST 2007
//
// Modifications:
//   Brad Whitlock, Mon Mar 26 12:01:37 PDT 2007
//   Added checkbox for turning off title.
//
//   Brad Whitlock, Mon Jul 21 10:30:12 PDT 2008
//   Qt 4.
//
//   Dave Bremer, Wed Oct  8 11:36:27 PDT 2008
//   Added orientationComboBox update
//
//   Brad Whitlock, Fri Oct 17 10:20:16 PDT 2008
//   Qt 4.
//
//   Hank Childs, Fri Jan 23 15:22:49 PST 2009
//   Add support for drawMinmax.
//
//   Kathleen Bonnell, Thu Oct  1 14:46:14 PDT 2009
//   Added controls that allow users to modifiy tick values and labels.
//
// ****************************************************************************

void
QvisLegendAttributesInterface::UpdateControls()
{
    // Set the manage layout bit.
    manageLayout->blockSignals(true);
    manageLayout->setChecked(GetBool(LEGEND_MANAGE_POSITION));
    manageLayout->blockSignals(false);

    // Set the start position.
    positionEdit->setPosition(annot->GetPosition()[0], annot->GetPosition()[1]);
    positionEdit->setEnabled(!GetBool(LEGEND_MANAGE_POSITION));
    positionLabel->setEnabled(!GetBool(LEGEND_MANAGE_POSITION));

    // Set the spinbox values for the width and height.  The 0.5 is added
    // to avoid numeric issues converting back and forth between float and
    // integer.
    int w(int(annot->GetPosition2()[0] * WIDTH_HEIGHT_PRECISION + 0.5f));
    widthSpinBox->blockSignals(true);
    widthSpinBox->setValue(w);
    widthSpinBox->blockSignals(false);

    int h(int(annot->GetPosition2()[1] * WIDTH_HEIGHT_PRECISION + 0.5f));
    heightSpinBox->blockSignals(true);
    heightSpinBox->setValue(h);
    heightSpinBox->blockSignals(false);

    // Set the orientation.  
    orientationComboBox->blockSignals(true);
    if (GetBool(LEGEND_ORIENTATION0))
    {
        if (GetBool(LEGEND_ORIENTATION1))
            orientationComboBox->setCurrentIndex(3);
        else
            orientationComboBox->setCurrentIndex(2);
    }
    else
    {
        if (GetBool(LEGEND_ORIENTATION1))
            orientationComboBox->setCurrentIndex(1);
        else
            orientationComboBox->setCurrentIndex(0);
    }
    orientationComboBox->blockSignals(false);

    int type = annot->GetIntAttribute3();
    //
    tickControl->blockSignals(true);
    tickControl->setChecked(GetBool(LEGEND_CONTROL_TICKS));
    tickControl->blockSignals(false);

    numTicksSpinBox->blockSignals(true);
    numTicksSpinBox->setEnabled(type == LEGEND_TYPE_VARIABLE && 
                                GetBool(LEGEND_CONTROL_TICKS));
    numTicksLabel->setEnabled(type == LEGEND_TYPE_VARIABLE &&
                              GetBool(LEGEND_CONTROL_TICKS));
    numTicksSpinBox->setValue(annot->GetIntAttribute2());
    numTicksSpinBox->blockSignals(false);

    minMaxCheckBox->blockSignals(true);
    minMaxCheckBox->setChecked(GetBool(LEGEND_MINMAX_INCLUSIVE));
    minMaxCheckBox->setEnabled(type == LEGEND_TYPE_VARIABLE &&
                               GetBool(LEGEND_CONTROL_TICKS));
    minMaxCheckBox->blockSignals(false);

    QString temp;
    size_t size;
    if (type == LEGEND_TYPE_VARIABLE)
    {
        doubleVector sv = annot->GetDoubleVector1();
        size = sv.size();
        ResizeSuppliedLabelsList(size);
        QString fmt(formatString->text());
        suppliedLabels->horizontalHeaderItem(0)->setText(tr("Values"));
        for (size_t i = 0; i < size; ++i)
        {
            temp.sprintf(fmt.toStdString().c_str(), sv[i]);
            suppliedLabels->item(i, 0)->setText(temp.simplified());
            suppliedLabels->item(i, 0)->setFlags(
                     Qt::ItemIsSelectable|Qt::ItemIsEditable|Qt::ItemIsEnabled);
        }
    }
    else 
    {
        stringVector sv = annot->GetStringVector2();
        size = sv.size();
        ResizeSuppliedLabelsList(size);
        suppliedLabels->horizontalHeaderItem(0)->setText(tr("Computed values"));
        suppliedLabels->resizeColumnToContents(0);
        for (size_t i = 0; i < size; ++i)
        {
            suppliedLabels->item(i, 0)->setText(sv[i].c_str());
            suppliedLabels->item(i, 0)->setFlags(Qt::NoItemFlags);
        }
    }
 
    stringVector sl = annot->GetStringVector1();
    size = sl.size();
    for (size_t i = 0; i < suppliedLabels->rowCount(); ++i)
    {
        if (i < size)
            suppliedLabels->item(i, 1)->setText(sl[i].c_str());
        else 
            suppliedLabels->item(i, 1)->setText("");
    }

    drawLabelsComboBox->blockSignals(true);
    int dv = GetBool(LEGEND_DRAW_VALUES) ? 1 : 0;
    int dl = GetBool(LEGEND_DRAW_LABELS) ? 2 : 0;
    drawLabelsComboBox->setCurrentIndex(dv+dl);
    drawLabelsComboBox->blockSignals(false);

    suppliedLabels->setEnabled(!GetBool(LEGEND_CONTROL_TICKS));
    addRowButton->setEnabled(type == LEGEND_TYPE_VARIABLE &&
                             !GetBool(LEGEND_CONTROL_TICKS));
    deleteRowButton->setEnabled(type == LEGEND_TYPE_VARIABLE &&
                               !GetBool(LEGEND_CONTROL_TICKS));

    //
    // Set the text color. If we're using the foreground color for the text
    // color then make the button be white and only let the user change the 
    // opacity.
    //
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity->blockSignals(true);
#endif
    QColor tc(annot->GetTextColor().Red(),
              annot->GetTextColor().Green(),
              annot->GetTextColor().Blue());
    textColorButton->setButtonColor(tc);
#ifdef TEXT_OPACITY_SUPPORTED
    textColorOpacity->setGradientColor(tc);
    textColorOpacity->setValue(annot->GetTextColor().Alpha());
    textColorOpacity->blockSignals(false);
    textColorOpacity->setEnabled(!annot->GetUseForegroundForTextColor());
#endif
    textColorButton->setEnabled(!annot->GetUseForegroundForTextColor());
    textColorLabel->setEnabled(!annot->GetUseForegroundForTextColor());

    // Set the bounding box color.
    drawBoundingBoxCheckBox->blockSignals(true);
    drawBoundingBoxCheckBox->setChecked(GetBool(LEGEND_DRAW_BOX));
    drawBoundingBoxCheckBox->blockSignals(false);
    boundingBoxOpacity->blockSignals(true);
    QColor bbc(annot->GetColor1().Red(),
               annot->GetColor1().Green(),
               annot->GetColor1().Blue());
    boundingBoxColorButton->setButtonColor(bbc);
    boundingBoxColorButton->setEnabled(GetBool(LEGEND_DRAW_BOX));
    boundingBoxOpacity->setEnabled(GetBool(LEGEND_DRAW_BOX));
    boundingBoxOpacity->setGradientColor(bbc);
    boundingBoxOpacity->setValue(annot->GetColor1().Alpha());
    boundingBoxOpacity->blockSignals(false);

    // Set the format string
    const stringVector &annotText = annot->GetText();
    if(annotText.size() > 0)
        formatString->setText(annotText[0].c_str());
    else
        formatString->setText("");

    // Set the "draw labels" box.
    drawTitleCheckBox->blockSignals(true);
    drawTitleCheckBox->setChecked(GetBool(LEGEND_DRAW_TITLE));
    drawTitleCheckBox->blockSignals(false);

    // Set the "draw labels" box.
    drawMinmaxCheckBox->blockSignals(true);
    drawMinmaxCheckBox->setChecked(GetBool(LEGEND_DRAW_MINMAX));
    drawMinmaxCheckBox->blockSignals(false);

    // Set the font height
    fontHeight->setText(QString().sprintf("%g", annot->GetDoubleAttribute1()));

    // Set the use foreground color check box.
    useForegroundColorCheckBox->blockSignals(true);
    useForegroundColorCheckBox->setChecked(annot->GetUseForegroundForTextColor());
    useForegroundColorCheckBox->blockSignals(false);

    // Set the font family
    fontFamilyComboBox->blockSignals(true);
    fontFamilyComboBox->setCurrentIndex(int(annot->GetFontFamily()));
    fontFamilyComboBox->blockSignals(false);

    // Set the bold check box.
    boldCheckBox->blockSignals(true);
    boldCheckBox->setChecked(annot->GetFontBold());
    boldCheckBox->blockSignals(false);

    // Set the italic check box.
    italicCheckBox->blockSignals(true);
    italicCheckBox->setChecked(annot->GetFontItalic());
    italicCheckBox->blockSignals(false);

    // Set the shadow check box.
    shadowCheckBox->blockSignals(true);
    shadowCheckBox->setChecked(annot->GetFontShadow());
    shadowCheckBox->blockSignals(false);
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::GetCurrentValues
//
// Purpose: 
//   Gets the current values for the text fields.
//
// Arguments:
//   which_widget : The widget for which we're getting the values. -1 for all.
//
// Programmer: Brad Whitlock
// Creation:   Tue Mar 20 16:00:25 PST 2007
//
// Modifications:
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Kathleen Bonnell, Thu Oct  1 14:49:42 PDT 2009
//   Added code for ID_intAttribute2, ID_doubleVector1, ID_stringVector1.
//
// ****************************************************************************

void
QvisLegendAttributesInterface::GetCurrentValues(int which_widget)
{
    bool doAll = (which_widget == -1);

    if(which_widget == 0 || doAll)
    {
        // Get the new position
        GetScreenPosition(positionEdit, tr("Lower left"));
    }

    if(which_widget == 1 || doAll)
    {
        QString fmt(formatString->text());
        int count = 0;
        for(int i = 0; i < fmt.length(); ++i)
            if(fmt[i] == '%')
                ++count;
        if(count == 0)
        {
            Error(tr("The format string for the legend was not used because it "
                     "does not contain a '%' character."));
        }
        else if(count > 1)
        {
            Error(tr("The format string for the legend was not used because it "
                     "contains multiple '%' characters."));
        }
        else
        {
            stringVector sv;
            sv.push_back(formatString->text().toStdString());
            annot->SetText(sv);
        }
    }

    if(which_widget == 2 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(widthSpinBox);
        int w = widthSpinBox->value();
        double pos2[3];
        pos2[0] = double(w) * (1. / WIDTH_HEIGHT_PRECISION);
        pos2[1] = annot->GetPosition2()[1];
        pos2[2] = annot->GetPosition2()[2];
        annot->SetPosition2(pos2);
    }

    if(which_widget == 3 || doAll)
    {
        // Get its new current value and store it in the atts.
        ForceSpinBoxUpdate(heightSpinBox);
        int h = heightSpinBox->value();
        double pos2[3];
        pos2[0] = annot->GetPosition2()[0];
        pos2[1] = double(h) * (1. / WIDTH_HEIGHT_PRECISION);
        pos2[2] = annot->GetPosition2()[2];
        annot->SetPosition2(pos2);
    }

    if(which_widget == 4 || doAll)
    {
        bool okay;
        double val = fontHeight->text().toDouble(&okay);
        if(okay)
            annot->SetDoubleAttribute1(val);
    }

    if (which_widget == AnnotationObject::ID_intAttribute2 || doAll)
    {
        annot->SetIntAttribute2(numTicksSpinBox->value());
    }
    if (which_widget == AnnotationObject::ID_doubleVector1 || doAll)
    {
        doubleVector temp;
        double d;
        QString txt; 
        int nRows = suppliedLabels->rowCount();
        bool okay;
        for (int rowNum = 0; rowNum < nRows; ++rowNum)
        {
            txt = suppliedLabels->item(rowNum, 0)->text().simplified();
            d = txt.toDouble(&okay);
            if (okay)
                temp.push_back(d);
        }
        annot->SetDoubleVector1(temp);
    }
    if (which_widget == AnnotationObject::ID_stringVector1 || doAll)
    {
        stringVector temp;
        QString txt; 
        int nRows = suppliedLabels->rowCount();
        bool allEmpty = true;
        for (int rowNum = 0; rowNum < nRows; ++rowNum)
        {
            txt = suppliedLabels->item(rowNum, 1)->text();
            temp.push_back(txt.trimmed().toStdString());
        }
        annot->SetStringVector1(temp);
    }
}

//
// Qt Slot functions
//

void
QvisLegendAttributesInterface::layoutChanged(bool val)
{
    SetBool(LEGEND_MANAGE_POSITION, val);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::positionChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   position line edit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::positionChanged(double x, double y)
{
    double pos[] = {x, y, 0.};
    annot->SetPosition(pos);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::widthChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the value of the width
//   spin box changes.
//
// Arguments:
//   w : The new width in percent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::widthChanged(int w)
{
    double pos2[3];
    pos2[0] = double(w) * (1. / WIDTH_HEIGHT_PRECISION);
    pos2[1] = annot->GetPosition2()[1];
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::heightChanged
//
// Purpose: 
//   This a Qt slot function that is called when the value of the height spin
//   box changes.
//
// Arguments:
//   h : The new height in percent.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::heightChanged(int h)
{
    double pos2[3];
    pos2[0] = annot->GetPosition2()[0];
    pos2[1] = double(h) * (1. / WIDTH_HEIGHT_PRECISION);
    pos2[2] = annot->GetPosition2()[2];
    annot->SetPosition2(pos2);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::orientationChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the orientation is changed.
//
// Arguments:
//   orientation: an int acting as an enum, mapping to 
//   enum avtLegend::LegendOrientation and the options in the combo box.
//
// Programmer: Dave Bremer
// Creation:   Fri Oct  3 13:57:16 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::orientationChanged(int orientation)
{
    SetBool(LEGEND_ORIENTATION0, (orientation==2 || orientation==3) );
    SetBool(LEGEND_ORIENTATION1, (orientation==1 || orientation==3) );
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textChanged
//
// Purpose: 
//   This is a Qt slot function that is called when return is pressed in the 
//   text line edit.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textChanged()
{
    GetCurrentValues(1);
    Apply();
}

// ****************************************************************************
// Class: QvisLegendAttributesInterface::fontHeightChanged
//
// Purpose:
//   This is a Qt slot function that is called when return is pressed in the 
//   fontHeight line edit.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 09:27:24 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::fontHeightChanged()
{
    GetCurrentValues(4);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start color is
//   selected.
//
// Arguments:
//   c : The new start color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textColorChanged(const QColor &c)
{
    int a = annot->GetTextColor().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetTextColor(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new start opacity is
//   selected.
//
// Arguments:
//   opacity : The new start opacity.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::textOpacityChanged(int opacity)
{
    ColorAttribute tc(annot->GetTextColor());
    tc.SetAlpha(opacity);
    annot->SetTextColor(tc);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: drawBoundingBoxToggled
//
// Programmer: Brad Whitlock
// Creation:   Wed Mar 21 15:34:13 PST 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::drawBoundingBoxToggled(bool val)
{
    SetBool(LEGEND_DRAW_BOX, val);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::boundingBoxColorChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new bounding box color is
//   selected.
//
// Arguments:
//   c : The new color.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boundingBoxColorChanged(const QColor &c)
{
    int a = annot->GetColor1().Alpha();
    ColorAttribute tc(c.red(), c.green(), c.blue(), a);
    annot->SetColor1(tc);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::textOpacityChanged
//
// Purpose: 
//   This is a Qt slot function that is called when a new bounding box opacity
//   is selected.
//
// Arguments:
//   opacity : The new bbox opacity.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boundingBoxOpacityChanged(int opacity)
{
    ColorAttribute c(annot->GetColor1());
    c.SetAlpha(opacity);
    annot->SetColor1(c);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::fontFamilyChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the font family is changed.
//
// Arguments:
//   family  : The new font family.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::fontFamilyChanged(int family)
{
    annot->SetFontFamily((AnnotationObject::FontFamily)family);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::drawMinmaxToggled
//
// Purpose: 
//   Called when the draw min/max checkbox is toggled.
//
// Programmer: Hank Childs
// Creation:   January 23, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::drawMinmaxToggled(bool val)
{
    SetBool(LEGEND_DRAW_MINMAX, val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::drawTitleToggled
//
// Purpose: 
//   Called when the draw title checkbox is toggled.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:02:51 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::drawTitleToggled(bool val)
{
    SetBool(LEGEND_DRAW_TITLE, val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::boldToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the bold checkbox is toggled.
//
// Arguments:
//   val : The new bold flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::boldToggled(bool val)
{
    annot->SetFontBold(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::italicToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the italic checkbox is toggled.
//
// Arguments:
//   val : The new italic flag.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::italicToggled(bool val)
{
    annot->SetFontItalic(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::shadowToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the shadow checkbox is
//   toggled.
//
// Arguments:
//   val : The new shadow setting.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::shadowToggled(bool val)
{
    annot->SetFontShadow(val);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisLegendAttributesInterface::useForegroundColorToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the useForegroundColor
//   check box is clicked.
//
// Arguments:
//   val : The new setting for useForegroundColor
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 26 12:03:56 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::useForegroundColorToggled(bool val)
{
    annot->SetUseForegroundForTextColor(val);
    Apply();
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::tickControlToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the tickControl
//   check box is clicked.
//
// Arguments:
//   val : The new setting for LEGEND_CONTROL_TICKS
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::tickControlToggled(bool val)
{
    SetBool(LEGEND_CONTROL_TICKS, val);
    Apply();
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::numTicksChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the numTicks
//   spin box is modified.
//
// Arguments:
//   val : The new setting for numTicks
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::numTicksChanged(int n)
{
    GetCurrentValues(AnnotationObject::ID_intAttribute2);
    Apply();
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::minMaxToggled
//
// Purpose: 
//   This is a Qt slot function that is called when the minMax
//   check box is clicked.
//
// Arguments:
//   val : The new setting for LEGEND_MINMAX_INCLUSIVE
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::minMaxToggled(bool val)
{
    SetBool(LEGEND_MINMAX_INCLUSIVE, val);
    Apply();
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::drawLabelsChanged
//
// Purpose: 
//   This is a Qt slot function that is called when the drawLabels
//   combo box is modified.
//
// Arguments:
//   val : The new setting for LEGEND_DRAW_VALUES
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::drawLabelsChanged(int val)
{
    SetBool(LEGEND_DRAW_VALUES, (val == 1 || val == 3));
    SetBool(LEGEND_DRAW_LABELS, (val == 2 || val == 3));
    SetUpdate(false);
    Apply();
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::addSuppliedLabelsRow
//
// Purpose: 
//   This is a Qt slot function that is called with the addRow push button
//   is clicked.
//
// Arguments:
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************
void
QvisLegendAttributesInterface::addSuppliedLabelsRow()
{
    ResizeSuppliedLabelsList(suppliedLabels->rowCount() + 1);
}


// ****************************************************************************
// Method: QvisLegendAttributesInterface::addSuppliedLabelsRow
//
// Purpose: 
//   This is a Qt slot function that is called with the deleteRow push button
//   is clicked.
//
// Arguments:
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::deleteSelectedRow()
{
    suppliedLabels->removeRow(suppliedLabels->currentRow());
}



// ****************************************************************************
// Method: QvisLegendAttributesInterface::ResizeSuppliedLabelsList
//
// Purpose: 
//   This is an internal method to change the number of rowsn in the 
//   QTableWidget where users enter their own values for labels.
//
// Arguments:
//   newSize : The new size for the table. 
//
// Programmer: Kathleen Bonnell 
// Creation:   October 1, 2009
//
// Modifications:
//   
// ****************************************************************************

void
QvisLegendAttributesInterface::ResizeSuppliedLabelsList(int newSize)
{
    int nRows = suppliedLabels->rowCount();
    if (nRows < newSize)
    {
        suppliedLabels->setRowCount(newSize);
        for (int i = nRows; i < newSize; ++i)
        {
            suppliedLabels->setItem(i, 0, new QTableWidgetItem(tr("")));
            suppliedLabels->setItem(i, 1, new QTableWidgetItem(tr("")));
        }
    }
    else if (nRows > newSize)
    {
        for (int i = nRows-1; i >= newSize; --i)
        {
            suppliedLabels->removeRow(i);
        }
    }
}
