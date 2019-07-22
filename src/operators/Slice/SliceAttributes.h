// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef SLICEATTRIBUTES_H
#define SLICEATTRIBUTES_H
#include <string>
#include <AttributeSubject.h>

#include <cmath>
#include <math.h>

// ****************************************************************************
// Class: SliceAttributes
//
// Purpose:
//    This class contains attributes for the arbitrary slice.
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//   
// ****************************************************************************

class SliceAttributes : public AttributeSubject
{
public:
    enum AxisType
    {
        XAxis,
        YAxis,
        ZAxis,
        Arbitrary,
        ThetaPhi
    };
    enum OriginType
    {
        Point,
        Intercept,
        Percent,
        Zone,
        Node
    };

    // These constructors are for objects of this class
    SliceAttributes();
    SliceAttributes(const SliceAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    SliceAttributes(private_tmfs_t tmfs);
    SliceAttributes(const SliceAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~SliceAttributes();

    virtual SliceAttributes& operator = (const SliceAttributes &obj);
    virtual bool operator == (const SliceAttributes &obj) const;
    virtual bool operator != (const SliceAttributes &obj) const;
private:
    void Init();
    void Copy(const SliceAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectOriginPoint();
    void SelectNormal();
    void SelectUpAxis();
    void SelectMeshName();

    // Property setting methods
    void SetOriginType(OriginType originType_);
    void SetOriginPoint(const double *originPoint_);
    void SetOriginIntercept(double originIntercept_);
    void SetOriginPercent(double originPercent_);
    void SetOriginZone(int originZone_);
    void SetOriginNode(int originNode_);
    void SetNormal(const double *normal_);
    void SetAxisType(AxisType axisType_);
    void SetUpAxis(const double *upAxis_);
    void SetProject2d(bool project2d_);
    void SetInteractive(bool interactive_);
    void SetFlip(bool flip_);
    void SetOriginZoneDomain(int originZoneDomain_);
    void SetOriginNodeDomain(int originNodeDomain_);
    void SetMeshName(const std::string &meshName_);
    void SetTheta(double theta_);
    void SetPhi(double phi_);

    // Property getting methods
    OriginType        GetOriginType() const;
    const double      *GetOriginPoint() const;
          double      *GetOriginPoint();
    double            GetOriginIntercept() const;
    double            GetOriginPercent() const;
    int               GetOriginZone() const;
    int               GetOriginNode() const;
    const double      *GetNormal() const;
          double      *GetNormal();
    AxisType          GetAxisType() const;
    const double      *GetUpAxis() const;
          double      *GetUpAxis();
    bool              GetProject2d() const;
    bool              GetInteractive() const;
    bool              GetFlip() const;
    int               GetOriginZoneDomain() const;
    int               GetOriginNodeDomain() const;
    const std::string &GetMeshName() const;
          std::string &GetMeshName();
    double            GetTheta() const;
    double            GetPhi() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);

    // Enum conversion functions
    static std::string AxisType_ToString(AxisType);
    static bool AxisType_FromString(const std::string &, AxisType &);
protected:
    static std::string AxisType_ToString(int);
public:
    static std::string OriginType_ToString(OriginType);
    static bool OriginType_FromString(const std::string &, OriginType &);
protected:
    static std::string OriginType_ToString(int);
public:

    // Keyframing methods
    virtual std::string               GetFieldName(int index) const;
    virtual AttributeGroup::FieldType GetFieldType(int index) const;
    virtual std::string               GetFieldTypeName(int index) const;
    virtual bool                      FieldsEqual(int index, const AttributeGroup *rhs) const;

    // User-defined methods
    void UpdateOrthogonalAxes();
    virtual bool EqualTo(const AttributeGroup *atts) const;
    virtual bool SetValue(const std::string &name, const int &value);
    virtual bool SetValue(const std::string &name, const bool &value);

    // IDs that can be used to identify fields in case statements
    enum {
        ID_originType = 0,
        ID_originPoint,
        ID_originIntercept,
        ID_originPercent,
        ID_originZone,
        ID_originNode,
        ID_normal,
        ID_axisType,
        ID_upAxis,
        ID_project2d,
        ID_interactive,
        ID_flip,
        ID_originZoneDomain,
        ID_originNodeDomain,
        ID_meshName,
        ID_theta,
        ID_phi,
        ID__LAST
    };

private:
    int         originType;
    double      originPoint[3];
    double      originIntercept;
    double      originPercent;
    int         originZone;
    int         originNode;
    double      normal[3];
    int         axisType;
    double      upAxis[3];
    bool        project2d;
    bool        interactive;
    bool        flip;
    int         originZoneDomain;
    int         originNodeDomain;
    std::string meshName;
    double      theta;
    double      phi;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define SLICEATTRIBUTES_TMFS "iDddiiDiDbbbiisdd"

#endif
