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

#ifndef QVIS_SAVE_WINDOW_H
#define QVIS_SAVE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

// Forward declarations.
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QPushButton;
class QSlider;
class SaveWindowAttributes;
class QRadioButton;
class QButtonGroup;

// ****************************************************************************
// Class: QvisSaveWindow
//
// Purpose:
//   This class contains the code necessary to create a window that
//   observes the save image attributes.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Feb 9 16:46:07 PST 2001
//
// Modifications:
//   Brad Whitlock, Wed Jan 23 15:13:23 PST 2002
//   Added quality and progressive toggles.
//
//   Hank Childs, Fri May 24 07:45:52 PDT 2002
//   Renamed SaveImageAtts to SaveWindowAtts.  Added support for STL.
//
//   Hank Childs, Sun May 26 17:31:18 PDT 2002
//   Added a toggle for binary vs ASCII.
//
//   Kathleen Bonnell, Thu Nov 13 12:14:30 PST 2003 
//   Added combo box for compression type. 
//
//   Brad Whitlock, Fri Jul 30 15:14:44 PST 2004
//   Added save path and a slot to save the image.
//
//   Kathleen Bonnell, Wed Dec 15 08:20:11 PST 2004 
//   Added slot 'saveButtonClicked'. 
//
//   Jeremy Meredith, Thu Apr  5 17:23:37 EDT 2007
//   Added button to force a merge of parallel geometry.
//
// ****************************************************************************

class GUI_API QvisSaveWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisSaveWindow(SaveWindowAttributes *subj, const char *caption = 0,
                   const char *shortName = 0,
                   QvisNotepadArea *notepad = 0);
    virtual ~QvisSaveWindow();
public slots:
    virtual void apply();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
protected slots:
    void outputToCurrentDirectoryToggled(bool);
    void processOutputDirectoryText();
    void processFilenameText();
    void familyToggled(bool);
    void stereoToggled(bool);
    void fileFormatChanged(int);
    void resConstraintToggled(bool);
    void processWidthText();
    void processHeightText();
    void progressiveToggled(bool);
    void binaryToggled(bool);
    void qualityChanged(int);
    void screenCaptureToggled(bool);
    void saveTiledToggled(bool);
    void compressionTypeChanged(int);
    void saveWindow();
    void selectOutputDirectory();
    void saveButtonClicked();
    void forceMergeToggled(bool);
private:
    QCheckBox           *outputToCurrentDirectoryCheckBox;
    QLabel              *outputDirectoryLabel;
    QLineEdit           *outputDirectoryLineEdit;
    QPushButton         *outputDirectorySelectButton;
    QLineEdit           *filenameLineEdit;
    QCheckBox           *familyCheckBox;
    QCheckBox           *stereoCheckBox;
    QComboBox           *fileFormatComboBox;
    QLabel              *qualityLabel;
    QSlider             *qualitySlider;
    QLabel              *compressionTypeLabel;
    QComboBox           *compressionTypeComboBox;
    QCheckBox           *progressiveCheckBox;
    QCheckBox           *binaryCheckBox;
    QGroupBox           *resolutionBox;
    QButtonGroup        *resConstraintButtonGroup;
    QRadioButton        *noResButton;
    QRadioButton        *oneToOneResButton;
    QRadioButton        *screenResButton;
    QLineEdit           *widthLineEdit;
    QLineEdit           *heightLineEdit;
    QCheckBox           *screenCaptureCheckBox;
    QCheckBox           *saveTiledCheckBox;
    QCheckBox           *forceMergeCheckBox;
    SaveWindowAttributes  *saveWindowAtts;
};

#endif
