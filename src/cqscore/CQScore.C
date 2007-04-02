/*****************************************************************************
*
* Copyright (c) 2000 - 2007, The Regents of the University of California
* Produced at the Lawrence Livermore National Laboratory
* All rights reserved.
*
* This file is part of VisIt. For details, see http://www.llnl.gov/visit/. The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or materials provided with the distribution.
*  - Neither the name of the UC/LLNL nor  the names of its contributors may be
*    used to  endorse or  promote products derived from  this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED.  IN  NO  EVENT  SHALL  THE  REGENTS  OF  THE  UNIVERSITY OF
* CALIFORNIA, THE U.S.  DEPARTMENT  OF  ENERGY OR CONTRIBUTORS BE  LIABLE  FOR
* ANY  DIRECT,  INDIRECT,  INCIDENTAL,  SPECIAL,  EXEMPLARY,  OR CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#include "CQScore.h"

#include <qbuttongroup.h>
#include <qcombobox.h>
#include <qfiledialog.h>
#include <qgroupbox.h>
#include <qlabel.h>
#include <qlayout.h>
#include <qlineedit.h>
#include <qlistview.h>
#include <qmenubar.h>
#include <qmessagebox.h>
#include <qpopupmenu.h>
#include <qpushbutton.h>
#include <qradiobutton.h>
#include <qscrollview.h>
#include <qsplitter.h>
#include <qtextedit.h>

#include <vector>
#include <string>
#include <map>
#include <algorithm>

#include <visitstream.h>

#include <stdlib.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>

using namespace std;

const int id_result_column = 2;


static vector<string> GetDatabaseList()
{
    char *cqdir = "/usr/local/rational/base/ClearQuest_Databases/2002.05.00/2002.05.00";

    char command[2000];
    sprintf(command, "/bin/ls -1 %s", cqdir);

    vector<string> dblist;

    FILE *file = popen(command, "r");
    while (!feof(file))
    {
        char line[1000];
        if (fgets(line, 1000, file))
        {
            if (line[strlen(line)-1] == '\n')
            {
                line[strlen(line)-1] = '\0';
            }

            string s = line;
            if (!s.empty())
            {
                dblist.push_back(s);
            }
        }
    }
    pclose(file);
    return dblist;
}

static string ReadFileToString(const char *filename)
{
    string result = "";

    FILE *file = fopen(filename, "r");
    if (!file)
        return result;

    while (!feof(file))
    {
        char line[1000];
        if (fgets(line, 1000, file))
        {
            if (line[strlen(line)-1] == '\n')
            {
                line[strlen(line)-1] = '\0';
            }

            result += line;
            result += "\n";
        }
    }
    fclose(file);
    return result;
}

void CQScore::PopulateScoringRuleChoices()
{
    char outfile[1000];
    char errfile[1000];
    sprintf(outfile, "/tmp/cqscoretmp_rules_outfile%d", getpid());
    sprintf(errfile, "/tmp/cqscoretmp_rules_errfile%d", getpid());

    QString cquser = userEdit->text();
    QString cqpass = passEdit->text();
    QString cqdb   = dbCombo->currentText();

    if (cquser.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a username.");
        return;
    }

    if (cqpass.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a password.");
        return;
    }

    if (cqdb.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a database.");
        return;
    }

    char command[2000];
    sprintf(command, "getscoringrules.pl -u %s -p %s -db %s 1>%s 2>%s",
            cquser.latin1(), cqpass.latin1(), cqdb.latin1(),
            outfile, errfile);

    scoreRuleCombo->clear();
    int err = system(command);

    if (err)
    {
        string errstring = ReadFileToString(errfile);
        if (errstring.empty())
            QMessageBox::warning(this, "Failed", "Unknown reason.");
        else
            QMessageBox::warning(this, "Failed", errstring.c_str());
        unlink(outfile);
        unlink(errfile);
        scoreRuleCombo->setCurrentText("Default");
        scoreRuleCombo->setEnabled(false);
        return;
    }

    scoreRuleCombo->setEnabled(true);

    scoreRuleCombo->clear();
    FILE *file = fopen(outfile, "r");
    if (!file)
    {
        QMessageBox::warning(this, "Failed", "Failure.  No output generated.");
        unlink(outfile);
        unlink(errfile);
        return;
    }

    while (!feof(file))
    {
        char line[1000];
        if (fgets(line, 1000, file))
        {
            if (strlen(line) >= 1 && line[strlen(line)-1] == '\n')
            {
                line[strlen(line)-1] = '\0';
            }

            if (strlen(line) > 0)
            {
                scoreRuleCombo->insertItem(line);
            }
        }
    }
    fclose(file);

    unlink(outfile);
    unlink(errfile);
}

int num_visible_columns = 6;
char *result_columns[] = {
    "Score",
    "Percentile",
    "ID",
    "State",
    "Headline",
    "Owner",
    "Li",
    "TC",
    "Se",
    "IU",
    "ID",
    "TG",
    "EF",
    "CP",
    "Bonus",
    "Version Targeted",
    "Version Resolved",
    "Description",
    NULL
};

int ncolumns = 17;
struct ResultData
{
    float  score;
    float  percentile;
    string ident;
    string state;
    string headl;
    string owner;
    int    li;
    int    tc;
    int    se;
    int    iu;
    int    id;
    int    tg;
    int    ef;
    int    cp;
    string descr;
    float  bonus;
    string vertgt;
    string verres;

  public:
    void AddToListView(QListView *resultList)
    {
        QListViewItem *item = new QListViewItem(resultList);
        char scorestr[100];
        if (score < 10)
            sprintf(scorestr,"  %.2f", score);
        else if (score < 100)
            sprintf(scorestr," %.2f", score);
        else
            sprintf(scorestr,"%.2f", score);

        char percentilestr[100];
        sprintf(percentilestr,"% 2d", int(percentile));

        item->setText(0, scorestr);
        if (percentile < 0)
            item->setText(1, "?");
        else
            item->setText(1, percentilestr);
        item->setText(2, ident);
        item->setText(3, state);
        item->setText(4, headl);
        item->setText(5, owner);
        resultList->insertItem(item);
    }
    static ResultData *ReadFromFile(CQScore *cqscore, FILE *file)
    {
        int column = 0;
        static char *buff = new char[1000000];
        int   pos = 0;
        ResultData *result = new ResultData;

        while (!feof(file))
        {
            char c = fgetc(file);
            if (c == '\t')
            {
                buff[pos] = '\0';
                pos = 0;
                switch (column)
                {
                  case  0: result->score   = atof(buff); break;
                  case  1: result->ident   = buff;       break;
                  case  2: result->state   = buff;       break;
                  case  3: result->headl   = buff;       break;
                  case  4: result->owner   = buff;       break;
                  case  5: result->li      = atoi(buff); break;
                  case  6: result->tc      = atoi(buff); break;
                  case  7: result->se      = atoi(buff); break;
                  case  8: result->iu      = atoi(buff); break;
                  case  9: result->id      = atoi(buff); break;
                  case 10: result->tg      = atoi(buff); break;
                  case 11: result->ef      = atoi(buff); break;
                  case 12: result->cp      = atoi(buff); break;
                  case 13: result->bonus   = atof(buff); break;
                  case 14: result->vertgt  = buff;       break;
                  case 15: result->verres  = buff;       break;
                  case 16: result->descr   = buff;       break;
                  default:
                    QMessageBox::warning(cqscore, "Failed",
                                         "Too many columns in result.");
                    return NULL;
                    
                }
                column++;
                if (column == ncolumns)
                {
                    if (fgetc(file) != '\n')
                    {
                        QMessageBox::warning(cqscore, "Failed",
                                             "Expected EOL between result items.");
                        return NULL;
                    }
                    return result;
                }
            }
            else
            {
                buff[pos++] = c;
            }
        }
        return NULL;
    }
};

map<QString, ResultData*> currentResults;
vector<float> percentileCurve;

void CQScore::PopulateSingleResultText(QListViewItem *item)
{
    QString ident = item->text(id_result_column);

    ResultData *result = currentResults[ident];
    if (!result)
    {
        resultText->setText(QString().sprintf("Unknown item '%s'",ident.latin1()));
    }
    else
    {
        int myindex = 1;
        QListViewItem *it = resultList->firstChild();
        while (it && it != item)
        {
            myindex++;
            it = it->nextSibling();
        }

        resultText->clear();

        resultText->setBold(false);
        resultText->setItalic(true);
        resultText->insert(QString().sprintf("Item at index %d\n",myindex));
        resultText->setItalic(false);

        resultText->setBold(true);
        resultText->insert("Ident");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %s    ", result->ident.c_str()));

        resultText->setBold(true);
        resultText->insert("State");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %s    ", result->state.c_str()));

        resultText->setBold(true);
        resultText->insert("Owner");
        resultText->setBold(false);
        if (result->owner.empty())
            resultText->insert(QString().sprintf(": %s    ", "<none>"));
        else
            resultText->insert(QString().sprintf(": %s    ", result->owner.c_str()));

        resultText->setBold(true);
        resultText->insert("Score");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %.2f\n", result->score));

        resultText->setBold(true);
        resultText->insert("Headline");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %s\n", result->headl.c_str()));

        resultText->setBold(true);
        resultText->insert("Li");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->li));

        resultText->setBold(true);
        resultText->insert("TC");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->tc));

        resultText->setBold(true);
        resultText->insert("Se");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->se));

        resultText->setBold(true);
        resultText->insert("IU");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->iu));

        resultText->setBold(true);
        resultText->insert("ID");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->id));

        resultText->setBold(true);
        resultText->insert("TG");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->tg));

        resultText->setBold(true);
        resultText->insert("EF");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d  ", result->ef));

        resultText->setBold(true);
        resultText->insert("CP");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(":%d\n", result->cp));

        resultText->setBold(true);
        resultText->insert("Score Bonus");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %.2f\n", result->bonus));

        resultText->setBold(true);
        resultText->insert("Version targeted");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %s    ", result->vertgt.c_str()));

        resultText->setBold(true);
        resultText->insert("Version resolved");
        resultText->setBold(false);
        resultText->insert(QString().sprintf(": %s\n", result->verres.c_str()));

        resultText->setBold(true);
        resultText->insert("Description:\n");
        resultText->setBold(false);
        resultText->insert(result->descr.c_str());

        resultText->moveCursor(QTextEdit::MoveHome, false);
        resultText->ensureCursorVisible();
    }
}


void CQScore::saveExcel()
{
    QString file = 
        QFileDialog::getSaveFileName(QString(),
                                     "Text (Tab-delimited) files (*.txt)",
                                     NULL, "saveexcel", "Save file...");
    if (file.isNull())
        return;

    ofstream out(file.latin1());
    if (!out)
    {
        QMessageBox::warning(this, "Failed", "Could not open file for writing.");
        return;
    }

    out << "ident" << "\t"
        << "state" << "\t"
        << "owner" << "\t"
        << "score" << "\t"
        << "percentile" << "\t"
        << "headline" << "\t"
        << "li" << "\t"
        << "tc" << "\t"
        << "se" << "\t"
        << "iu" << "\t"
        << "id" << "\t"
        << "tg" << "\t"
        << "ef" << "\t"
        << "cp" << "\t"
        << "bonus" << "\t"
        << "targeted" << "\t"
        << "resolved" << "\t"
        //<< "descr" << "\t"
        << endl;

    QListViewItem *item = resultList->firstChild();
    while (item)
    {
        ResultData *result = currentResults[item->text(id_result_column)];
        out << result->ident << "\t"
            << result->state << "\t"
            << result->owner << "\t"
            << result->score << "\t"
            << int(result->percentile) << "\t"
            << result->headl << "\t"
            << result->li << "\t"
            << result->tc << "\t"
            << result->se << "\t"
            << result->iu << "\t"
            << result->id << "\t"
            << result->tg << "\t"
            << result->ef << "\t"
            << result->cp << "\t"
            << result->bonus << "\t"
            << result->vertgt << "\t"
            << result->verres << "\t"
            //<< result->descr << "\t"
            << endl;

        item = item->nextSibling();
    }
    out.close();
}

void CQScore::writeCurve()
{
    QString file = 
        QFileDialog::getSaveFileName(QString(),
                                     "Curve (ultra) files (*.curve)",
                                     NULL, "writecurve", "Save file...");
    if (file.isNull())
        return;

    ofstream out(file.latin1());
    if (!out)
    {
        QMessageBox::warning(this, "Failed", "Could not open file for writing.");
        return;
    }

    vector<float> curve;
    PopulatePercentileCurve(curve);

    out << "# percentiles\n";
    int n = curve.size();
    if (n == 1)
    {
        out << 100 << "\t" << curve[0] <<endl;
    }
    else
    {
        for (int i=0; i<n; i++)
        {
            out << float(100*i)/float(n-1) << "\t" << curve[i] << endl;
        }
    }
}

void CQScore::PopulatePercentileCurve(vector<float> &curve)
{
    curve.clear();
    for (map<QString, ResultData*>::iterator it = currentResults.begin();
         it != currentResults.end();
         it++)
    {
        ResultData *result = it->second;
        curve.push_back(result->score);
    }

    sort(curve.begin(), curve.end());
}


void CQScore::PopulatePercentileCurve()
{
    PopulatePercentileCurve(percentileCurve);
    RepopulateResults();
}

void CQScore::exportHTML()
{
    QString file = 
        QFileDialog::getSaveFileName(QString(),
                                     "HTML files (*.html)",
                                     NULL, "exporthtml", "Export HTML...");
    if (file.isNull())
        return;

    ofstream out(file.latin1());
    if (!out)
    {
        QMessageBox::warning(this, "Failed", "Could not open file for writing.");
        return;
    }

    out << "<html><head><title>ClearQuest Scoring Results</title></head>" << endl;
    out << "<body><h1>ClearQuest Scoring Results</h1>" << endl;
    out << "<hr>" << endl;

    QListViewItem *item = resultList->firstChild();
    while (item)
    {
        ResultData *result = currentResults[item->text(id_result_column)];
        out << "<p>\n";
        out << " <b>Ident</b>: "<<result->ident<<"<br>\n";
        out << " <b>State</b>: "<<result->state<<"<br>\n";
        out << " <b>Owner</b>: "<<result->owner<<"<br>\n";
        out << " <b>Score</b>: "<<result->score<<"<br>\n";
        if (result->percentile >= 0)
            out << " <b>Percentile</b>: "<<int(result->percentile)<<"<br>\n";
        else
            out << " <b>Percentile</b>: ?<br>\n";
        out << " <b>Headline</b>: "<<result->headl<<"<br>\n";
        out << " <b>li</b>="<<result->li<<" <b>tc</b>="<<result->tc
            << " <b>se</b>="<<result->se<<" <b>iu</b>="<<result->iu
            << " <b>id</b>="<<result->id<<" <b>tg</b>="<<result->tg
            << " <b>ef</b>="<<result->ef<<" <b>cp</b>="<<result->cp<<"<br>\n";
        out << " <b>Score Bonus</b>: "<<result->bonus<<"<br>\n";
        out << " <b>Version Targeted</b>: "<<result->vertgt<<"<br>\n";
        out << " <b>Version Resolved</b>: "<<result->verres<<"<br>\n";
        out << " <b>Description</b>:<br>\n";

        int descrlen = result->descr.length();
        for (int c=0; c<descrlen; c++)
        {
            if (result->descr[c] == '\n')
                out << "<br>\n";
            else
                out << result->descr[c];
        }
        out << "\n</p><hr>\n";
        item = item->nextSibling();
    }
    out.close();
}

void CQScore::CalculatePercentile(ResultData *item)
{
    int n = percentileCurve.size();
    if (n == 0)
    {
        item->percentile = -1;
    }
    else if (n == 1)
    {
        item->percentile = 100;
    }
    else
    {
        item->percentile = 100;
        for (int i=0; i<n; i++)
        {
            if (percentileCurve[i] >= item->score)
            {
                item->percentile = float(100*i)/float(n-1);
                break;
            }
        }
    }
}

void CQScore::PopulateResults()
{
    char outfile[1000];
    char errfile[1000];
    sprintf(outfile, "/tmp/cqscoretmp_results_outfile%d", getpid());
    sprintf(errfile, "/tmp/cqscoretmp_results_errfile%d", getpid());

    QString cquser = userEdit->text();
    QString cqpass = passEdit->text();
    QString cqdb   = dbCombo->currentText();
    QString cqrule = scoreRuleCombo->currentText();

    int     cqvalid   = scoreGroup->selectedId();
    QString cqproject = projectEdit->text();
    QString cqdate    = dateEdit->text();
    QString cqstate   = stateEdit->text();
    QString cqvertgt  = vertgtEdit->text();
    QString cqverres  = verresEdit->text();
    QString cqowner   = ownerEdit->text();
    QString cqid      = idEdit->text();

    if (cquser.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a username.");
        return;
    }

    if (cqpass.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a password.");
        return;
    }

    if (cqdb.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a database.");
        return;
    }

    if (cqrule.isEmpty())
    {
        QMessageBox::warning(this, "Failed", "Must supply a scoring rule.");
        return;
    }

    QString command("score.pl");
    command += QString().sprintf(" -u %s", cquser.latin1());
    command += QString().sprintf(" -p %s", cqpass.latin1());
    command += QString().sprintf(" -db %s", cqdb.latin1());
    command += QString().sprintf(" -r %s", cqrule.latin1());

    if (cqvalid >= 0)
    {
        if (cqvalid == 0)
            command += QString().sprintf(" -validscore yes");
        else if (cqvalid == 1)
            command += QString().sprintf(" -validscore no");
    }

    if (!cqproject.isEmpty())
        command += QString().sprintf(" -project %s", cqproject.latin1());

    if (!cqdate.isEmpty())
        command += QString().sprintf(" -date %s", cqdate.latin1());

    if (!cqstate.isEmpty())
        command += QString().sprintf(" -state %s", cqstate.latin1());

    if (!cqvertgt.isEmpty())
        command += QString().sprintf(" -vertgt %s", cqvertgt.latin1());

    if (!cqverres.isEmpty())
        command += QString().sprintf(" -verres %s", cqverres.latin1());

    if (!cqowner.isEmpty())
        command += QString().sprintf(" -owner %s", cqowner.latin1());

    if (!cqid.isEmpty())
        command += QString().sprintf(" -id %s", cqid.latin1());

    command += QString().sprintf(" -o - 1>%s 2>%s", outfile, errfile);


    //cerr << command.latin1() << endl;

    resultList->clear();
    resultText->clear();
    int err = system(command.latin1());

    if (err)
    {
        string errstring = ReadFileToString(errfile);
        if (errstring.empty())
            QMessageBox::warning(this, "Failed", "Unknown reason.");
        else
            QMessageBox::warning(this, "Failed", errstring.c_str());
        unlink(outfile);
        unlink(errfile);
        return;
    }

    currentResults.clear();
    FILE *file = fopen(outfile, "r");
    if (!file)
    {
        QMessageBox::warning(this, "Failed", "Failure.  No output generated.");
        unlink(outfile);
        unlink(errfile);
        return;
    }

    ResultData *item;
    while ((item = ResultData::ReadFromFile(this, file)) != NULL)
    {
        CalculatePercentile(item);
        currentResults[QString(item->ident.c_str())] = item;
        item->AddToListView(resultList);
    }

    fclose(file);

    unlink(outfile);
    unlink(errfile);
}

void CQScore::RepopulateResults()
{
    resultList->clear();
    resultText->clear();

    for (map<QString, ResultData*>::iterator it = currentResults.begin();
         it != currentResults.end();
         it++)
    {
        ResultData *result = it->second;
        CalculatePercentile(result);
        result->AddToListView(resultList);
    }
}

CQScore::CQScore(QWidget *p, const QString &n)
    : QMainWindow(p,n)
{
    setCaption("CQScore");

    QPopupMenu *filemenu = new QPopupMenu( this );
    menuBar()->insertItem(tr("&File"),filemenu);
    filemenu->insertItem( "&Save to Excel",  this, SLOT(saveExcel()),  CTRL+Key_S );
    filemenu->insertItem( "&Export to HTML",  this, SLOT(exportHTML()),  CTRL+Key_E );
    filemenu->insertItem( "&Write Score Curve",  this, SLOT(writeCurve()),  CTRL+Key_W );
    filemenu->insertSeparator();
    filemenu->insertItem( "&Quit", this, SLOT(close()),  CTRL+Key_Q );

    QSplitter *central = new QSplitter(Qt::Vertical, this);
    central->setMargin(5);

    //QVBoxLayout *topLayout = new QVBoxLayout(central, 5);

    // ------------------------------------------------------------------------

    QGroupBox *userBox = new QGroupBox("User info", central);
    central->setResizeMode(userBox, QSplitter::KeepSize);
    //topLayout->addWidget(userBox);

    QGridLayout *userLayout = new QGridLayout(userBox, 4, 2,  10,5);
    userLayout->setRowSpacing(0, 15);
    userLayout->setColSpacing(1, 10);

    QLabel *userLabel = new QLabel("User:",     userBox);
    QLabel *passLabel = new QLabel("Password:", userBox);
    QLabel *dbLabel   = new QLabel("Database:", userBox);

    userEdit = new QLineEdit(getenv("USER"), userBox);
    passEdit = new QLineEdit("", userBox);
    passEdit->setEchoMode(QLineEdit::Password);

    dbCombo = new QComboBox(true, userBox);
    vector<string> dblist = GetDatabaseList();
    for (int i=0; i<dblist.size(); i++)
    {
        dbCombo->insertItem(dblist[i].c_str());
        if (dblist[i] == "VisIt")
        {
            dbCombo->setCurrentItem(i);
        }
    }

    userLayout->addWidget(userLabel, 1,0);
    userLayout->addWidget(userEdit,  1,2);
    userLayout->addWidget(passLabel, 2,0);
    userLayout->addWidget(passEdit,  2,2);
    userLayout->addWidget(dbLabel,   3,0);
    userLayout->addWidget(dbCombo,   3,2);

    // ------------------------------------------------------------------------

    QGroupBox *scoreRuleBox = new QGroupBox("Scoring rule", central);
    central->setResizeMode(scoreRuleBox, QSplitter::KeepSize);
    //topLayout->addWidget(scoreRuleBox);

    QGridLayout *scoreRuleLayout = new QGridLayout(scoreRuleBox, 2, 6,  10,5);
    scoreRuleLayout->setRowSpacing(0, 15);

    QLabel *scoreRuleLabel = new QLabel("Scoring rule:", scoreRuleBox);
    scoreRuleCombo = new QComboBox(true, scoreRuleBox);
    scoreRuleCombo->setCurrentText("Default");
    QPushButton *scoreRuleButton = new QPushButton("Get Rules", scoreRuleBox);
    QPushButton *executeButton = new QPushButton("Execute", scoreRuleBox);
    QPushButton *percentileButton = new QPushButton("Use Results for Percentiles", scoreRuleBox);

    scoreRuleCombo->setEnabled(false);

    scoreRuleLayout->addWidget(scoreRuleLabel, 1,0);
    scoreRuleLayout->addWidget(scoreRuleCombo, 1,2);
    scoreRuleLayout->addWidget(scoreRuleButton, 1,3);
    scoreRuleLayout->addWidget(executeButton, 1,4);
    scoreRuleLayout->addWidget(percentileButton, 1,5);

    // ------------------------------------------------------------------------

    QGroupBox *filterBox = new QGroupBox("Filters", central);
    central->setResizeMode(filterBox, QSplitter::KeepSize);

    QGridLayout *filterBoxLayout = new QGridLayout(filterBox, 9, 2,  10,5);
    filterBoxLayout->setRowSpacing(0, 15);

    QLabel *validLabel   = new QLabel("Valid Score:", filterBox);
    QLabel *projectLabel = new QLabel("Project:", filterBox);
    QLabel *dateLabel    = new QLabel("Left S state since (Y-M-D):", filterBox);
    QLabel *stateLabel   = new QLabel("Current States (SIAORVPDC):", filterBox);
    QLabel *vertgtLabel  = new QLabel("Version Targeted:", filterBox);
    QLabel *verresLabel  = new QLabel("Version Resolved:", filterBox);
    QLabel *ownerLabel   = new QLabel("Owner:", filterBox);
    QLabel *idLabel      = new QLabel("ID:", filterBox);

    filterBoxLayout->addWidget(validLabel,   1, 0);
    filterBoxLayout->addWidget(projectLabel, 2, 0);
    filterBoxLayout->addWidget(dateLabel,    3, 0);
    filterBoxLayout->addWidget(stateLabel,   4, 0);
    filterBoxLayout->addWidget(vertgtLabel,  5, 0);
    filterBoxLayout->addWidget(verresLabel,  6, 0);
    filterBoxLayout->addWidget(ownerLabel,   7, 0);
    filterBoxLayout->addWidget(idLabel,      8, 0);

    QFrame *scoreFrame = new QFrame(filterBox);
    QGridLayout *scoreLayout = new QGridLayout(scoreFrame, 1,3, 0,0);
    scoreGroup = new QButtonGroup(NULL);
    QRadioButton *scoreYes    = new QRadioButton("Yes",    scoreFrame);
    QRadioButton *scoreNo     = new QRadioButton("No",     scoreFrame);
    QRadioButton *scoreEither = new QRadioButton("Either", scoreFrame);
    scoreGroup->insert(scoreYes);
    scoreGroup->insert(scoreNo);
    scoreGroup->insert(scoreEither);
    scoreGroup->setButton(0);
    scoreLayout->addWidget(scoreYes,    0,0);
    scoreLayout->addWidget(scoreNo,     0,1);
    scoreLayout->addWidget(scoreEither, 0,2);

    projectEdit = new QLineEdit("", filterBox);
    dateEdit    = new QLineEdit("", filterBox);
    stateEdit   = new QLineEdit("", filterBox);
    vertgtEdit  = new QLineEdit("", filterBox);
    verresEdit  = new QLineEdit("", filterBox);
    ownerEdit   = new QLineEdit("", filterBox);
    idEdit      = new QLineEdit("", filterBox);

    filterBoxLayout->addWidget(scoreFrame,  1, 1);
    filterBoxLayout->addWidget(projectEdit, 2, 1);
    filterBoxLayout->addWidget(dateEdit,    3, 1);
    filterBoxLayout->addWidget(stateEdit,   4, 1);
    filterBoxLayout->addWidget(vertgtEdit,  5, 1);
    filterBoxLayout->addWidget(verresEdit,  6, 1);
    filterBoxLayout->addWidget(ownerEdit,   7, 1);
    filterBoxLayout->addWidget(idEdit,      8, 1);

    // ------------------------------------------------------------------------

    QGroupBox *resultBox = new QGroupBox("Results", central);
    //topLayout->addWidget(resultBox);

    QGridLayout *resultLayout = new QGridLayout(resultBox, 2, 1,  10,5);
    resultLayout->setRowSpacing(0, 15);

    QSplitter *resultSplitter = new QSplitter(Qt::Vertical, resultBox);

    resultList = new QListView(resultSplitter);
    resultList->setAllColumnsShowFocus(true);
    resultList->setShowSortIndicator(true);
    for (int column = 0; column < num_visible_columns; column++)
    {
        resultList->addColumn(result_columns[column]);
    }

    resultText = new QTextEdit(resultSplitter);
    resultText->setReadOnly(true);

    //resultLayout->addWidget(resultList, 1,0);
    //resultLayout->addWidget(resultText, 2,0);
    resultLayout->addWidget(resultSplitter, 1,0);

    setCentralWidget(central);

    connect(scoreRuleButton, SIGNAL(clicked()),
            this, SLOT(PopulateScoringRuleChoices()));
    connect(executeButton, SIGNAL(clicked()),
            this, SLOT(PopulateResults()));
    connect(percentileButton, SIGNAL(clicked()),
            this, SLOT(PopulatePercentileCurve()));
    connect(resultList, SIGNAL(selectionChanged(QListViewItem*)),
            this, SLOT(PopulateSingleResultText(QListViewItem*)));
}

