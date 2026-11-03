// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_RENDERING_WIDGET_H
#define ANARI_RENDERING_WIDGET_H

#include <gui_exports.h>
#include <QWidget>

#include <anari/anari_cpp.hpp>
#include <MapNode.h>
#include <vectortypes.h>

#include <memory>

class RenderingAttributes;
class AnariAttributes;
class QvisRenderingWindow;
class QGroupBox;
class QComboBox;
class QSpinBox;
class QPushButton;
class QLineEdit;
class QCheckBox;
class QStackedLayout;
class AnariParameterInfo;

namespace anari_visit
{
    enum class BackendType
    {
        NONE,
        EXAMPLE,
        USD,
        VISRTX,
        VISGL,
        OSPRAY,
        RADEONPRORENDER,
        PHENOCRYST
    };
}

using BackendType = anari_visit::BackendType;

class GUI_API AnariRenderingWidget : public QWidget
{
    Q_OBJECT
public:
    AnariRenderingWidget(QvisRenderingWindow *,
                         RenderingAttributes *,
                         QWidget *parent = nullptr);
    ~AnariRenderingWidget() = default;

    int GetRowCount() const { return (topRows + bottomRows); }

    // General
    void UpdateAnariAttributes(const AnariAttributes &);
    void SetChecked(const bool);
    void UpdateLibraryName(const std::string &);
    void UpdateLibrarySubtypes(const std::string &);
    void UpdateRendererSubtypes(const std::string &);

    // Dynamic
    void UpdateRendererParameters(const stringVector &);
    void UpdateUSDParameters(const stringVector &);

    // Called by QvisRenderingWindow when the engine reports ANARI device
    // info (library subtypes / renderer subtypes / renderer parameters)
    // in response to a request issued by this widget.
    void UpdateDeviceInfo(const MapNode &info);

    static QColor TextToColor(ANARIDataType, const std::string &);
    static std::string ColorToText(ANARIDataType, const QColor &);

signals:
    void currentBackendChanged(int);

private slots:
    void renderingToggled(bool);
    void libraryChanged();
    void librarySubtypeChanged(const QString &);
    void rendererSubtypeChanged(const QString &);

    void selectButtonPressed();

    // Dynamic
    void spinBoxValueChanged(int);
    void lineEditingFinished();
    void comboBoxTextChanged(const QString &);
    void checkBoxToggled(bool);
    
private:
    QWidget *CreateGeneralWidget(int &);
    QWidget *CreateUSDWidget(int &);
    void CreateDynamicWidget(const MapNode &parameters, const std::string &, bool isUSD = false);
    QPushButton *CreateColorButton(QWidget *, ANARIDataType, const std::string &);
    void SetColorButtonSwatch(QPushButton *, const QColor &);

    BackendType GetBackendType(const std::string &) const;
    AnariParameterInfo BuildParameterInfoFromMapNode(const std::string &name, const MapNode &paramNode);
    QWidget *MakeWidgetFromParameterInfo(const AnariParameterInfo &);
    void UpdateRenderingAttributes(const bool);
    void ClearAnariParameterAttributes();
    void RequestDeviceInfo(const std::string &libraryName,
                           const std::string &librarySubtype,
                           const std::string &rendererSubtype);

    QvisRenderingWindow *renderingWindow;
    AnariAttributes *anariAttributes;
    QStackedLayout *dynamicLayouts; // Caches the dynamic widgets

    // Mapping of dynamic widget key (backend:subtype:renderer) to index in
    // dyamicLayouts
    std::map<std::string, int> dynamicLayoutMap;
    int topRows;
    int bottomRows;

    // General Widget Components
    QGroupBox   *renderingGroup;
    QComboBox   *libraryName;
    QComboBox   *librarySubtypes;
    QComboBox   *rendererSubtypes;

    // File Chooser
    QString     currentDirectory;
    QLineEdit   *dirLineEdit;

    static const std::string USD_WIDGET_KEY;
    static const std::string DEFAULT_WIDGET_KEY;
};

#endif