// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_ELEMENT_BUTTON_H
#define QVIS_ELEMENT_BUTTON_H
#include <gui_exports.h>
#include <QColor>
#include <QPushButton>
#include <vector>

class QPainter;
class QMenu;
class QvisPeriodicTableWidget;
class QvisElementSelectionWidget;

// ****************************************************************************
// Class: QvisElementButton
//
// Purpose:
//   This class represents a color button widget that can be used to select
//   colors for materials, isocontours, and other items in the gui.
//
// Notes:      
//
// Programmer: Jeremy Meredith
// Creation:   August 29, 2006
//
// Notes: Taken largely from QvisColorButton
//
// Modifications:
//    Jeremy Meredith, Mon Feb 11 16:46:57 EST 2008
//    Changed to use the element selection widget instead of the
//    simple periodic table widget; the former was created to contain
//    both a periodic table widget and a "match any element" button
//    to allow wildcards.
//   
//    Jeremy Meredith, Tue Feb 12 14:01:52 EST 2008
//    Added support for hinting some elements to the user, e.g. to highlight
//    the elements that are actually in the database.
//
//    Brad Whitlock, Tue Jun  3 14:43:22 PDT 2008
//    Qt 4.
//
// ****************************************************************************

class GUI_API QvisElementButton : public QPushButton
{
    Q_OBJECT

    typedef std::vector<QvisElementButton *> ElementButtonVector;
public:
    QvisElementButton(QWidget *parent = 0, const void *userData = 0);
    virtual ~QvisElementButton();
    virtual QSize sizeHint() const;
    virtual QSizePolicy sizePolicy () const;

    int elementNumber() const;
    void setElementNumber(int);
    void setHintedElements(const std::vector<int>&);

signals:
    void selectedElement(int element);
protected:
private slots:
    void popupPressed();
    void elementSelected(int element);
private:
    int                                number;
    const void                        *userData;

    static QvisElementSelectionWidget *sharedpopup;
    static ElementButtonVector         buttons;
};

#endif
