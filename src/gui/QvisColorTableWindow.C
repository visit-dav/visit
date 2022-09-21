// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QvisColorTableWindow.h>
#include <ColorTableAttributes.h>
#include <QApplication>
#include <QButtonGroup>
#include <QLayout>
#include <QPushButton>
#include <QCheckBox>
#include <QComboBox>
#include <QGroupBox>
#include <QLabel>
#include <QLineEdit>
#include <QTreeWidget>
#include <QTreeWidgetItem>
#include <QTreeWidgetItemIterator>
#include <QHeaderView>
#include <QRadioButton>
#include <QSlider>
#include <QSpinBox>
#include <QDesktopWidget>

#include <QvisSpectrumBar.h>
#include <QvisColorSelectionWidget.h>
#include <QvisColorGridWidget.h>
#include <QvisNoDefaultColorTableButton.h>
#include <ColorControlPoint.h>
#include <ColorControlPointList.h>
#include <DataNode.h>
#include <ViewerProxy.h>
#include <DebugStream.h>


// Defines. Make these part of ColorTableAttributes sometime.
#define DEFAULT_DISCRETE_COLS    6
#define DEFAULT_DISCRETE_ROWS    5
#define DEFAULT_DISCRETE_NCOLORS (DEFAULT_DISCRETE_ROWS * DEFAULT_DISCRETE_COLS)

#define SELECT_FOR_CONTINUOUS 0
#define SELECT_FOR_DISCRETE   1

#define ADDTAG        0
#define REMOVETAG     1
#define ADDTAGSTR    "0"
#define REMOVETAGSTR "1"

// ****************************************************************************
// Method: QvisColorTableWindow::QvisColorTableWindow
//
// Purpose:
//   This is the constructor for the QvisColorTableWindow class.
//
// Arguments:
//   colorAtts : The ColorTableAttributes that the window observes.
//   caption   : The caption that appears in the window's title.
//   shortName : The name that appears in the notepad area when posted.
//   notepad   : The notepad area to which the window posts.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:58:08 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Jun 20 13:41:16 PST 2001
//   Added initialization of an observer.
//
//   Brad Whitlock, Wed Nov 20 16:04:01 PST 2002
//   Added initialization of defaultCT.
//
//   Brad Whitlock, Wed Feb 26 10:59:45 PDT 2003
//   Initialized colorTableTypeGroup.
//
//   Brad Whitlock, Wed Apr  9 11:59:35 PDT 2008
//   QString for caption, shortName.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Added new tag-related vars to constructor.
// 
//   Justin Privitera, Thu Jul 14 16:57:42 PDT 2022
//   Added new searching-related vars to the constructor.
// 
//   Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//   TagEdit added.
//
// ****************************************************************************

QvisColorTableWindow::QvisColorTableWindow(
    ColorTableAttributes *colorAtts_, const QString &caption,
    const QString &shortName, QvisNotepadArea *notepad) :
    QvisPostableWindowObserver(colorAtts_, caption, shortName, notepad,
                               QvisPostableWindowObserver::ApplyButton, false),
    currentColorTable("none"), ctObserver(colorAtts_)
{
    colorAtts = colorAtts_;
    colorCycle = 0;
    sliding = false;
    colorSelect = 0;
    colorTableTypeGroup = 0;
    tagsVisible = false;
    tagsMatchAny = true;
    searchingOn = false;
    searchTerm = QString("");
    tagEdit = QString("");
}

// ****************************************************************************
// Method: QvisColorTableWindow::~QvisColorTableWindow
//
// Purpose:
//   Destructor for the QvisColorTableWindow class.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:59:14 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 10:59:56 PDT 2003
//   Deleted colorTableTypeGroup.
//
// ****************************************************************************

QvisColorTableWindow::~QvisColorTableWindow()
{
    colorAtts = 0;
    delete colorSelect;
}

// ****************************************************************************
// Method: QvisColorTableWindow::CreateWindowContents
//
// Purpose:
//   Creates the widgets for the window.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 12:04:27 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 13 16:52:58 PST 2002
//   Added a little spacing.
//
//   Brad Whitlock, Wed Nov 20 15:29:46 PST 2002
//   I added support for discrete color tables.
//
//   Brad Whitlock, Wed Feb 26 10:58:14 PDT 2003
//   I changed the window so that discrete color tables can have an arbitrary
//   number of colors.
//
//   Brad Whitlock, Tue Jul 1 16:38:50 PST 2003
//   I added an export button.
//
//   Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//   Refactoring of color grid widget caused index to lose "color" in names.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:11:50 EST 2008
//   Added show index hints checkbox for discrete color tables.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
//   Brad Whitlock, Fri Apr 27 15:07:13 PDT 2012
//   I changed smoothing method to a combo box.
//
//   Kathleen Biagas, Mon Aug  4 15:45:44 PDT 2014
//   Added a groupToggle. Change discrete/default buttons to color table
//   buttons.
//
//   Kathleen Biagas, Wed Jun  8 17:10:30 PDT 2016
//   Set keyboard tracking to false for spin boxes so that 'valueChanged'
//   signal will only emit when 'enter' is pressed or spinbox loses focus.
//
//   Mark C. Miller, Wed Feb 28 14:27:38 PST 2018
//   Handle "smoothing" label correctly.
//
//   Kathleen Biagas, Mon Jun 22 10:08:41 PDT 2020
//   Change colorNumColors spin box max to 256.
//
//   Kathleen Biagas, Thu Jan 21, 2021
//   Remove unused var 'QString n'.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Completely redid the gui to remove categories and add tags.
// 
//   Justin Privitera, Wed Jul 13 15:24:42 PDT 2022
//   Called `QvisNoDefaultColorTableButton` constructor with its new boolean
//   argument that signals if the button is discrete or continuous.
//
//   Justin Privitera, Thu Jul 14 16:57:42 PDT 2022
//   Added searchbox gui element and hooked up signals and slots for searching.
// 
//   Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//   Added tag editor gui elements.
//
// ****************************************************************************

