// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVISSPREADSHEETPLOTWINDOW_H
#define QVISSPREADSHEETPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class SpreadsheetAttributes;
class QLabel;
class QCheckBox;
class QComboBox;
class QLineEdit;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisDialogLineEdit;

// ****************************************************************************
// Class: QvisSpreadsheetPlotWindow
//
// Purpose: 
//   Defines QvisSpreadsheetPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: Brad Whitlock
// Creation:   Wed Feb 21 12:02:25 PDT 2007
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
// ****************************************************************************

class QvisSpreadsheetPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSpreadsheetPlotWindow(const int type,
                         SpreadsheetAttributes *subj,
                         const QString &caption = QString(),
                         const QString &shortName = QString(),
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisSpreadsheetPlotWindow();
    virtual void CreateWindowContents();
public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
    void UpdateSubsetNames();
private slots:
    void formatStringProcessText();
    void useColorTableChanged(bool val);
    void colorTableNameChanged(bool useDefault, const QString &ctName);
    void showPatchOutlineChanged(bool val);
    void showCurrentCellOutlineChanged(bool val);
    void showTracerPlaneChanged(bool val);
    void tracerColorChanged(const QColor &color);
    void tracerOpacityChanged(int);
    void normalChanged(int val);
    void subsetNameChanged(const QString &);
    void fontNameChanged(const QString &newFont);
private:
    int plotType;
    QComboBox *subsetName;
    QLineEdit *formatString;
    QvisDialogLineEdit *fontName;
    QCheckBox *useColorTable;
    QvisColorTableButton *colorTableName;
    QCheckBox *showPatchOutline;
    QCheckBox *showCurrentCellOutline;
    QCheckBox *showTracerPlane;
    QvisColorButton   *tracerColor;
    QvisOpacitySlider *tracerOpacity;

    QButtonGroup *normal;

    QLabel *subsetNameLabel;
    QLabel *formatStringLabel;
    QLabel *tracerColorLabel;
    QLabel *normalLabel;

    SpreadsheetAttributes *atts;
    int silTopSet;
    int silNumSets;
    int silNumCollections;

    static const char *defaultItem;
};

#endif
