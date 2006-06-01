/*****************************************************************************
*
* Copyright (c) 2000 - 2006, The Regents of the University of California
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

#ifndef QVIS_ANNOTATION_OBJECT_INTERFACE_H
#define QVIS_ANNOTATION_OBJECT_INTERFACE_H
#include <gui_exports.h>
#include <GUIBase.h>
#include <qgroupbox.h>

class AnnotationObject;
class QLineEdit;
class QSpinBox;
class QVBoxLayout;
class QvisScreenPositionEdit;

// ****************************************************************************
// Class: QvisAnnotationObjectInterface
//
// Purpose:
//   Base class for an annotation object interface.
//
// Notes:      
//
// Programmer: Brad Whitlock
// Creation:   Fri Oct 31 10:24:32 PDT 2003
//
// Modifications:
//   Brad Whitlock, Mon Mar 6 11:14:15 PDT 2006
//   Added a new helper method.
//
// ****************************************************************************

class GUI_API QvisAnnotationObjectInterface : public QGroupBox, public GUIBase
{
    Q_OBJECT
public:
    QvisAnnotationObjectInterface(QWidget *parent, const char *name = 0);
    virtual ~QvisAnnotationObjectInterface();

    virtual QString GetName() const = 0;
    virtual QString GetMenuText(const AnnotationObject &) const;
    virtual void Update(AnnotationObject *);

    virtual void GetCurrentValues(int which);
signals:
    void applyChanges();
    void setUpdateForWindow(bool);
protected:
    virtual void Apply();
    virtual void UpdateControls() = 0;
    void SetUpdate(bool);

    bool GetCoordinate(QLineEdit *le, double c[3], bool force2D);
    void GetPosition(QLineEdit *le, const QString &name);
    void GetScreenPosition(QvisScreenPositionEdit *spe, const QString &name);
    void GetPosition2(QLineEdit *le, const QString &name);
    void GetScreenPosition2(QvisScreenPositionEdit *spe, const QString &name);
    void ForceSpinBoxUpdate(QSpinBox *sb);

    QVBoxLayout      *topLayout;

    AnnotationObject *annot;
};

#endif
