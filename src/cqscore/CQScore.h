#ifndef CQ_SCORE_H
#define CQ_SCORE_H

#include <qmainwindow.h>
#include <qstring.h>

class QButtonGroup;
class QComboBox;
class QLineEdit;
class QListView;
class QListViewItem;
class QTextEdit;

#include <vector>

// ****************************************************************************
//  Class:  CQScore
//
//  Purpose:
//    Main window for the clearquest scoring tool.
//
//  Programmer:  Jeremy Meredith
//  Creation:    July 25, 2005
//
// ****************************************************************************
class CQScore : public QMainWindow
{
    Q_OBJECT
  public:
    CQScore(QWidget *p, const QString &n);
  public slots:
    void saveExcel();
    void exportHTML();
    void writeCurve();
    void CalculatePercentile(class ResultData*);
    void RepopulateResults();
    void PopulateResults();
    void PopulatePercentileCurve();
    void PopulatePercentileCurve(std::vector<float>&);
    void PopulateSingleResultText(QListViewItem*);
    void PopulateScoringRuleChoices();
  protected:
  private:
    QLineEdit *userEdit;
    QLineEdit *passEdit;
    QComboBox *dbCombo;

    QComboBox *scoreRuleCombo;

    QButtonGroup *scoreGroup;
    QLineEdit *projectEdit;
    QLineEdit *dateEdit;
    QLineEdit *stateEdit;
    QLineEdit *vertgtEdit;
    QLineEdit *verresEdit;
    QLineEdit *ownerEdit;
    QLineEdit *idEdit;

    QListView *resultList;
    QTextEdit *resultText;
};

#endif
