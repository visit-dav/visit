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
    static int  getColorTableIndex(const QString &ctName);
    void regeneratePopupMenu();
    static QIcon getIcon(const QString &);
    static QIcon makeIcon(const QString &);

    QString                        colorTable;

    static int                     numInstances;
    QMenu                         *colorTableMenu;
    static QActionGroup           *colorTableMenuActionGroup;
    static bool                    popupHasEntriesDiscrete;
    static bool                    popupHasEntriesContinuous;
    static ColorTableButtonVector  buttons;

    static QStringList             colorTableNames;
    static ColorTableAttributes   *colorTableAtts;
    bool                           defDiscrete;
};

#endif