void
QvisColorTableWindow::CreateWindowContents()
{
    // Create the widgets needed to set the default color tables.
    topLayout->setMargin(2);
    defaultGroup = new QGroupBox(central);
    defaultGroup->setTitle(tr("Default Color Table"));
    topLayout->addWidget(defaultGroup, 5);
    
    QVBoxLayout *innerDefaultTopLayout = new QVBoxLayout(defaultGroup);
    QGridLayout *innerDefaultLayout = new QGridLayout();
    innerDefaultTopLayout->addLayout(innerDefaultLayout);
    innerDefaultLayout->setColumnMinimumWidth(1, 10);

    defaultContinuous = new QvisNoDefaultColorTableButton(defaultGroup, false);
    connect(defaultContinuous, SIGNAL(selectedColorTable(const QString &)),
            this, SLOT(setDefaultContinuous(const QString &)));
    innerDefaultLayout->addWidget(defaultContinuous, 0, 1);
    defaultContinuousLabel = new QLabel(tr("Continuous"), defaultGroup);
    innerDefaultLayout->addWidget(defaultContinuousLabel, 0, 0);

    defaultDiscrete = new QvisNoDefaultColorTableButton(defaultGroup, true);
    connect(defaultDiscrete, SIGNAL(selectedColorTable(const QString &)),
            this, SLOT(setDefaultDiscrete(const QString &)));
    innerDefaultLayout->addWidget(defaultDiscrete, 1, 1);
    defaultDiscreteLabel = new QLabel(tr("Discrete"), defaultGroup);
    innerDefaultLayout->addWidget(defaultDiscreteLabel, 1, 0);

    // Create the widget group that contains all of the color table
    // management stuff.
    colorTableWidgetGroup = new QGroupBox(central);
    colorTableWidgetGroup->setTitle(tr("Manager"));
    topLayout->addWidget(colorTableWidgetGroup, 5);
    QVBoxLayout *innerColorTableLayout = new QVBoxLayout(colorTableWidgetGroup);

    // Create the color management widgets.
    mgLayout = new QGridLayout();
    innerColorTableLayout->addLayout(mgLayout);

    newButton = new QPushButton(tr("New"), colorTableWidgetGroup);
    connect(newButton, SIGNAL(clicked()), this, SLOT(addColorTable()));
    mgLayout->addWidget(newButton, 0, 0, 1, 2);

    deleteButton = new QPushButton(tr("Delete"), colorTableWidgetGroup);
    connect(deleteButton, SIGNAL(clicked()), this, SLOT(deleteColorTable()));
    mgLayout->addWidget(deleteButton, 0, 2, 1, 2);

    exportButton = new QPushButton(tr("Export"), colorTableWidgetGroup);
    connect(exportButton, SIGNAL(clicked()), this, SLOT(exportColorTable()));
    mgLayout->addWidget(exportButton, 0, 4, 1, 2);

    tagFilterToggle = new QCheckBox(tr("Filter tables by Tag"), colorTableWidgetGroup);
    connect(tagFilterToggle, SIGNAL(toggled(bool)),
            this, SLOT(taggingToggled(bool)));
    mgLayout->addWidget(tagFilterToggle, 1, 0, 1, 6);

    tagCombiningBehaviorChoice = new QComboBox(colorTableWidgetGroup);
    tagCombiningBehaviorChoice->addItem(tr("Colortables must match any selected tag"));
    tagCombiningBehaviorChoice->addItem(tr("Colortables must match every selected tag"));
    if (tagsMatchAny)
        tagCombiningBehaviorChoice->setCurrentIndex(0);
    else
        tagCombiningBehaviorChoice->setCurrentIndex(1);        
    connect(tagCombiningBehaviorChoice, SIGNAL(activated(int)),
            this, SLOT(tagCombiningChanged(int)));
    mgLayout->addWidget(tagCombiningBehaviorChoice, 1, 2, 1, 4);

    nameListBox = new QTreeWidget(colorTableWidgetGroup);
    nameListBox->setMinimumHeight(100);
    nameListBox->setColumnCount(1);
    // don't want the header
    nameListBox->header()->close();
    connect(nameListBox, SIGNAL(currentItemChanged(QTreeWidgetItem *, QTreeWidgetItem*)),
            this, SLOT(highlightColorTable(QTreeWidgetItem *, QTreeWidgetItem*)));
    mgLayout->addWidget(nameListBox, 3, 0, 1, 6);

    tagTable = new QTreeWidget(colorTableWidgetGroup);
    QStringList headers;
    headers << tr("Enabled") << tr("Tag Name");
    tagTable->setHeaderLabels(headers);
    tagTable->header()->setSectionResizeMode(0, QHeaderView::ResizeToContents);
    tagTable->header()->setSectionResizeMode(1, QHeaderView::ResizeToContents);
    connect(tagTable, SIGNAL(itemChanged(QTreeWidgetItem *, int)), 
            this, SLOT(tagTableItemSelected(QTreeWidgetItem *, int)));
    tagTable->clear();
    tagTable->setSortingEnabled(true);
    tagTable->setMinimumHeight(100);
    tagTable->setMinimumWidth(250);
    tagTable->setColumnCount(2);
    mgLayout->addWidget(tagTable, 3, 0, 1, 3);

    colorTableName = new QLabel(tr("Name"), colorTableWidgetGroup);
    mgLayout->addWidget(colorTableName, 4, 0, 1, 1, Qt::AlignLeft);
    nameLineEdit = new QLineEdit(colorTableWidgetGroup);
    connect(nameLineEdit, SIGNAL(textEdited(const QString &)),
            this, SLOT(searchEdited(const QString &)));
    mgLayout->addWidget(nameLineEdit, 4, 1, 1, 2);

    searchToggle = new QCheckBox(tr("Enable Searching"), colorTableWidgetGroup);
    connect(searchToggle, SIGNAL(toggled(bool)),
            this, SLOT(searchingToggled(bool)));
    mgLayout->addWidget(searchToggle, 4, 3, 1, 3);

    tagLabel = new QLabel(tr("Tags"), colorTableWidgetGroup);
    mgLayout->addWidget(tagLabel, 5, 0, 1, 1, Qt::AlignLeft);
    tagLineEdit = new QLineEdit(colorTableWidgetGroup);
    mgLayout->addWidget(tagLineEdit, 5, 1, 1, 5);

    // Tag editor
    tagEditorLabel = new QLabel(tr("Tag Editor"), colorTableWidgetGroup);
    mgLayout->addWidget(tagEditorLabel, 6, 0, 1, 1, Qt::AlignLeft);
    tagEditorLineEdit = new QLineEdit(colorTableWidgetGroup);
    connect(tagEditorLineEdit, SIGNAL(editingFinished()),
            this, SLOT(tagEdited()));
    mgLayout->addWidget(tagEditorLineEdit, 6, 1, 1, 3);
    tagAddRemoveButton = new QPushButton(tr("Add/Remove Tag"), colorTableWidgetGroup);
    connect(tagAddRemoveButton, SIGNAL(clicked()), this, SLOT(addRemoveTag()));
    mgLayout->addWidget(tagAddRemoveButton, 6, 4, 1, 2);

    // Add the group box that will contain the color-related widgets.
    colorWidgetGroup = new QGroupBox(central);
    colorWidgetGroup->setTitle(tr("Editor"));
    topLayout->addWidget(colorWidgetGroup, 100);
    QVBoxLayout *innerColorLayout = new QVBoxLayout(colorWidgetGroup);

    // Create controls to set the number of colors in the color table.
    QGridLayout *colorInfoLayout = new QGridLayout();
    innerColorLayout->addLayout(colorInfoLayout);
    colorNumColors = new QSpinBox(colorWidgetGroup);
    colorNumColors->setKeyboardTracking(false);
    colorNumColors->setRange(2,256);
    colorNumColors->setSingleStep(1);
    connect(colorNumColors, SIGNAL(valueChanged(int)),
            this, SLOT(resizeColorTable(int)));
    colorInfoLayout->addWidget(colorNumColors, 0, 1, 1, 2);
    colorInfoLayout->addWidget(new QLabel(tr("Number of colors"),
                                          colorWidgetGroup), 0, 0);

    // Create radio buttons to convert the color table between color table types.
    colorInfoLayout->addWidget(new QLabel(tr("Color table type")), 1, 0);
    colorTableTypeGroup = new QButtonGroup(colorWidgetGroup);
    QRadioButton *rb = new QRadioButton(tr("Continuous"),colorWidgetGroup);
    colorTableTypeGroup->addButton(rb,0);
    colorInfoLayout->addWidget(rb, 1, 1);
    rb = new QRadioButton(tr("Discrete"),colorWidgetGroup);
    colorTableTypeGroup->addButton(rb,1);
    colorInfoLayout->addWidget(rb, 1, 2);
    connect(colorTableTypeGroup, SIGNAL(buttonClicked(int)),
            this, SLOT(setColorTableType(int)));


    // Create the buttons that help manipulate the spectrum bar.
    QHBoxLayout *seLayout = new QHBoxLayout();
    innerColorLayout->addLayout(seLayout);

    alignPointButton = new QPushButton(tr("Align"), colorWidgetGroup);
    connect(alignPointButton, SIGNAL(clicked()),
            this, SLOT(alignControlPoints()));
    seLayout->addWidget(alignPointButton);
    seLayout->addStretch(10);

    smoothLabel = new QLabel(tr("Smoothing"), colorWidgetGroup);
    seLayout->addWidget(smoothLabel);
    smoothingMethod = new QComboBox(colorWidgetGroup);
    smoothingMethod->addItem(tr("None"));
    smoothingMethod->addItem(tr("Linear"));
    smoothingMethod->addItem(tr("Cubic Spline"));
    connect(smoothingMethod, SIGNAL(activated(int)),
            this, SLOT(smoothingMethodChanged(int)));
    seLayout->addWidget(smoothingMethod);

    equalCheckBox = new QCheckBox(tr("Equal"), colorWidgetGroup);
    connect(equalCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(equalSpacingToggled(bool)));
    seLayout->addWidget(equalCheckBox);

    // Create the spectrum bar.
    spectrumBar = new QvisSpectrumBar(colorWidgetGroup);
    spectrumBar->setMinimumHeight(100);
    spectrumBar->setMaximumHeight(110);
    spectrumBar->addControlPoint(QColor(255,0,0),   0.);
    spectrumBar->addControlPoint(QColor(255,255,0), 0.25);
    spectrumBar->addControlPoint(QColor(0,255,0),   0.5);
    spectrumBar->addControlPoint(QColor(0,255,255), 0.75);
    spectrumBar->addControlPoint(QColor(0,0,255),   1.);
    connect(spectrumBar, SIGNAL(controlPointMoved(int,float)),
            this, SLOT(controlPointMoved(int,float)));
    connect(spectrumBar, SIGNAL(selectColor(int, const QPoint &)),
            this, SLOT(chooseContinuousColor(int, const QPoint &)));
    connect(spectrumBar, SIGNAL(activeControlPointChanged(int)),
            this, SLOT(activateContinuousColor(int)));

    innerColorLayout->addWidget(spectrumBar, 100);

    // Create the discrete color table widgets.
    discreteColors = new QvisColorGridWidget(colorWidgetGroup);
    discreteColors->setMinimumHeight(100);
    discreteColors->setMaximumHeight(110);
    QColor *tmpColors = new QColor[DEFAULT_DISCRETE_NCOLORS];
    for(int i = 0; i < DEFAULT_DISCRETE_NCOLORS; ++i)
    {
        int c = int((float(i)/float(DEFAULT_DISCRETE_NCOLORS - 1)) * 255);
        tmpColors[i] = QColor(c,c,c);
    }
    discreteColors->setPaletteColors(tmpColors, DEFAULT_DISCRETE_NCOLORS, DEFAULT_DISCRETE_COLS);
    discreteColors->setFrame(false);
    discreteColors->setShowIndexHints(false);
    discreteColors->setBoxSize(16);
    discreteColors->setBoxPadding(8);
    discreteColors->setSelectedIndex(0);
    connect(discreteColors, SIGNAL(selectedColor(const QColor &, int)),
            this, SLOT(activateDiscreteColor(const QColor &, int)));
    connect(discreteColors, SIGNAL(activateMenu(const QColor &, int, int, const QPoint &)),
            this, SLOT(chooseDiscreteColor(const QColor &, int, int, const QPoint &)));
    delete [] tmpColors;
    innerColorLayout->addWidget(discreteColors, 100);

    // Add a check box for index hinting
    showIndexHintsCheckBox = new QCheckBox(tr("Show index hints"), colorWidgetGroup);
    showIndexHintsCheckBox->setChecked(false);
    connect(showIndexHintsCheckBox, SIGNAL(toggled(bool)),
            this, SLOT(showIndexHintsToggled(bool)));
    innerColorLayout->addWidget(showIndexHintsCheckBox);


    // Create the discrete color table sliders, text fields.
    QGridLayout *discreteLayout = new QGridLayout();
    innerColorLayout->addLayout(discreteLayout);
    QString cnames[4];
    cnames[0] = tr("Red");
    cnames[1] = tr("Green");
    cnames[2] = tr("Blue");
    cnames[3] = tr("Alpha");
    for(int j = 0; j < 4; ++j)
    {
        componentSliders[j] = new QSlider(Qt::Horizontal,colorWidgetGroup);
        componentSliders[j]->setRange(0, 255);
        componentSliders[j]->setPageStep(10);
        componentSliders[j]->setValue(0);

        discreteLayout->addWidget(componentSliders[j], j, 1);

        componentLabels[j] = new QLabel(cnames[j], colorWidgetGroup);
        discreteLayout->addWidget(componentLabels[j], j, 0);

        componentSpinBoxes[j] = new QSpinBox(colorWidgetGroup);
        componentSpinBoxes[j]->setKeyboardTracking(false);
        componentSpinBoxes[j]->setRange(0,255);
        componentSpinBoxes[j]->setSingleStep(1);


        // Hook up some signals and slots
        if(j == 0)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(redValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(redValueChanged(int)));
        }
        else if(j == 1)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(greenValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(greenValueChanged(int)));
        }
        else if(j == 2)
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(blueValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(blueValueChanged(int)));
        }
        else
        {
            connect(componentSliders[j], SIGNAL(valueChanged(int)),
                    this, SLOT(alphaValueChanged(int)));
            connect(componentSpinBoxes[j], SIGNAL(valueChanged(int)),
                    this, SLOT(alphaValueChanged(int)));
        }
        connect(componentSliders[j], SIGNAL(sliderPressed()),
                this, SLOT(sliderPressed()));
        connect(componentSliders[j], SIGNAL(sliderReleased()),
                this, SLOT(sliderReleased()));

        discreteLayout->addWidget(componentSpinBoxes[j], j, 2);
    }
    innerColorLayout->addStretch(5);

    // Create the color selection widget.
    colorSelect = new QvisColorSelectionWidget(NULL,Qt::Popup);
    connect(colorSelect, SIGNAL(selectedColor(const QColor &)),
            this, SLOT(selectedColor(const QColor &)));
}

// ****************************************************************************
// Method: QvisColorTableWindow::StringifyTagChanges
//
// Purpose: Packs the tag changes data structure into a string vector so it
//    can be easily written to a node.
//
// Programmer: Justin Privitera
// Creation:   Thu Aug 25 15:04:55 PDT 2022
//
// Modifications:
//
// ****************************************************************************
stringVector
QvisColorTableWindow::StringifyTagChanges()
{
    stringVector retval;
    for (const auto& mapitem : tagChanges)
    {
        std::stringstream changes;
        changes << mapitem.first << ":";
        for (const auto& setitem : mapitem.second)
            changes << setitem.first << "," << setitem.second << ";";
        retval.emplace_back(changes.str());
    }
    return retval;
}

// ****************************************************************************
// Method: QvisColorTableWindow::UnstringifyAndMergeTagChanges
//
// Purpose: Converts a stringvector representation of tag changes into 
//    actionable tag changes. Adds them to the tag changes data structure and
//    makes the changes happen.
//
// Programmer: Justin Privitera
// Creation:   Thu Aug 25 15:04:55 PDT 2022
//
// Modifications:
//
// ****************************************************************************
void
QvisColorTableWindow::UnstringifyAndMergeTagChanges(stringVector changes)
{
    std::for_each(changes.begin(), changes.end(), 
        [this](std::string ctTagChanges)
        {
            size_t titleSep = ctTagChanges.find(":");
            std::string tagName{ctTagChanges.substr(0, titleSep)};
            std::string table{ctTagChanges.substr(titleSep + 1)};

            size_t addRemoveSep{table.find(",")};
            size_t entrySep{0};

            while (addRemoveSep != std::string::npos)
            {
                std::string addRemoveText{table.substr(entrySep, addRemoveSep - entrySep)};
                entrySep = table.find(";", entrySep);
                addRemoveSep += 1;
                std::string ctName{table.substr(addRemoveSep, entrySep - addRemoveSep)};
                addRemoveSep = table.find(",", addRemoveSep);
                entrySep += 1;

                auto *ccpl{const_cast<ColorControlPointList *>(colorAtts->GetColorControlPoints(ctName))};
                auto result{ccpl->ValidateTag(tagName)};
                if (result.first)
                {
                    if (addRemoveText == ADDTAGSTR && ! ccpl->HasTag(tagName))
                        addTagToColorTable(ctName, tagName, ccpl);
                    else if (addRemoveText == REMOVETAGSTR && ccpl->HasTag(tagName))
                        removeTagFromColorTable(ctName, tagName, ccpl);
                }
                else
                {
                    debug1 << "QvisColorTableWindow WARNING: " << result.second;
                }
            }
        });
}


