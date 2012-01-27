#ifndef DYNA3D_FILE_H
#define DYNA3D_FILE_H
#include <stdio.h>
#include <string>
#include <vtkFloatArray.h>
#include <vtkDataArray.h>
#include <vtkUnstructuredGrid.h>

#include <vector>
#include <vectortypes.h>
#include <avtMatrix.h>

#include <MaterialProperties.h>

// ****************************************************************************
//  Class: Dyna3DFile
//
//  Purpose:
//      Reads in Dyna3D files.
//
//  Programmer: Brad Whitlock
//  Creation:   Mon Nov 27 14:00:02 PST 2006
//
//  Modifications:
//    Brad Whitlock, Fri May 11 13:42:07 PST 2007
//    Moved out of VisIt, renamed, and changed interface.
//
//    Brad Whitlock, Fri Mar 19 10:21:46 PDT 2010
//    Updated from VisIt's newer version.
//
//    Brad Whitlock, Thu Jun 10 10:35:22 PDT 2010
//    I added a Write method.
//
//    Brad Whitlock, Mon Oct 4 15:25:39 PST 2010
//    I renamed MaterialCard_t to MaterialProperties and I moved it outside
//    of the class. Then I added set/get methods so we can see more of the
//    material values.
//
// ****************************************************************************

class Dyna3DFile
{
public:
    Dyna3DFile();
    Dyna3DFile(const Dyna3DFile &);
    virtual ~Dyna3DFile();

    bool HasBeenRead() const;

    bool Read(const std::string &filename);
    bool Identify(const std::string &filename);

    void           FreeUpResources(void); 

    // Access methods for material data.
    void                      GetMaterials(intVector &matnos, 
                                           stringVector &matnames, 
                                           doubleVector &matdens);
    const MaterialPropertiesVector &GetMaterials() const;
    int                       GetNumMaterials() const;
    MaterialProperties        GetMaterial(int i) const;
    void                      SetMaterial(int i, const MaterialProperties &);

    // Methods for getting meshes and variables.
    vtkDataSet    *GetMesh(const char *);
    vtkDataArray  *GetVar(const char *);
    vtkDataArray  *GetVectorVar(const char *);

protected:
    typedef struct
    {
        int   nMaterials;
        int   nPoints;
        int   nSolidHexes;
        int   nBeamElements;
        int   nShellElements4;
        int   nShellElements8;
        int   nInterfaceSegments;
        float interfaceInterval;
        float shellTimestep;
    } Card2_t;
 
    typedef struct
    {
        Card2_t card2;
    } ControlCards_t;

    void SkipComments(ifstream &ifile, const char *sectionName,
                      bool &, bool &);
    bool SkipToSection(ifstream &ifile, const char *section);
    void GetLine(ifstream &ifile);

    bool ReadControlCards(ifstream &ifile);
    bool ReadControlCard2(ifstream &);
    void ReadControlCard3(ifstream &);
    void ReadControlCard4(ifstream &);
    void ReadControlCard5(ifstream &);
    void ReadControlCard6(ifstream &);
    void ReadControlCard7(ifstream &);
    void ReadControlCard8(ifstream &);
    void ReadControlCard9(ifstream &);
    void ReadMaterialCards(ifstream &);

    bool ReadControlCardsNewFormat(ifstream &ifile);

    bool ReadFile(const std::string &, int nLines);

    // DATA MEMBERS
    vtkUnstructuredGrid      *ugrid;
    ControlCards_t            cards;
    MaterialPropertiesVector  materialCards;
    int                      *matNumbers;
    char                     *line;
    vtkFloatArray            *velocity;
};

#endif
