#ifndef QVIS_COLORTABLE_BUTTON_H
#define QVIS_COLORTABLE_BUTTON_H
#include <gui_exports.h>
#include <vector>
#include <qpushbutton.h>

// Forward declarations.
class QPopupMenu;

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
// ****************************************************************************

class GUI_API QvisColorTableButton : public QPushButton
{
    Q_OBJECT

    typedef std::vector<QvisColorTableButton *> ColorTableButtonVector;
public:
    QvisColorTableButton(QWidget *parent, const char *name = 0);
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
signals:
    void selectedColorTable(bool useDefault, const QString &ctName);
private slots:
    void popupPressed();
    void colorTableSelected(int index);
private:
    static int  getColorTableIndex(const QString &ctName);
    static void regeneratePopupMenu();

    QString                        colorTable;

    static int                     numInstances;
    static QPopupMenu             *colorTablePopup;
    static bool                    popupHasEntries;
    static ColorTableButtonVector  buttons;

    static int                     numColorTableNames;
    static QString                *colorTableNames;
};

#endif
