#ifndef QVIS_GLOBALLINEOUT_WINDOW_H
#define QVIS_GLOBALLINEOUT_WINDOW_H
#include <gui_exports.h>
#include <QvisPostableWindowObserver.h>

class GlobalLineoutAttributes;
class QCheckBox;

// ****************************************************************************
// Class: QvisGlobalLineoutWindow
//
// Purpose: 
//   Defines QvisGlobalLineoutWindow class.
//
// Notes:      This class was automatically generated!

// Programmer: xml2window
// Creation:   Mon Jan 13 15:34:51 PST 2003
//
// Modifications:
//   
// ****************************************************************************

class GUI_API QvisGlobalLineoutWindow : public QvisPostableWindowObserver
{
    Q_OBJECT
  public:
    QvisGlobalLineoutWindow(
                         GlobalLineoutAttributes *subj,
                         const char *caption = 0,
                         const char *shortName = 0,
                         QvisNotepadArea *notepad = 0);
    virtual ~QvisGlobalLineoutWindow();
    virtual void CreateWindowContents();
  public slots:
    virtual void apply();
  protected:
    void UpdateWindow(bool doAll);
    void GetCurrentValues(int which_widget);
    void Apply(bool ignore = false);
  private slots:
    void DynamicChanged(bool val);
  private:
    QCheckBox *Dynamic;

    GlobalLineoutAttributes *atts;
};



#endif
