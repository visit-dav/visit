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

#ifndef QVIS_SLICE_WINDOW_H
#define QVIS_SLICE_WINDOW_H
#include <QvisOperatorWindow.h>

#include <QNarrowLineEdit.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QLineEdit;
class QLabel;
class QButtonGroup;
class SliceAttributes;
class QSlider;
class QGridLayout;
class QHBoxLayout;

// ****************************************************************************
// Class: QvisSliceWindow
//
// Purpose:
//   This class is a postable window that watches slice operator
//   attributes and always represents their current state.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Aug 4 10:13:05 PDT 2000
//
// Modifications:
//   Jeremy Meredith, Wed Sep  5 15:50:26 PDT 2001
//   Renamed to match plugin name.
//
//   Brad Whitlock, Tue Oct 9 17:35:43 PST 2001
//   Added an "interactive" toggle.
//
//   Brad Whitlock, Tue Feb 26 14:18:00 PST 2002
//   Added a new slot function.
//
//   Brad Whitlock, Fri Apr 12 12:58:39 PDT 2002
//   Made it inherit from QvisOperatorWindow.
//
//   Brad Whitlock, Mon Nov 18 10:49:05 PDT 2002
//   Added upAxisLabel.
//
//   Jeremy Meredith, Mon May  5 14:47:53 PDT 2003
//   Reworked a lot of stuff for changes to how origin is handled.
//
//   Jeremy Meredith, Fri Jun 13 12:08:17 PDT 2003
//   Added a domain number for slice-by-zone and -by-node.
//
//   Kathleen Bonnell, Tue Jan 25 07:59:28 PST 2005 
//   Added a meshName comboBox to be associated with domain-node/zone. 
//
// ****************************************************************************

class QvisSliceWindow : public QvisOperatorWindow
{
    Q_OBJECT
public:
    QvisSliceWindow(const int type,
                    SliceAttributes *subj,
                    const char *caption = 0,
                    const char *shortName = 0,
                    QvisNotepadArea *notepad = 0);
    virtual ~QvisSliceWindow();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
    void UpdateOriginArea();
private slots:
    void processNormalText();
    void processUpAxisText();
    void projectToggled(bool);
    void interactiveToggled(bool);
    void flipNormalToggled(bool);
    void normalTypeChanged(int index);
    void originTypeChanged(int index);

    void processOriginPointText();
    void processOriginInterceptText();
    void processOriginPercentText();
    void processOriginZoneText();
    void processOriginNodeText();
    void processOriginZoneDomainText();
    void processOriginNodeDomainText();

    void originPercentSliderPressed();
    void originPercentSliderReleased();
    void originPercentSliderChanged(int pct);

    void meshNameChanged();
private:
    void             UpdateMeshNames();

    QCheckBox        *interactiveToggle;

    QButtonGroup     *normalTypeGroup;
    QLineEdit        *normalLineEdit;
    QCheckBox        *flipNormalToggle;

    QButtonGroup     *originTypeGroup;
    QHBoxLayout      *originPointLayout;
    QLineEdit        *originPointLineEdit;
    QHBoxLayout      *originInterceptLayout;
    QLineEdit        *originInterceptLineEdit;
    QHBoxLayout      *originPercentLayout;
    QNarrowLineEdit  *originPercentLineEdit;
    QHBoxLayout      *originZoneLayout;
    QLineEdit        *originZoneLineEdit;
    QLineEdit        *originZoneDomainLineEdit;
    QHBoxLayout      *originNodeLayout;
    QLineEdit        *originNodeLineEdit;
    QLineEdit        *originNodeDomainLineEdit;

    QSlider          *originPercentSlider;
    bool              sliderDragging;

    QLabel           *originPointLabel;
    QLabel           *originInterceptLabel;
    QLabel           *originPercentLabel;
    QLabel           *originZoneLabel;
    QLabel           *originNodeLabel;
    QLabel           *originZoneDomainLabel;
    QLabel           *originNodeDomainLabel;

    QCheckBox        *projectToggle;
    QLineEdit        *upAxisLineEdit;
    QLabel           *upAxisLabel;

    QLabel           *meshLabel;
    QComboBox        *meshName;

    QString           defaultItem;

    SliceAttributes  *sliceAtts;
};
#endif
