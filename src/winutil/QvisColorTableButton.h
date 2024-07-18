// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLORTABLE_BUTTON_H
#define QVIS_COLORTABLE_BUTTON_H
#include <winutil_exports.h>
#include <QPushButton>

// Forward declarations.
class QAction;
class QActionGroup;
class QMenu;
class ColorTableAttributes;

// ****************************************************************************
// Class: QvisColorTableButton
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
//   Removed category from addColorTable() and removed mappedColorTableNames.
//
//   Kathleen Biagas, Wed May 8, 2024
//   Added static method for connecting to the gui's openColorTableWindows,
//   with necessary Create/Slot members.
//
// ****************************************************************************

class WINUTIL_API QvisColorTableButton : public QPushButton
{
    Q_OBJECT

    typedef std::vector<QvisColorTableButton *> ColorTableButtonVector;
public:
    QvisColorTableButton(QWidget *parent);
    virtual ~QvisColorTableButton();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy () const;

    void setColorTable(const QString &ctName);
    const QString &getColorTable() const;
    void useDefaultColorTable();

    // Methods to set the list of internal color tables.
    static void clearAllColorTables();
    static void addColorTable(const QString &ctName);
    static void updateColorTableButtons();
    static void setColorTableAttributes(ColorTableAttributes *cAtts);

    static void ConnectColorTableOpen(QObject *, const char*);
signals:
    void selectedColorTable(bool useDefault, const QString &ctName);
private slots:
    void popupPressed();
    void colorTableSelected(QAction *);
private:
    static int  getColorTableIndex(const QString &ctName);
    static void regeneratePopupMenu();
    static QIcon getIcon(const QString &);
    static QIcon makeIcon(const QString &);

    QString                        colorTable;

    static int                     numInstances;
    static QMenu                  *colorTableMenu;
    static QActionGroup           *colorTableMenuActionGroup;
    static bool                    popupHasEntries;
    static ColorTableButtonVector  buttons;

    static QStringList             colorTableNames;
    static ColorTableAttributes   *colorTableAtts;

    static QObject                *colorTableOpenCreator;
    static const char             *colorTableOpenSlot;
};

#endif
