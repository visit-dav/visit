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
//    Kathleen Biagas, Wed Apr 6, 2022
//    Fix QT_VERSION test to use Qt's QT_VERSION_CHECK.
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        int w = fm.horizontalAdvance("0");
#else
        int w = fm.width("0");
#endif
        size.setWidth(w * 4); // 4 characters
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
#if (QT_VERSION >= QT_VERSION_CHECK(5, 11, 0))
        int w = fm.horizontalAdvance("0");
#else
        int w = fm.width("0");
#endif
        size.setWidth(w * 6); // 6 characters
        return size;
    }
};
