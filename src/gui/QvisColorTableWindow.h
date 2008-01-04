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

#ifndef QVIS_COLOR_TABLE_WINDOW_H
#define QVIS_COLOR_TABLE_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>
#include <ColorTableObserver.h>

// Forward declarations
class ColorControlPointList;
class ColorTableAttributes;
class DataNode;
class QVBoxLayout;
class QPushButton;
class QButtonGroup;
class QCheckBox;
class QComboBox;
class QGroupBox;
class QLabel;
class QLineEdit;
class QListBox;
class QSlider;
class QSpinBox;
class QvisSpectrumBar;
class QvisColorSelectionWidget;
class QvisColorGridWidget;

// ****************************************************************************
// Class: QvisColorTableWindow
//
// Purpose:
//   This class contains the widgets that manipulate the color table.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Jun 8 09:58:12 PDT 2001
//
// Modifications:
//   Brad Whitlock, Wed Mar 13 17:59:26 PST 2002
//   Modified the popupColorSelect slot function.
//
//   Brad Whitlock, Wed Nov 20 15:12:35 PST 2002
//   I added support for discrete colortables.
//
//   Brad Whitlock, Wed Feb 26 11:09:16 PDT 2003
//   I changed things so that discrete color tables can have an arbitrary
//   number of colors.
//
//   Brad Whitlock, Tue Jul 1 16:37:41 PST 2003
//   I added an Export button.
//
//   Brad Whitlock, Mon Mar 6 09:09:46 PDT 2006
//   I added code to save the current color table to the settings.
//
// ****************************************************************************

class GUI_API QvisColorTableWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisColorTableWindow(ColorTableAttributes *volumeAtts_,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisColorTableWindow();
    virtual void CreateWindowContents();

    virtual void CreateNode(DataNode *);
    virtual void SetFromNode(DataNode *, const int *borders);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void UpdateEditor();
    void UpdateColorControlPoints();
    void UpdateDiscreteSettings();
    void UpdateNames();
    void Apply(bool ignore = false);
    void GetCurrentValues(int which_widget);
    const ColorControlPointList *GetActiveColorControlPoints() const;
          ColorControlPointList *GetActiveColorControlPoints();
    void ShowSelectedColor(const QColor &c);
    void ChangeSelectedColor(const QColor &c);
    void PopupColorSelect(const QColor &, const QPoint &p);
    QColor GetNextColor();

private slots:
    void resizeColorTable(int);
    void setColorTableType(int);
    void redValueChanged(int r);
    void greenValueChanged(int g);
    void blueValueChanged(int b);
    void activateDiscreteColor(const QColor &, int);
    void activateContinuousColor(int index);
    void chooseContinuousColor(int, const QPoint &);
    void chooseDiscreteColor(const QColor &, int, int, const QPoint &);
    void sliderPressed();
    void sliderReleased();
    void setActiveContinuous(const QString &ct);
    void setActiveDiscrete(const QString &ct);

    void alignControlPoints();
    void controlPointMoved(int index, float position);
    void selectedColor(const QColor &color);
    void smoothToggled(bool val);
    void equalSpacingToggled(bool val);
    void addColorTable();
    void deleteColorTable();
    void exportColorTable();
    void highlightColorTable(int index);
private:
    ColorTableAttributes     *colorAtts;
    int                      colorCycle;
    QString                  currentColorTable;
    int                      popupMode;
    bool                     sliding;

    // Widgets and layouts.
    QGroupBox                *activeGroup;
    QComboBox                *activeContinuous;
    QLabel                   *activeContinuousLabel;
    QComboBox                *activeDiscrete;
    QLabel                   *activeDiscreteLabel;

    QPushButton              *applyButton;
    QPushButton              *dismissButton;
    QGroupBox                *colorTableWidgetGroup;
    QPushButton              *newButton;
    QPushButton              *deleteButton;
    QPushButton              *exportButton;
    QLineEdit                *nameLineEdit;
    QListBox                 *nameListBox;

    QGroupBox                *colorWidgetGroup;

    QSpinBox                 *colorNumColors;
    QButtonGroup             *colorTableTypeGroup;

    QCheckBox                *smoothCheckBox;
    QCheckBox                *equalCheckBox;
    QvisSpectrumBar          *spectrumBar;
    QvisColorSelectionWidget *colorSelect;
//    QPushButton              *addPointButton;
//    QPushButton              *rmPointButton;
    QPushButton              *alignPointButton;

    QvisColorGridWidget      *discreteColors;
    QLabel                   *componentLabels[3];
    QSlider                  *componentSliders[3];
    QSpinBox                 *componentSpinBoxes[3];

    // This object also observes the color table attributes.
    ColorTableObserver       ctObserver;
    QString                  activeCT;
};

#endif
