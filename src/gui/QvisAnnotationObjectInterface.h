#ifndef QVIS_ANNOTATION_OBJECT_INTERFACE_H
#define QVIS_ANNOTATION_OBJECT_INTERFACE_H
#include <gui_exports.h>
#include <GUIBase.h>
#include <qgroupbox.h>

class AnnotationObject;
class QLineEdit;
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

    bool GetCoordinate(QLineEdit *le, float c[3], bool force2D);
    void GetPosition(QLineEdit *le, const QString &name);
    void GetScreenPosition(QvisScreenPositionEdit *spe, const QString &name);
    void GetPosition2(QLineEdit *le, const QString &name);

    QVBoxLayout      *topLayout;

    AnnotationObject *annot;
};

#endif
