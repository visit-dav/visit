// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_COLOR_SWATCH_LIST_WIDGET_H
#define QVIS_COLOR_SWATCH_LIST_WIDGET_H
#include <QListWidget>
#include <gui_exports.h>

// ****************************************************************************
// Class: QvisColorSwatchListWidget
//
// Purpose:
//   This is a list widget class that draws a color swatch next to the text
//   in the items.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Jul 17 14:29:12 PDT 2008
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisColorSwatchListWidget : public QListWidget
{
    Q_OBJECT
public:
    QvisColorSwatchListWidget(QWidget *parent);
    virtual ~QvisColorSwatchListWidget();

    void addItem(const QString &, const QColor &);

    QString text(int) const;
    void    setText(int, const QString &);
    QColor  color(int) const;
    void    setColor(int, const QColor &);
};

#endif
