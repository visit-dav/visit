#ifndef STATUS_SUBJECT_H
#define STATUS_SUBJECT_H
#include <Subject.h>
#include <qstring.h>

// *******************************************************************
// Class: StatusSubject
//
// Purpose:
//   This struct contains attributes that describe a message to put
//   on the main window's statusbar.
//
// Notes:      
//   It's a Subject instead of an AttributeSubject because I don't
//   care about partial selection and it does not go across the network.
//
// Programmer: Brad Whitlock
// Creation:   Sat Sep 16 14:25:45 PST 2000
//
// Modifications:
//   
// *******************************************************************

struct StatusSubject : public Subject
{
    QString text;
    int     milliseconds;
    bool    clearing;

    StatusSubject() : Subject(), text() {milliseconds=10000; clearing=false;};
    virtual ~StatusSubject() { };
};

#endif
