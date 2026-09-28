// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_PARAMETER_INFO_H
#define ANARI_PARAMETER_INFO_H

#include <anari/anari_cpp.hpp>
#include <gui_exports.h>

#include <iostream>
#include <vector>

// ****************************************************************************
// Class: AnariParameterInfo
//
// Purpose:
//
// Notes:
//
// Programmer: Kevin Griffin
// Creation:   Wed Sep 18 14:33:24 PST 2002
//
// Modifications:
//
// ****************************************************************************
class GUI_API AnariParameterInfo
{
public:
    // Default Constructor
    AnariParameterInfo() {}

    // Move Constructor
    AnariParameterInfo(AnariParameterInfo &&other);

    // Copy Constructor
    AnariParameterInfo(const AnariParameterInfo &) = delete;

    // Destructor
    ~AnariParameterInfo() = default;

    // move assignment operator
    AnariParameterInfo &operator=(AnariParameterInfo &&other);

    // copy assignment operator
    AnariParameterInfo &operator=(const AnariParameterInfo &) = delete;

    // Setters and Getters
    // ==================
    void SetName(const char *n) { m_name = std::string(n); }
    std::string GetName() const { return m_name; }

    void SetType(ANARIDataType t) { m_type = t; }
    ANARIDataType GetType() const { return m_type; }

    void SetDescription(const void *);
    std::string GetDescription() const { return m_description; }

    void SetAcceptedValues(const char **);
    void SetAcceptedValues(const std::vector<std::string> &v) { m_acceptedValues = v; }
    std::vector<std::string> GetAcceptedValues() const { return m_acceptedValues; }

    // Text-based value accessors, used when the parameter info was
    // deserialized from an engine-provided MapNode (e.g. AnariDeviceInfoRPC)
    // rather than obtained from a live ANARI device (see m_minimum,
    // m_maximum, m_defaultValue below).
    void SetMinimumText(const std::string &s) { m_minimumText = s; }
    void SetMaximumText(const std::string &s) { m_maximumText = s; }
    void SetDefaultValueText(const std::string &s) { m_defaultValueText = s; }
    const std::string &GetMinimumText() const { return m_minimumText; }
    const std::string &GetMaximumText() const { return m_maximumText; }
    const std::string &GetDefaultValueText() const { return m_defaultValueText; }

    // Flags
    // =====
    bool HasMinimum() const { bool val = (m_minimum != NULL && m_minimum != nullptr); return val; }
    bool HasMaximum() const { bool val = (m_maximum != NULL && m_maximum != nullptr); return val; }
    bool HasDefaultValue() const { bool val = (m_defaultValue != NULL && m_defaultValue != nullptr); return val; }
    bool HasMinimumText() const { return !m_minimumText.empty(); }
    bool HasMaximumText() const { return !m_maximumText.empty(); }
    bool HasDefaultValueText() const { return !m_defaultValueText.empty(); }

    // Raw pointers to the memory returned by ANARI. Only populated when this
    // AnariParameterInfo was built from a live anari::Device (see
    // AnariVolumeWidget). Instances built from an engine-provided MapNode
    // (see AnariRenderingWidget) instead populate the *Text members above.
    const void *m_minimum = nullptr;
    const void *m_maximum = nullptr;
    const void *m_defaultValue = nullptr;

  private:
    std::string m_minimumText;
    std::string m_maximumText;
    std::string m_defaultValueText;

private:
    // ANARIParameter
    std::string     m_name;
    ANARIDataType   m_type = ANARI_UNKNOWN;

    // Anari Parameter Info
    std::string                 m_description;
    std::vector<std::string>    m_acceptedValues;
};

#endif // ANARI_PARAMETER_INFO_H
