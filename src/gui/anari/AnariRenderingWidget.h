// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_RENDERING_WIDGET_H
#define ANARI_RENDERING_WIDGET_H

#include <gui_exports.h>
#include <QWidget>

#include <anari/anari_cpp.hpp>
#include <vectortypes.h>

#include <memory>

class RenderingAttributes;
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
        RADEONPRORENDER
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
    void SetChecked(const bool);
    void UpdateLibraryName(const std::string);
    void UpdateLibrarySubtypes(const std::string);
    void UpdateRendererSubtypes(const std::string);

    // Dynamic
    void UpdateRendererParameters(const stringVector &);
    void UpdateUSDParameters(const stringVector &);

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
    void CreateDynamicWidget(anari::Device, const char *, const std::string &, bool isUSD = false);

    BackendType GetBackendType(const std::string &) const;
    AnariParameterInfo GetParameterInfo(anari::Device, ANARIDataType, const char *, const ANARIParameter *);
    QWidget *MakeWidgetFromParameterInfo(const AnariParameterInfo &);
    void UpdateRenderingAttributes(const bool);
    void ClearAnariParameterAttributes();
    void UpdateLibraryUI(anari::Library, const std::string &);

    QvisRenderingWindow *renderingWindow;
    RenderingAttributes *renderingAttributes;
    QStackedLayout *dynamicLayouts; // Caches the dynamic widgets

    // Mapping of dynamic widget key (backend:subtype:renderer) to index in
    // dyamicLayouts
    std::map<std::string, int> dynamicLayoutMap;
    int topRows;
    int bottomRows;

    // General Widget Components
    QGroupBox   *renderingGroup;
    QLineEdit   *libraryName;
    QComboBox   *librarySubtypes;
    QComboBox   *rendererSubtypes;

    // File Chooser
    QString     currentDirectory;
    QLineEdit   *dirLineEdit;

    static const std::string USD_WIDGET_KEY;
    static const std::string DEFAULT_WIDGET_KEY;
};

#endif