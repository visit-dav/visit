// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLORTABLE_WIDGET_H
#define QVIS_COLORTABLE_WIDGET_H
#include <winutil_exports.h>
#include <QWidget>

// Forward declarations.
class QCheckBox;
class QvisColorTableButton;

// ****************************************************************************
// Class: QvisColorTableWidget
//
// Purpose:
//   This contains the color table button, plus a toggle for inverting the
//   color table.
//
// Programmer: Kathleen Bonnell 
// Creation:   January 17, 2011
//
// Modifications:
//
// ****************************************************************************

class WINUTIL_API QvisColorTableWidget : public QWidget
{
    Q_OBJECT
public:
    QvisColorTableWidget(QWidget *parent, 
                         bool showInvertButton = false);
    virtual ~QvisColorTableWidget();
    virtual QSize sizeHint() const;

    void setColorTable(const QString &ctName);
    void setInvertColorTable(bool val);

signals:
    void selectedColorTable(bool useDefault, const QString &ctName);
    void invertColorTableToggled(bool);

private slots:
    void handleSelectedColorTable(bool useDefault, const QString &ctName);
    void invertToggled(bool);

private:
    QvisColorTableButton          *colorTableButton;
    QCheckBox                     *invertToggle; 
};

#endif
