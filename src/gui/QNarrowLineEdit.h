#include <qlineedit.h>

// ****************************************************************************
//  Class:  QNarrowLineEdit
//
//  Purpose:
//    A QLineEdit that has a narrower default size.
//
//  Programmer:  Jeremy Meredith
//  Creation:    September 25, 2001
//
// ****************************************************************************
class QNarrowLineEdit : public QLineEdit
{
  public:
    QNarrowLineEdit(QWidget *p, const char *n=0)
        : QLineEdit(p, n)
    {
    }
    QNarrowLineEdit(const QString &s, QWidget *p, const char *n=0)
        : QLineEdit(s, p, n)
    {
    }
    QSize sizeHint() const
    {
        QSize size = QLineEdit::sizeHint();
        QFontMetrics fm(font());
        int w = fm.width('x') * 4; // 4 characters
        size.setWidth(w);
        return size;
    }
};

