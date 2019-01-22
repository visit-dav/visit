/*****************************************************************************
*
* Copyright (c) 2000 - 2019, Lawrence Livermore National Security, LLC
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
*****************************************************************************/

#ifndef QVIS_CINEMA_WIZARD_H
#define QVIS_CINEMA_WIZARD_H
#include <QvisWizard.h>
#include <map>
#include <vectortypes.h>

class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QSpinBox;
class QTextEdit;
class QVBoxLayout;

// ****************************************************************************
// Class: QvisCinemaWizard
//
// Purpose: 
//   This class contains the "Save Cinema" wizard that leads the user through
//   all of the questions needed to save to Cinema.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 14 13:57:01 PDT 2017
//
// Modifications:
//   Kathleen Biagas, Thursday December 13, 2018
//   Changed signature for page0_processFileName, so the lineedit can connect
//   to 'returnPressed' and 'editingFinished' signals. The textChanged signal
//   is processed every time a character is entered, which is excessive for
//   this use case.
//
// ****************************************************************************

class QvisCinemaWizard : public QvisWizard
{
    Q_OBJECT
public:
    QvisCinemaWizard(AttributeSubject *, QWidget *parent);
    virtual ~QvisCinemaWizard();

    int Exec();
    void SetDefaultImageSize(int,int);
    void SetDefaultNumFrames(int);
    int  GetDefaultNumFrames();

    virtual int nextId() const;
    virtual bool validateCurrentPage();

    QString FullyQualified(const QString &filename) const;
protected:
    virtual void initializePage(int id);
private slots:
    void page0_specificationChanged(int);
    void page0_processFileName();
    void page0_selectFileName();
    void page0_formatActivated(int);
    void page0_compositeStateChanged(int);
    void page0_screenCaptureStateChanged(int);
    void page0_widthValueChanged(int);
    void page0_heightValueChanged(int);

    void page0_cameraModeActivated(int);
    void page0_thetaValueChanged(int);
    void page0_phiValueChanged(int);

    void page1_frameStartValueChanged(int);
    void page1_frameEndValueChanged(int);
    void page1_frameStrideValueChanged(int);

private:
    QWidget *CreateFilenameControl(QWidget *parent);

    enum {
        Page_Main,                 // page0
        Page_NumFrames             // page1
    };

    void CreateMainPage();         // page0
    void CreateNumFramesPage();    // page1

    float                   default_image_size[2];
    int                     default_num_frames;

    // page 0
    QWizardPage            *page0;
    QLineEdit              *page0_fileNameLineEdit;
    QButtonGroup           *page0_specification;

    QComboBox              *page0_format;
    QCheckBox              *page0_composite;
    QCheckBox              *page0_screenCapture;
    QLabel                 *page0_widthLabel;
    QSpinBox               *page0_width;
    QLabel                 *page0_heightLabel;
    QSpinBox               *page0_height;

    QComboBox              *page0_cameraMode;
    QLabel                 *page0_thetaLabel;
    QSpinBox               *page0_theta;
    QLabel                 *page0_phiLabel;
    QSpinBox               *page0_phi;

    // page 1
    QWizardPage            *page1;
    QSpinBox               *page1_frameStart;
    QSpinBox               *page1_frameEnd;
    QSpinBox               *page1_frameStride;
};

#endif
