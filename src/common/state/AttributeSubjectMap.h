// ************************************************************************* //
//                           AttributeSubjectMap.h                           //
// ************************************************************************* //
 
#ifndef ATTRIBUTE_SUBJECT_MAP_H
#define ATTRIBUTE_SUBJECT_MAP_H

#include <state_exports.h>

class AttributeSubject;

// ****************************************************************************
//  Class: AttributeSubjectMap
//
//  Purpose:
//    AttributeSubjectMap is an indexed list of attribute subjects.
//
//  Programmer: Eric Brugger
//  Creation:   November 15, 2002
//
// ****************************************************************************
 
class STATE_API AttributeSubjectMap
{
  public:
    AttributeSubjectMap();
    ~AttributeSubjectMap();

    void SetAtts(const AttributeSubject *);
    void SetAtts(const int, const AttributeSubject *);
    void SetAtts(const int, const AttributeSubject *, int &, int &);
    void GetAtts(const int, AttributeSubject *) const;
    void ClearAtts();
    bool DeleteAtts(const int, int &, int &);
    bool MoveAtts(int, int, int &, int &);
    int GetNIndices() const;
    const int *GetIndices(int &) const;
    AttributeSubject *CreateCompatible(const std::string &) const;
    bool CopyAttributes(const AttributeSubjectMap *);

  private:
    void ResizeMap(const int);

    int                 nIndices;
    int                 maxIndices;
    AttributeSubject  **atts;
    int                *indices;
};

#endif
