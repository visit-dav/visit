// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_MANAGER_WIDGET_H
#define QVIS_COLOR_MANAGER_WIDGET_H
#include <gui_exports.h>
#include <vector>
#include <QColor>
#include <QLayout>
#include <QScrollArea>
#include <QString>

// Forward declarations.
class QGrid;
class QLabel;
class QvisColorButton;
class QvisOpacitySlider;

// ****************************************************************************
// Class: QvisColorManagerWidget
//
// Purpose:
//   This class contains a list of names items and can select colors and
//   opacities for them.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Dec 8 16:28:39 PST 2000
//
// Modifications:
//   Brad Whitlock, Fri Feb 16 17:46:57 PST 2001
//   Made the column header labels class members and added methods to set
//   the text that is displayed in them.
//
//   Brad Whitlock, Thu Sep 6 15:25:19 PST 2001
//   Added a method that is called when the palette changes.
//
//   Brad Whitlock, Thu Aug 22 12:12:25 PDT 2002
//   I added an override for the setEnabled method.
//
//   Brad Whitlock, Tue Jul 15 15:01:03 PDT 2008
//   Qt 4.
//
// ****************************************************************************

class GUI_API QvisColorManagerWidget : public QScrollArea
{
    Q_OBJECT

    typedef struct
    {
        QLabel            *nameLabel;
        QvisColorButton   *colorBox;
        QvisOpacitySlider *opacitySlider;
    } ColorEntry;

    typedef std::vector<ColorEntry *> ColorEntryVector;
public:
    QvisColorManagerWidget(QWidget *parent = 0);
    virtual ~QvisColorManagerWidget();
    virtual QSize sizeHint() const;
    virtual void setEnabled(bool val);

    void addEntry(const QString &name, const QColor &color, int opacity);
    void removeLastEntry();
    void setEntry(int index, const QString &name, const QColor &color,
                  int opacity);

    void setAttributeName(int index, const QString &name);
    void setColor(int index, const QColor &color);
    void setOpacity(int index, int opacity);
    void setNameLabelText(const QString &s);
    void setColorLabelText(const QString &s);
    void setOpacityLabelText(const QString &s);

    int numEntries() const;
    QString name(int index) const;
    QColor  color(int index) const;
    int opacity(int index) const;
signals:
    void colorChanged(const QColor &color, int index);
    void opacityChanged(int opacity, int index);
private slots:
    void selectedColor(const QColor &color, const void *userData);
    void changedOpacity(int opacity, const void *userData);
private:
    QWidget          *top;
    QGridLayout      *grid;
    QLabel           *nameLabel;
    QLabel           *colorLabel;
    QLabel           *opacityLabel;

    ColorEntryVector colorEntries;
};

#endif
