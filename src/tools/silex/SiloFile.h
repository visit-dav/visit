#ifndef SILOFILE_H
#define SILOFILE_H

#include <silo.h>
#include <qstring.h>
#include <vector>
using std::vector;

// ****************************************************************************
//  Class:  SiloDir
//
//  Purpose:
//    Encapsulation of a silo directory (TOC).
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
//  Modifications:
//
//    Mark C. Miller, Tue Sep 13 20:09:49 PDT 2005
//    Added support for new silo objects; defvars, csgmesh/vars
// ****************************************************************************
class SiloDir
{
  public:
    QString          name;
    QString          path;
    vector<SiloDir*> subdir;

    vector<QString>  curve;
    vector<QString>  csgmesh;
    vector<QString>  csgvar;
    vector<QString>  defvars;
    vector<QString>  multimesh;
    vector<QString>  multivar;
    vector<QString>  multimat;
    vector<QString>  multimatspecies;
    vector<QString>  qmesh;
    vector<QString>  qvar;
    vector<QString>  ucdmesh;
    vector<QString>  ucdvar;
    vector<QString>  ptmesh;
    vector<QString>  ptvar;
    vector<QString>  mat;
    vector<QString>  matspecies;
    vector<QString>  var;
    vector<QString>  obj;
    vector<QString>  array;
    vector<QString>  dir;
  public:
    SiloDir(DBfile *db, const QString &name, const QString &path);
    ~SiloDir();
};

// ****************************************************************************
//  Class:  SiloFile
//
//  Purpose:
//    Encapsulation of a Silo file.
//
//  Programmer:  Jeremy Meredith
//  Creation:    November 12, 2001
//
// ****************************************************************************
class SiloFile
{
  public:
    SiloDir *root;

  public:
    SiloFile(const QString &name);
    ~SiloFile();

    void         *GetVar(const QString &name);
    int           GetVarType(const QString &name);
    int           GetVarLength(const QString &name);
    DBobject     *GetObject(const QString &name);
    void         *GetComponent(const QString &oname, const QString &cname);
    int           GetComponentType(const QString &oname, const QString &cname);
    DBObjectType  InqVarType(const QString &name);

  private:
    DBfile *db;
};

#endif
