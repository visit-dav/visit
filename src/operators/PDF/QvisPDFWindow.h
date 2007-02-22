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

#ifndef QVISPDFWINDOW_H
#define QVISPDFWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class PDFAttributes;
class QLabel;
class QCheckBox;
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
// Class: QvisPDFWindow
//
// Purpose: 
//   Defines QvisPDFWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Nov 21 08:16:58 PDT 2005
//
// Modifications:
//   
// ****************************************************************************

class QvisPDFWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisPDFWindow(const int type,
                         PDFAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisPDFWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void var1ProcessText();
    void var1MinFlagChanged(bool val);
    void var1MaxFlagChanged(bool val);
    void var1MinProcessText();
    void var1MaxProcessText();
    void var1ScalingChanged(int val);
    void var1SkewFactorProcessText();
    void var1NumSamplesProcessText();
    void var2ProcessText();
    void var2MinFlagChanged(bool val);
    void var2MaxFlagChanged(bool val);
    void var2MinProcessText();
    void var2MaxProcessText();
    void var2ScalingChanged(int val);
    void var2SkewFactorProcessText();
    void var2NumSamplesProcessText();
    void numAxesChanged(int val);
    void var3ProcessText();
    void var3MinFlagChanged(bool val);
    void var3MaxFlagChanged(bool val);
    void var3MinProcessText();
    void var3MaxProcessText();
    void var3ScalingChanged(int val);
    void var3SkewFactorProcessText();
    void var3NumSamplesProcessText();
    void scaleCubeChanged(bool val);
    void densityTypeChanged(int val);
  private:
    QLineEdit *var1;
    QCheckBox *var1MinFlag;
    QCheckBox *var1MaxFlag;
    QLineEdit *var1Min;
    QLineEdit *var1Max;
    QButtonGroup *var1Scaling;
    QLineEdit *var1SkewFactor;
    QLineEdit *var1NumSamples;
    QLineEdit *var2;
    QCheckBox *var2MinFlag;
    QCheckBox *var2MaxFlag;
    QLineEdit *var2Min;
    QLineEdit *var2Max;
    QButtonGroup *var2Scaling;
    QLineEdit *var2SkewFactor;
    QLineEdit *var2NumSamples;
    QButtonGroup *numAxes;
    QLineEdit *var3;
    QCheckBox *var3MinFlag;
    QCheckBox *var3MaxFlag;
    QLineEdit *var3Min;
    QLineEdit *var3Max;
    QButtonGroup *var3Scaling;
    QLineEdit *var3SkewFactor;
    QLineEdit *var3NumSamples;
    QCheckBox *scaleCube;
    QButtonGroup *densityType;
    QLabel *var1Label;
    QLabel *var1MinFlagLabel;
    QLabel *var1MaxFlagLabel;
    QLabel *var1MinLabel;
    QLabel *var1MaxLabel;
    QLabel *var1ScalingLabel;
    QLabel *var1SkewFactorLabel;
    QLabel *var1NumSamplesLabel;
    QLabel *var2Label;
    QLabel *var2MinFlagLabel;
    QLabel *var2MaxFlagLabel;
    QLabel *var2MinLabel;
    QLabel *var2MaxLabel;
    QLabel *var2ScalingLabel;
    QLabel *var2SkewFactorLabel;
    QLabel *var2NumSamplesLabel;
    QLabel *numAxesLabel;
    QLabel *var3Label;
    QLabel *var3MinFlagLabel;
    QLabel *var3MaxFlagLabel;
    QLabel *var3MinLabel;
    QLabel *var3MaxLabel;
    QLabel *var3ScalingLabel;
    QLabel *var3SkewFactorLabel;
    QLabel *var3NumSamplesLabel;
    QLabel *scaleCubeLabel;
    QLabel *densityTypeLabel;

    PDFAttributes *atts;
};



#endif
