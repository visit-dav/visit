/*****************************************************************************
*
* Copyright (c) 2000 - 2008, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
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
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef QVISPARALLELAXISPLOTWINDOW_H
#define QVISPARALLELAXISPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class ParallelCoordinatesAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListView;
class QSlider;
class QSpinBox;
class QVBox;
class QvisColorButton;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisVariableButton;
class QListViewItem;

// ****************************************************************************
// Class: QvisParallelCoordinatesPlotWindow
//
// Purpose: 
//   Defines QvisParallelCoordinatesPlotWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   March 21, 2007
//
// Notes: initial implementation taken from the ParallelAxis plot window.
//
// Modifications:
//    Jeremy Meredith, Fri Feb  8 12:34:19 EST 2008
//    Added ability to unify extents across all axes.
//   
//    Jeremy Meredith, Fri Feb  8 16:12:06 EST 2008
//    Changed axis list to QListView to support multiple columns.
//    Added min/max extents columns for each axis, and a button to reset them.
//
// ****************************************************************************

class QvisParallelCoordinatesPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisParallelCoordinatesPlotWindow(const int type,
                         ParallelCoordinatesAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisParallelCoordinatesPlotWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void drawLinesChanged(bool val);
    void linesColorChanged(const QColor &color);
    void drawContextChanged(bool val);
    void contextGammaProcessText();
    void contextGammaSliderChanged(int val);
    void contextGammaSliderReleased();
    void contextNumPartitionsProcessText();
    void contextNumPartitionsSliderChanged(int val);
    void contextNumPartitionsSliderReleased();
    void contextColorChanged(const QColor &color);
    void axisSelected(QListViewItem*);
    void addAxis(const QString &axisToAdd);
    void delAxis();
    void moveAxisUp();
    void moveAxisDown();
    void resetAxisExtents();
    void linesOnlyIfExtentsToggled(bool);
    void unifyAxisExtentsToggled(bool);
  private:
    int plotType;

    QGroupBox *axisGroup;
    QListView *axisList;
    QvisVariableButton *axisNewButton;
    QPushButton *axisDelButton;
    QPushButton *axisUpButton;
    QPushButton *axisDownButton;
    QPushButton *axisResetExtentsButton;

    QGroupBox *drawLines;
    QCheckBox *linesOnlyIfExtents;
    QvisColorButton *linesColor;

    QGroupBox *drawContext;
    QLineEdit *contextGamma;
    QSlider   *contextGammaSlider;
    QLineEdit *contextNumPartitions;
    QSlider   *contextNumPartitionsSlider;
    QvisColorButton *contextColor;

    QLabel *linesColorLabel;
    QLabel *contextGammaLabel;
    QLabel *contextNumPartitionsLabel;
    QLabel *contextColorLabel;

    QCheckBox *unifyAxisExtents;

    ParallelCoordinatesAttributes *atts;
};



#endif
