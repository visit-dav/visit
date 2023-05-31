// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ELEMENT_SELECTION_WIDGET_H
#define QVIS_ELEMENT_SELECTION_WIDGET_H
#include <gui_exports.h>
#include <QWidget>
#include <vector>

// Forward declarations.
class QPushButton;
class QTimer;
class QVBoxLayout;
class QvisPeriodicTableWidget;

// ****************************************************************************
// Class: QvisElementSelectionWidget
//
// Purpose:
//   This contains the periodic table widget plus a new button that
//   allows a wildcard ("match any element").
//
// Programmer: Jeremy Meredith
// Creation:   February 11, 2008
//
// Note: borrowed heavily from Brad's color selection widget
//
// Modifications:
//    Jeremy Meredith, Tue Feb 12 14:01:52 EST 2008
//    Added support for hinting some elements to the user, e.g. to highlight
//    the elements that are actually in the database.
//
//    Brad Whitlock, Tue Jun  3 14:44:16 PDT 2008
//    Qt 4.
//
// ****************************************************************************

class GUI_API QvisElementSelectionWidget : public QWidget
{
    Q_OBJECT
  public:
    QvisElementSelectionWidget(QWidget *parent = 0, Qt::WindowFlags f = Qt::Widget);
    virtual ~QvisElementSelectionWidget();
    virtual QSize sizeHint() const;

    void setSelectedElement(int);
    void setHintedElements(const std::vector<int>&);

  signals:
    void selectedElement(int);
            public slots:
            virtual void show();
    virtual void hide();
  protected:
    virtual void mouseMoveEvent(QMouseEvent *e);
    virtual void mouseReleaseEvent(QMouseEvent *e);
             private slots:
             void matchAnyElementClicked();
    void handleSelectedElement(int);
  private:
    QvisPeriodicTableWidget *periodicTable;
    QPushButton             *matchAnyElementButton;
    QTimer                  *timer;
};

#endif
