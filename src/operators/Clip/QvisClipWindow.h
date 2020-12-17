// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_CLIP_WINDOW_H
#define QVIS_CLIP_WINDOW_H
#include <QvisOperatorWindow.h>
#include <QGroupBox>

class QCheckBox;
class QButtonGroup;
class QLineEdit;
class ClipAttributes;


// ****************************************************************************
// Class: QPlaneGroup
//
// Purpose: 
//   Widget that encapsualtes the options for a single clipping plane.
//
// Programmer: Cyrus Harrison
// Creation:   Thu Aug 21 13:45:03 PDT 2008
//
// Modifications:
//
// ****************************************************************************
class QPlaneGroup: public QGroupBox
{
Q_OBJECT
public:
    QPlaneGroup(const QString &title,QWidget *parent=0);
    
    virtual ~QPlaneGroup();
    
    void SetOrigin(double val[3]);
    void SetNormal(double val[3]);
    bool GetOrigin(double val[3]);
    bool GetNormal(double val[3]);

signals:
    void OriginChanged();
    void NormalChanged();
    
private:
    QLineEdit *origin;
    QLineEdit *normal;
    
};


// ****************************************************************************
// Class: QvisClipWindow
//
// Purpose:
//   This class is a postable window that watches clip operator
//   attributes and always represents their current state.
//
// Notes:      
//
// Programmer: Kathleen Bonnell 
// Creation:   May 7, 2001 
//
// Modifications:
//   Brad Whitlock, Fri Apr 12 13:04:35 PST 2002
//   Made it inherit from QvisOperatorWindow.
//
//   Kathleen Bonnell, Mon Dec  6 14:35:14 PST 2004 
//   Made plane*Status be checkable QVGroupBox, instead of QButtonGroup.
//   Renamed plane*StatusCliced slots to plane*StatusToggled.
//
//   Brad Whitlock, Tue Dec 21 09:13:49 PDT 2004
//   Added Qt version-specific coding so we can still use versions older than
//   3.2.
//
//   Cyrus Harrison, Wed Mar  5 10:25:39 PST 2008
//   Removed tabWidget and slot for tabWidgetChanged 
//   (to Match Sean's changes in QvisClipWindow.C)
//
//   Cyrus Harrison, Thu Aug 21 09:48:43 PDT 2008
//   Qt4 Port.
//
//   Alister Maguire, Fri Nov 13 14:07:54 PST 2020
//   Added support for the crinkle clip.
//
// ****************************************************************************

class QvisClipWindow : public QvisOperatorWindow
{
    Q_OBJECT
public:
    QvisClipWindow(const int type,
                    ClipAttributes *subj,
                    const QString &caption = QString(),
                    const QString &shortName = QString(),
                    QvisNotepadArea *notepad = 0);
    virtual ~QvisClipWindow();
protected:
    virtual void CreateWindowContents();
    void UpdateWindow(bool doAll);
    virtual void GetCurrentValues(int which_widget);
private slots:
    void processPlane1Origin();
    void processPlane2Origin();
    void processPlane3Origin();
    void processPlane1Normal();
    void processPlane2Normal();
    void processPlane3Normal();
    void processCenterText();
    void processRadiusText();
    void planeInverseToggled(bool);
    void planeToolControlledClipPlaneChanged(int val);
    void qualityChanged(int);
    void sliceTypeChanged(int);
    void sphereInverseToggled(bool);
    void plane1StatusToggled(bool);
    void plane2StatusToggled(bool);
    void plane3StatusToggled(bool);
    void crinkleClipToggled(bool);

private:
    QButtonGroup *qualityGroup;
    QButtonGroup *typeGroup;
    
    QPlaneGroup  *plane1Group;
    QPlaneGroup  *plane2Group;
    QPlaneGroup  *plane3Group;
    
    QLineEdit    *centerLineEdit;
    QLineEdit    *radiusLineEdit;
    QCheckBox    *planeInverse;
    QButtonGroup *planeToolControlledClipPlane;
    QCheckBox    *sphereInverse;
    QWidget      *planeWidgets;
    QWidget      *sphereWidgets;

    ClipAttributes *atts;
};
#endif
