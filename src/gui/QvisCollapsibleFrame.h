// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

/*****************************************************************************
*  This code file was originally inspired by the Mantid Project:
*  http://www.mantidproject.org/.
*
*****************************************************************************/

#ifndef QVISCOLLAPSIBLEFRAME_H_
#define QVISCOLLAPSIBLEFRAME_H_

#include <QLabel>

#include <gui_exports.h>

class QVBoxLayout;

// ****************************************************************************
// Class: QvisTitleBar
//
// Purpose:
//   This class is a title bar (aka QLabel) with a state variable
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
class GUI_API QvisTitleBar : public QLabel
{
    Q_OBJECT

  public:
    enum LayoutState { Hidden, Exposed };

    QvisTitleBar(const QString& title, QWidget* parent);

    void mousePressEvent(QMouseEvent* event);
    void paintEvent(QPaintEvent *event);
    
    void setShow();
    void setHide();
    
   signals: // Used to communicate with the associated frame.
    void showOrHide(QvisTitleBar::LayoutState state);

  private:
    LayoutState state;
};

// ****************************************************************************
// Class: QvisCollapsibleFrame
//
// Purpose:
//   This class creates a titlebar (label) and a frame (layout) that contains
//   a widget that get shown or hidden. 
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
class GUI_API QvisCollapsibleFrame : public QWidget
{
    Q_OBJECT

  public:
    QvisCollapsibleFrame(const QString& title, QWidget* parent);
    ~QvisCollapsibleFrame();
    
    void setWidget(QWidget* w);
    
    void setShow();
    void setHide();
    
  signals: // Used to communicate with the parent layout.
    void updateLayout();

  private slots:  // Used to communicate with the associated titlebar.
    void showOrHide(QvisTitleBar::LayoutState state);

  private:
    QvisTitleBar* titleBar;
    QVBoxLayout* layout;
    QWidget *widget;
};

// ****************************************************************************
// Class: QvisCollapsibleLayout
//
// Purpose:
//   This class (QWidget) manages the collapsable frames using a layout
//
// Programmer: Allen Sanderson
// Creation:   23 July 2014
//
class GUI_API QvisCollapsibleLayout : public QWidget
{
    Q_OBJECT

  public:
    QvisCollapsibleLayout(QWidget* parent);
    ~QvisCollapsibleLayout();
    
    QvisCollapsibleFrame* addFrame(const QString& title, QWidget* widget);

  private slots:  // Used to communicate with the child frames.
    void updateLayout();
    void adjustWindowSize();

  private:
    QVBoxLayout* layout;
};

#endif /*QVISCOLLAPSIBLEFRAME_H_*/
