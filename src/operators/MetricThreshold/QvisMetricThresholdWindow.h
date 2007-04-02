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

#ifndef QVISMETRICTHRESHOLDWINDOW_H
#define QVISMETRICTHRESHOLDWINDOW_H

#include <QvisOperatorWindow.h>
#include <AttributeSubject.h>

class MetricThresholdAttributes;
class QLabel;
class QCheckBox;
class QLineEdit;
class QSpinBox;
class QVBox;
class QComboBox;
class QvisColorTableButton;
class QvisOpacitySlider;
class QvisColorButton;
class QvisLineStyleWidget;
class QvisLineWidthWidget;

// ****************************************************************************
// Class: QvisMetricThresholdWindow
//
// Purpose: 
//   Defines QvisMetricThresholdWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Jul 29 14:33:19 PST 2002
//
// Modifications:
//   
// ****************************************************************************

class QvisMetricThresholdWindow : public QvisOperatorWindow
{
    Q_OBJECT
  public:
    QvisMetricThresholdWindow(const int type,
                         MetricThresholdAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisMetricThresholdWindow();
    virtual void CreateWindowContents();
  protected:
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
  private slots:
    void presetChanged(int val);
    void HexahedronChanged(bool val);
    void hex_lowerProcessText();
    void hex_upperProcessText();
    void TetrahedronChanged(bool val);
    void tet_lowerProcessText();
    void tet_upperProcessText();
    void WedgeChanged(bool val);
    void wed_lowerProcessText();
    void wed_upperProcessText();
    void PyramidChanged(bool val);
    void pyr_lowerProcessText();
    void pyr_upperProcessText();
    void TriangleChanged(bool val);
    void tri_lowerProcessText();
    void tri_upperProcessText();
    void QuadChanged(bool val);
    void quad_lowerProcessText();
    void quad_upperProcessText();
  private:
    QComboBox *preset;
    QCheckBox *Hexahedron;
    QLineEdit *hex_lower;
    QLineEdit *hex_upper;
    QCheckBox *Tetrahedron;
    QLineEdit *tet_lower;
    QLineEdit *tet_upper;
    QCheckBox *Wedge;
    QLineEdit *wed_lower;
    QLineEdit *wed_upper;
    QCheckBox *Pyramid;
    QLineEdit *pyr_lower;
    QLineEdit *pyr_upper;
    QCheckBox *Triangle;
    QLineEdit *tri_lower;
    QLineEdit *tri_upper;
    QCheckBox *Quad;
    QLineEdit *quad_lower;
    QLineEdit *quad_upper;

    MetricThresholdAttributes *atts;
};



#endif
