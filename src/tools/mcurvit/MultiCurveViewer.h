/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
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
#ifndef MULTI_CURVE_VIEWER_H
#define MULTI_CURVE_VIEWER_H
#include <QMainWindow>

#include <SimpleObserver.h>

class VisItViewer;

class QTabWidget;
class QWidget;

class vtkQtRenderWindow;
class QvisAnnotationWidget;
class QvisAppearanceWidget;
class QvisGeneralWidget;
class QvisMessageDialog;

class AnnotationAttributes;
class AnnotationObjectList;
class AttributeSubject;
class MessageAttributes;
class ViewAxisArrayAttributes;

// ****************************************************************************
// Class: MultiCurveViewer
//
// Purpose:
//   This application creates a multi curve viewer.  It only handles files
//   with 2d rectilinear node centered variables.  It applies an index select
//   operator to the data to allow the user to subselect portions of the
//   data. It also allows the user to control many of the viewing attributes
//   of the plot.
//
// Notes:      
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 12:39:46 PST 2009
//
// Modifications:
//   
// ****************************************************************************

class MultiCurveViewer : public QMainWindow, SimpleObserver
{
    Q_OBJECT
public:
    MultiCurveViewer(VisItViewer *);
    virtual ~MultiCurveViewer();

    virtual void Update(Subject *subject);

    void AttachSubjects();

public slots:
    virtual void show();
private slots:
    void selectFile();
    void variableChanged(const QString &);
    void indexSelectChanged(const AttributeSubject *atts);
    void multiCurveChanged(const AttributeSubject *atts);
    void annotationChanged(const AnnotationAttributes *atts);
    void viewAxisArrayChanged(const ViewAxisArrayAttributes *atts);
    void annotationObjectListChanged(const AnnotationObjectList *atts);
    void saveWindow();
    void openGUI();
    void finishPlot();
private:
    static vtkQtRenderWindow *ReturnVisWin(void *);
    void CreateWindow();
    void CreatePlot();
    void ResetWindow();

    QTabWidget            *tabs;
    QWidget               *pageGeneral;
    QvisGeneralWidget     *generalWidget;
    QWidget               *pageAppearance;
    QvisAppearanceWidget  *appearanceWidget;
    QWidget               *pageAnnotation;
    QvisAnnotationWidget  *annotationWidget;

    QvisMessageDialog     *messageDialog;
    
    vtkQtRenderWindow      *viswin;
    VisItViewer            *viewer;

    MessageAttributes      *messageAtts;
};

#endif