// ****************************************************************************
// Method: QvisColorTableWindow::CreateNode
//
// Purpose:
//   Saves the windows settings.
//
// Arguments:
//   parentNode : The node that will contain the settings.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 09:18:09 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Added ability for tag settings to be written to config/session files.
// 
//   Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//   Write tag changes to node.
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Now plays nice with the new tag data structure.
//
// ****************************************************************************

void
QvisColorTableWindow::CreateNode(DataNode *parentNode)
{
    // Call the base class's method to save the generic window attributes.
    QvisPostableWindowSimpleObserver::CreateNode(parentNode);

    if(saveWindowDefaults &&
       !currentColorTable.isEmpty() &&
       currentColorTable != "none")
    {
        DataNode *node = parentNode->GetNode(windowTitle().toStdString());

        // Save the current color table.
        std::string ct(currentColorTable.toStdString());
        node->AddNode(new DataNode("currentColorTable", ct));
        stringVector tagNames;
        boolVector activeTags;
        for (const auto mapitem : tagList)
        {
            tagNames.emplace_back(mapitem.first);
            activeTags.push_back(mapitem.second.active);
        }
        node->AddNode(new DataNode("tagList", tagNames));
        node->AddNode(new DataNode("activeTags", activeTags));
        node->AddNode(new DataNode("tagsVisible", tagsVisible));
        node->AddNode(new DataNode("tagsMatchAny", tagsMatchAny));
        node->AddNode(new DataNode("tagChanges", StringifyTagChanges()));
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::SetFromNode
//
// Purpose:
//   Gets the window's settings.
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar 6 09:18:41 PDT 2006
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Added ability for tag settings to be read from config/session files.
// 
//   Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//   Read tag changes from node if possible.
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Now plays nice with the new tag data structure.
//
// ****************************************************************************

void
QvisColorTableWindow::SetFromNode(DataNode *parentNode, const int *borders)
{
    DataNode *winNode = parentNode->GetNode(windowTitle().toStdString());
    if(winNode == 0)
        return;

    // Get the active tab and show it.
    DataNode *node, *node2;
    if((node = winNode->GetNode("currentColorTable")) != 0)
        currentColorTable = QString(node->AsString().c_str());
    if((node = winNode->GetNode("tagList")) != 0 && (node2 = winNode->GetNode("activeTags")) != 0)
    {
        stringVector tagNames{node->AsStringVector()};
        std::vector<bool> activeTags{node2->AsBoolVector()};
        if (tagNames.size() == activeTags.size())
        {
            for (int i = 0; i < tagNames.size(); i ++)
                tagList[tagNames[i]].active = activeTags[i];
        }
        // TODO else
    }
    if((node = winNode->GetNode("tagsVisible")) != 0)
        tagsVisible = node->AsBool();
    if((node = winNode->GetNode("tagsMatchAny")) != 0)
        tagsMatchAny = node->AsBool();
    if((node = winNode->GetNode("tagChanges")) != 0)
        UnstringifyAndMergeTagChanges(node->AsStringVector());

    // Call the base class's function.
    QvisPostableWindowSimpleObserver::SetFromNode(parentNode, borders);
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateWindow
//
// Purpose:
//   This method is called when the window's subject is changed. The
//   subject tells this window what attributes changed and we put the
//   new values into those widgets.
//
// Arguments:
//   doAll : If this flag is true, update all the widgets regardless
//           of whether or not they are selected.
//
// Returns:
//
// Note:
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 12:07:55 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:11:25 PST 2002
//   I added support for discrete color tables.
//
//   Brad Whitlock, Mon Mar 6 10:18:18 PDT 2006
//   I changed the code so it only uses the first colortable name as a last
//   resort if the default color table is set to something invalid.
//
//   Brad Whitlock, Fri Dec 14 16:59:58 PST 2007
//   Made it use ids.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:01:40 EST 2008
//   Avoid use of temporaries that was referencing freed memory.
//
//   Kathleen Biagas, Mon Aug  4 15:45:44 PDT 2014
//   Handle new groupingFlag, change in default/discrete button types.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added tags.
// 
//   Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//   The tag label and tag line edit are now always visible so they do not
//   need to have their visibility set in this function.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateWindow(bool doAll)
{
    bool updateNames = false;
    bool updateColorPoints = false;

    //
    // If our default color table, for some reason, does not appear in the
    // list of color tables then we should choose a new colortable. Note that
    // if we've not set the color table yet, it will be "none" and it will get
    // set here, if possible.
    //
    int nct = 3;
    bool invalidCt = true;
    QString ctNames[4];
    ctNames[0] = currentColorTable;
    ctNames[1] = colorAtts->GetDefaultContinuous().c_str();
    ctNames[2] = colorAtts->GetDefaultDiscrete().c_str();
    if(colorAtts->GetNames().size() > 0)
    {
        ctNames[3] = colorAtts->GetNames()[0].c_str();
        ++nct;
    }
    for(int c = 0; c < nct && invalidCt; ++c)
    {
        if(colorAtts->GetColorTableIndex(ctNames[c].toStdString()) != -1)
        {
            currentColorTable = ctNames[c];
            invalidCt = false;
        }
        else
            updateColorPoints = true;
    }
    if(invalidCt)
    {
        currentColorTable = "none";
        updateColorPoints = true;
    }

    //
    // Loop through all the attributes and do something for
    // each of them that changed. This function is only responsible
    // for displaying the state values and setting widget sensitivity.
    //
    for(int i = 0; i < colorAtts->NumAttributes(); ++i)
    {
        if(!doAll)
        {
            if(!colorAtts->IsSelected(i))
                continue;
        }

        switch(i)
        {
        case ColorTableAttributes::ID_names:
            updateNames = true;
            break;
        case ColorTableAttributes::ID_colorTables:
            updateColorPoints = true;
            break;
        case ColorTableAttributes::ID_defaultContinuous:
            defaultContinuous->blockSignals(true);
            defaultContinuous->setColorTable(colorAtts->GetDefaultContinuous().c_str());
            defaultContinuous->blockSignals(false);
            break;
        case ColorTableAttributes::ID_defaultDiscrete:
            defaultDiscrete->blockSignals(true);
            defaultDiscrete->setColorTable(colorAtts->GetDefaultDiscrete().c_str());
            defaultDiscrete->blockSignals(false);
            break;
        case ColorTableAttributes::ID_taggingFlag:
            tagFilterToggle->blockSignals(true);
            tagFilterToggle->setChecked(colorAtts->GetTaggingFlag());
            tagsVisible = colorAtts->GetTaggingFlag();
            tagTable->setVisible(tagsVisible);
            updateNameBoxPosition(tagsVisible);
            tagCombiningBehaviorChoice->setVisible(tagsVisible);
            tagFilterToggle->blockSignals(false);
            updateNames = true;
            break;
        }
    }

    //
    // Update the editor area.
    //
    if(updateColorPoints)
        UpdateEditor();

    //
    // Update the names.
    //
    if(updateNames)
        UpdateNames();

    // If there are no color tables, disable the color editor widgets.
    colorWidgetGroup->setEnabled(colorAtts->GetNumColorTables() > 0);
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateEditor
//
// Purpose:
//   Updates the editor area of the window.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 16:33:02 PST 2002
//
// Modifications:
//   Brad Whitlock, Wed Feb 26 11:04:52 PDT 2003
//   I changed the widgets for discrete color tables.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Jeremy Meredith, Wed Dec 31 16:11:50 EST 2008
//   Added show index hints checkbox for discrete color tables.
//
//   Mark C. Miller, Wed Feb 28 14:28:01 PST 2018
//   Handle "smoothing" label correctly.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
// ****************************************************************************

void
QvisColorTableWindow::UpdateEditor()
{
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    if(ccpl)
    {
        if(ccpl->GetDiscreteFlag())
        {
            UpdateDiscreteSettings();

            smoothLabel->hide();
            smoothingMethod->hide();
            equalCheckBox->hide();
            spectrumBar->hide();
            alignPointButton->hide();

            discreteColors->show();
            showIndexHintsCheckBox->show();

            // When discrete set the smoothing to none so the legend is correct.
            ccpl->SetSmoothing(ColorControlPointList::None);

            smoothingMethod->blockSignals(true);
            smoothingMethod->setCurrentIndex(0);
            smoothingMethod->blockSignals(false);
        }
        else
        {
            UpdateColorControlPoints();

            smoothLabel->show();
            smoothingMethod->show();
            equalCheckBox->show();
            spectrumBar->show();
            alignPointButton->show();

            discreteColors->hide();
            showIndexHintsCheckBox->hide();
        }

        colorTableTypeGroup->blockSignals(true);
        colorTableTypeGroup->button(ccpl->GetDiscreteFlag()?1:0)->setChecked(true);
        colorTableTypeGroup->blockSignals(false);

        colorNumColors->blockSignals(true);
        colorNumColors->setValue(ccpl->GetNumControlPoints());
        colorNumColors->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::AddToTagTable
//
// Purpose:
//   Adds entries to the tag table. Called by AddGlobalTag().
//
// Programmer: Justin Privitera
// Creation:   Mon Jun 27 17:30:16 PDT 2022
//
// Modifications:
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    Eliminated tag index arg as well as need for secret tag table column.
//
// ****************************************************************************

void
QvisColorTableWindow::AddToTagTable(std::string currtag)
{
    QTreeWidgetItem *item = new QTreeWidgetItem(tagTable);
    tagList[currtag].tagTableItem = item;
    item->setCheckState(0, tagList[currtag].active ? Qt::Checked : Qt::Unchecked);
    item->setText(1, currtag.c_str());
}

// ****************************************************************************
// Method: QvisColorTableWindow::AddGlobalTag
//
// Purpose:
//   Adds a tag discovered when looking through each color table to the global 
//   tag list. Also adds to the tag table if the tag is not in it.
//
// Programmer: Justin Privitera
// Creation:   Tue Jun  7 12:36:55 PDT 2022
//
// Modifications:
//    Justin Privitera, Mon Jun 27 17:33:23 PDT 2022
//    Added call to AddToTagTable() to reduce code bloat.
//    Renamed `run_before` to `first_time`.
// 
//    Justin Privitera, Fri Aug 19 20:57:38 PDT 2022
//    We now throw an error if there are too many tags.
// 
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    No limit on the number of tags.
//    Refactor allows for much cleaner interface for working with tag data.
//    No need to collect indices of tags anymore due to refactor.
//    Calculate refcount for each tag on the very first iteration through.
//
// ****************************************************************************

void
QvisColorTableWindow::AddGlobalTag(std::string currtag, bool first_time)
{
    // if the given tag is NOT in the global tag list
    if (tagList.find(currtag) == tagList.end())
    {
        // make the "Standard" tag active the very first time the tags are enabled
        tagList[currtag].active = currtag == "Standard" && first_time;
        AddToTagTable(currtag);
    }
    else
    {
        // We only want to run this check if the first case is not true.
        QList<QTreeWidgetItem*> items = tagTable->findItems(
            QString::fromStdString(currtag), Qt::MatchExactly, 1);
        // If the given tag IS in the global tag list but does not have a tagTable entry
        if (items.count() == 0)
            AddToTagTable(currtag);
    }
    // Only the very first time can we guarantee that each reference to each
    // tag has not been encountered before, so it is safe to increment here.
    if (first_time)
        // We have to do this logic AFTER the above logic because otherwise 
        // currtag will already be added to the tagList, which will mess up
        // our searching for it.
        tagList[currtag].numrefs ++;

}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateTags
//
// Purpose:
//   Updates the global tag list to reflect current available tags.
//
// Programmer: Justin Privitera
// Creation:   Tue Jun  7 12:36:55 PDT 2022
// 
// Notes:
//    Signal blocking and unblocking SHOULD occur in the caller.
//
// Modifications:
//    Justin Privitera, Wed Jun 29 17:50:24 PDT 2022
//    Renamed `run_before` to `first_time`.
//    Added guard to make sure code to fill tag table and tag list
//    is only run as much as it needs to be run.
// 
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    Run the tag table generation the first time so we can set up the tagInfo
//    map. Purge tagList and tagTable entries that have 0 refcount.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateTags()
{
    // We want the 'Standard' tag to be checked the very first time tag
    // filtering is enabled, hence the inclusion of `first_time`.
    static bool first_time = true;
    if (tagFilterToggle->isChecked() || first_time)
    {
        // populate tags list
        // iterate thru each color table
        for (int i = 0; i < colorAtts->GetNumColorTables(); i ++)
        {
            // only try to add tags if the ccpl thinks it has new info
            if (colorAtts->GetColorTables(i).GetTagChangesMade())
            {
                // if this table doesn't have tags, then add the no-tags tag
                if (colorAtts->GetColorTables(i).GetNumTags() == 0)
                    colorAtts->GetColorTables(i).AddTag("No Tags");

                // iterate thru each tag in the given color table
                for (int j = 0; j < colorAtts->GetColorTables(i).GetNumTags(); j ++)
                {
                    // add the tag if it is not already in the global tag list
                    AddGlobalTag(colorAtts->GetColorTables(i).GetTag(j), first_time);
                }
                // tell the ccpl that we have taken note of all of its tag changes
                colorAtts->GetColorTables(i).SetTagChangesMade(false);
            }
        }
        first_time = false;

        // Purge tagList/tagTable entries that have 0 refcount.
        for (auto itr{tagList.begin()}; itr != tagList.end();)
        {
            if (itr->second.numrefs <= 0)
            {
                if (QTreeWidgetItem *tagTableItem = itr->second.tagTableItem)
                {
                    auto index = tagTable->indexOfTopLevelItem(tagTableItem);
                    // For some reason, the item is not in the tag table. This 
                    // should not be possible, but if it does happen, we can 
                    // recover.
                    if (index != -1)
                    {
                        tagTable->takeTopLevelItem(index);
                        delete tagTableItem;
                    }
                }
                itr = tagList.erase(itr);
            }
            else
                itr ++;
        }
        tagTable->sortByColumn(1, Qt::AscendingOrder);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateNames
//
// Purpose:
//   This method takes the list of names in the color table attributes and
//   puts it into the color table name list box.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 19:58:19 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:41:22 PST 2002
//   I changed the code so it uses currentColorTable.
//
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//   Kathleen Biagas, Mon Aug  4 15:46:54 PDT 2014
//   Handle grouping if requested.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added tags (so lots of logic to control what 
//   happens when tags are enabled). Also added guard at the end to make 
//   sure the observer updates the color table buttons if settings are 
//   loaded from config files.
// 
//   Justin Privitera, Wed Jun 29 17:50:24 PDT 2022
//   Refactored the block that fills the namelistbox.
// 
//   Justin Privitera, Thu Jul 14 16:57:42 PDT 2022
//   Added logic for searching for color tables. Now there is a search filter
//   applied at the end of the function.
// 
//   Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//   The tag line edit only needs to be populated if searching is disabled.
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Rework for accessing tag information b/c of refactor.
//   Ensure current CT name is one of the existing names.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateNames()
{
    tagTable->blockSignals(true);
    nameListBox->blockSignals(true);
    defaultDiscrete->blockSignals(true);
    defaultContinuous->blockSignals(true);

    UpdateTags();

    // Clear out the existing names.
    nameListBox->clear();
    nameListBox->setRootIsDecorated(false);

    // if tagging is not enabled
    if(! tagFilterToggle->isChecked())
        colorAtts->SetAllActive(); // set all color tables to active
    else // tagging is enabled
    {
        for (int i = 0; i < colorAtts->GetNumColorTables(); i ++)
        {
            bool tagFound = false;
            // go thru global tags
            for (const auto& mapitem : tagList)
            {
                // if the global tag is active
                if (mapitem.second.active)
                {
                    tagFound = false;
                    // go thru local tags
                    for (int k = 0; k < colorAtts->GetColorTables(i).GetNumTags(); k ++)
                    {
                        // if the current global tag is the same as our local tag
                        if (mapitem.first == colorAtts->GetColorTables(i).GetTag(k))
                        {
                            tagFound = true;
                            break;
                        }
                    }
                    if (tagFound == tagsMatchAny)
                    // If both are true, that means...
                    // 1) tagsMatchAny is true so we only need one tag from 
                    //    the global tag list to be present in the local tag
                    //    list.
                    // 2) tagFound is true, so there is no need to keep 
                    //    searching for a tag that is in both the local and
                    //    global tag lists. Thus we can end iteration early.
                    // If both are false, that means...
                    // 1) tagsMatchAny is false so we need every tag from the
                    //    global tag list to be present in the local tag list.
                    // 2) tagFound is false, so there exists a global tag that
                    //    is not in the local tag list, hence we can give up 
                    //    early because we know that this color table does not
                    //    have every tag in the global tag list.
                        break;
                }
            }
            // we mark the color table as active or inactive
            colorAtts->SetActiveElement(i, tagFound);
        }
    }

    // actually populate the name list box
    for (int i = 0; i < colorAtts->GetNumColorTables(); i ++)
    {
        // if the color table is active
        if (colorAtts->GetActiveElement(i))
        {
            QString ctName(colorAtts->GetNames()[i].c_str());
            if (searchingOn)
            {
                if (ctName.contains(searchTerm, Qt::CaseInsensitive))
                {
                    QTreeWidgetItem *treeItem = new QTreeWidgetItem(nameListBox);
                    treeItem->setText(0, ctName);
                    nameListBox->addTopLevelItem(treeItem);  
                }
            }
            else
            {
                QTreeWidgetItem *treeItem = new QTreeWidgetItem(nameListBox);
                treeItem->setText(0, ctName);
                nameListBox->addTopLevelItem(treeItem);
            }
        }
    }

    // 
    // Select the default color table.
    // 

    // First, make sure that the nameListBox is not currently empty.
    if (nameListBox->topLevelItemCount() != 0)
    {
        // Then check that the currentColorTable is actually in the box.
        QList<QTreeWidgetItem*> items = nameListBox->findItems(
            currentColorTable, Qt::MatchExactly, 0);
        QTreeWidgetItem *item;
        // If the currentColorTable is NOT in the box, change it to one that is
        if (items.count() == 0)
        {
            item = nameListBox->topLevelItem(0);
            currentColorTable = item->text(0);
        }
        // If the currentColorTable IS in the box...
        else
            item = items[0];
        nameListBox->setCurrentItem(item);
        item->setSelected(true);

        // Set the text of the default color table into the name line edit.
        auto index = colorAtts->GetColorTableIndex(currentColorTable.toStdString());
        if (index >= 0 && !searchingOn)
        {
            nameLineEdit->setText(QString(colorAtts->GetNames()[index].c_str()));
            tagLineEdit->setText(QString(colorAtts->GetColorTables(index).GetTagsAsString().c_str()));
        }
    }

    tagTable->blockSignals(false);
    nameListBox->blockSignals(false);
    defaultContinuous->blockSignals(false);
    defaultDiscrete->blockSignals(false);

    // Set the enabled state of the delete button.
    deleteButton->setEnabled(colorAtts->GetNumColorTables() > 1);

    static bool run_before = false;
    if (!run_before)
    {
        // This only needs to happen the very first time for loading options.
        // If visit isn't opened with saved config and guiconfig files, then
        // this is redundant, but doesn't hurt. If it happens more than once
        // then VisIt will crash.
        run_before = true;
        colorAtts->SetChangesMade(true);
        ctObserver.SetUpdate(true);
        Apply(true);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetDefaultColorControlPoints
//
// Purpose:
//   Returns a const pointer to the color control points of our default
//   color table.
//
// Returns:    A const pointer to the color control points of our default
//             color table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:19:43 PST 2002
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//   In this case renamed the function.
//
// ****************************************************************************

const ColorControlPointList *
QvisColorTableWindow::GetDefaultColorControlPoints() const
{
    return colorAtts->GetColorControlPoints(currentColorTable.toStdString());
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetDefaultColorControlPoints
//
// Purpose:
//   Returns a pointer to the color control points of our default
//   color table.
//
// Returns:    A pointer to the color control points of our default
//             color table.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:19:43 PST 2002
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//   In this case I changed the name of the function.
//
// ****************************************************************************

ColorControlPointList *
QvisColorTableWindow::GetDefaultColorControlPoints()
{
    return (ColorControlPointList *)colorAtts->GetColorControlPoints(currentColorTable.toStdString());
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateColorControlPoints
//
// Purpose:
//   This method is called when the color control points must be updated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 23:35:25 PST 2001
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
//   Brad Whitlock, Fri Apr 27 15:09:27 PDT 2012
//   I added more smoothing types.
//
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
// ****************************************************************************

void
QvisColorTableWindow::UpdateColorControlPoints()
{
    // Get a pointer to the default color table's control points.
    const ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    if(ccpl)
    {
        const ColorControlPointList &cpts = *ccpl;
        int   i;

        spectrumBar->setSuppressUpdates(true);
        spectrumBar->blockSignals(true);

        if(spectrumBar->numControlPoints() == cpts.GetNumControlPoints())
        {
            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }
        }
        else if(spectrumBar->numControlPoints() < cpts.GetNumControlPoints())
        {
            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }

            // We need to add control points.
            for(i = spectrumBar->numControlPoints(); i < cpts.GetNumControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);
                spectrumBar->addControlPoint(ctmp, cpts[i].GetPosition());
            }
        }
        else
        {
            // We need to remove control points.
            for(i = spectrumBar->numControlPoints()-1; i >= cpts.GetNumControlPoints(); --i)
                spectrumBar->removeControlPoint();

            // Set the control points' colors and positions
            for(i = 0; i < spectrumBar->numControlPoints(); ++i)
            {
                QColor ctmp((int)cpts[i].GetColors()[0],
                            (int)cpts[i].GetColors()[1],
                            (int)cpts[i].GetColors()[2],
                            (int)cpts[i].GetColors()[3]);

                spectrumBar->setControlPointColor(i, ctmp);
                spectrumBar->setControlPointPosition(i, cpts[i].GetPosition());
            }
        }

        // Set the smoothing and equal spacing flags in the spectrumbar.
        spectrumBar->setEqualSpacing(cpts.GetEqualSpacingFlag());
        switch(cpts.GetSmoothing())
        {
        case ColorControlPointList::None:
            spectrumBar->setSmoothing(QvisSpectrumBar::None);
            break;
        default:
        case ColorControlPointList::Linear:
            spectrumBar->setSmoothing(QvisSpectrumBar::Linear);
            break;
        case ColorControlPointList::CubicSpline:
            spectrumBar->setSmoothing(QvisSpectrumBar::CubicSpline);
            break;
        }

        spectrumBar->blockSignals(false);
        spectrumBar->setSuppressUpdates(false);
        spectrumBar->update();

        // Update equalSpacing and smoothingMethod.
        smoothingMethod->blockSignals(true);
        smoothingMethod->setCurrentIndex((int)cpts.GetSmoothing());
        smoothingMethod->blockSignals(false);

        equalCheckBox->blockSignals(true);
        equalCheckBox->setChecked(cpts.GetEqualSpacingFlag());
        equalCheckBox->blockSignals(false);

        // Update the color sliders, etc.
        ShowSelectedColor(spectrumBar->controlPointColor(spectrumBar->activeControlPoint()));
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::UpdateDiscreteSettings
//
// Purpose:
//   Updates the widgets associated with discrete color tables.
//
// Programmer: Brad Whitlock
// Creation:   Wed Nov 20 17:14:54 PST 2002
//
// Modifications:
//   Brad Whitlock, Thu Feb 6 14:17:51 PST 2003
//   Added code to extend the color table.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//
// ****************************************************************************

void
QvisColorTableWindow::UpdateDiscreteSettings()
{
    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl && ccpl->GetDiscreteFlag())
    {
        bool doNotify = false;

        // Create an array of colors to set into the color grid widget.
        QColor *c = new QColor[ccpl->GetNumControlPoints()];
        int i;
        for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
        {
            const ColorControlPoint &ccp = ccpl->operator[](i);
            int r = int(ccp.GetColors()[0]);
            int g = int(ccp.GetColors()[1]);
            int b = int(ccp.GetColors()[2]);
            c[i] = QColor(r, g, b);
        }

        // Set the color palette into the color grid widget.
        int cols = (ccpl->GetNumControlPoints() > 30) ?
            (2 * DEFAULT_DISCRETE_COLS) : DEFAULT_DISCRETE_COLS;
        discreteColors->setPaletteColors(c, ccpl->GetNumControlPoints(), cols);
        delete [] c;

        // Now update the sliders and the spin buttons.
        QColor sc(discreteColors->selectedColor());
        for(i = 0; i < 4; ++i)
        {
            componentSliders[i]->blockSignals(true);
            componentSpinBoxes[i]->blockSignals(true);
        }
        componentSliders[0]->setValue(sc.red());
        componentSpinBoxes[0]->setValue(sc.red());
        componentSliders[1]->setValue(sc.green());
        componentSpinBoxes[1]->setValue(sc.green());
        componentSliders[2]->setValue(sc.blue());
        componentSpinBoxes[2]->setValue(sc.blue());
        componentSliders[3]->setValue(sc.alpha());
        componentSpinBoxes[3]->setValue(sc.alpha());
        for(i = 0; i < 4; ++i)
        {
            componentSliders[i]->blockSignals(false);
            componentSpinBoxes[i]->blockSignals(false);
        }

        // If we've modified the color table, tell the viewer.
        if(doNotify)
        {
            SetUpdate(false);
            colorAtts->Notify();
            GetViewerMethods()->UpdateColorTable(currentColorTable.toStdString());
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::PopupColorSelect
//
// Purpose:
//   This is method pops up the color selection widget.
//
// Arguments:
//   index : The index of the color control point that wants changed.
//   p     : The point where the menu should appear.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:06:35 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Sep 19 15:42:53 PST 2001
//   I removed an unused variable.
//
//   Brad Whitlock, Wed Mar 13 18:00:14 PST 2002
//   I added the point argument.
//
//   Brad Whitlock, Thu Nov 21 13:06:29 PST 2002
//   I made it more general.
//
// ****************************************************************************

void
QvisColorTableWindow::PopupColorSelect(const QColor &c, const QPoint &p)
{
    // Set the popup's initial color.
    colorSelect->blockSignals(true);
    colorSelect->setSelectedColor(c);
    colorSelect->blockSignals(false);

    // Figure out a good place to popup the menu.
    int menuW = colorSelect->sizeHint().width();
    int menuH = colorSelect->sizeHint().height();
    int menuX = p.x();
    int menuY = p.y() - (menuH >> 1);

    // Fix the X dimension.
    if(menuX < 0)
        menuX = 0;
    else if(menuX + menuW > QApplication::desktop()->width())
        menuX -= (menuW + 5);

    // Fix the Y dimension.
    if(menuY < 0)
        menuY = 0;
    else if(menuY + menuH > QApplication::desktop()->height())
        menuY -= ((menuY + menuH) - QApplication::desktop()->height());

    // Show the popup menu.
    colorSelect->move(menuX, menuY);
    colorSelect->show();
}

// ****************************************************************************
// Method: QvisColorTableWindow::ShowSelectedColor
//
// Purpose:
//   Makes the discrete color widgets display the specified color.
//
// Arguments:
//   c : The color to display.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:38:52 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//    Refactoring of color grid widget caused index to lose "color" in names.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Added new `skip_update` argument which will prevent the current ctrl pt
//    from having its color changed if enabled.
//
// ****************************************************************************

void
QvisColorTableWindow::ShowSelectedColor(const QColor &c, bool skip_update)
{
    int i;

    if (!skip_update)
    {
        const ColorControlPointList *ccpl = GetDefaultColorControlPoints();
        bool updateDiscrete = (ccpl && ccpl->GetDiscreteFlag());

        // Update the color
        if(updateDiscrete)
        {
            discreteColors->blockSignals(true);
            discreteColors->setPaletteColor(c, discreteColors->selectedIndex());
            discreteColors->blockSignals(false);
        }
        else
        {
            spectrumBar->blockSignals(true);
            spectrumBar->setControlPointColor(spectrumBar->activeControlPoint(), c);
            spectrumBar->blockSignals(false);
        }
    }

    // Disable signals in the sliders.
    for(i = 0; i < 4; ++i)
    {
        componentSliders[i]->blockSignals(true);
        componentSpinBoxes[i]->blockSignals(true);
    }

    // Update the sliders and the text fields.
    componentSliders[0]->setValue(c.red());
    componentSpinBoxes[0]->setValue(c.red());
    componentSliders[1]->setValue(c.green());
    componentSpinBoxes[1]->setValue(c.green());
    componentSliders[2]->setValue(c.blue());
    componentSpinBoxes[2]->setValue(c.blue());
    componentSliders[3]->setValue(c.alpha());
    componentSpinBoxes[3]->setValue(c.alpha());

    // Enable signals in the sliders.
    for(i = 0; i < 4; ++i)
    {
        componentSliders[i]->blockSignals(false);
        componentSpinBoxes[i]->blockSignals(false);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::ChangeSelectedColor
//
// Purpose:
//   Change the default discrete color to the specified color.
//
// Arguments:
//   c : the new color.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:38:22 PST 2002
//
// Modifications:
//    Jeremy Meredith, Fri Aug 11 17:14:32 EDT 2006
//    Refactoring of color grid widget caused index to lose "color" in names.
//
//    Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//    Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//
// ****************************************************************************

void
QvisColorTableWindow::ChangeSelectedColor(const QColor &c)
{
    // Change the color in the colorAtts.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        int index;

        if(ccpl->GetDiscreteFlag())
        {
            // Use the selected color in the discreteColors widget to determine
            // which color needs to be replaced in the color control point list.
            index = discreteColors->selectedIndex();
        }
        else
            index = spectrumBar->activeControlPoint();

        // If a valid index was calculated, poke the new color value into
        // the specified color control point and update.
        if(index >= 0 && index < ccpl->GetNumControlPoints())
        {
            ColorControlPoint &ccp = ccpl->GetControlPoints(index);
            ccp.GetColors()[0] = (unsigned char)c.red();
            ccp.GetColors()[1] = (unsigned char)c.green();
            ccp.GetColors()[2] = (unsigned char)c.blue();
            ccp.GetColors()[3] = (unsigned char)c.alpha();
            ccp.SelectColors();
            ccpl->SelectControlPoints();

            // If we're sliding, don't tell the viewer about it until we're done.
            if(!sliding)
            {
                // Prevent updates so we can redraw just the widgets that we want.
                SetUpdate(false);
                Apply();
            }
        }
    }

    // Redraw only the discrete color table widgets so it is faster.
    ShowSelectedColor(c);
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetNextColor
//
// Purpose:
//   Returns the next color in the sequence.
//
// Returns:    A color.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:11:49 PST 2003
//
// Modifications:
//
// ****************************************************************************

QColor
QvisColorTableWindow::GetNextColor()
{
    // Table of default control point colors.
    static int colors[] = {
        255, 0, 0,
        255, 255, 0,
        0, 255, 0,
        0, 255, 255,
        0, 0, 255,
        255, 0, 255,
        0, 0, 0,
        255, 255, 255};

    QColor retval(colors[colorCycle*3],
                  colors[colorCycle*3+1],
                  colors[colorCycle*3+2]);

    // Move on to the next color.
    colorCycle = (colorCycle < 7) ? (colorCycle + 1) : 0;

    return retval;
}

// ****************************************************************************
// Method: QvisColorTableWindow::GetCurrentValues
//
// Purpose:
//   Gets the current values from certain widgets and stores the values in the
//   state object.
//
// Arguments:
//   which_widget : A number indicating which widget that we should get the
//                  values from.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 18:42:52 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 16:46:46 PST 2002
//   I changed the code so it supports discrete color tables.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
//   Brad Whitlock, Fri Apr 27 15:12:21 PDT 2012
//   Added other smoothing types.
//
//   Kathleen Biagas, Fri Aug 8 08:43:49 PDT 2014
//   Handle category.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added logic to preserve the tags.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Logic to preserve builtin attribute.
//
// ****************************************************************************

void
QvisColorTableWindow::GetCurrentValues(int which_widget)
{
    // Get the rgb colors from the spectrumbar and put them into the state
    // object's rgbaColors array.
    if(which_widget == 0 || which_widget == -1)
    {
        // Store the widget's control points.
        ColorControlPointList cpts;
        cpts.ClearControlPoints();
        switch(spectrumBar->smoothing())
        {
        case QvisSpectrumBar::None:
            cpts.SetSmoothing(ColorControlPointList::None);
            break;
        default:
        case QvisSpectrumBar::Linear:
            cpts.SetSmoothing(ColorControlPointList::Linear);
            break;
        case QvisSpectrumBar::CubicSpline:
            cpts.SetSmoothing(ColorControlPointList::CubicSpline);
            break;
        }

        cpts.SetEqualSpacingFlag(spectrumBar->equalSpacing());
        for(int i = 0; i < spectrumBar->numControlPoints(); ++i)
        {
            QColor c(spectrumBar->controlPointColor(i));
            float  pos = spectrumBar->controlPointPosition(i);

            ColorControlPoint pt;
            unsigned char ptColors[4];
            ptColors[0] = (unsigned char)c.red();
            ptColors[1] = (unsigned char)c.green();
            ptColors[2] = (unsigned char)c.blue();
            ptColors[3] = (unsigned char)c.alpha();
            pt.SetColors(ptColors);
            pt.SetPosition(pos);
            cpts.AddControlPoints(pt);
        }

        // Get a pointer to the default color table's control points.
        ColorControlPointList *ccpl = GetDefaultColorControlPoints();
        // preserve the tags
        cpts.SetTagNames(ccpl->GetTagNames());
        // preserve the `built-in` attribute
        cpts.SetBuiltIn(ccpl->GetBuiltIn());
        if(ccpl)
        {
            ColorControlPointList &activeControlPoints = *ccpl;
            activeControlPoints = cpts;
        }
    }

    // Get the name of the default color table. If it differs from the default
    // color table in the state object, set it into the state object.
    if(which_widget == 1 || which_widget == -1)
    {
        QString temp = nameLineEdit->displayText().simplified();
        bool okay = !temp.isEmpty();
        if(okay)
        {
            currentColorTable = temp;
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::Apply
//
// Purpose:
//   This is a Qt slot function that is called when the Apply button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:00:18 PST 2001
//
// Modifications:
//    Kathleen Biagas, Fri Aug 8 08:44:12 PDT 2014
//    Handle category.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories.
//
// ****************************************************************************

void
QvisColorTableWindow::Apply(bool ignore)
{
    if(AutoUpdate() || ignore)
    {
        // Send the color table definitions to the viewer.
        GetCurrentValues(1);
        colorAtts->Notify();

        // Make the viewer update the plots that use the specified colortable.
        GetViewerMethods()->UpdateColorTable(currentColorTable.toStdString());
    }
    else
        colorAtts->Notify();
}

//
// Qt Slot functions...
//

// ****************************************************************************
// Method: QvisColorTableWindow::apply
//
// Purpose:
//   This is a Qt slot function that is called when the window's Apply
//   button is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 13:42:22 PST 2001
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::apply()
{
    Apply(true);
}

// ****************************************************************************
// Method: QvisColorTableWindow::alignControlPoints
//
// Purpose:
//   This is a Qt slot function that tells the spectrum bar to align its
//   control points.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 10:04:17 PDT 2001
//
// Modifications:
//   Brad Whitlock, Mon Jul 14 15:04:07 PST 2003
//   Added code to block signals.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table.
//
// ****************************************************************************

void
QvisColorTableWindow::alignControlPoints()
{
    // built-in CTs should not be editable
    if (colorAtts->GetColorControlPoints(currentColorTable.toStdString())->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot edit a built-in color table.");
        Error(tmp);
        return;
    }

    // Align the control points.
    spectrumBar->blockSignals(true);
    spectrumBar->alignControlPoints();
    spectrumBar->blockSignals(false);

    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::controlPointMoved
//
// Purpose:
//   This is a Qt slot function that is called when the spectrum bar's control
//   points are moved.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 11:39:32 PDT 2001
//
// Modifications:
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::controlPointMoved(int index, float position)
{
    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    // built-in CTs should not be editable
    if (ccpl->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot edit a built-in color table.");
        Error(tmp);
        spectrumBar->blockSignals(true);
        // This is overkill, but it gets the job done.
        const int num_ctrl_pts = ccpl->GetNumControlPoints();
        for (int i = 0; i < num_ctrl_pts; i ++)
        {
            float pos = ccpl->GetControlPoints(i).GetPosition();
            unsigned char *colors = ccpl->GetControlPoints(i).GetColors();
            QColor c;
            c.setRgb(colors[0], colors[1], colors[2], colors[3]);
            spectrumBar->setControlPointPosition(i, pos);
            spectrumBar->setControlPointColor(i, c);
        }
        QColor selectedColor = spectrumBar->controlPointColor(index);
        ShowSelectedColor(selectedColor, true);
        spectrumBar->blockSignals(false);        
        return;
    }
    
    // Get the current attributes.
    GetCurrentValues(0);
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::chooseContinuousColor
//
// Purpose:
//   This is a Qt slot function that is called when a color control point
//   in the spectrum bar is clicked such that the color selection widget
//   needs to be activated in order to select a new color.
//
// Arguments:
//   index : The index of the default color control point.
//   p     : The point that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:06:47 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::chooseContinuousColor(int index, const QPoint &p)
{
    popupMode = SELECT_FOR_CONTINUOUS;
    PopupColorSelect(spectrumBar->controlPointColor(index), p);
}

// ****************************************************************************
// Method: QvisColorTableWindow::chooseDiscreteColor
//
// Purpose:
//   This is a Qt slot function that is called when a discrete color is clicked
//   such that the color selection widget needs to be activated in order to
//   select a new color.
//
// Arguments:
//   c : The discrete color.
//   p : The point that was clicked.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 13:06:47 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::chooseDiscreteColor(const QColor &c, int, int,
    const QPoint &p)
{
    popupMode = SELECT_FOR_DISCRETE;
    PopupColorSelect(c, p);
}

// ****************************************************************************
// Method: QvisColorTableWindow::selectedColor
//
// Purpose:
//   This is a Qt slot function that is called by the color popup menu when
//   a new color has been selected.
//
// Arguments:
//   color : The new color that was selected.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 14:07:31 PST 2001
//
// Modifications:
//   Brad Whitlock, Thu Nov 21 12:54:01 PDT 2002
//   I modified the routine so it behaves differently if the menu was
//   activated by choosing a discrete color table.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::selectedColor(const QColor &color)
{
    // Hide the popup menu.
    colorSelect->hide();

    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    // built-in CTs should not be editable
    if (ccpl->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot edit a built-in color table.");
        Error(tmp);
        smoothingMethod->blockSignals(true);
        smoothingMethod->setCurrentIndex(ccpl->GetSmoothing());
        smoothingMethod->blockSignals(false);
        return;
    }

    if(color.isValid())
    {
        if(popupMode == SELECT_FOR_CONTINUOUS)
        {
            // Set the color of the default color control point.
            spectrumBar->setControlPointColor(spectrumBar->activeControlPoint(),
                                              color);

            // Get the current values for the spectrum bar.
            GetCurrentValues(0);
            Apply();
            ShowSelectedColor(color);
        }
        else
        {
            // Change the default discrete color to the new color.
            ChangeSelectedColor(color);
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::smoothingMethodChanged
//
// Purpose:
//   This is a Qt slot function that is called when the window's smooth
//   combobox is activated.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 15:38:06 PST 2001
//
// Modifications:
//   Brad Whitlock, Fri Apr 27 15:14:20 PDT 2012
//   Added more types of smoothing.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::smoothingMethodChanged(int val)
{
    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    // built-in CTs should not be editable
    if (ccpl->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot edit a built-in color table.");
        Error(tmp);
        smoothingMethod->blockSignals(true);
        smoothingMethod->setCurrentIndex(ccpl->GetSmoothing());
        smoothingMethod->blockSignals(false);
        return;
    }

    if(ccpl)
    {
        ccpl->SetSmoothing(ColorControlPointList::SmoothingMethod(val));
        colorAtts->SelectColorTables();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::showIndexHintsToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's
//   show index hints toggle is clicked.
//
// Programmer: Jeremy Meredith
// Creation:   December 31, 2008
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::showIndexHintsToggled(bool val)
{
    discreteColors->setShowIndexHints(val);
}

// ****************************************************************************
// Method: QvisColorTableWindow::equalSpacingToggled
//
// Purpose:
//   This is a Qt slot function that is called when the window's equal spacing
//   toggle is clicked.
//
// Programmer: Brad Whitlock
// Creation:   Mon Jun 11 15:38:06 PST 2001
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
// 
// ****************************************************************************

void
QvisColorTableWindow::equalSpacingToggled(bool)
{
    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    // built-in CTs should not be editable
    if (ccpl->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot edit a built-in color table.");
        Error(tmp);
        equalCheckBox->blockSignals(true);
        equalCheckBox->setChecked(ccpl->GetEqualSpacingFlag());
        equalCheckBox->blockSignals(false);
        return;
    }

    if(ccpl)
    {
        ccpl->SetEqualSpacingFlag(!ccpl->GetEqualSpacingFlag());
        colorAtts->SelectColorTables();
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::addColorTable
//
// Purpose:
//   This is a Qt slot function that adds a new color table that, if possible,
//   is based on the current colortable.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:02:15 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Nov 20 17:11:36 PST 2002
//   I changed the code so discrete color tables are supported. I also
//   removed the code to make the new color table the default color table.
//
//   Brad Whitlock, Tue Apr  8 09:27:26 PDT 2008
//   Support for internationalization.
//
//   Brad Whitlock, Fri Apr 27 15:15:12 PDT 2012
//   Add other smoothing types.
//
//    Kathleen Biagas, Fri Aug 8 08:44:12 PDT 2014
//    Handle category.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added default tag for user defined tables.
// 
//   Justin Privitera, Wed Jun 29 17:50:24 PDT 2022
//   SetTagChangesMade for copied color tables.
// 
//   Justin Privitera, Wed Jul 20 14:18:20 PDT 2022
//   Added error if users try to add a color table while searching is enabled.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Set builtin flag to false for new color tables.
//
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Update tag refcount on creation of a new CT.
//
// ****************************************************************************

void
QvisColorTableWindow::addColorTable()
{
    if (searchingOn)
    {
        QString tmp;
        tmp = tr("Cannot add a color table while searching is enabled. "
                 "Please disable searching first.");
        Error(tmp);
        return;
    }

    // Get a pointer to the default color table's control points.
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();

    // Get the name of the new colortable. This stores a new name into
    // the currentColorTable variable.
    GetCurrentValues(1);
    if(colorAtts->GetColorTableIndex(currentColorTable.toStdString()) < 0)
    {
        // Add the new colortable to colorAtts.
        if(ccpl)
        {
            // Copy the default color table into the new color table.
            ColorControlPointList cpts(*ccpl);
            cpts.AddTag("User Defined");
            cpts.SetTagChangesMade(true); // need to set manually b/c orig val was copied
            cpts.SetBuiltIn(false);
            colorAtts->AddColorTable(currentColorTable.toStdString(), cpts);
            for (auto tag : cpts.GetTagNames())
                tagList[tag].numrefs ++;
        }
        else
        {
            // There is no default color table so create a default color table
            // and add it with the specified name.
            ColorControlPointList cpts;
            cpts.AddControlPoints(ColorControlPoint(0., 255,0,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.25, 255,255,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.5, 0,255,0,255));
            cpts.AddControlPoints(ColorControlPoint(0.75, 0,255,255,255));
            cpts.AddControlPoints(ColorControlPoint(1., 0,0,255,255));
            cpts.SetSmoothing(ColorControlPointList::Linear);
            cpts.SetEqualSpacingFlag(false);
            cpts.SetDiscreteFlag(false);
            cpts.AddTag("User Defined");
            cpts.SetBuiltIn(false);
            colorAtts->AddColorTable(currentColorTable.toStdString(), cpts);
            for (auto tag : cpts.GetTagNames())
                tagList[tag].numrefs ++;
        }

        // Tell all of the observers to update.
        Apply();
    }
    else
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is already in the color table list. You must provide a "
                 "unique name for the new color table before it can be added.");
        Error(tmp);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::deleteColorTable
//
// Purpose:
//   This is a Qt slot function that is called to delete a color table.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:03:26 PST 2001
//
// Modifications:
//   Cyrus Harrison, Tue Jun 10 10:04:26 PDT 20
//   Initial Qt4 Port.
//
//    Kathleen Biagas, Fri Aug 8 08:44:12 PDT 2014
//    nameListBox object is now a QTreeWidget.
//
//    Justin Privitera, Wed Jul 20 14:18:20 PDT 2022
//    Error when deleting a CT while searching is enabled.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table.
// 
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    Error when attempting to delete a CT when there are no CTs.
//    Error when attempting to delete a CT when one is not selected.
//    Error when attempting to delete the last continuous or discrete CT.
//    Update tag refcount before deleting CT.
// 
// ****************************************************************************

void
QvisColorTableWindow::deleteColorTable()
{
    if (searchingOn)
    {
        QString tmp;
        tmp = tr("Cannot delete a color table while searching is enabled. "
                 "Please disable searching first.");
        Error(tmp);
        return;
    }

    if (nameListBox->topLevelItemCount() == 0)
    {
        QString tmp;
        tmp = tr("Not able to delete a color table; there are no color tables"
                 " to delete.");
        Error(tmp);
        return;
    }

    // Get the index of the currently selected color table and tell the viewer
    // to remove it from the list of color tables.
    if (QTreeWidgetItem *item = nameListBox->currentItem())
    {
        std::string ctName = item->text(0).toStdString();
        auto ccpl{colorAtts->GetColorControlPoints(ctName)};
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot delete a built-in color table.");
            Error(tmp);
            return;
        }
        if (tagList["Continuous"].numrefs == 1 && ccpl->HasTag("Continuous"))
        {
            QString tmp;
            tmp = tr("This is the last Continuous Color Table. There must be"
                     " at least one Continuous Color Table, so this Color"
                     " Table will not be deleted.");
            Error(tmp);
            return;
        }
        if (tagList["Discrete"].numrefs == 1 && ccpl->HasTag("Discrete"))
        {
            QString tmp;
            tmp = tr("This is the last Discrete Color Table. There must be"
                     " at least one Discrete Color Table, so this Color"
                     " Table will not be deleted.");
            Error(tmp);
            return;
        }
        for (auto tag : ccpl->GetTagNames())
            tagList[tag].numrefs --;
        GetViewerMethods()->DeleteColorTable(ctName.c_str());
    }
    else
    {
        QString tmp = tr("No color table selected; cannot delete.");
        Error(tmp);
        return;
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::highlightColorTable
//
// Purpose:
//   This is a Qt slot function that is called when a color table is
//   highligted. It serves to make the highlighted plot the new default color
//   table.
//
// Arguments:
//   index : The index of the color table that was highlighted.
//
// Programmer: Brad Whitlock
// Creation:   Sat Jun 16 20:03:56 PST 2001
//
// Modifications:
//   Brad Whitlock, Mon Mar 4 15:32:49 PST 2002
//   Added a little code to set the name line edit's contents. We normally
//   do not set widget state outside of the UpdateWindow method but this
//   is a special case. This makes auto update work.
//
//   Brad Whitlock, Thu Nov 21 14:25:25 PST 2002
//   I rewrote the method.
//
//    Kathleen Biagas, Fri Aug 8 08:44:12 PDT 2014
//    Rewritten to reflect changes in how the names are stored.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added tags.
// 
//   Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//   The tag line edit is always visible now so it must be updated even if
//   tagging is disabled.
//
// ****************************************************************************

void
QvisColorTableWindow::highlightColorTable(QTreeWidgetItem *current,
    QTreeWidgetItem *)
{
    // only react to selection of leaves in the tree
    if (current->childCount() == 0)
    {
        currentColorTable = current->text(0);
        nameLineEdit->setText(currentColorTable);
        int index = colorAtts->GetColorTableIndex(currentColorTable.toStdString());
        tagLineEdit->setText(QString(colorAtts->GetColorTables(index).GetTagsAsString().c_str()));
        UpdateEditor();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::tagTableItemSelected
//
// Purpose:
//   A Qt slot function to handle selection of tags in the tag table.
//
// Arguments:
//   QTreeWidgetItem *item      the selected item
//   int column                 the column of the selected item
//
// Programmer: Justin Privitera
// Creation:   Mon Jun  6 14:02:16 PDT 2022
//
// Modifications:
//    Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//    The secret tag table column is gone; there is no need to read the index
//    from it anymore. We can use the map instead.
//
// ****************************************************************************

void
QvisColorTableWindow::tagTableItemSelected(QTreeWidgetItem *item, int column)
{
    tagList[item->text(1).toStdString()].active = item->checkState(0) == Qt::Checked;
    UpdateNames();
    colorAtts->SetChangesMade(true);
    ctObserver.SetUpdate(true);
    Apply(true);
}

// ****************************************************************************
// Method: QvisColorTableWindow::setColorTableType
//
// Purpose:
//   This is a Qt slot function that sets the type of the color table.
//
// Arguments:
//   index : The new color table type. 0=continuous, 1=discrete.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 11:10:52 PDT 2003
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::setColorTableType(int index)
{
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            colorTableTypeGroup->blockSignals(true);
            colorTableTypeGroup->button(ccpl->GetDiscreteFlag()?1:0)->setChecked(true);
            colorTableTypeGroup->blockSignals(false);
            return;
        }
        ccpl->SetDiscreteFlag(index == 1);
        colorAtts->SelectColorTables();
        // When discrete set the smoothing to none so the legend is correct.
        if(index == 1)
          ccpl->SetSmoothing(ColorControlPointList::None);
        Apply();
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::activateContinuousColor
//
// Purpose:
//   This is a Qt slot function that is called when we click on a new
//   color control point in the spectrum bar. This function updates the
//   color sliders to the right values when we select a new control point.
//
// Arguments:
//   index : The index of the new color control point.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 12:49:41 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::activateContinuousColor(int index)
{
    ShowSelectedColor(spectrumBar->controlPointColor(index));
}

// ****************************************************************************
// Method: QvisColorTableWindow::activateDiscreteColor
//
// Purpose:
//   This is a Qt slot function that is called when we click on a discrete
//   color thus making it default.
//
// Arguments:
//   c      : The color value.
//   row    : The color's row in the color grid.
//   column : The color's column in the color grid.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:43:38 PST 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::activateDiscreteColor(const QColor &c, int)
{
    ShowSelectedColor(c);
}

// ****************************************************************************
// Method: QvisColorTableWindow::redValueChanged
//
// Purpose:
//   This is a Qt slot function that is called when the default discrete color
//   changes due to the red slider or spin box.
//
// Arguments:
//   r : The new red value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::redValueChanged(int r)
{
    const ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            componentSpinBoxes[0]->blockSignals(true);
            componentSliders[0]->blockSignals(true);
            componentSpinBoxes[0]->setValue(c.red());
            componentSliders[0]->setValue(c.red());
            componentSpinBoxes[0]->blockSignals(false);
            componentSliders[0]->blockSignals(false);
            return;
        }

        c.setRgb(r, c.green(), c.blue(), c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::greenValueChanged
//
// Purpose:
//   This is a Qt slot function that is called when the default discrete color
//   changes due to the green slider or spin box.
//
// Arguments:
//   g : The new green value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::greenValueChanged(int g)
{
    const ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            componentSpinBoxes[1]->blockSignals(true);
            componentSliders[1]->blockSignals(true);
            componentSpinBoxes[1]->setValue(c.green());
            componentSliders[1]->setValue(c.green());
            componentSpinBoxes[1]->blockSignals(false);
            componentSliders[1]->blockSignals(false);
            return;
        }

        c.setRgb(c.red(), g, c.blue(), c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::blueValueChanged
//
// Purpose:
//   This is a Qt slot function that is called when the default discrete color
//   changes due to the blue slider or spin box.
//
// Arguments:
//   b : The new blue value.
//
// Programmer: Brad Whitlock
// Creation:   Thu Nov 21 14:44:44 PST 2002
//
// Modifications:
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::blueValueChanged(int b)
{
    const ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            componentSpinBoxes[2]->blockSignals(true);
            componentSliders[2]->blockSignals(true);
            componentSpinBoxes[2]->setValue(c.blue());
            componentSliders[2]->setValue(c.blue());
            componentSpinBoxes[2]->blockSignals(false);
            componentSliders[2]->blockSignals(false);
            return;
        }

        c.setRgb(c.red(), c.green(), b, c.alpha());
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::alphaValueChanged
//
// Purpose:
//   This is a Qt slot function that is called when the default discrete color
//   changes due to the alpha slider or spin box.
//
// Arguments:
//   a : The new alpha value.
//
// Programmer: Jeremy Meredith
// Creation:   February 20, 2009
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on edit of a builtin color table and reset original values.
//
// ****************************************************************************

void
QvisColorTableWindow::alphaValueChanged(int a)
{
    const ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        QColor c;

        if(ccpl->GetDiscreteFlag())
            c = discreteColors->selectedColor();
        else
            c = spectrumBar->controlPointColor(spectrumBar->activeControlPoint());

        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            componentSpinBoxes[3]->blockSignals(true);
            componentSliders[3]->blockSignals(true);
            componentSpinBoxes[3]->setValue(c.alpha());
            componentSliders[3]->setValue(c.alpha());
            componentSpinBoxes[3]->blockSignals(false);
            componentSliders[3]->blockSignals(false);
            return;
        }

        c.setRgb(c.red(), c.green(), c.blue(), a);
        ChangeSelectedColor(c);
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::sliderPressed
//
// Purpose:
//   This is a Qt slot function that tells the window that the slider is
//   being dragged so we can prevent the window from sending Updates to the
//   viewer for each grad event.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:53:43 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::sliderPressed()
{
    sliding = true;
}

// ****************************************************************************
// Method: QvisColorTableWindow::sliderReleased
//
// Purpose:
//   Tells the window that the slider was released and that changes were made
//   so we should update.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:18 PDT 2002
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::sliderReleased()
{
    sliding = false;
    SetUpdate(false);
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::setDefaultContinuous
//
// Purpose:
//   This is Qt slot function that sets the default continuous color table.
//
// Arguments:
//   ct : The name of the new default color table.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:53 PDT 2002
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//   In this case I changed the name of the function.
//
// ****************************************************************************

void
QvisColorTableWindow::setDefaultContinuous(const QString &ct)
{
    colorAtts->SetDefaultContinuous(ct.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::setDefaultDiscrete
//
// Purpose:
//   This is Qt slot function that sets the default discrete color table.
//
// Arguments:
//   ct : The name of the new default color table.
//
// Programmer: Brad Whitlock
// Creation:   Mon Nov 25 10:55:53 PDT 2002
//
// Modifications:
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
//   In this case I changed the name of the function.
//
// ****************************************************************************

void
QvisColorTableWindow::setDefaultDiscrete(const QString &ct)
{
    colorAtts->SetDefaultDiscrete(ct.toStdString());
    Apply();
}

// ****************************************************************************
// Method: QvisColorTableWindow::resizeColorTable
//
// Purpose:
//   This routine resizes the color table.
//
// Arguments:
//   size : The new color table size.
//
// Programmer: Brad Whitlock
// Creation:   Wed Feb 26 15:12:20 PST 2003
//
// Modifications:
//   Brad Whitlock, Mon Jul 14 14:35:27 PST 2003
//   I added code to prevent the spectrum bar from emitting signals while
//   it is having color control points added or removed.
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on resize of a builtin color table.
//
// ****************************************************************************

void
QvisColorTableWindow::resizeColorTable(int size)
{
    ColorControlPointList *ccpl = GetDefaultColorControlPoints();
    if(ccpl)
    {
        // built-in CTs should not be editable
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + currentColorTable + QString("\"") +
                  tr(" is built-in. You cannot edit a built-in color table.");
            Error(tmp);
            colorNumColors->blockSignals(true);
            colorNumColors->setValue(ccpl->GetNumControlPoints());
            colorNumColors->blockSignals(false);
            return;
        }

        int i;

        if(ccpl->GetDiscreteFlag())
        {
            if(size < ccpl->GetNumControlPoints())
            {
                // Remove colors from the end of the list.
                int rmPoints = ccpl->GetNumControlPoints() - size;
                for(i = 0; i < rmPoints; ++i)
                    ccpl->RemoveControlPoints(ccpl->GetNumControlPoints()-1);

                // Reposition the points evenly.
                for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
                {
                    ColorControlPoint &c = ccpl->GetControlPoints(i);
                    c.SetPosition(float(i) / float(ccpl->GetNumControlPoints() - 1));
                }

                colorAtts->SelectColorTables();
                Apply();
            }
            else if(size > ccpl->GetNumControlPoints())
            {
                // Add colors to the end of the list.
                int addPoints = size - ccpl->GetNumControlPoints();
                for(i = 0; i < addPoints; ++i)
                {
                    QColor c(GetNextColor());
                    ColorControlPoint newPt(1., c.red(), c.green(), c.blue(), c.alpha());
                    ccpl->AddControlPoints(newPt);
                }

                // Reposition the points evenly.
                for(i = 0; i < ccpl->GetNumControlPoints(); ++i)
                {
                    ColorControlPoint &c = ccpl->GetControlPoints(i);
                    c.SetPosition(float(i) / float(ccpl->GetNumControlPoints() - 1));
                }

                colorAtts->SelectColorTables();
                Apply();
            }
        }
        else
        {
            //
            // If the color table is continuous, add points through the spectrum
            // bar so that they are added and removed in the right places. I know
            // this is a little weird but we need to do it like this.
            //
            if(size < ccpl->GetNumControlPoints())
            {
                int rmPoints = ccpl->GetNumControlPoints() - size;
                spectrumBar->blockSignals(true);
                spectrumBar->setSuppressUpdates(true);
                for(int i = 0; i < rmPoints; ++i)
                    spectrumBar->removeControlPoint();
                spectrumBar->blockSignals(false);
                spectrumBar->setSuppressUpdates(false);
                spectrumBar->update();

                GetCurrentValues(0);
                SetUpdate(false);
                Apply();
            }
            else if(size > spectrumBar->numControlPoints())
            {
                int addPoints = size - spectrumBar->numControlPoints();
                spectrumBar->blockSignals(true);
                spectrumBar->setSuppressUpdates(true);
                for(int i = 0; i < addPoints; ++i)
                    spectrumBar->addControlPoint(GetNextColor());
                spectrumBar->blockSignals(false);
                spectrumBar->setSuppressUpdates(false);
                spectrumBar->update();

                GetCurrentValues(0);
                SetUpdate(false);
                Apply();
            }
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::exportColorTable
//
// Purpose:
//   This is a Qt slot function that tells the viewer to export the currently
//   selected color table.
//
// Programmer: Brad Whitlock
// Creation:   Tue Jul 1 16:40:39 PST 2003
//
// Modifications:
//    Justin Privitera, Wed Jul 20 14:18:20 PDT 2022
//    Error when trying to export a CT while searching is enabled.
// 
//    Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//    Error on export of a builtin color table.
//
// ****************************************************************************

void
QvisColorTableWindow::exportColorTable()
{
    if (searchingOn)
    {
        QString tmp;
        tmp = tr("Cannot export a color table while searching is enabled. "
                 "Please disable searching first.");
        Error(tmp);
        return;
    }
    else if (colorAtts->GetColorControlPoints(
        currentColorTable.toStdString())->GetBuiltIn())
    {
        QString tmp;
        tmp = tr("The color table ") +
              QString("\"") + currentColorTable + QString("\"") +
              tr(" is built-in. You cannot export a built-in color table.");
        Error(tmp);
        return;
    }
    else
        GetViewerMethods()->ExportColorTable(currentColorTable.toStdString());
}


// ****************************************************************************
// Method: QvisColorTableWindow::taggingToggled
//
// Purpose:
//   This is a Qt slot function that controls toggling the tags.
//
// Programmer: Justin Privitera
// Creation:   Fri Jun  3 15:06:17 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::taggingToggled(bool val)
{
    colorAtts->SetTaggingFlag(val);
    colorAtts->SetChangesMade(true);
    Apply(true);
}


// ****************************************************************************
// Method: QvisColorTableWindow::tagCombiningChanged
//
// Purpose:
//   This is a Qt slot function that controls changing how tags are combined.
//
// Programmer: Justin Privitera
// Creation:   Fri Jun  3 15:06:17 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::tagCombiningChanged(int index)
{
    bool old_val = tagsMatchAny;
    if (index == 0)
        tagsMatchAny = true;
    else
        tagsMatchAny = false;
    // have any changes actually been made?
    if (old_val != tagsMatchAny)
    {
        UpdateNames();
        colorAtts->SetChangesMade(true);
        ctObserver.SetUpdate(true);
        Apply(true);
    }
}


// ****************************************************************************
// Method: QvisColorTableWindow::searchingToggled
//
// Purpose:
//   This is a Qt slot function that enables or disables searching.
//
// Programmer: Justin Privitera
// Creation:   Thu Jul  7 10:22:58 PDT 2022
//
// Modifications:
//    Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//    The tag line edit is cleared when searching is enabled.
// 
//    Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//    The tag editor line edit is cleared when searching is enabled.
//
// ****************************************************************************

void
QvisColorTableWindow::searchingToggled(bool checked)
{
    searchingOn = checked;
    if (!searchingOn)
        searchTerm = QString("");
    else
    {
        nameLineEdit->setText(searchTerm);
        tagLineEdit->setText(QString(""));
        tagEditorLineEdit->setText(QString(""));
    }
    Apply(true);
}


// ****************************************************************************
// Method: QvisColorTableWindow::searchEdited
//
// Purpose:
//   This is a Qt slot function that updates the search term.
//
// Programmer: Justin Privitera
// Creation:   Thu Jul  7 10:22:58 PDT 2022
//
// Modifications:
//   Justin Privitera, Wed Jul 20 14:18:20 PDT 2022
//   Added guard to prevent Apply() from being called when searching is off.
// 
//   Justin Privitera, Wed Aug  3 19:46:13 PDT 2022
//   The tag line edit is cleared when searching is ongoing.
//
// ****************************************************************************

void
QvisColorTableWindow::searchEdited(const QString &newSearchTerm)
{
    if (searchingOn)
    {
        searchTerm = newSearchTerm;
        tagLineEdit->setText(QString(""));
        Apply(true);
    }
}


// ****************************************************************************
// Method: QvisColorTableWindow::tagEdited
//
// Purpose:
//   Qt slot function that sets the tagEdit once the tagEditorLineEdit is done
//   being edited.
//
// Programmer: Justin Privitera
// Creation:   Wed Aug 10 15:35:58 PDT 2022
//
// Modifications:
//
// ****************************************************************************
void
QvisColorTableWindow::tagEdited()
{
    tagEdit = tagEditorLineEdit->text();
}


// ****************************************************************************
// Method: QvisColorTableWindow::addTagToColorTable
//
// Purpose:
//    Helper function for adding tags to color tables... carefully!
//
// Programmer: Justin Privitera
// Creation:   Thu Aug 25 15:04:55 PDT 2022
//
// Modifications:
//
// ****************************************************************************
void
QvisColorTableWindow::addTagToColorTable(const std::string ctName, 
                                         const std::string tagName,
                                         ColorControlPointList* ccpl)
{
    auto tagChangeReverseAction{std::make_pair(REMOVETAG, ctName)};
    if (tagChanges[tagName].find(tagChangeReverseAction) != 
        tagChanges[tagName].end())
        tagChanges[tagName].erase(tagChangeReverseAction);
    else
        tagChanges[tagName].insert(std::make_pair(ADDTAG, ctName));
    ccpl->AddTag(tagName);
    tagList[tagName].numrefs ++;
}


// ****************************************************************************
// Method: QvisColorTableWindow::removeTagFromColorTable
//
// Purpose:
//    Helper function for removing tags from color tables... carefully!
//
// Programmer: Justin Privitera
// Creation:   Thu Aug 25 15:04:55 PDT 2022
//
// Modifications:
//
// ****************************************************************************
void
QvisColorTableWindow::removeTagFromColorTable(const std::string ctName, 
                                              const std::string tagName,
                                              ColorControlPointList* ccpl)
{
    auto tagChangeReverseAction{std::make_pair(ADDTAG, ctName)};
    if (tagChanges[tagName].find(tagChangeReverseAction) != 
        tagChanges[tagName].end())
    {
        tagChanges[tagName].erase(tagChangeReverseAction);
        ccpl->RemoveTag(tagName);
        tagList[tagName].numrefs --;
    }
    else
    {
        // if this is a built in color table, you can only remove a 
        // tag if the add tag action is already there (the previous case)
        if (ccpl->GetBuiltIn())
        {
            QString tmp;
            tmp = tr("The color table ") +
                  QString("\"") + QString(ctName.c_str()) + QString("\"") +
                  tr(" is built-in. You cannot delete a default tag "
                     "from a built-in color table.");
            Error(tmp);
            return;
        }
        else
        {
            tagChanges[tagName].insert(std::make_pair(REMOVETAG, ctName));
            ccpl->RemoveTag(tagName);
            tagList[tagName].numrefs --;
        }
    }
}

// ****************************************************************************
// Method: QvisColorTableWindow::addRemoveTag
//
// Purpose:
//    Qt slot function that is called when a user attempts to either add
//    or remove a tag from a color table.
//
// Programmer: Justin Privitera
// Creation:   Wed Aug 10 15:35:58 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void
QvisColorTableWindow::addRemoveTag()
{
    auto tagName{tagEdit.toStdString()};
    auto *ccpl{GetDefaultColorControlPoints()};
    if (ccpl)
    {
        auto result{ccpl->ValidateTag(tagName)};
        if (result.first)
        {
            auto index{colorAtts->GetColorTableIndex(currentColorTable.toStdString())};
            auto ctName(static_cast<std::string>(colorAtts->GetNames()[index]));
            if (ccpl->HasTag(tagName))
                removeTagFromColorTable(ctName, tagName, ccpl);
            else
                addTagToColorTable(ctName, tagName, ccpl);
        }
        else
        {
            auto tmp{tr("Tag Editing WARNING: ") + QString(result.second.c_str())};
            Error(tmp);
            return;
        }
    }
    Apply();
}


// ****************************************************************************
// Method: QvisColorTableWindow::updateNameBoxPosition
//
// Purpose:
//   Updates the name box position to the given coords.
//
// Programmer: Justin Privitera
// Creation:   Thu Jun 16 12:52:17 PDT 2022
//
// Modifications:
//
// ****************************************************************************

void 
QvisColorTableWindow::updateNameBoxPosition(bool tagsOn)
{
    nameListBox->blockSignals(true);
    mgLayout->removeWidget(nameListBox);
    if (tagsOn)
        mgLayout->addWidget(nameListBox, 3, 3, 1, 3);
    else
        mgLayout->addWidget(nameListBox, 3, 0, 1, 6);
    nameListBox->blockSignals(false);
}
