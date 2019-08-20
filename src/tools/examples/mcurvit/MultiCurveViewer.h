// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

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
