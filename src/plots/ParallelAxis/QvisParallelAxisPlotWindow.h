/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
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

class ParallelAxisAttributes;
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QSlider;
class QSpinBox;
class QVBox;
class QvisColorButton;
class QvisColorTableButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisOpacitySlider;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisParallelAxisPlotWindow
//
// Purpose: 
//   Defines QvisParallelAxisPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Thu Mar 15 13:59:40 PST 2007
//
// Modifications:
//   
// ****************************************************************************

class QvisParallelAxisPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisParallelAxisPlotWindow(const int type,
                         ParallelAxisAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisParallelAxisPlotWindow();
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
    void axisSelected(int);
    void addAxis(const QString &axisToAdd);
    void delAxis();
    void moveAxisUp();
    void moveAxisDown();
  private:
    int plotType;

    QGroupBox *axisGroup;
    QListBox  *axisList;
    QvisVariableButton *axisNewButton;
    QPushButton *axisDelButton;
    QPushButton *axisUpButton;
    QPushButton *axisDownButton;

    QGroupBox *drawLines;
    QGroupBox *drawContext;

    QvisColorButton *linesColor;
    QLineEdit *contextGamma;
    QSlider   *contextGammaSlider;
    QLineEdit *contextNumPartitions;
    QSlider   *contextNumPartitionsSlider;
    QvisColorButton *contextColor;


    QLabel *orderedAxisNamesLabel;
    QLabel *axisInfoFlagSetsLabel;
    QLabel *axisXPositionsLabel;
    QLabel *axisAttributeVariablesLabel;
    QLabel *linesColorLabel;
    QLabel *contextGammaLabel;
    QLabel *contextNumPartitionsLabel;
    QLabel *contextColorLabel;

    ParallelAxisAttributes *atts;
};



#endif
