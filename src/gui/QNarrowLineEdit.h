// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include <QLineEdit>

// ****************************************************************************
//  Class:  QNarrowLineEdit
//
//  Purpose:
//    A QLineEdit that has a narrower default size.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
//  Modifications:
//    Eric Brugger, Mon Aug 31 10:26:57 PDT 2015
//    I added QSixCharLineEdit.
//
// ****************************************************************************
class QNarrowLineEdit : public QLineEdit
{
  public:
    QNarrowLineEdit(QWidget *p)
        : QLineEdit(p)
    {
    }
    QNarrowLineEdit(const QString &s, QWidget *p)
        : QLineEdit(s, p)
    {
    }
    QSize sizeHint() const
    {
        QSize size = QLineEdit::sizeHint();
        QFontMetrics fm(font());
        int w = fm.horizontalAdvance('0') * 4; // 4 characters
        size.setWidth(w);
        return size;
    }
};

class QSixCharLineEdit : public QLineEdit
{
  public:
    QSixCharLineEdit(QWidget *p)
        : QLineEdit(p)
    {
    }
    QSixCharLineEdit(const QString &s, QWidget *p)
        : QLineEdit(s, p)
    {
    }
    QSize sizeHint() const
    {
        QSize size = QLineEdit::sizeHint();
        QFontMetrics fm(font());
        int w = fm.horizontalAdvance('0') * 6; // 6 characters
        size.setWidth(w);
        return size;
    }
};
