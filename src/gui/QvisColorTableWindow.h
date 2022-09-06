// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_TABLE_WINDOW_H
#define QVIS_COLOR_TABLE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>
#include <ColorTableObserver.h>

// Forward declarations
class ColorControlPointList;
class ColorTableAttributes;
class DataNode;
class QVBoxLayout;
class QGridLayout;
class QPushButton;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QTreeWidget;
class QTreeWidgetItem;
class QSlider;
class QSpinBox;
class QvisSpectrumBar;
class QvisColorSelectionWidget;
class QvisColorGridWidget;
class QvisNoDefaultColorTableButton;

class TagInfo
{
public:
    bool active = false;
    int numrefs = 0;
    QTreeWidgetItem *tagTableItem = nullptr;
};

// ****************************************************************************
// Class: QvisColorTableWindow
//
// Purpose:
//   This class contains the widgets that manipulate the color table.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 8 09:58:12 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 13 17:59:26 PST 2002
//   Modified the popupColorSelect slot function.
//
//   Brad Whitlock, Wed Nov 20 15:12:35 PST 2002
//   I added support for discrete colortables.
//
//   Brad Whitlock, Wed Feb 26 11:09:16 PDT 2003
//   I changed things so that discrete color tables can have an arbitrary
//   number of colors.
//
//   Brad Whitlock, Tue Jul 1 16:37:41 PST 2003
//   I added an Export button.
//
//   Brad Whitlock, Mon Mar 6 09:09:46 PDT 2006
//   I added code to save the current color table to the settings.
//
//   Brad Whitlock, Wed Apr  9 11:58:57 PDT 2008
//   QString for caption, shortName.
//
//   Jeremy Meredith, Wed Dec 31 15:27:54 EST 2008
//   Added support for showing hints such as the color index or an
//   element name (if we're working with an atomic color table).
//
//   Jeremy Meredith, Fri Feb 20 15:03:25 EST 2009
//   Added alpha channel support.
//
//   Brad Whitlock, Fri Apr 27 15:03:10 PDT 2012
//   Add smoothing method instead of a check box for it.
//
//   Kathleen Biagas, Fri Aug 8 08:50:31 PDT 2014
//   Added support for grouping color tables according to a category name.
//
//   Mark C. Miller, Wed Feb 28 14:56:09 PST 2018
//   Handling "smoothing" label correctly.
// 
//   Justin Privitera, Wed May 18 11:25:46 PDT 2022
//   Changed *active* to *default* for everything related to color tables.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed categories and added infrastructure for tags.
// 
//   Justin Privitera, Wed Jun 29 17:50:24 PDT 2022
//   Added new function `AddToTagTable()`.
// 
//   Justin Privitera, Thu Jul 14 16:57:42 PDT 2022
//   Added search capabilities for color tables. In this file, added boolean
//   `searchingOn`, QString `searchTerm`, QCheckBox `searchToggle`, and 
//   functions `searchingToggled` and `searchEdited`.
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Added `TagInfo` class to store tag info all in one place.
//   I removed the tagList and activeTags stringVectors and replaced them with
//   the new tagList, which is a map from tagnames to `TagInfo`s.
// 
// ****************************************************************************

class GUI_API QvisColorTableWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisColorTableWindow(ColorTableAttributes *volumeAtts_,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisColorTableWindow();
    virtual void CreateWindowContents();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void UpdateEditor();
    void UpdateColorControlPoints();
    void UpdateDiscreteSettings();
    void AddGlobalTag(std::string currtag, bool run_before);
    void AddToTagTable(std::string currtag);
    void UpdateTags();
    void UpdateNames();
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    const ColorControlPointList *GetDefaultColorControlPoints() const;
          ColorControlPointList *GetDefaultColorControlPoints();
    void ShowSelectedColor(const QColor &c);
    void ChangeSelectedColor(const QColor &c);
    void PopupColorSelect(const QColor &, const QPoint &p);
    QColor GetNextColor();

private slots:
    void resizeColorTable(int);
    void setColorTableType(int);
    void redValueChanged(int r);
    void greenValueChanged(int g);
    void blueValueChanged(int b);
    void alphaValueChanged(int b);
    void activateDiscreteColor(const QColor &, int);
    void activateContinuousColor(int index);
    void chooseContinuousColor(int, const QPoint &);
    void chooseDiscreteColor(const QColor &, int, int, const QPoint &);
    void sliderPressed();
    void sliderReleased();
    void setDefaultContinuous(const QString &ct);
    void setDefaultDiscrete(const QString &ct);

    void alignControlPoints();
    void controlPointMoved(int index, float position);
    void selectedColor(const QColor &color);
    void smoothingMethodChanged(int val);
    void equalSpacingToggled(bool val);
    void addColorTable();
    void deleteColorTable();
    void exportColorTable();
    void highlightColorTable(QTreeWidgetItem *, QTreeWidgetItem*);
    void tagTableItemSelected(QTreeWidgetItem *, int);
    void showIndexHintsToggled(bool val);
    void taggingToggled(bool val);
    void tagCombiningChanged(int index);
    void searchingToggled(bool checked);
    void searchEdited(const QString &newSearchTerm);
    void updateNameBoxPosition(bool tagsOn);
private:
    ColorTableAttributes     *colorAtts;
    int                      colorCycle;
    QString                  currentColorTable;
    int                      popupMode;
    bool                     sliding;
    std::map<std::string, TagInfo> tagList;
    bool                     tagsVisible;
    bool                     tagsMatchAny;
    bool                     searchingOn;
    QString                  searchTerm;

    // Widgets and layouts.
    QGroupBox                *defaultGroup;
    QvisNoDefaultColorTableButton *defaultContinuous;
    QLabel                   *defaultContinuousLabel;
    QvisNoDefaultColorTableButton *defaultDiscrete;
    QLabel                   *defaultDiscreteLabel;
    QCheckBox                *tagFilterToggle;
    QComboBox                *tagCombiningBehaviorChoice;
    QCheckBox                *searchToggle;
    QGridLayout              *mgLayout;

    QGroupBox                *colorTableWidgetGroup;
    QPushButton              *newButton;
    QPushButton              *deleteButton;
    QPushButton              *exportButton;
    QLineEdit                *nameLineEdit;
    QTreeWidget              *nameListBox;
    QLabel                   *tagLabel;
    QLineEdit                *tagLineEdit;
    QTreeWidget              *tagTable;

    QGroupBox                *colorWidgetGroup;

    QSpinBox                 *colorNumColors;
    QButtonGroup             *colorTableTypeGroup;

    QLabel                   *smoothLabel;
    QComboBox                *smoothingMethod;
    QCheckBox                *equalCheckBox;
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
    QPushButton              *alignPointButton;
    QCheckBox                *showIndexHintsCheckBox;

    QvisColorGridWidget      *discreteColors;
    QLabel                   *componentLabels[4];
    QSlider                  *componentSliders[4];
    QSpinBox                 *componentSpinBoxes[4];

    // This object also observes the color table attributes.
    ColorTableObserver       ctObserver;
};

#endif
