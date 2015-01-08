/*****************************************************************************
*
* Copyright (c) 2000 - 2015, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-442911
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
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
