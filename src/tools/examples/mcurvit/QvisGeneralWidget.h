// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef QVIS_GENERAL_WIDGET_H
#define QVIS_GENERAL_WIDGET_H
#include <QWidget>
#include <SimpleObserver.h>

class QLabel;
class QListWidget;
class QSpinBox;

class VisItViewer;

class AttributeSubject;

// ****************************************************************************
// Class: QvisGeneralWidget
//
// Purpose:
//   Widget that handles the general widgets.
//
// Programmer: Eric Brugger
// Creation:   Tue Feb  3 09:54:15 PST 2009
//
// Modifications:
//
// ****************************************************************************

class QvisGeneralWidget : public QWidget, public SimpleObserver
{
    Q_OBJECT
public:
    QvisGeneralWidget(QWidget *parent, VisItViewer *v);
    virtual ~QvisGeneralWidget();

    virtual void Update(Subject *subject);
    virtual void SubjectRemoved(Subject *subject);

    void ConnectIndexSelectAttributes(AttributeSubject *m);

    void CreateWindow();
    void UpdateIndexSelectWidgets();

    void ClearVariableList();
    void AddVariable(const char *var);
    void SetCurrentVariable(const int iVar);
    int  GetCurrentVariable();
    QString GetCurrentVariableName();

signals:
    void variableTextChanged(const QString &);
    void indexSelectChanged(const AttributeSubject *);
private slots:
    void variableChanged(const QString &);
    void domainMinChanged(int min);
    void domainMaxChanged(int max);
    void rangeMinChanged(int min);
    void rangeMaxChanged(int max);
private:
    VisItViewer             *viewer;

    AttributeSubject        *indexSelectAtts;

    QLabel                  *variableLabel;
    QListWidget             *variableList;
    QLabel                  *domainLabel;
    QLabel                  *domainMinLabel;
    QSpinBox                *domainMin;
    QLabel                  *domainMaxLabel;
    QSpinBox                *domainMax;
    QLabel                  *rangeLabel;
    QLabel                  *rangeMinLabel;
    QSpinBox                *rangeMin;
    QLabel                  *rangeMaxLabel;
    QSpinBox                *rangeMax;
};

#endif
