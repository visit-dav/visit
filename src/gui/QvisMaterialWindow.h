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

#ifndef QVISMATERIALWINDOW_H
#define QVISMATERIALWINDOW_H

#include <AttributeSubject.h>
#include <QvisPostableWindowObserver.h>

class MaterialAttributes;
class QLabel;
class QCheckBox;
class QNarrowLineEdit;
class QButtonGroup;
class QComboBox;

// ****************************************************************************
// Class: QvisMaterialWindow
//
// Purpose: 
//   Defines QvisMaterialWindow class.
//
// Programmer: Jeremy Meredith
// Creation:   October 24, 2002
//
// Modifications:
//    Jeremy Meredith, Wed Jul 30 10:46:04 PDT 2003
//    Added the toggle for forcing full connectivity.
//
//    Jeremy Meredith, Fri Sep  5 15:17:20 PDT 2003
//    Added a toggle for the new MIR algorithm.
//
//    Hank Childs, Tue Aug 16 15:36:43 PDT 2005
//    Add a toggle for "simplify heavily mixed zones".
//
//    Jeremy Meredith, Thu Aug 18 16:14:59 PDT 2005
//    Changed algorithm selection to a multiple-choice.
//    Added VF for isovolume method.
//
//    Brad Whitlock, Wed Apr  9 11:36:37 PDT 2008
//    QString for caption, shortName.
//
//    Jeremy Meredith, Fri Feb 13 12:11:07 EST 2009
//    Added material iteration capability.
//
// ****************************************************************************

class QvisMaterialWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisMaterialWindow(MaterialAttributes *subj,
                       const QString &caption = QString::null,
                       const QString &shortName = QString::null,
                       QvisNotepadArea *notepad = 0);
    virtual ~QvisMaterialWindow();
    virtual void CreateWindowContents();

  public slots:
    virtual void apply();
    virtual void makeDefault();
    virtual void reset();
  protected:
    void UpdateWindow(bool doAll);
    void Apply(bool iIgnore = false);
    void GetCurrentValues(int which_widget);
  private slots:
    void smoothingChanged(bool val);
    void forceFullConnectivityChanged(bool val);
    void forceMIRChanged(bool val);
    void cleanZonesOnlyChanged(bool val);
    void algorithmChanged(int val);
    void simplifyHeavilyMixedZonesChanged(bool val);
    void maxMatsPerZoneProcessText(void);
    void isoVolumeFractionProcessText(void);
    void numIterationsProcessText(void);
    void iterationDampingProcessText(void);
    void enableIterationChanged(bool val);
  private:
    MaterialAttributes *atts;

    QCheckBox *smoothing;
    QCheckBox *forceFullConnectivity;
    QCheckBox *forceMIR;
    QCheckBox *cleanZonesOnly;
    QComboBox *algorithm;
    QCheckBox *simplifyHeavilyMixedZones;
    QNarrowLineEdit *maxMatsPerZone;
    QNarrowLineEdit *isoVolumeFraction;
    QCheckBox *enableIteration;
    QNarrowLineEdit *numIterations;
    QNarrowLineEdit *iterationDamping;
    QLabel *maxMatsPerZoneLabel;
    QLabel *algorithmLabel;
    QLabel *isoVolumeFractionLabel;
    QLabel *numIterationsLabel;
    QLabel *iterationDampingLabel;
};

#endif
