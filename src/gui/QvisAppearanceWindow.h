#ifndef QVISAPPEARANCEWINDOW_H
#define QVISAPPEARANCEWINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>
#include <AttributeSubject.h>

// Forward declarations.
class AppearanceAttributes;
class QComboBox;
class QvisColorButton;

// ****************************************************************************
// Class: QvisAppearancePlotWindow
//
// Purpose: 
//   This class creates a window that has controls for changing GUI color,
//   style, and font.
//
// Programmer: Brad Whitlock
// Creation:   Thu Sep 6 12:23:10 PDT 2001
//
// Modifications:
//   Brad Whitlock, Tue Jan 29 13:15:07 PST 2002
//   Added orientation combo box.
//
// ****************************************************************************

class GUI_API QvisAppearanceWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
public:
    QvisAppearanceWindow(AppearanceAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisAppearanceWindow();
    virtual void CreateWindowContents();
signals:
    void changeAppearance(bool);
public slots:
    virtual void apply();
protected:
    void UpdateWindow(bool doAll);
    void Apply(bool ignore = false);
private slots:
    void backgroundChanged(const QColor &bg);
    void foregroundChanged(const QColor &fg);
    void styleChanged(int index);
    void handleFontClicked();
    void orientationChanged(int index);
private:
    QvisColorButton *backgroundColorButton;
    QvisColorButton *foregroundColorButton;
    QComboBox       *styleComboBox;
    QComboBox       *orientationComboBox;
};

#endif
