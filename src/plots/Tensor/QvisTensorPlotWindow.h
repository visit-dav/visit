/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
*****************************************************************************/

#ifndef QVIS_VECTOR_WINDOW_H
#define QVIS_VECTOR_WINDOW_H
#include <QvisPostableWindowObserver.h>

// Forward declarations
class QButtonGroup;
class QCheckBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;
class TensorAttributes;
class QvisOpacitySlider;
class QvisColorTableButton;

// ****************************************************************************
// Class: QvisTensorPlotWindow
//
// Purpose:
//   This class is a postable window that watches tensor plot attributes and
//   always represents their current state.
//
// Notes:      
//
// Programmer: Hank Childs
// Creation:   September 23, 2003
//
// Modifications:
//   Eric Brugger, Wed Nov 24 11:39:58 PST 2004
//   Added scaleByMagnitude and autoScale.
//
// ****************************************************************************

class QvisTensorPlotWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisTensorPlotWindow(const int type, TensorAttributes *_vecAtts,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisTensorPlotWindow();
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
    void tensorColorChanged(const QColor &color);
    void processScaleText();
    void scaleByMagnitudeToggled();
    void autoScaleToggled();
    void reduceMethodChanged(int index);
    void processNTensorsText();
    void processStrideText();
    void legendToggled();
    void colorModeChanged(int);
    void colorTableClicked(bool useDefault, const QString &ctName);
private:
    int                  plotType;
    TensorAttributes     *tensorAtts;

    QGroupBox            *colorGroupBox;
    QvisColorButton      *tensorColor;
    QButtonGroup         *colorButtonGroup; 
    QvisColorTableButton *colorTableButton;
    QGroupBox            *scaleGroupBox;    
    QLineEdit            *scaleLineEdit;
    QCheckBox            *scaleByMagnitudeToggle;
    QCheckBox            *autoScaleToggle;
    QGroupBox            *reduceGroupBox;
    QButtonGroup         *reduceButtonGroup;
    QLineEdit            *nTensorsLineEdit;
    QLineEdit            *strideLineEdit;
    QCheckBox            *legendToggle;
};

#endif
