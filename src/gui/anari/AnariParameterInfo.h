// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef ANARI_PARAMETER_INFO_H
#define ANARI_PARAMETER_INFO_H

#include <anari/anari_cpp.hpp>

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
class AnariParameterInfo
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
    std::vector<std::string> GetAcceptedValues() const { return m_acceptedValues; }

    // Flags
    // =====
    bool HasMinimum() const { bool val = (m_minimum != NULL && m_minimum != nullptr); return val; }
    bool HasMaximum() const { bool val = (m_maximum != NULL && m_maximum != nullptr); return val; }

    // Raw pointers to the memory returned by ANARI
    const void *m_minimum = nullptr;
    const void *m_maximum = nullptr;
    const void *m_defaultValue = nullptr;

private:
    // ANARIParameter
    std::string     m_name;
    ANARIDataType   m_type = ANARI_UNKNOWN;

    // Anari Parameter Info
    std::string                 m_description;
    std::vector<std::string>    m_acceptedValues;
};

#endif // ANARI_PARAMETER_INFO_H
