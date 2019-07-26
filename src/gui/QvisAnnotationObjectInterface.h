// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ANNOTATION_OBJECT_INTERFACE_H
#define QVIS_ANNOTATION_OBJECT_INTERFACE_H
#include <gui_exports.h>
#include <GUIBase.h>
#include <QGroupBox>

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
//   Brad Whitlock, Tue Mar 20 15:16:06 PST 2007
//   Added AllowInstantiation.
//
// ****************************************************************************

class GUI_API QvisAnnotationObjectInterface : public QGroupBox, public GUIBase
{
    Q_OBJECT
public:
    QvisAnnotationObjectInterface(QWidget *parent);
    virtual ~QvisAnnotationObjectInterface();

    virtual QString GetName() const = 0;
    virtual QString GetMenuText(const AnnotationObject &) const;
    virtual void Update(AnnotationObject *);

    virtual void GetCurrentValues(int which);

    virtual bool AllowInstantiation() const { return true; }
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

    QVBoxLayout      *topLayout;

    AnnotationObject *annot;
};

#endif
