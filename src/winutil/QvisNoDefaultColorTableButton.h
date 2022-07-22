// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_NODEFAULT_COLORTABLE_BUTTON_H
#define QVIS_NODEFAULT_COLORTABLE_BUTTON_H
#include <winutil_exports.h>
#include <QPushButton>

// Forward declarations.
class QAction;
class QActionGroup;
class QMenu;
class ColorTableAttributes;

// ****************************************************************************
// Class: QvisNoDefaultColorTableButton
//
// Purpose:
//   This is a type of push button that is aware of the different color tables
//   that can be used for plots.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Wed Jun 20 12:37:24 PDT 2001
//
// Modifications:
//   Brad Whitlock, Thu Feb 14 13:37:41 PST 2002
//   Added a counter.
//
//   Brad Whitlock, Tue Feb 20 11:47:37 PDT 2007
//   Changed API.
//
//   Brad Whitlock, Fri May  9 11:20:10 PDT 2008
//   Qt 4.
//
//   Brad Whitlock, Wed Apr 25 16:06:56 PDT 2012
//   Add color table icons.
//
//   Kathleen Biagas, Mon Aug  4 15:51:11 PDT 2014
//   Change colorTableNames to a QSringList, add mappedColorTableNames
//   to aid in grouping.  Add category argument to addColorTable.
// 
//   Justin Privitera, Thu Jun 16 18:01:49 PDT 2022
//   Removed category arg from addColorTable and removed mappedColorTableNames.
// 
//   Justin Privitera, Wed Jul 13 14:48:42 PDT 2022
//   Made numerous changes throughout the class to enforce color table 
//   filtering, so that the Continuous ct button can only select continuous
//   color tables, and the Discrete ct button can only select discrete color
//   tables. Note in this file the changes to the static variables; now many
//   are arrays with two elements. This is to facilitate different behavior for
//   the different buttons.
//
// ****************************************************************************

class WINUTIL_API QvisNoDefaultColorTableButton : public QPushButton
{
    Q_OBJECT

    typedef std::vector<QvisNoDefaultColorTableButton *> ColorTableButtonVector;
public:
    QvisNoDefaultColorTableButton(QWidget *parent, bool discrete);
    virtual ~QvisNoDefaultColorTableButton();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy () const;

    void setColorTable(const QString &ctName);
    const QString &getColorTable() const;
    const int getButtonType() const;

    // Methods to set the list of internal color tables.
    static void clearAllColorTables();
    static void addColorTable(const QString &ctName);
    static void updateColorTableButtons();
    static void setColorTableAttributes(ColorTableAttributes *cAtts);
signals:
    void selectedColorTable(const QString &ctName);
private slots:
    void popupPressed();
    void colorTableSelected(QAction *);
private:
    static int  getColorTableIndex(const QString &ctName, const int whichButton);
    void regeneratePopupMenu();
    static QIcon getIcon(const QString &);
    static QIcon makeIcon(const QString &);

    QString                        colorTable;
    int                            buttonType;

    static int                     numInstances;
    static QMenu                  *colorTableMenu[2];
    static QActionGroup           *colorTableMenuActionGroup[2];
    static bool                    popupHasEntries[2];
    static ColorTableButtonVector  buttons;

    static QStringList             colorTableNames[2];
    static ColorTableAttributes   *colorTableAtts;
};

#endif
