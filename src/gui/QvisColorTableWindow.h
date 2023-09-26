// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_TABLE_WINDOW_H
#define QVIS_COLOR_TABLE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>
#include <ColorTableObserver.h>
#include <set>

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
//   Justin Privitera, Wed Jul 27 12:23:56 PDT 2022
//   Added `skip_update` option to `ShowSelectedColor()`.
// 
//   Justin Privitera, Wed Aug 10 13:24:26 PDT 2022
//   Reorganized gui elements so they appear in the header file in the same 
//   order they appear in the gui.
// 
//   Justin Privitera, Thu Aug 25 15:04:55 PDT 2022
//   Made the following changes:
//    - Included set
//    - Added StringifyTagChanges, UnstringifyAndMergeTagChanges, tagEdited,
//      addRemoveTag, addTagToColorTable, and removeTagFromColorTable.
//    - Added tagEdit (a string for editing tags)
//    - Added a data structure to store changes to tags
// 
//   Justin Privitera, Fri Sep  2 16:46:21 PDT 2022
//   Added `TagInfo` class to store tag info all in one place.
//   I removed the tagList and activeTags stringVectors and replaced them with
//   the new tagList, which is a map from tagnames to `TagInfo`s.
// 
//   Justin Privitera, Wed Sep 21 16:51:24 PDT 2022
//   `TagInfo` now includes lengthy comments describing each member.
//   The tagList and tagChanges data structures have been merged into
//   the tagList. Thus TagInfo contains a new member representing the 
//   tag changes.
// 
//   Justin Privitera, Thu Sep 22 10:50:46 PDT 2022
//   Moved TagInfo class implementation to the .C file.
// 
//   Justin Privitera, Mon Feb 13 14:32:02 PST 2023
//   Removed taggingToggled and added tagsSelectAll.
//   Removed tagFilterToggle and added tagsSelectAllButton.
//   Removed tagsVisible.
// 
//   Justin Privitera, Thu May 11 12:31:12 PDT 2023
//   Removed searchingToggled(), removed bool searchingOn, and added QLineEdit 
//   *searchBar, as searching is always enabled now.
//   Removed gui elements from the header that were not used outside the
//   CreateWindowContents() function.
// 
//   Justin Privitera, Mon Aug 28 11:22:47 PDT 2023
//   Removed the tagList and TagInfo class declaration.
//   Removed the tagsMatchAny flag.
//   Removed the AddGlobalTag(), AddToTagTable(), StringifyTagChanges(),
//   UnstringifyAndMergeTagChanges(), addTagToColorTable(), and
//   removeTagFromColorTable() functions.
//   All tagging infrastructure has been moved to the CTAtts.
// 
//   Justin Privitera, Tue Sep  5 12:49:42 PDT 2023
//   Change UpdateTags to UpdateTagTable.
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
    void UpdateTagTable();
    void UpdateNames();
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    const ColorControlPointList *GetDefaultColorControlPoints() const;
          ColorControlPointList *GetDefaultColorControlPoints();
    void ShowSelectedColor(const QColor &c, bool skip_update = false);
    void ChangeSelectedColor(const QColor &c);
    void PopupColorSelect(const QColor &, const QPoint &p);
    QColor GetNextColor();

private slots:
    void resizeColorTable(int);
    void setColorTableType(int);
    void colorValueChanged(int rgba, int value);
    void redValueChanged(int r);
    void greenValueChanged(int g);
    void blueValueChanged(int b);
    void alphaValueChanged(int a);
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
    void tagsSelectAll();
    void tagCombiningChanged(int index);
    void searchEdited(const QString &newSearchTerm);
    void tagEdited();
    void addRemoveTag();
private:
    ColorTableAttributes     *colorAtts;
    int                      colorCycle;
    QString                  currentColorTable;
    int                      popupMode;
    bool                     sliding;
    QString                  searchTerm;
    QString                  tagEdit;

    // a map from tag names to tab table item pointers
    std::map<std::string, QTreeWidgetItem*> tagTableItems;

    // This object also observes the color table attributes.
    ColorTableObserver       ctObserver;    

    // 
    // Widgets and layouts.
    // 

    // Default Color Table
    QvisNoDefaultColorTableButton *defaultContinuous;
    QvisNoDefaultColorTableButton *defaultDiscrete;
    // End Default Color Table

    // Manager
    QPushButton              *newButton;
    QPushButton              *deleteButton;
    QPushButton              *exportButton;

    QPushButton              *tagsSelectAllButton;
    QComboBox                *tagCombiningBehaviorChoice;

    QTreeWidget              *tagTable;
    QTreeWidget              *nameListBox;

    QLineEdit                *searchBar;
    QLineEdit                *nameLineEdit;
    QLineEdit                *tagLineEdit;

    QLineEdit                *tagEditorLineEdit;
    QPushButton              *tagAddRemoveButton;
    // End Manager

    // Editor
    QGroupBox                *colorWidgetGroup;

    QSpinBox                 *colorNumColors;
    QButtonGroup             *colorTableTypeGroup;

    QPushButton              *alignPointButton;
    QLabel                   *smoothLabel;
    QComboBox                *smoothingMethod;
    QCheckBox                *equalCheckBox;
    
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
    QCheckBox                *showIndexHintsCheckBox;

    QvisColorGridWidget      *discreteColors;
    QLabel                   *componentLabels[4];
    QSlider                  *componentSliders[4];
    QSpinBox                 *componentSpinBoxes[4];
    // End Editor
};

#endif
