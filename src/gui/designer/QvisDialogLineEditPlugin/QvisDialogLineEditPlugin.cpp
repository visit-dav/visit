#include "QvisDialogLineEdit.h"
#include "QvisDialogLineEditPlugin.h"

#include <QtPlugin>

QvisDialogLineEditPlugin::QvisDialogLineEditPlugin(QObject *parent)
    : QObject(parent)
{
    initialized = false;
}

void QvisDialogLineEditPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (initialized)
        return;

    initialized = true;
}

bool QvisDialogLineEditPlugin::isInitialized() const
{
    return initialized;
}

QWidget *QvisDialogLineEditPlugin::createWidget(QWidget *parent)
{
    return new QvisDialogLineEdit(parent);
}

QString QvisDialogLineEditPlugin::name() const
{
    return "QvisDialogLineEdit";
}

QString QvisDialogLineEditPlugin::group() const
{
    return "VisIt";
}

QIcon QvisDialogLineEditPlugin::icon() const
{
    return QIcon();
}

QString QvisDialogLineEditPlugin::toolTip() const
{
    return "dialog line edit widget";
}

QString QvisDialogLineEditPlugin::whatsThis() const
{
    return "Lets the user use various dialogs to select data which gets inserted into the widget's line edit.";
}

bool QvisDialogLineEditPlugin::isContainer() const
{
    return false;
}

QString QvisDialogLineEditPlugin::domXml() const
{
    return "<widget class=\"QvisDialogLineEdit\" name=\"QvisDialogLineEdit\">\n"
           " <property name=\"geometry\">\n"
           "  <rect>\n"
           "   <x>0</x>\n"
           "   <y>0</y>\n"
           "   <width>300</width>\n"
           "   <height>22</height>\n"
           "  </rect>\n"
           " </property>\n"
           "</widget>\n";
}

QString QvisDialogLineEditPlugin::includeFile() const
{
    return "QvisDialogLineEdit.h";
}

Q_EXPORT_PLUGIN2(QvisDialogLineEditPlugin, QvisDialogLineEditPlugin)
