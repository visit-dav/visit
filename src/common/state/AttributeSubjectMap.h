// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ****************************************************************************
//  AttributeSubjectMap.h
// ****************************************************************************

#ifndef ATTRIBUTE_SUBJECT_MAP_H
#define ATTRIBUTE_SUBJECT_MAP_H

#include <state_exports.h>

class AttributeSubject;
class DataNode;

// ****************************************************************************
//  Class: AttributeSubjectMap
//
//  Purpose:
//    AttributeSubjectMap is an indexed list of attribute subjects.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
//  Modifications:
//   Brad Whitlock, Tue Dec 30 17:04:12 PST 2003
//   I added a copy constructor and an assignment operator.
//
//   Mark C. Miller, Thu Mar 25 10:45:18 PST 2004
//   Added method, SetAttsLe
//
//   Brad Whitlock, Wed Feb 13 14:36:23 PST 2008
//   Added ProcessOldVersions.
//
// ****************************************************************************

class STATE_API AttributeSubjectMap
{
public:
    AttributeSubjectMap();
    AttributeSubjectMap(const AttributeSubjectMap &);
    ~AttributeSubjectMap();

    void operator = (const AttributeSubjectMap &);

    void SetAtts(const AttributeSubject *);
    void SetAtts(const int, const AttributeSubject *);
    void SetAtts(const int, const AttributeSubject *, int &, int &);
    void SetAttsLe(const int, const AttributeSubject *);
    void GetAtts(const int, AttributeSubject *) const;
    void ClearAtts();
    bool DeleteAtts(const int, int &, int &);
    bool MoveAtts(int, int, int &, int &);
    int GetNIndices() const;
    const int *GetIndices(int &) const;
    AttributeSubject *CreateCompatible(const std::string &) const;
    bool CopyAttributes(const AttributeSubjectMap *);

    bool CreateNode(DataNode *parentNode);
    void SetFromNode(DataNode *parentNode, AttributeSubject *factoryObj);
    void ProcessOldVersions(DataNode *parentNode, const std::string &configVersion,
                            AttributeSubject *factoryObj);

  private:
    void ResizeMap(const int);

    int                 nIndices;
    int                 maxIndices;
    AttributeSubject  **atts;
    int                *indices;
};

#endif
