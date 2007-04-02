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

#ifndef QVISHISTOGRAMPLOTWINDOW_H
#define QVISHISTOGRAMPLOTWINDOW_H

#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

class HistogramAttributes;
class QLabel;
class QCheckBox;
class QGroupBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QButtonGroup;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class QvisVariableButton;

// ****************************************************************************
// Class: QvisHistogramPlotWindow
//
// Purpose: 
//   Defines QvisHistogramPlotWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: Cyrus Harrison - generated using xml2window
// Creation:   Thu Mar 8 08:20:00 PDT 2007
//
// Modifications:
//   
// ****************************************************************************

class QvisHistogramPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisHistogramPlotWindow(const int type,
                         HistogramAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisHistogramPlotWindow();
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
    void basedOnChanged(int val);
    void histogramTypeChanged(int val);
    void twoDAmountChanged(int val);
    void specifyRangeChanged(bool val);
    void minProcessText();
    void maxProcessText();
    void numBinsProcessText();
    void domainProcessText();
    void zoneProcessText();
    void useBinWidthsChanged(bool val);
    void outputTypeChanged(int val);
    void lineStyleChanged(int style);
    void lineWidthChanged(int style);
    void colorChanged(const QColor &color);
  private:
    int plotType;
    QButtonGroup *basedOn;
    QButtonGroup *histogramType;
    QButtonGroup *twoDAmount;
    QCheckBox *specifyRange;
    QLineEdit *min;
    QLineEdit *max;
    QLineEdit *numBins;
    QLineEdit *domain;
    QLineEdit *zone;
    QCheckBox *useBinWidths;
    QButtonGroup *outputType;
    QvisLineStyleWidget *lineStyle;
    QvisLineWidthWidget *lineWidth;
    QvisColorButton *color;
    QLabel *basedOnLabel;
    QLabel *histogramTypeLabel;
    QLabel *twoDAmountLabel;
    QLabel *minLabel;
    QLabel *maxLabel;
    QLabel *numBinsLabel;
    QLabel *domainLabel;
    QLabel *zoneLabel;
    QLabel *outputTypeLabel;
    QLabel *lineStyleLabel;
    QLabel *lineWidthLabel;
    QLabel *colorLabel;

    QGroupBox *histGroupBox;
    QGroupBox *barGroupBox;
    QGroupBox *styleGroupBox;

    HistogramAttributes *atts;
};



#endif
