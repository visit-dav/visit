// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#ifndef CINEMAATTRIBUTES_H
#define CINEMAATTRIBUTES_H
#include <state_exports.h>
#include <string>
#include <AttributeSubject.h>


// ****************************************************************************
// Class: CinemaAttributes
//
// Purpose:
//    This class contains the attributes used for saving Cinema
//
// Notes:      Autogenerated by xml2atts.
//
// Programmer: xml2atts
// Creation:   omitted
//
// Modifications:
//
// ****************************************************************************

class STATE_API CinemaAttributes : public AttributeSubject
{
public:
    // These constructors are for objects of this class
    CinemaAttributes();
    CinemaAttributes(const CinemaAttributes &obj);
protected:
    // These constructors are for objects derived from this class
    CinemaAttributes(private_tmfs_t tmfs);
    CinemaAttributes(const CinemaAttributes &obj, private_tmfs_t tmfs);
public:
    virtual ~CinemaAttributes();

    virtual CinemaAttributes& operator = (const CinemaAttributes &obj);
    virtual bool operator == (const CinemaAttributes &obj) const;
    virtual bool operator != (const CinemaAttributes &obj) const;
private:
    void Init();
    void Copy(const CinemaAttributes &obj);
public:

    virtual const std::string TypeName() const;
    virtual bool CopyAttributes(const AttributeGroup *);
    virtual AttributeSubject *CreateCompatible(const std::string &) const;
    virtual AttributeSubject *NewInstance(bool) const;

    // Property selection methods
    virtual void SelectAll();
    void SelectSpecification();
    void SelectFileName();
    void SelectCameraMode();
    void SelectFormat();

    // Property setting methods
    void SetSpecification(const std::string &specification_);
    void SetFileName(const std::string &fileName_);
    void SetCameraMode(const std::string &cameraMode_);
    void SetTheta(int theta_);
    void SetPhi(int phi_);
    void SetRoll(int roll_);
    void SetFormat(const std::string &format_);
    void SetComposite(bool composite_);
    void SetUseScreenCapture(bool useScreenCapture_);
    void SetWidth(int width_);
    void SetHeight(int height_);
    void SetFrameStart(int frameStart_);
    void SetFrameEnd(int frameEnd_);
    void SetFrameStride(int frameStride_);

    // Property getting methods
    const std::string &GetSpecification() const;
          std::string &GetSpecification();
    const std::string &GetFileName() const;
          std::string &GetFileName();
    const std::string &GetCameraMode() const;
          std::string &GetCameraMode();
    int               GetTheta() const;
    int               GetPhi() const;
    int               GetRoll() const;
    const std::string &GetFormat() const;
          std::string &GetFormat();
    bool              GetComposite() const;
    bool              GetUseScreenCapture() const;
    int               GetWidth() const;
    int               GetHeight() const;
    int               GetFrameStart() const;
    int               GetFrameEnd() const;
    int               GetFrameStride() const;

    // Persistence methods
    virtual bool CreateNode(DataNode *node, bool completeSave, bool forceAdd);
    virtual void SetFromNode(DataNode *node);


    // IDs that can be used to identify fields in case statements
    enum {
        ID_specification = 0,
        ID_fileName,
        ID_cameraMode,
        ID_theta,
        ID_phi,
        ID_roll,
        ID_format,
        ID_composite,
        ID_useScreenCapture,
        ID_width,
        ID_height,
        ID_frameStart,
        ID_frameEnd,
        ID_frameStride,
        ID__LAST
    };

private:
    std::string specification;
    std::string fileName;
    std::string cameraMode;
    int         theta;
    int         phi;
    int         roll;
    std::string format;
    bool        composite;
    bool        useScreenCapture;
    int         width;
    int         height;
    int         frameStart;
    int         frameEnd;
    int         frameStride;

    // Static class format string for type map.
    static const char *TypeMapFormatString;
    static const private_tmfs_t TmfsStruct;
};
#define CINEMAATTRIBUTES_TMFS "sssiiisbbiiiii"

#endif
