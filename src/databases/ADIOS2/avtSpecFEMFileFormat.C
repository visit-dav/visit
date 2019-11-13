// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

// ************************************************************************* //
//                            avtADIOSFileFormat.C                           //
// ************************************************************************* //

#include <avtMTMDFileFormatInterface.h>
#include <avtParallel.h>
#include <avtSpecFEMFileFormat.h>
#include <vtkFloatArray.h>
#include <vtkDoubleArray.h>
#include <vtkPointData.h>
#include <avtDatabaseMetaData.h>
#include <DBOptionsAttributes.h>
#include <InvalidDBTypeException.h>
#include <InvalidVariableException.h>
#include <DebugStream.h>
#include <avtDatabase.h>
#include <vtkUnstructuredGrid.h>
#include <vtkTriangle.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>
#include <VisItStreamUtil.h>
#include <vtkEarthSource.h>
#include <vtkImplicitBoolean.h>
#include <vtkPlane.h>
#include <vtkClipPolyData.h>
#include <vtkAppendPolyData.h>
#include <vtkCellArray.h>

#include <set>
#include <string>
#include <vector>

using namespace std;

int avtSpecFEMFileFormat::NUM_REGIONS = 3;

static inline void
convertToLatLon(double x, double y, double z, double &nx, double &ny, double &nz);

//#define POINT_MESH
#define USE_IBOOL

// ****************************************************************************
//  Method: avtSpecFEMFileFormat::Identify
//
//  Purpose:
//      Determine if this file is of this flavor.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

bool
avtSpecFEMFileFormat::Identify(const char *fname)
{
    string meshNm, dataNm;
    bool valid = false;

    if (avtSpecFEMFileFormat::GenerateFileNames(fname, meshNm, dataNm))
    {
        ifstream mFile(meshNm.c_str()), dFile(dataNm.c_str());
        if (!mFile.fail() && !dFile.fail())
            valid = true;

        if (valid)
            if (! avtSpecFEMFileFormat::IsMeshFile(meshNm))
                valid = false;
        if (valid)
            if (! avtSpecFEMFileFormat::IsDataFile(dataNm))
                valid = false;
    }

    return valid;
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat::CreateInterface
//
//  Purpose:
//      Create an interface for this reader.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtFileFormatInterface *
avtSpecFEMFileFormat::CreateInterface(const char *const *list,
                                      int nList,
                                      int nBlock)
{
    int nTimestepGroups = nList / nBlock;
    avtMTMDFileFormat **ffl = new avtMTMDFileFormat*[nTimestepGroups];
    for (int i = 0 ; i < nTimestepGroups ; i++)
        ffl[i] = new avtSpecFEMFileFormat(list[i*nBlock]);

    return new avtMTMDFileFormatInterface(ffl, nTimestepGroups);
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat constructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtSpecFEMFileFormat::avtSpecFEMFileFormat(const char *nm)
    : avtMTMDFileFormat(nm),
#ifdef PARALLEL
      adiosMesh(std::make_shared<adios2::ADIOS>((MPI_Comm)VISIT_MPI_COMM, adios2::DebugON)),
      adiosData(std::make_shared<adios2::ADIOS>((MPI_Comm)VISIT_MPI_COMM, adios2::DebugON))
#else
      adiosMesh(std::make_shared<adios2::ADIOS>(adios2::DebugON)),
      adiosData(std::make_shared<adios2::ADIOS>(adios2::DebugON))
#endif
{
    string filename(nm), meshNm, dataNm;
    GenerateFileNames(filename, meshNm, dataNm);

    meshIO = adios2::IO(adiosMesh->DeclareIO("ReadBP"));
    meshReader = meshIO.Open(meshNm, adios2::Mode::Read);

    dataIO = adios2::IO(adiosData->DeclareIO("ReadBP"));
    dataReader = dataIO.Open(dataNm, adios2::Mode::Read);

    initialized = false;
    //This needs to be put into the file.
    ngllx = nglly = ngllz = 5;
    //ngllx = nglly = ngllz = 1;
    kernelFile = false;
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat destructor
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

avtSpecFEMFileFormat::~avtSpecFEMFileFormat()
{
}


// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetNTimesteps
//
//  Purpose:
//      Tells the rest of the code how many timesteps there are in this file.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

int
avtSpecFEMFileFormat::GetNTimesteps()
{
    return 1;
    //return file->NumTimeSteps();
}

// ****************************************************************************
//  Method: avtEMSTDFileFormat::GetCycles
//
//  Purpose:
//
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtSpecFEMFileFormat::GetCycles(std::vector<int> &cycles)
{
    cycles.push_back(0);
}


// ****************************************************************************
// Method:  avtSpecFEMFileFormat::GetTimes
//
// Purpose:
//
// Programmer:  Dave Pugmire
// Creation:    January 26, 2011
//
// ****************************************************************************

void
avtSpecFEMFileFormat::GetTimes(std::vector<double> &times)
{
    times.push_back(0.0);
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::FreeUpResources
//
//  Purpose:
//      When VisIt is done focusing on a particular timestep, it asks that
//      timestep to free up any resources (memory, file descriptors) that
//      it has associated with it.  This method is the mechanism for doing
//      that.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
// ****************************************************************************

void
avtSpecFEMFileFormat::FreeUpResources(void)
{
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::PopulateDatabaseMetaData
//
//  Purpose:
//      This database meta-data object is like a table of contents for the
//      file.  By populating it, you are telling the rest of VisIt what
//      information it can request from you.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Add expressions.
//
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

void
avtSpecFEMFileFormat::PopulateDatabaseMetaData(avtDatabaseMetaData *md, int timeState)
{
    debug5<<"avtSpecFEMFileFormat::PopulateDatabaseMetaData()"<<endl;
    Initialize();
    md->SetFormatCanDoDomainDecomposition(false);

    //Add the entire mesh.
#ifdef USE_IBOOL
    AddMeshToMetaData(md, "mesh", AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
    AddMeshToMetaData(md, "LatLon_mesh", AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
#endif
#ifdef POINT_MESH
    AddMeshToMetaData(md, "mesh", AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
    AddMeshToMetaData(md, "LatLon_mesh", AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
#endif

    for (int i = 0; i < regions.size(); i++)
    {
        if (regions[i])
        {
            avtMeshMetaData *mmd = new avtMeshMetaData;
            char nm[128], nm2[128];
            sprintf(nm, "reg%d/mesh", i+1);
            sprintf(nm2, "reg%d/LatLon_mesh", i+1);
#ifdef USE_IBOOL
            AddMeshToMetaData(md, nm, AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
            AddMeshToMetaData(md, nm2, AVT_UNSTRUCTURED_MESH, NULL, numBlocks, 0, 3, 3);
#endif
#ifdef POINT_MESH
            AddMeshToMetaData(md, nm, AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
            AddMeshToMetaData(md, nm2, AVT_POINT_MESH, NULL, numBlocks, 0, 3, 1);
#endif
        }
    }

    AddMeshToMetaData(md, "hotSpots", AVT_POINT_MESH, NULL, 1, 0, 3, 1);
    AddMeshToMetaData(md, "LatLon_hotSpots", AVT_POINT_MESH, NULL, 1, 0, 3, 1);
    AddMeshToMetaData(md, "volcanos", AVT_POINT_MESH, NULL, 1, 0, 3, 1);
    AddMeshToMetaData(md, "LatLon_volcanos", AVT_POINT_MESH, NULL, 1, 0, 3, 1);

    AddMeshToMetaData(md, "continents", AVT_SURFACE_MESH, NULL, 1, 0, 3, 2);
    AddMeshToMetaData(md, "LatLon_continents", AVT_SURFACE_MESH, NULL, 1, 0, 3, 2);

    AddMeshToMetaData(md, "plates", AVT_SURFACE_MESH, NULL, 1, 0, 3, 2);
    AddMeshToMetaData(md, "LatLon_plates", AVT_SURFACE_MESH, NULL, 1, 0, 3, 2);

    bool allRegionsPresent = true;
    for (int i = 0; i < regions.size(); i++)
        allRegionsPresent &= regions[i];

    auto variables = dataIO.AvailableVariables();
    for (auto &v : variables)
    {
        auto vname = v.first;
        if (vname.find("/array") == string::npos)
            continue;
        vname = GetVariable(vname);
        //cout<<"VARNAME= "<<vname<<endl;

        if (kernelFile)
        {
            if (vname != string("betav_kl_crust_mantle"))
                continue;
            AddScalarVarToMetaData(md, vname, "reg1/mesh", AVT_NODECENT);
            char var[128];
            sprintf(var, "LatLon/%s", vname.c_str());
            AddScalarVarToMetaData(md, var, "reg1/LatLon_mesh", AVT_NODECENT);
        }
        else
        {
            //Add var only if all regions present.
            if (allRegionsPresent)
            {
                AddScalarVarToMetaData(md, vname, "mesh", AVT_NODECENT);
                AddScalarVarToMetaData(md, vname, "LatLon_mesh", AVT_NODECENT);

                AddVectorVarToMetaData(md, "LatLonR_coords", "mesh", AVT_NODECENT, 3);
            }

            for (int i = 0; i < regions.size(); i++)
            {
                if (regions[i])
                {
                    char mesh[128], mesh2[128], var[128], var2[128];
                    sprintf(mesh, "reg%d/mesh", i+1);
                    sprintf(mesh2, "reg%d/LatLon_mesh", i+1);
                    sprintf(var, "reg%d/%s", i+1, vname.c_str());
                    sprintf(var2, "reg%d/LatLon/%s", i+1, vname.c_str());
                    AddScalarVarToMetaData(md, var, mesh, AVT_NODECENT);
                    AddScalarVarToMetaData(md, var2, mesh2, AVT_NODECENT);

                    sprintf(var, "reg%d/LatLonR_coords", i+1);
                    AddVectorVarToMetaData(md, var, mesh, AVT_NODECENT, 3);
                }
            }
        }
    }
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetHotSpotsMesh
//
// Purpose:
//   Get mesh of hot spots.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetHotSpotsMesh(bool xyzMesh)
{
    vector<pair<int, int> > hotspots;

    hotspots.push_back(make_pair(12, 42)); //"AFAR
    hotspots.push_back(make_pair(-37, 77)); //AMSTERDAM/ST.PAUL
    hotspots.push_back(make_pair(-8, -14)); //ASCENSION
    hotspots.push_back(make_pair(38, -28)); //AZORES
    hotspots.push_back(make_pair(27, -113)); //BAJA/GUADALUPE
    hotspots.push_back(make_pair(-67, 165)); //BALLENY
    hotspots.push_back(make_pair(30, -60)); //BERMUDA
    hotspots.push_back(make_pair(-54, 3)); //BOUVET
    hotspots.push_back(make_pair(54, -136)); //"BOWIE SEAMOUNT
    hotspots.push_back(make_pair(-54, 9)); //CAMEROON LINE
    hotspots.push_back(make_pair(28, -17)); //CANARY ISLANDS
    hotspots.push_back(make_pair(15, -24)); //CAPE VERDE
    hotspots.push_back(make_pair(5, 166)); //CAROLINE ISLANDS
    hotspots.push_back(make_pair(-35, 80)); //CHRISTMAS
    hotspots.push_back(make_pair(46, -130)); //COBB
    hotspots.push_back(make_pair(-12, 44)); //COMORES ISLANDS
    hotspots.push_back(make_pair(-29, -140)); //COOK-AUSTRALS
    hotspots.push_back(make_pair(-46, 45)); //CROZET/PRINCE EDWARD
    hotspots.push_back(make_pair(13, 24)); //DARFUR
    hotspots.push_back(make_pair(-42, 0)); //DISCOVERY
    hotspots.push_back(make_pair(6, 34)); //E.AFRICA
    hotspots.push_back(make_pair(-38, 143)); //E.AUSTRALIA
    hotspots.push_back(make_pair(-27, -109)); //EASTER/SALA Y GOMEZ
    hotspots.push_back(make_pair(50, 7)); //EIFEL
    hotspots.push_back(make_pair(-4, -32)); //FERNANDO DO NORONA
    hotspots.push_back(make_pair(-39, -111)); //FOUNDATION SMTS.
    hotspots.push_back(make_pair(1, -86)); //GALAPAGOS
    hotspots.push_back(make_pair(-40, -10)); //GOUGH
    hotspots.push_back(make_pair(30, 28)); //GREAT METEOR
    hotspots.push_back(make_pair(29, -118)); //GUADALUPE
    hotspots.push_back(make_pair(20, -155)); //HAWAII
    hotspots.push_back(make_pair(23, 6)); //"HOGGAR MOUNTAINS
    hotspots.push_back(make_pair(64, -20)); //ICELAND
    hotspots.push_back(make_pair(71, -8)); //JAN MAYEN
    hotspots.push_back(make_pair(-34, -79)); //JUAN FERNANDEZ
    hotspots.push_back(make_pair(-49, 63)); //KERGUELEN
    hotspots.push_back(make_pair(-31, 159)); //LORD HOWE
    hotspots.push_back(make_pair(-51, -138)); //LOUISVILLE HOTSPOT
    hotspots.push_back(make_pair(-29, -140)); //MACDONALD
    hotspots.push_back(make_pair(33, -17)); //MADEIRA
    hotspots.push_back(make_pair(-47, 38)); //MARION
    hotspots.push_back(make_pair(-8, -138)); //MARQUESAS ISLANDS
    hotspots.push_back(make_pair(-21, -154)); //MARSHALL-GILBERT ISLANDS
    hotspots.push_back(make_pair(-21, -29)); //MARTEN VAZ
    hotspots.push_back(make_pair(-52, 1)); //METEOR
    hotspots.push_back(make_pair(-78, 167)); //MOUNT EREBUS
    hotspots.push_back(make_pair(-32, 28)); //NEW ENGLAND
    hotspots.push_back(make_pair(-24, -130)); //PITCAIRN ISLAND
    hotspots.push_back(make_pair(36, -104)); //"RATON
    hotspots.push_back(make_pair(-21, 56)); //REUNION (FOURNAISE) (GVP)
    hotspots.push_back(make_pair(-77, 168)); //ROSS SEA
    hotspots.push_back(make_pair(-14, -170)); //SAMOA
    hotspots.push_back(make_pair(-26, -80)); //SAN FELIX
    hotspots.push_back(make_pair(-52, 0)); //SHONA
    hotspots.push_back(make_pair(19, -111)); //SOCORRO
    hotspots.push_back(make_pair(-17, -10)); //ST. HELENA
    hotspots.push_back(make_pair(-39, 78)); //ST. PAUL ISLAND (GVP)
    hotspots.push_back(make_pair(-18, -150)); //TAHITI
    hotspots.push_back(make_pair(-39, 157)); //TASMANTID
    hotspots.push_back(make_pair(21, 17)); //"TIBESTI
    hotspots.push_back(make_pair(-21, -29)); //TRINIDADE/MARTEN
    hotspots.push_back(make_pair(-37, -12)); //TRISTAN DA CUNHA
    hotspots.push_back(make_pair(-32, 16)); //VEMA SEAMOUNT
    hotspots.push_back(make_pair(44, -111)); //YELLOWSTONE

    int nPts = hotspots.size();
    vtkPoints *pts = vtkPoints::New();
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    mesh->SetPoints(pts);

    vtkIdType vid;
    const double toRad = M_PI/180.0;
    for (int i = 0; i < nPts; i++)
    {
        //first, convert to x,y,z
        double lat = hotspots[i].first * toRad;
        double lon = hotspots[i].second * toRad;
        double x = cos(lat)*cos(lon);
        double y = cos(lat)*sin(lon);
        double z = sin(lat);

        if (xyzMesh)
            vid = pts->InsertNextPoint(x,y,z);
        else
        {
            double nx, ny, nz;
            convertToLatLon(x,y,z, nx,ny,nz);
            vid = pts->InsertNextPoint(nx,ny,nz);
        }
        mesh->InsertNextCell(VTK_VERTEX, 1, &vid);
    }
    pts->Delete();

    return mesh;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVolcanoMesh
//
// Purpose:
//   Get mesh of hot spots.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetVolcanoMesh(bool xyzMesh)
{
    vector<pair<float, float> > volcanos;

    volcanos.push_back(make_pair(131.60, 34.50)); //Abu_Japan
    volcanos.push_back(make_pair(-67.62, -23.30)); //Acamarachi_Chile
    volcanos.push_back(make_pair(-90.88, 14.50)); //Acatenango_Guatemala
    volcanos.push_back(make_pair(34.52, 38.57)); //Acigl-Nevsehir_Turkey
    volcanos.push_back(make_pair(-121.49, 46.21)); //Adams_USA
    volcanos.push_back(make_pair(-129.27, -25.37)); //Adams_Seamount_Pacific_Ocean
    volcanos.push_back(make_pair(140.29, 37.64)); //Adatara_Japan
    volcanos.push_back(make_pair(40.84, 10.07)); //Adwa_Ethiopia
    volcanos.push_back(make_pair(40.85, 13.08)); //Afder_Ethiopia
    volcanos.push_back(make_pair(145.67, 18.77)); //Agrigan_Mariana_Islands
    volcanos.push_back(make_pair(-90.74, 14.47)); //Agua_Guatemala
    volcanos.push_back(make_pair(-25.47, 37.77)); //Agua_de_Pau_Azores
    volcanos.push_back(make_pair(-73.75, -50.33)); //Aguilera_Chile
    volcanos.push_back(make_pair(115.51, -8.34)); //Agung_Lesser_Sunda_Islands
    volcanos.push_back(make_pair(145.03, 20.42)); //Ahyi_Mariana_Islands
    volcanos.push_back(make_pair(159.45, 53.98)); //Akademia_Nauk_Russia
    volcanos.push_back(make_pair(139.20, 36.56)); //Akagi_Japan
    volcanos.push_back(make_pair(144.01, 43.38)); //Akan_Japan
    volcanos.push_back(make_pair(158.65, 55.43)); //Akhtang_Russia
    volcanos.push_back(make_pair(140.80, 39.76)); //Akita_Komagatake_Japan
    volcanos.push_back(make_pair(140.76, 39.96)); //Akita_Yakeyama_Japan
    volcanos.push_back(make_pair(129.60, 29.46)); //Akuseki-jima_Japan
    volcanos.push_back(make_pair(-165.99, 54.13)); //Akutan_USA
    volcanos.push_back(make_pair(155.55, 50.86)); //Alaid_Russia
    volcanos.push_back(make_pair(145.83, 17.60)); //Alamagan_Mariana_Islands
    volcanos.push_back(make_pair(40.57, 12.88)); //Alayta_Ethiopia
    volcanos.push_back(make_pair(12.70, 41.73)); //Alban_Hills_Italy
    volcanos.push_back(make_pair(-91.12, -0.43)); //Alcedo_Galpagos_Islands
    volcanos.push_back(make_pair(40.63, 13.52)); //Ale_Bagu_Ethiopia
    volcanos.push_back(make_pair(39.92, 14.88)); //Alid_Eritrea
    volcanos.push_back(make_pair(-67.96, -22.44)); //Altiplano_Puna_Volcanic_Complex
    volcanos.push_back(make_pair(168.12, -16.25)); //Ambrym_Vanuatu
    volcanos.push_back(make_pair(145.67, 16.35)); //Anatahan_Mariana_Islands
    volcanos.push_back(make_pair(169.78, -20.20)); //Aneityum_Vanuatu
    volcanos.push_back(make_pair(-158.17, 56.88)); //Aniakchak_USA
    volcanos.push_back(make_pair(-78.14, -0.48)); //Antisana_Ecuador
    volcanos.push_back(make_pair(167.83, -15.40)); //Aoba_Vanuatu
    volcanos.push_back(make_pair(44.30, 39.70)); //Ararat_Turkey
    volcanos.push_back(make_pair(-84.70, 10.46)); //Arenal_Costa_Rica
    volcanos.push_back(make_pair(-69.05, -18.73)); //Arintica_Chile
    volcanos.push_back(make_pair(157.83, 52.36)); //Asacha_Russia
    volcanos.push_back(make_pair(138.53, 36.40)); //Asama_Japan
    volcanos.push_back(make_pair(-14.37, -7.95)); //Ascensin_South_Atlantic_Ocean
    volcanos.push_back(make_pair(-16.75, 65.03)); //Askja_Iceland
    volcanos.push_back(make_pair(131.11, 32.88)); //Aso_Japan
    volcanos.push_back(make_pair(-91.19, 14.58)); //Atitln_Guatemala
    volcanos.push_back(make_pair(-174.15, 52.38)); //Atka_USA
    volcanos.push_back(make_pair(147.14, 44.81)); //Atsonupuri_Russia
    volcanos.push_back(make_pair(-153.43, 59.36)); //Augustine_USA
    volcanos.push_back(make_pair(158.83, 53.26)); //Avachinsky_Russia
    volcanos.push_back(make_pair(125.50, 3.67)); //Awu_Indonesia
    volcanos.push_back(make_pair(-130.00, 45.95)); //Axial_Seamount_Pacific_Ocean
    volcanos.push_back(make_pair(40.70, 10.08)); //Ayelu_Ethiopia
    volcanos.push_back(make_pair(-68.24, -21.79)); //Azufre_Chile
    volcanos.push_back(make_pair(140.25, 37.73)); //Azuma_Japan
    volcanos.push_back(make_pair(121.90, 19.50)); //Babuyon_Claro_Philippines
    volcanos.push_back(make_pair(121.60, 43.80)); //Bachelor_USA
    volcanos.push_back(make_pair(155.20, -6.10)); //Bagana_Papua_New_Guinea
    volcanos.push_back(make_pair(-121.82, 48.79)); //Baker_USA
    volcanos.push_back(make_pair(144.85, -3.60)); //Bam_Papua_New_Guinea
    volcanos.push_back(make_pair(151.20, -5.20)); //Bamus_Papua_New_Guinea
    volcanos.push_back(make_pair(140.10, 37.60)); //Bandai_Japan
    volcanos.push_back(make_pair(93.88, 12.29)); //Barren_Island_India
    volcanos.push_back(make_pair(36.57, 2.32)); //Barrier_Kenya
    volcanos.push_back(make_pair(60.00, 28.00)); //Bazman_Iran
    volcanos.push_back(make_pair(-8.20, 71.10)); //Beerenberg_Norway
    volcanos.push_back(make_pair(-121.83, 44.28)); //Belknap_USA
    volcanos.push_back(make_pair(160.58, 55.98)); //Bezymianny_Russia
    volcanos.push_back(make_pair(124.50, 11.50)); //Biliran_Philippines
    volcanos.push_back(make_pair(160.02, 54.32)); //Bolshoi_Semiachik_Russia
    volcanos.push_back(make_pair(124.10, 12.80)); //Bulusan_Philippines
    volcanos.push_back(make_pair(38.35, 8.05)); //Butajira_Ethiopia
    volcanos.push_back(make_pair(9.17, 4.20)); //Cameroon_Mount_Cameroon
    volcanos.push_back(make_pair(121.85, 18.83)); //Camiguin_de_Babuyanes_Philippines
    volcanos.push_back(make_pair(123.13, 10.41)); //Canlaon_Philippines
    volcanos.push_back(make_pair(-104.09, 36.45)); //Capulin_USA
    volcanos.push_back(make_pair(-106.00, 33.70)); //Carrizozo_USA
    volcanos.push_back(make_pair(-77.99, 0.03)); //Cayambe_Ecuador
    volcanos.push_back(make_pair(108.41, -6.90)); //Cereme_Indonesia
    volcanos.push_back(make_pair(2.97, 45.78)); //Chaine_des_Puys_France
    volcanos.push_back(make_pair(-156.99, 57.13)); //Chiginagak_USA
    volcanos.push_back(make_pair(155.46, 50.33)); //Chikurachki_Russia
    volcanos.push_back(make_pair(-78.82, -1.46)); //Chimborazo_Ecuador
    volcanos.push_back(make_pair(-169.95, 52.83)); //Cleveland_USA
    volcanos.push_back(make_pair(-89.54, 13.86)); //Coatepeque_El_Salvador
    volcanos.push_back(make_pair(103.62, 19.51)); //Colima_Mexico
    volcanos.push_back(make_pair(85.60, 11.50)); //Concepcin_Nicaragua
    volcanos.push_back(make_pair(-71.17, -37.85)); //Copahue_Argentina
    volcanos.push_back(make_pair(-87.56, 12.98)); //Cosigina_Nicaragua
    volcanos.push_back(make_pair(-78.44, -0.68)); //Cotopaxi_Ecuador
    volcanos.push_back(make_pair(-122.11, 42.94)); //Crater_Lake_USA
    volcanos.push_back(make_pair(-113.50, 43.42)); //Craters_of_the_Moon_USA
    volcanos.push_back(make_pair(142.90, 43.70)); //Daisetsu_Japan
    volcanos.push_back(make_pair(150.10, -5.00)); //Dakataua_Papua_New_Guinea
    volcanos.push_back(make_pair(52.11, 35.95)); //Damavand_Iran
    volcanos.push_back(make_pair(103.10, -4.00)); //Dempo_Indonesia
    volcanos.push_back(make_pair(-120.90, 43.50)); //Devils_Garden_USA
    volcanos.push_back(make_pair(-104.70, 44.60)); //Devils_Tower_USA
    volcanos.push_back(make_pair(-118.70, 43.10)); //Diamond_Craters_USA
    volcanos.push_back(make_pair(109.90, -7.20)); //Dieng_Volcanic_Complex_Indonesia
    volcanos.push_back(make_pair(156.98, 51.46)); //Diky_Greben_Russia
    volcanos.push_back(make_pair(41.80, 13.60)); //Dubbi_Eritrea
    volcanos.push_back(make_pair(127.87, 1.70)); //Dukono_Indonesia
    volcanos.push_back(make_pair(168.40, -16.70)); //East_Epi_Vanuatu
    volcanos.push_back(make_pair(156.02, 50.68)); //Ebeko_Russia
    volcanos.push_back(make_pair(174.10, -39.30)); //Egmont_Taranaki_New_Zealand
    volcanos.push_back(make_pair(122.45, -8.70)); //Egon_Indonesia
    volcanos.push_back(make_pair(-93.23, 17.36)); //El_Chichon_Mexico
    volcanos.push_back(make_pair(42.45, 43.33)); //Elbrus_Russia
    volcanos.push_back(make_pair(167.20, -77.50)); //Erebus_Antarctica
    volcanos.push_back(make_pair(40.70, 13.60)); //Erta_Ale_Ethiopia
    volcanos.push_back(make_pair(15.00, 37.73)); //Etna_Italy
    volcanos.push_back(make_pair(39.93, 8.98)); //Fantale_Ethiopia
    volcanos.push_back(make_pair(-91.55, -0.37)); //Fernandina_Ecuador
    volcanos.push_back(make_pair(-164.42, -54.63)); //Fisher_USA
    volcanos.push_back(make_pair(-24.35, 14.95)); //Fogo_Cape_Verde_Islands
    volcanos.push_back(make_pair(-121.07, 43.37)); //Fort_Rock_USA
    volcanos.push_back(make_pair(-162.81, 55.08)); //Frosty_Peak_USA
    volcanos.push_back(make_pair(-90.88, 14.47)); //Fuego_Guatemala
    volcanos.push_back(make_pair(-14.02, 28.36)); //Fuerteventura_Spain
    volcanos.push_back(make_pair(138.73, 35.36)); //Fuji_Japan
    volcanos.push_back(make_pair(-25.33, 37.76)); //Furnas_Azores_Portugal
    volcanos.push_back(make_pair(155.25, 50.27)); //Fuss_Peak_Russia
    volcanos.push_back(make_pair(-77.37, 1.22)); //Galeras_Colombia
    volcanos.push_back(make_pair(108.08, -7.26)); //Galunggung_Indonesia
    volcanos.push_back(make_pair(127.33, 0.81)); //Gamalama_Indonesia
    volcanos.push_back(make_pair(160.68, 54.97)); //Gamchen_Russia
    volcanos.push_back(make_pair(127.53, 1.38)); //Gamkonora_Indonesia
    volcanos.push_back(make_pair(178.80, 51.80)); //Gareloi_USA
    volcanos.push_back(make_pair(167.50, -14.27)); //Gaua_Vanuatu
    volcanos.push_back(make_pair(106.94, -6.77)); //Gede
    volcanos.push_back(make_pair(-121.11, 48.11)); //Glacier_Peak_USA
    volcanos.push_back(make_pair(158.03, 52.56)); //Gorely_Russia
    volcanos.push_back(make_pair(-27.97, 39.02)); //Graciosa_Azores_Portugal
    volcanos.push_back(make_pair(15.58, 28.00)); //Gran_Canaria_Canary_Islands_Spain
    volcanos.push_back(make_pair(-176.13, 52.07)); //Great_Sitkin_USA
    volcanos.push_back(make_pair(-155.09, 58.35)); //Griggs_USA
    volcanos.push_back(make_pair(-17.33, 64.42)); //Grmsvtn_Iceland
    volcanos.push_back(make_pair(-78.60, -0.17)); //Guagua_Pichincha
    volcanos.push_back(make_pair(-69.09, -18.42)); //Guallatiri_Chile
    volcanos.push_back(make_pair(145.84, 17.31)); //Guguan_USA
    volcanos.push_back(make_pair(139.02, 35.23)); //Hakone
    volcanos.push_back(make_pair(73.50, -53.10)); //Heard_Australia
    volcanos.push_back(make_pair(124.70, 9.20)); //Hibok-hibok_Philippines
    volcanos.push_back(make_pair(-18.03, 27.73)); //Hierro_Spain
    volcanos.push_back(make_pair(-121.70, 45.40)); //Hood_USA
    volcanos.push_back(make_pair(-155.87, 19.69)); //Hualalai_USA
    volcanos.push_back(make_pair(127.63, 1.49)); //Ibu_Indonesia
    volcanos.push_back(make_pair(114.24, -8.06)); //Ijen_Indonesia
    volcanos.push_back(make_pair(-153.09, 60.03)); //Iliamna_USA
    volcanos.push_back(make_pair(123.26, -8.34)); //Iliboleng_Indonesia
    volcanos.push_back(make_pair(-78.71, -0.66)); //Iliniza_Ecuador
    volcanos.push_back(make_pair(-89.05, 13.67)); //Ilopango_El_Salvador
    volcanos.push_back(make_pair(157.20, 51.49)); //Ilyinsky_Russia
    volcanos.push_back(make_pair(141.29, 24.75)); //Ioto_Iwo-jima_Japan
    volcanos.push_back(make_pair(122.01, 20.47)); //Iraya_Philippines
    volcanos.push_back(make_pair(83.85, 9.98)); //Irazu_Costa_Rica
    volcanos.push_back(make_pair(-163.72, 54.77)); //Isanotski_USA
    volcanos.push_back(make_pair(-89.63, 13.81)); //Izalco_El_Salvador
    volcanos.push_back(make_pair(-98.64, 19.18)); //Iztaccihuatl_Mexico
    volcanos.push_back(make_pair(139.10, 34.90)); //Izu-Tobu_Japan
    volcanos.push_back(make_pair(127.42, 1.08)); //Jailolo_Indonesia
    volcanos.push_back(make_pair(-121.80, 44.67)); //Jefferson_USA
    volcanos.push_back(make_pair(-117.40, 43.10)); //Jordan_Craters_USA
    volcanos.push_back(make_pair(144.63, -3.63)); //Kadovar_Papua_New_Guinea
    volcanos.push_back(make_pair(-154.06, 58.62)); //Kaguyak_United_States
    volcanos.push_back(make_pair(-156.57, 20.55)); //Kahoolawe_United_States
    volcanos.push_back(make_pair(156.87, 51.30)); //Kambalny_Russia
    volcanos.push_back(make_pair(-177.17, 51.92)); //Kanaga_United_States
    volcanos.push_back(make_pair(123.13, 10.41)); //Kanlaon_Philippines
    volcanos.push_back(make_pair(125.40, 2.78)); //Karangetang_[Api_Siau]_Indonesia
    volcanos.push_back(make_pair(29.45, -1.51)); //Karisimbi_Congo
    volcanos.push_back(make_pair(43.38, -11.75)); //Karthala_Comoros
    volcanos.push_back(make_pair(159.45, 54.05)); //Karymsky_Russia
    volcanos.push_back(make_pair(-19.05, 63.63)); //Katla_Iceland
    volcanos.push_back(make_pair(-154.96, 58.28)); //Katmai_United_States
    volcanos.push_back(make_pair(29.92, -0.08)); //Katwe-Kikorongo_Uganda
    volcanos.push_back(make_pair(157.95, -9.02)); //Kavachi_Solomon_Islands
    volcanos.push_back(make_pair(157.35, 51.65)); //Kell_Russia
    volcanos.push_back(make_pair(112.31, -7.93)); //Kelut_Indonesia
    volcanos.push_back(make_pair(101.26, 1.70)); //Kerinci_Indonesia
    volcanos.push_back(make_pair(157.70, 52.06)); //Khodutka_Russia
    volcanos.push_back(make_pair(-156.75, 57.20)); //Kialagvik_United_States
    volcanos.push_back(make_pair(-61.64, 12.30)); //Kick_em_Jenny_Grenada
    volcanos.push_back(make_pair(130.31, 30.79)); //Kikai_Japan
    volcanos.push_back(make_pair(160.25, 54.49)); //Kikhpinych_Russia
    volcanos.push_back(make_pair(-155.29, 19.42)); //Kilauea_United_States
    volcanos.push_back(make_pair(37.35, 3.07)); //Kilimanjaro_Tanzania
    volcanos.push_back(make_pair(130.86, 31.93)); //Kirishima_Japan
    volcanos.push_back(make_pair(160.64, 56.06)); //Kliuchevskoi_Russia
    volcanos.push_back(make_pair(140.68, 42.06)); //Komagatake_Japan
    volcanos.push_back(make_pair(39.69, 8.80)); //Kone_Ethiopia
    volcanos.push_back(make_pair(158.69, 53.32)); //Koryaksky_Russia
    volcanos.push_back(make_pair(156.75, 51.36)); //Koshelev_Russia
    volcanos.push_back(make_pair(-16.78, 65.73)); //Krafla_Iceland
    volcanos.push_back(make_pair(105.42, -6.10)); //Krakatau_Indonesia
    volcanos.push_back(make_pair(160.27, 54.59)); //Krasheninnikov_Russia
    volcanos.push_back(make_pair(160.53, 54.75)); //Kronotsky_Russia
    volcanos.push_back(make_pair(157.53, 51.80)); //Ksudach_Russia
    volcanos.push_back(make_pair(-154.36, 58.45)); //Kukak_United_States
    volcanos.push_back(make_pair(138.54, 36.64)); //Kusatsu-Shirane_Japan
    volcanos.push_back(make_pair(144.44, 43.61)); //Kutcharo_Japan
    volcanos.push_back(make_pair(141.16, 42.49)); //Kuttara_Japan
    volcanos.push_back(make_pair(168.54, -16.83)); //Kuwae_Vanuatu
    volcanos.push_back(make_pair(-17.83, 28.57)); //La_Palma_Spain
    volcanos.push_back(make_pair(148.15, -8.95)); //Lamington_Papua_New_Guinea
    volcanos.push_back(make_pair(148.42, -5.53)); //Langila_Papua_New_Guinea
    volcanos.push_back(make_pair(-13.63, 29.03)); //Lanzarote_Spain
    volcanos.push_back(make_pair(-67.73, -23.37)); //Lscar_Chile
    volcanos.push_back(make_pair(122.84, -8.36)); //Lereboleng_Indonesia
    volcanos.push_back(make_pair(122.78, -8.54)); //Lewotobi_Indonesia
    volcanos.push_back(make_pair(123.51, -8.27)); //Lewotolo_Indonesia
    volcanos.push_back(make_pair(-62.80, 17.37)); //Liamuiga_St_Kitts_and_Nevis
    volcanos.push_back(make_pair(14.95, 38.48)); //Lipari_Italy
    volcanos.push_back(make_pair(178.54, 51.95)); //Little_Sitkin_United_States
    volcanos.push_back(make_pair(-155.27, 18.92)); //Loihi_United_States
    volcanos.push_back(make_pair(124.79, 1.36)); //Lokon-Empung_Indonesia
    volcanos.push_back(make_pair(150.51, -5.47)); //Lolo_Papua_New_Guinea
    volcanos.push_back(make_pair(151.16, -4.92)); //Lolobau_Papua_New_Guinea
    volcanos.push_back(make_pair(155.62, -6.52)); //Loloru_Papua_New_Guinea
    volcanos.push_back(make_pair(147.12, -5.36)); //Long_Island_Papua_New_Guinea
    volcanos.push_back(make_pair(36.45, -0.91)); //Longonot_Kenya
    volcanos.push_back(make_pair(168.35, -16.51)); //Lopevi_Vanuatu
    volcanos.push_back(make_pair(-85.52, 11.45)); //Maderas_Nicaragua
    volcanos.push_back(make_pair(155.25, 58.20)); //Mageik_United_States
    volcanos.push_back(make_pair(124.86, 1.36)); //Mahawu_Indonesia
    volcanos.push_back(make_pair(127.40, 0.32)); //Makian_Indonesia
    volcanos.push_back(make_pair(159.67, 54.13)); //Maly_Semiachik_Russia
    volcanos.push_back(make_pair(145.04, -4.08)); //Manam_Papua_New_Guinea
    volcanos.push_back(make_pair(120.47, 14.52)); //Mariveles_Philippines
    volcanos.push_back(make_pair(37.97, 2.32)); //Marsabit_Kenya
    volcanos.push_back(make_pair(-86.16, 11.98)); //Masaya_Nicaragua
    volcanos.push_back(make_pair(156.72, 51.10)); //Mashkovtsev_Russia
    volcanos.push_back(make_pair(144.57, 43.57)); //Mashu_Japan
    volcanos.push_back(make_pair(-155.47, 19.82)); //Mauna_Kea_United_States
    volcanos.push_back(make_pair(-155.61, 19.48)); //Mauna_Loa_United_States
    volcanos.push_back(make_pair(123.69, 13.26)); //Mayon_Philippines
    volcanos.push_back(make_pair(72.60, -53.03)); //McDonald_Islands_Australia
    volcanos.push_back(make_pair(-122.32, 42.45)); //McLoughlin_United_States
    volcanos.push_back(make_pair(4.08, 37.42)); //Mega_Basalt_Field_Ethiopia
    volcanos.push_back(make_pair(-111.96, 43.70)); //Menan_Buttes_United_States
    volcanos.push_back(make_pair(36.07, -0.20)); //Menengai_Kenya
    volcanos.push_back(make_pair(110.44, -7.54)); //Merapi_Indonesia
    volcanos.push_back(make_pair(110.43, -7.45)); //Merbabu_Indonesia
    volcanos.push_back(make_pair(36.75, -3.25)); //Meru_Tanzania
    volcanos.push_back(make_pair(23.34, 37.62)); //Methana_Greece
    volcanos.push_back(make_pair(-174.87, -19.18)); //Metis_Shoal_Tonga
    volcanos.push_back(make_pair(-26.45, -57.78)); //Michael_United_Kingdom
    volcanos.push_back(make_pair(-101.75, 19.85)); //Michoacan-Guanajuato_Mexico
    volcanos.push_back(make_pair(24.44, 36.70)); //Mlos_Greece
    volcanos.push_back(make_pair(139.53, 34.08)); //Miyake-jima_Japan
    volcanos.push_back(make_pair(-176.75, 51.94)); //Moffett_United_States
    volcanos.push_back(make_pair(-78.27, 0.13)); //Mojanda_Ecuador
    volcanos.push_back(make_pair(-86.54, 12.42)); //Momotombo_Nicaragua
    volcanos.push_back(make_pair(-25.88, 37.60)); //Monaco_Bank_Portugal
    volcanos.push_back(make_pair(-119.03, 38.00)); //Mono_Lake_Vol_Field_United_States
    volcanos.push_back(make_pair(-177.19, -25.89)); //Monowai_Seamount_New_Zealand
    volcanos.push_back(make_pair(127.40, 0.45)); //Moti_Indonesia
    volcanos.push_back(make_pair(110.88, -6.62)); //Muria_Indonesia
    volcanos.push_back(make_pair(139.48, 36.76)); //Nantai_Japan
    volcanos.push_back(make_pair(139.97, 37.12)); //Nasu_Japan
    volcanos.push_back(make_pair(-86.70, 12.51)); //Negro_Cerro_Nicaragua
    volcanos.push_back(make_pair(154.81, 49.57)); //Nemo_Peak_Russia
    volcanos.push_back(make_pair(-62.58, 17.15)); //Nevis_Peak_St_Kitts_and_Nevis
    volcanos.push_back(make_pair(-121.23, 43.72)); //Newberry_United_States
    volcanos.push_back(make_pair(142.50, -37.77)); //Newer_Volcanics_Province_Australia
    volcanos.push_back(make_pair(175.64, -39.13)); //Ngauruhoe_New_Zealand
    volcanos.push_back(make_pair(139.38, 36.80)); //Nikko-Shirane_Japan
    volcanos.push_back(make_pair(143.04, 43.45)); //Nipesotsu-Maruyama_Japan
    volcanos.push_back(make_pair(27.16, 36.59)); //Nisyros_Greece
    volcanos.push_back(make_pair(37.87, 0.23)); //Nyambeni_Hills_Kenya
    volcanos.push_back(make_pair(29.20, -1.41)); //Nyamuragira_Democratic_Republic_of_Congo
    volcanos.push_back(make_pair(29.25, -1.52)); //Nyiragongo_Democratic_Republic_of_Congo
    volcanos.push_back(make_pair(-68.55, -27.12)); //Ojos_del_Salado_Chile/Argentina
    volcanos.push_back(make_pair(176.50, -38.12)); //Okataina_New_Zealand
    volcanos.push_back(make_pair(-168.13, 53.43)); //Okmok_United_States
    volcanos.push_back(make_pair(10.50, 6.25)); //Oku_Volcanic_Field_Cameroon
    volcanos.push_back(make_pair(39.91, -2.76)); //Ol_Doinyo_Lengai_Tanzania
    volcanos.push_back(make_pair(157.34, 52.54)); //Opala_Russia
    volcanos.push_back(make_pair(-16.65, 64.00)); //Oraefajokull_Iceland
    volcanos.push_back(make_pair(139.40, 34.72)); //Oshima_Japan
    volcanos.push_back(make_pair(-90.60, 14.38)); //Pacaya_Guatemala
    volcanos.push_back(make_pair(145.80, 18.13)); //Pagan_United_States
    volcanos.push_back(make_pair(150.52, -5.58)); //Pago_Papua_New_Guinea
    volcanos.push_back(make_pair(107.73, -7.32)); //Papandayan_Indonesia
    volcanos.push_back(make_pair(-102.20, 19.50)); //Paricutin_Mexico
    volcanos.push_back(make_pair(-69.15, -18.17)); //Parinacota_Chile/Bolivia
    volcanos.push_back(make_pair(156.97, 51.45)); //Pauzhetka_Russia
    volcanos.push_back(make_pair(-161.89, 55.42)); //Pavlof_United_States
    volcanos.push_back(make_pair(-161.84, 55.45)); //Pavlof_Sister_United_States
    volcanos.push_back(make_pair(-61.17, 14.82)); //Pelee_France
    volcanos.push_back(make_pair(96.33, 4.91)); //Peuet_Sague_Indonesia
    volcanos.push_back(make_pair(-28.40, 38.47)); //Pico_Portugal
    volcanos.push_back(make_pair(-85.62, 11.54)); //Pilas_Las_Nicaragua
    volcanos.push_back(make_pair(-113.50, 31.77)); //Pinacate_Mexico
    volcanos.push_back(make_pair(120.35, 15.13)); //Pinatubo_Phillippines
    volcanos.push_back(make_pair(-116.63, 34.75)); //Pisgah_Lava_Field_United_States
    volcanos.push_back(make_pair(55.71, -21.23)); //Piton_de_la_Fournaise_France
    volcanos.push_back(make_pair(-84.23, 10.20)); //Poas_Costa_Rica
    volcanos.push_back(make_pair(-98.62, 19.02)); //Popocatepetl_Mexico
    volcanos.push_back(make_pair(152.12, 47.02)); //Prevo_Peak_Russia
    volcanos.push_back(make_pair(-76.40, 2.32)); //Purace_Colombia
    volcanos.push_back(make_pair(-62.96, 17.48)); //The_Quill_Netherlands
    volcanos.push_back(make_pair(152.20, -4.27)); //Rabaul_Papua_New_Guinea
    volcanos.push_back(make_pair(124.50, 7.70)); //Ragang_Phillippines
    volcanos.push_back(make_pair(153.25, 48.29)); //Raikoke_Russia
    volcanos.push_back(make_pair(-121.76, 46.85)); //Rainier_United_States
    volcanos.push_back(make_pair(105.63, -5.78)); //Rajabasa_Indonesia
    volcanos.push_back(make_pair(-177.92, -29.27)); //Raoul_New_Zealand
    volcanos.push_back(make_pair(153.02, 47.77)); //Rasshua_Russia
    volcanos.push_back(make_pair(114.04, -8.13)); //Raung_Indonesia
    volcanos.push_back(make_pair(145.13, 44.07)); //Rausu_Japan
    volcanos.push_back(make_pair(-152.74, 60.49)); //Redoubt_United_States
    volcanos.push_back(make_pair(-77.66, -0.08)); //Reventador_Ecuador
    volcanos.push_back(make_pair(-85.32, 10.83)); //Rincon_de_la_Vieja_Costa_Rica
    volcanos.push_back(make_pair(-163.59, 54.80)); //Roundtop_United_States
    volcanos.push_back(make_pair(175.57, -39.28)); //Ruapehu_New_Zealand
    volcanos.push_back(make_pair(149.83, 45.88)); //Rudakov_Russia
    volcanos.push_back(make_pair(-75.32, 4.89)); //Ruiz_Nevado_del_Colombia
    volcanos.push_back(make_pair(178.05, -36.13)); //Rumble_IV_New_Zealand
    volcanos.push_back(make_pair(-63.23, 17.63)); //Saba_Netherlands
    volcanos.push_back(make_pair(-71.85, -15.78)); //Sabancaya_Peru
    volcanos.push_back(make_pair(148.09, -5.41)); //Sakar_Papua_New_Guinea
    volcanos.push_back(make_pair(130.66, 31.59)); //Sakura-jima_Japan
    volcanos.push_back(make_pair(106.73, -6.72)); //Salak_Indonesia
    volcanos.push_back(make_pair(-87.00, 12.70)); //San_Cristobal_Nicaragua
    volcanos.push_back(make_pair(-89.29, 13.73)); //San_Salvador_El_Salvador
    volcanos.push_back(make_pair(-25.00, 16.90)); //San_Vicente_Cape_Verde_Islands
    volcanos.push_back(make_pair(-144.13, 62.22)); //Sanford_United_States
    volcanos.push_back(make_pair(-78.34, -2.00)); //Sangay_Ecuador
    volcanos.push_back(make_pair(-89.63, 13.85)); //Santa_Ana_El_Salvador
    volcanos.push_back(make_pair(-91.55, 14.76)); //Santa_Maria_Guatemala
    volcanos.push_back(make_pair(25.40, 36.40)); //Santorini_Greece
    volcanos.push_back(make_pair(153.20, 48.09)); //Sarychev_Peak_Russia
    volcanos.push_back(make_pair(159.82, -9.13)); //Savo_Solomon_Islands
    volcanos.push_back(make_pair(178.14, 52.02)); //Segula_United_States
    volcanos.push_back(make_pair(112.92, -8.11)); //Semeru_Indonesia
    volcanos.push_back(make_pair(179.58, 51.93)); //Semisopochnoi_United_States
    volcanos.push_back(make_pair(-25.78, 37.87)); //Sete_Cidades_Portugal
    volcanos.push_back(make_pair(-122.19, 41.41)); //Shasta_United_States
    volcanos.push_back(make_pair(161.36, 56.65)); //Sheveluch_Russia
    volcanos.push_back(make_pair(141.38, 42.69)); //Shikotsu_Japan
    volcanos.push_back(make_pair(145.17, 44.13)); //Shiretoko-Iwo-zan_Japan
    volcanos.push_back(make_pair(-163.97, 54.76)); //Shishaldin_United_States
    volcanos.push_back(make_pair(154.18, 48.88)); //Sinarka_Russia
    volcanos.push_back(make_pair(146.14, 44.42)); //Smirnov_Russia
    volcanos.push_back(make_pair(-71.52, -38.97)); //Sollipulli_Chile
    volcanos.push_back(make_pair(124.73, 1.11)); //Soputan_Indonesia
    volcanos.push_back(make_pair(-61.67, 16.05)); //Soufriere_Guadeloupe_France
    volcanos.push_back(make_pair(-62.18, 16.72)); //Soufrire_Hills_United_Kingdom
    volcanos.push_back(make_pair(-61.18, 13.33)); //Soufrire_St_Vincent_St_Vincent
    volcanos.push_back(make_pair(36.60, 2.63)); //South_Island
    volcanos.push_back(make_pair(-111.60, 35.60)); //SP_Mountain_United_States
    volcanos.push_back(make_pair(-152.25, 61.30)); //Spurr_United_States
    volcanos.push_back(make_pair(152.92, 47.60)); //Srednii_Russia
    volcanos.push_back(make_pair(-154.40, 58.40)); //Steller_United_States
    volcanos.push_back(make_pair(15.21, 38.79)); //Stromboli_Italy
    volcanos.push_back(make_pair(-77.63, -0.54)); //Sumaco_Ecuador
    volcanos.push_back(make_pair(110.07, -7.38)); //Sumbing_Indonesia
    volcanos.push_back(make_pair(109.99, -7.30)); //Sundoro_Indonesia
    volcanos.push_back(make_pair(-111.50, 35.37)); //Sunset_Craters_United_States
    volcanos.push_back(make_pair(36.35, -1.18)); //Suswa_Kenya
    volcanos.push_back(make_pair(129.72, 29.64)); //Suwanose-jima_Japan
    volcanos.push_back(make_pair(120.99, 14.00)); //Taal_Philippines
    volcanos.push_back(make_pair(100.68, -0.98)); //Talang_Indonesia
    volcanos.push_back(make_pair(118.00, -8.25)); //Tambora_Indonesia
    volcanos.push_back(make_pair(118.00, -8.25)); //Tambora_Indonesia
    volcanos.push_back(make_pair(-178.15, 51.89)); //Tanaga_and_Takawangha_United_States
    volcanos.push_back(make_pair(-178.15, 51.89)); //Tanaga_and_Takawangha_United_States
    volcanos.push_back(make_pair(107.60, -6.77)); //Tangkubanparahu_Indonesia
    volcanos.push_back(make_pair(154.70, 49.35)); //Tao-Rusyr_Caldera_Russia
    volcanos.push_back(make_pair(17.28, 20.92)); //Tarso_Voon_Chad
    volcanos.push_back(make_pair(-86.85, 12.60)); //Telica_Volcano_Nicaragua
    volcanos.push_back(make_pair(-16.64, 28.27)); //Tenerife_Spain
    volcanos.push_back(make_pair(112.95, -7.94)); //Tengger_Caldera_and_Bromo_Indonesia
    volcanos.push_back(make_pair(-121.80, 44.50)); //Three_Fingered_Jack_United_States
    volcanos.push_back(make_pair(121.77, 44.10)); //Three_Sisters_United_States
    volcanos.push_back(make_pair(-27.37, -59.45)); //Thule_Islands_United_Kingdom
    volcanos.push_back(make_pair(146.26, 44.35)); //Tiatia_Russia
    volcanos.push_back(make_pair(98.83, 2.58)); //Toba_Indonesia
    volcanos.push_back(make_pair(127.47, 1.25)); //Todoko-Ranu_Indonesia
    volcanos.push_back(make_pair(142.69, 43.42)); //Tokachi
    volcanos.push_back(make_pair(160.33, 55.83)); //Tolbachik_Russia
    volcanos.push_back(make_pair(-91.19, 14.61)); //Tolimn_Guatemala
    volcanos.push_back(make_pair(140.92, 40.47)); //Towada_Japan
    volcanos.push_back(make_pair(169.23, -18.75)); //Traitors_Head_Vanuatu
    volcanos.push_back(make_pair(149.92, 45.93)); //Tri_Sestry_Russia
    volcanos.push_back(make_pair(-12.28, -37.09)); //Tristan_da_Cunha_United_Kingdom
    volcanos.push_back(make_pair(131.43, 33.28)); //Tsurumi_Japan
    volcanos.push_back(make_pair(-78.44, -1.47)); //Tungurahua_Ecuador
    volcanos.push_back(make_pair(-117.45, 37.02)); //Ubehebe_Craters_United_States
    volcanos.push_back(make_pair(-70.90, -16.36)); //Ubinas_Peru
    volcanos.push_back(make_pair(160.53, 55.76)); //Udina_Russia
    volcanos.push_back(make_pair(-156.37, 57.75)); //Ugashik_and_Peulik_United_States
    volcanos.push_back(make_pair(-113.13, 36.38)); //Uinkaret_Volcanic_Field_United_States
    volcanos.push_back(make_pair(151.33, -5.05)); //Ulawun_Papua_New_Guinea
    volcanos.push_back(make_pair(147.88, -5.59)); //Umboi_Papua_New_Guinea
    volcanos.push_back(make_pair(144.75, -18.25)); //Undara_Australia
    volcanos.push_back(make_pair(110.33, -7.18)); //Ungaran_and_Telomoyo
    volcanos.push_back(make_pair(130.29, 32.76)); //Unzen_Japan
    volcanos.push_back(make_pair(152.25, 47.12)); //Urataman_Russia
    volcanos.push_back(make_pair(152.80, 47.52)); //Ushishur_Russia
    volcanos.push_back(make_pair(140.84, 42.54)); //Usu_Japan
    volcanos.push_back(make_pair(159.97, 54.50)); //Uzon_Russia
    volcanos.push_back(make_pair(-159.38, 56.17)); //Veniaminof_United_States
    volcanos.push_back(make_pair(155.97, 50.55)); //Vernadskii_Ridge_Russia
    volcanos.push_back(make_pair(14.43, 40.82)); //Vesuvius_Italy
    volcanos.push_back(make_pair(-71.93, -39.42)); //Villarrica_Chile
    volcanos.push_back(make_pair(158.28, 52.70)); //Vilyuchik_Russia
    volcanos.push_back(make_pair(29.49, -1.47)); //Visoke_Congo/Rwanda
    volcanos.push_back(make_pair(-168.69, 53.13)); //Vsevidof_and_Recheschnoi_Volcanoes_United_States
    volcanos.push_back(make_pair(14.96, 38.40)); //Vulcano
    volcanos.push_back(make_pair(-121.80, 44.30)); //Washington_United_States
    volcanos.push_back(make_pair(17.55, 25.05)); //Wau-en-Namus_Libya
    volcanos.push_back(make_pair(-164.65, 54.52)); //Westdahl_United_States
    volcanos.push_back(make_pair(177.18, -37.52)); //White_Island_New_Zealand
    volcanos.push_back(make_pair(-144.02, 62.00)); //Wrangell_United_States
    volcanos.push_back(make_pair(126.12, 48.72)); //Wudalianchi_China
    volcanos.push_back(make_pair(137.59, 36.22)); //Yake-dake_Japan
    volcanos.push_back(make_pair(27.14, 36.67)); //Yali_Greece
    volcanos.push_back(make_pair(-157.19, 57.02)); //Yantarni_United_States
    volcanos.push_back(make_pair(169.44, -19.53)); //Yasur_Vanuatu
    volcanos.push_back(make_pair(-110.67, 44.43)); //Yellowstone_United_States
    volcanos.push_back(make_pair(151.95, 46.93)); //Zavaritzki_Caldera_Russia
    volcanos.push_back(make_pair(157.32, 51.57)); //Zheltovsky
    volcanos.push_back(make_pair(159.15, 53.59)); //Zhupanovsky
    volcanos.push_back(make_pair(160.60, 55.86)); //Zimina_Russia

    int nPts = volcanos.size();
    vtkPoints *pts = vtkPoints::New();
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    mesh->SetPoints(pts);

    vtkIdType vid;
    const double toRad = M_PI/180.0;
    for (int i = 0; i < nPts; i++)
    {
        //first, convert to x,y,z
        double lon = volcanos[i].first * toRad;
        double lat = volcanos[i].second * toRad;
        double x = cos(lat)*cos(lon);
        double y = cos(lat)*sin(lon);
        double z = sin(lat);

        if (xyzMesh)
            vid = pts->InsertNextPoint(x,y,z);
        else
        {
            double nx, ny, nz;
            convertToLatLon(x,y,z, nx,ny,nz);
            vid = pts->InsertNextPoint(nx,ny,nz);
        }
        mesh->InsertNextCell(VTK_VERTEX, 1, &vid);
    }
    pts->Delete();

    return mesh;

}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetHotSpotsMesh
//
// Purpose:
//   Get mesh of hot spots.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetContinents(bool xyzMesh)
{
    vtkEarthSource *es = vtkEarthSource::New();
    es->SetRadius(1.0);
    es->SetOnRatio(0);
    es->SetOutline(1);
    es->Update();

    vtkDataSet *ds = es->GetOutput();

    if (xyzMesh)
        return ds;

    return LatLonClip(ds);
}

#include <platePoints.h>

vtkDataSet *
avtSpecFEMFileFormat::GetPlates(bool xyzMesh)
{
    vtkPolyData *ds = vtkPolyData::New();
    vtkPoints *pts = vtkPoints::New();
    vtkCellArray *lines = vtkCellArray::New();


    //First, make an XYZ plates mesh.
    const double toRad = M_PI/180.;
    int idx = 0, numPts;
    vtkIdType ptID = 0;
    while (1)
    {
        numPts = (int)platePts[idx];
        if (numPts == 0)
            break;

        idx++;
        std::vector<vtkIdType> ids(numPts);
        for (int i = 0; i < numPts; i++, idx+=2)
        {
            double lon_rad = toRad*(platePts[idx]-180);
            double lat_rad = toRad*platePts[idx+1];
            double x = cos(lat_rad) * cos(lon_rad);
            double y = cos(lat_rad) * sin(lon_rad);
            double z = sin(lat_rad);

            pts->InsertNextPoint(x,y,z);
            ids[i] = ptID;
            ptID++;
        }
        lines->InsertNextCell(numPts, &(ids[0]));
    }

    ds->SetPoints(pts);
    ds->SetLines(lines);
    pts->Delete();
    lines->Delete();

    if (xyzMesh)
        return ds;
    return LatLonClip(ds);
}

vtkDataSet *
avtSpecFEMFileFormat::LatLonClip(vtkDataSet *ds)
{
    //For lat/lon, we need to clip it and merge.
    //create the earth from 180 to 360
    vtkImplicitBoolean *func0 = vtkImplicitBoolean::New();
    vtkPlane *pln0 = vtkPlane::New();
    pln0->SetOrigin(0,0,0);
    pln0->SetNormal(0,1,0);
    func0->AddFunction(pln0);
    vtkClipPolyData *clip0 = vtkClipPolyData::New();
    clip0->SetInputData(ds);
    clip0->SetClipFunction(func0);
    clip0->SetInsideOut(true);
    clip0->Update();
    vtkDataSet *ds0 = clip0->GetOutput();

    //convert to lat/lon
    vtkPolyData *pd = static_cast<vtkPolyData *>(ds0);
    vtkIdType nPts = pd->GetPoints()->GetNumberOfPoints();
    double pt[3];
    for (vtkIdType i = 0; i < nPts; i++)
    {
        pd->GetPoints()->GetPoint(i, pt);

        double nx, ny, nz;
        convertToLatLon(pt[0], pt[1], pt[2], nx,ny,nz);
        if (nx < 180.0) nx = 360.0;
        pt[0] = nx;
        pt[1] = ny;
        pt[2] = nz;
        pd->GetPoints()->SetPoint(i, pt);
    }

    //create the earth from 0 to 180
    vtkImplicitBoolean *func1 = vtkImplicitBoolean::New();
    vtkPlane *pln1 = vtkPlane::New();
    pln1->SetOrigin(0,0,0);
    pln1->SetNormal(0,-1,0);
    func1->AddFunction(pln1);
    vtkClipPolyData *clip1 = vtkClipPolyData::New();
    clip1->SetInputData(ds);
    clip1->SetClipFunction(func1);
    clip1->SetInsideOut(true);
    clip1->Update();
    vtkDataSet *ds1 = clip1->GetOutput();

    //convert to lat/lon
    pd = static_cast<vtkPolyData *>(ds1);
    nPts = pd->GetPoints()->GetNumberOfPoints();
    for (vtkIdType i = 0; i < nPts; i++)
    {
        pd->GetPoints()->GetPoint(i, pt);

        double nx, ny, nz;
        convertToLatLon(pt[0], pt[1], pt[2], nx,ny,nz);
        if (nx > 359) nx = 0.0;
        pt[0] = nx;
        pt[1] = ny;
        pt[2] = nz;
        pd->GetPoints()->SetPoint(i, pt);
    }

    vtkAppendPolyData *app = vtkAppendPolyData::New();
    app->AddInputData(static_cast<vtkPolyData *>(ds0));
    app->AddInputData(static_cast<vtkPolyData *>(ds1));
    app->Update();

    vtkDataSet *out_ds = app->GetOutput();
    out_ds->Register(NULL);

    ds->Delete();
    pln0->Delete();
    func0->Delete();
    clip0->Delete();
    pln1->Delete();
    func1->Delete();
    clip1->Delete();
    app->Delete();

    return out_ds;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetWholeMesh
//
// Purpose:
//   Get mesh containing all regions.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetWholeMesh(int ts, int dom, bool xyzMesh)
{
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    mesh->SetPoints(pts);

    int nPts, ptOffset = 0;
    char nP[128];
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        AddRegionMesh(ts, dom, i+1, mesh, xyzMesh, ptOffset);

        sprintf(nP, "reg%d/nglob", i+1);
        auto attr = meshIO.AvailableAttributes()[nP];
        nPts = std::stoi(attr["Value"]);
        ptOffset += nPts;
    }
    pts->Delete();

    return mesh;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::AddRegionMesh
//
// Purpose:
//   Get the mesh for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

void
avtSpecFEMFileFormat::AddRegionMesh(int ts, int dom, int region, vtkDataSet *ds,
                                    bool xyzMesh, int ptOffset)
{
    //cout<<"AddRegionMesh: "<<dom<<endl;

    vtkUnstructuredGrid *mesh = (vtkUnstructuredGrid*)ds;
    vtkPoints *pts = mesh->GetPoints();

    char xNm[128], yNm[128], zNm[128], iNm[128], nE[128], nP[128];
    sprintf(xNm, "reg%d/xstore/array", region);
    sprintf(yNm, "reg%d/ystore/array", region);
    sprintf(zNm, "reg%d/zstore/array", region);
    sprintf(iNm, "reg%d/ibool/array", region);
    sprintf(nE, "reg%d/nspec", region);
    sprintf(nP, "reg%d/nglob", region);

    int nElem, nPts;
    meshReader.Get(meshIO.InquireVariable<int>(nE), &nElem, adios2::Mode::Sync);
    meshReader.Get(meshIO.InquireVariable<int>(nP), &nPts, adios2::Mode::Sync);

    //Process which points are needed....
    //meshFile->ReadScalarData(iNm, ts, dom, &ib);
    //int N = ib->GetNumberOfTuples();
    adios2::Variable<int> ibV = meshIO.InquireVariable<int>(iNm);
    auto ibBlkInfo = (meshReader.BlocksInfo(ibV, 0))[dom];
    //cout<<"ib blocks: "<<meshReader.BlocksInfo(ibV, 0).size()<<endl;
    int N = ibBlkInfo.Count[0];
    vtkIntArray *ib = vtkIntArray::New();
    ib->SetNumberOfTuples(N);

    ibV.SetBlockSelection(0);
    meshReader.Get(ibV, (int*)ib->GetVoidPointer(0), adios2::Mode::Sync);

    int *ibl = (int*)ib->GetVoidPointer(0);
    vector<bool> ptMask(N, false);
    vector<int> ptID(N);
    int ptCnt = 0;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            ptID[idx] = ptCnt;
            ptCnt++;
            ptMask[idx] = true;
        }
    }
    for (int i = 0; i < N; i++)
        ptMask[i] = false;

    vtkDoubleArray *x = vtkDoubleArray::New();
    vtkDoubleArray *y = vtkDoubleArray::New();
    vtkDoubleArray *z = vtkDoubleArray::New();
    x->SetNumberOfTuples(N);
    y->SetNumberOfTuples(N);
    z->SetNumberOfTuples(N);
    adios2::Variable<double> xV = meshIO.InquireVariable<double>(xNm);
    adios2::Variable<double> yV = meshIO.InquireVariable<double>(yNm);
    adios2::Variable<double> zV = meshIO.InquireVariable<double>(zNm);

    xV.SetBlockSelection(dom);
    yV.SetBlockSelection(dom);
    zV.SetBlockSelection(dom);
    meshReader.Get(xV, (double*)x->GetVoidPointer(0), adios2::Mode::Sync);
    meshReader.Get(yV, (double*)y->GetVoidPointer(0), adios2::Mode::Sync);
    meshReader.Get(zV, (double*)z->GetVoidPointer(0), adios2::Mode::Sync);
    vtkIdType vid;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            double ptX = x->GetTuple1(i);
            double ptY = y->GetTuple1(i);
            double ptZ = z->GetTuple1(i);

            if (xyzMesh)
                vid = pts->InsertNextPoint(ptX, ptY, ptZ);
            else
            {
                double newX, newY, newZ;
                convertToLatLon(ptX, ptY, ptZ, newX, newY, newZ);
                vid = pts->InsertNextPoint(newX, newY, newZ);
            }
            ptMask[idx] = true;
#ifdef POINT_MESH
            mesh->InsertNextCell(VTK_VERTEX, 1, &vid);
#endif
        }
    }

#ifdef USE_IBOOL
    int di=1, dj=1, dk=1, eCnt = 0;
    vtkIdType v[8];

    #define INDEX(x,i,j,k,e) x[(i)+(j)*ngllx + (k)*ngllx*nglly + (e)*ngllx*nglly*ngllz] - 1;
    for (int e = 0; e < nElem; e++)
        for (int k = 0; k < ngllz-1; k+=dk)
            for (int j = 0; j < nglly-1; j+=dj)
                for (int i = 0; i < ngllx-1; i+=di)
                {
                    v[0] = INDEX(ibl,i,j,k,e);
                    v[1] = INDEX(ibl,i+di,j,k,e);
                    v[2] = INDEX(ibl,i+di,j+dj,k,e);
                    v[3] = INDEX(ibl,i,j+dj,k,e);

                    v[4] = INDEX(ibl,i,j,k+dk,e);
                    v[5] = INDEX(ibl,i+di,j,k+dk,e);
                    v[6] = INDEX(ibl,i+di,j+dj,k+dk,e);
                    v[7] = INDEX(ibl,i,j+dj,k+dk,e);
                    for (int vi = 0; vi < 8; vi++)
                        v[vi] += ptOffset;

                    if (xyzMesh)
                    {
                        mesh->InsertNextCell(VTK_HEXAHEDRON, 8, v);
                        /*
                        if (eCnt == 43011 || eCnt == 43010)
                        {
                            double verts[8][3];
                            for (int p = 0; p < 8; p++)
                                pts->GetPoint(v[p], verts[p]);
                            cout<<eCnt<<": "<<endl;
                            for (int p = 0; p < 8; p++)
                            {
                                double px = verts[p][0];
                                double py = verts[p][1];
                                double pz = verts[p][2];
                                double RR = px*px+py*py;
                                double R = sqrt(RR + pz*pz);
                                double nx = R;
                                double ny = (R==0.0 ? 0.0 : acos(pz/R));
                                double nz = (RR==0.0 ? 0.0 : M_PI + atan2(-py, -px));

                                cout<<p<<" ["<<verts[p][0]<<" "<<verts[p][1]<<" "<<verts[p][2]<<"] ["<<nx<<" "<<ny<<" "<<nz<<"] 2pi= "<<2.0*M_PI<<endl;
                            }
                        }
                        */
                        eCnt++;
                    }
                    else
                    {
                        //Check for wrap-around cells, and toss them.
                        double verts[8][3];
                        for (int p = 0; p < 8; p++)
                            pts->GetPoint(v[p], verts[p]);

                        bool cellGood = true;
                        for (int p = 0; cellGood && p < 8; p++)
                            for (int q = 0; cellGood && q < 8; q++)
                                if (p != q)
                                {
                                    double dx = fabs(verts[p][0]-verts[q][0]);
                                    double dy = fabs(verts[p][1]-verts[q][1]);
                                    if (dx > M_PI) cellGood = false;
                                    if (dy > M_PI) cellGood = false;
                                }
                        if (cellGood)
                        {
                            mesh->InsertNextCell(VTK_HEXAHEDRON, 8, v);
                            eCnt++;
                        }
                    }
                }
#endif

    x->Delete();
    y->Delete();
    z->Delete();
    ib->Delete();
}


//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetRegionMesh
//
// Purpose:
//   Get the mesh for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetRegionMesh(int ts, int dom, int region, bool xyzMesh)
{
    vtkUnstructuredGrid *mesh = vtkUnstructuredGrid::New();
    vtkPoints *pts = vtkPoints::New();
    mesh->SetPoints(pts);

    AddRegionMesh(ts, dom, region, mesh, xyzMesh);

    pts->Delete();
    return mesh;
}


// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetMesh
//
//  Purpose:
//      Gets the mesh associated with this file.  The mesh is returned as a
//      derived type of vtkDataSet (ie vtkRectilinearGrid, vtkStructuredGrid,
//      vtkUnstructuredGrid, etc).
//
//  Arguments:
//      timestate   The index of the timestate.  If GetNTimesteps returned
//                  'N' time steps, this is guaranteed to be between 0 and N-1.
//      meshname    The name of the mesh of interest.  This can be ignored if
//                  there is only one mesh.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications
//   Dave Pugmire, Thu Jan 27 11:39:46 EST 2011
//   Support for new Pixle file format.
//
// ****************************************************************************

vtkDataSet *
avtSpecFEMFileFormat::GetMesh(int ts, int domain, const char *meshname)
{
    debug1 << "avtSpecFEMFileFormat::GetMesh " << meshname << endl;
    Initialize();
    if (!strcmp(meshname, "hotSpots"))
        return GetHotSpotsMesh(true);
    else if (!strcmp(meshname, "LatLon_hotSpots"))
        return GetHotSpotsMesh(false);
    else if (!strcmp(meshname, "volcanos"))
        return GetVolcanoMesh(true);
    else if (!strcmp(meshname, "LatLon_volcanos"))
        return GetVolcanoMesh(false);
    else if (!strcmp(meshname, "continents"))
        return GetContinents(true);
    else if (!strcmp(meshname, "LatLon_continents"))
        return GetContinents(false);
    else if (!strcmp(meshname, "plates"))
        return GetPlates(true);
    else if (!strcmp(meshname, "LatLon_plates"))
        return GetPlates(false);

    bool xyzMesh = string(meshname).find("LatLon") == string::npos;
    bool wholeMesh = string(meshname).find("reg") == string::npos;
    if (wholeMesh)
        return GetWholeMesh(ts, domain, xyzMesh);
    else
        return GetRegionMesh(ts, domain, GetRegion(meshname), xyzMesh);
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVarRegion
//
// Purpose:
//   Get the variable for a particular region.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVarRegion(std::string &nm, int ts, int dom)
{
    char iNm[128];
    //Read the ibool array.
    int region = GetRegion(nm);
    sprintf(iNm, "reg%d/ibool/array", region);

    adios2::Variable<int> ibV = meshIO.InquireVariable<int>(iNm);
    auto ibBlkInfo = (meshReader.BlocksInfo(ibV, 0))[dom];
    int N = ibBlkInfo.Count[0];
    vtkIntArray *ib = vtkIntArray::New();
    ib->SetNumberOfTuples(N);
    ibV.SetBlockSelection(dom);
    meshReader.Get(ibV, (int*)ib->GetVoidPointer(0), adios2::Mode::Sync);

    vector<bool> ptMask(N, false);
    vector<int> ptID(N);
    int *ibl = (int*)ib->GetVoidPointer(0);

    int ptCnt = 0;
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            ptID[idx] = ptCnt;
            ptCnt++;
            ptMask[idx] = true;
        }
    }
    for (int i = 0; i < N; i++)
        ptMask[i] = false;

    string vname = nm+"/array";

    adios2::Variable<float> arrV = dataIO.InquireVariable<float>(vname);
    auto arrBlkInfo = (dataReader.BlocksInfo(arrV, 0))[dom];
    N = arrBlkInfo.Count[0];
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(N);
    arrV.SetBlockSelection(dom);
    dataReader.Get(arrV, (float*)arr->GetVoidPointer(0), adios2::Mode::Sync);

    vtkFloatArray *var = vtkFloatArray::New();
    var->SetNumberOfTuples(ptCnt);
    for (int i = 0; i < N; i++)
    {
        int idx = ibl[i];
        if (!ptMask[idx])
        {
            var->SetTuple1(ptID[idx], arr->GetTuple1(i));
            ptMask[idx] = true;
        }
    }
    ib->Delete();
    arr->Delete();

    return var;
}

// ****************************************************************************
//  Method: avtSpecFEMFileFormat::GetVar
//
//  Purpose:
//      Gets a scalar variable associated with this file.  Although VTK has
//      support for many different types, the best bet is vtkFloatArray, since
//      that is supported everywhere through VisIt.
//
//  Arguments:
//      timestate  The index of the timestate.  If GetNTimesteps returned
//                 'N' time steps, this is guaranteed to be between 0 and N-1.
//      varname    The name of the variable requested.
//
//  Programmer: Dave Pugmire
//  Creation:   Wed Mar 17 15:29:24 EDT 2010
//
//  Modifications:
//
//  Dave Pugmire, Wed Mar 24 16:43:32 EDT 2010
//  Handle time varying variables correctly.
//
// ****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVar(int ts, int domain, const char *varname)
{
    string vName = varname;
    size_t i;
    if ((i = vName.find("LatLon")) != string::npos)
        vName = vName.substr(0, i) + vName.substr((i+7), string::npos);

    if (vName.find("reg") != string::npos || kernelFile)
        return GetVarRegion(vName, ts, domain);

    //Determine how many total values.
    int n = 0, nPts;
    char tmp[128];
    vector<int> regNPts;
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        sprintf(tmp, "reg%d/nglob", i+1);
        int nPts = std::stoi(meshIO.AvailableVariables()[tmp]["Value"]);
        regNPts.push_back(nPts);
        n += nPts;
    }
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfTuples(n);

    //Append in each region to the whole.
    int offset = 0;
    for (int i = 0; i < avtSpecFEMFileFormat::NUM_REGIONS; i++)
    {
        sprintf(tmp, "reg%d/%s", i+1, varname);
        string vNm = tmp;
        vtkDataArray *v = GetVarRegion(vNm, ts, domain);

        float *ptr = &((float *)arr->GetVoidPointer(0))[offset];
        for (int j = 0; j < v->GetNumberOfTuples(); j++)
            ptr[j] = v->GetTuple1(j);

        offset += regNPts[i];
        v->Delete();
    }

    return arr;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVectorVar
//
// Purpose:
//   Retreive a vector variable.
//
// Arguments:
//
//
// Programmer:  Dave Pugmire
// Creation:    December 19, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVectorVar(int ts, int domain, const char *varname)
{
    Initialize();

    string vName = varname;
    if (vName.find("reg") != string::npos)
        return GetVectorVarRegion(vName, ts, domain);

    EXCEPTION0(ImproperUseException);
    return NULL;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVectorVarRegion
//
// Purpose:
//   Retreive a vector variable.
//
// Arguments:
//
//
// Programmer:  Dave Pugmire
// Creation:    December 19, 2014
//
// Modifications:
//
//****************************************************************************

vtkDataArray *
avtSpecFEMFileFormat::GetVectorVarRegion(std::string &nm, int ts, int dom)
{
    if (nm.find("LatLonR_coords") == string::npos)
        return NULL;

    int region = GetRegion(nm);
    vtkDataSet *mesh = GetRegionMesh(ts, dom, region, true);

    int nPts = mesh->GetNumberOfPoints();
    vtkFloatArray *arr = vtkFloatArray::New();
    arr->SetNumberOfComponents(3);
    arr->SetNumberOfTuples(nPts);

    double lat = 0.0, lon = 0.0, r = 0.0;
    double pt[3];
    for (int i = 0; i < nPts; i++)
    {
        mesh->GetPoint(i, pt);
        convertToLatLon(pt[0], pt[1], pt[2], lon, lat, r);

        arr->SetComponent(i, 0, lon);
        arr->SetComponent(i, 1, lat);
        arr->SetComponent(i, 2, r);
    }

    mesh->Delete();
    return arr;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::Initialize
//
// Purpose:
//   Create world peace.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

void
avtSpecFEMFileFormat::Initialize()
{
    if (initialized)
        return;

    //See which regions we have.
    regions.resize(NUM_REGIONS, false);

    //See if it's a kernel file.
    auto variables = dataIO.AvailableVariables();
    //cout<<"dataVars: "<<variables<<endl;
    if (variables.find("betav_kl_crust_mantle") != variables.end())
        kernelFile = true;

    //Find number of regions and numBlocks.
    numBlocks = -1;
    for (auto &var : variables)
    {
        //We want array vars in a region.
        if (var.first.find("reg") == string::npos || var.first.find("array") == string::npos)
            continue;

        //cout<<"VAR= "<<var<<endl;
        regions[GetRegion(var.first)-1] = true;
        if (numBlocks == -1)
        {
            if (var.second["Type"] == "float")
            {
                auto vinq = dataIO.InquireVariable<float>(var.first);
                auto blockInfo = dataReader.BlocksInfo(vinq, 0);
                numBlocks = blockInfo.size();
                /*
                for (int i = 0; i < numBlocks; i++)
                {
                    auto blk = blockInfo[i];
                    cout<<i<<": "<<blk.BlockID<<" "<<blk.Start<<" "<<blk.Count<<endl;
                }
                */
            }
            else if (var.second["Type"] == "double")
            {
                auto vinq = dataIO.InquireVariable<double>(var.first);
                auto blockInfo = dataReader.BlocksInfo(vinq, 0);
                numBlocks = blockInfo.size();
            }
            if (numBlocks > 0)
                break;
        }
    }
    initialized = true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GenerateFileNames
//
// Purpose:
//   Generate the mesh data file names.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************


bool
avtSpecFEMFileFormat::GenerateFileNames(const std::string &nm,
                                        std::string &meshNm, std::string &dataNm)
{
    //They have opened the mesh file... Can't continue.
    if (nm.find("solver_data.bp") != string::npos)
        return false;

    dataNm = nm;

    //Create the meshNm.
    string::size_type idx = dataNm.rfind("/");
    meshNm = dataNm.substr(0,idx+1) + "solver_data.bp";

    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::IsMeshFile
//
// Purpose:
//   See if this file contains the mesh
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
avtSpecFEMFileFormat::IsMeshFile(const string &fname)
{
#if 0
    {
        #ifdef PARALLEL
        adios2::ADIOS adios((MPI_Comm)VISIT_MPI_COMM, adios2::DebugON);
        adios2::IO io = adios2::IO(adios.DeclareIO("ReadBP"));
        io.SetEngine("BP");
        adios2::Engine reader = io.Open(fname, adios2::Mode::Read);
        auto attributes = io.AvailableAttributes();
        auto variables = io.AvailableVariables();
        #endif
        return true;
    }
#endif

    shared_ptr<adios2::ADIOS> adios = std::make_shared<adios2::ADIOS>(adios2::DebugON);
    adios2::IO io = adios2::IO(adios->DeclareIO("ReadBP"));
    io.SetEngine("BP");
    adios2::Engine reader = io.Open(fname, adios2::Mode::Read);
    auto attributes = io.AvailableAttributes();
    auto variables = io.AvailableVariables();

    /*
    cout<<"MESH: "<<endl;
    cout<<"  ATTR:"<<endl;
    for (auto &a : attributes)
    {
        cout<<"       "<<a.first<<" "<<a.second<<endl;
    }
    cout<<"  VARS:"<<endl;
    for (auto &v : variables)
        cout<<"       "<<v.first<<" "<<v.second<<endl;
    */

    if (variables.find("reg1/nspec") == variables.end() ||
        variables.find("reg1/nglob") == variables.end() ||
        variables.find("reg1/x_global/local_dim") == variables.end() ||
        variables.find("reg1/x_global/global_dim") == variables.end() ||
        variables.find("reg1/x_global/offset") == variables.end() ||
        variables.find("reg1/y_global/local_dim") == variables.end() ||
        variables.find("reg1/y_global/global_dim") == variables.end() ||
        variables.find("reg1/y_global/offset") == variables.end() ||
        variables.find("reg1/z_global/local_dim") == variables.end() ||
        variables.find("reg1/z_global/global_dim") == variables.end() ||
        variables.find("reg1/z_global/offset") == variables.end() ||
        variables.find("reg1/xstore/local_dim") == variables.end() ||
        variables.find("reg1/xstore/global_dim") == variables.end() ||
        variables.find("reg1/ystore/local_dim") == variables.end() ||
        variables.find("reg1/ystore/global_dim") == variables.end() ||
        variables.find("reg1/zstore/local_dim") == variables.end() ||
        variables.find("reg1/zstore/global_dim") == variables.end())
    {
        return false;
    }

    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::IsDataFile
//
// Purpose:
//   See if this file contains data
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

bool
avtSpecFEMFileFormat::IsDataFile(const string &fname)
{
    return true;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetRegion
//
// Purpose:
//   Extract the region number from a variable name.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

int
avtSpecFEMFileFormat::GetRegion(const string &str)
{
    if (kernelFile)
        return 1;

    int region, n;
    char t1[128];

    n = sscanf(str.c_str(), "reg%d/%s", &region, t1);
    if (n != 2 || region < 1 || region > avtSpecFEMFileFormat::NUM_REGIONS)
        EXCEPTION1(ImproperUseException, "Invalid region");

    return region;
}

//****************************************************************************
// Method:  avtSpecFEMFileFormat::GetVariable
//
// Purpose:
//   Extract the variable name from a string with region information in it.
//
// Programmer:  Dave Pugmire
// Creation:    April  9, 2014
//
// Modifications:
//
//****************************************************************************

string
avtSpecFEMFileFormat::GetVariable(const string &str)
{
    string::size_type i0 = (kernelFile?0:i0 = str.find("/"));
    string::size_type i1 = str.rfind("/");

    if (i0 == string::npos || i1 == string::npos)
        EXCEPTION1(ImproperUseException, "Invalid variable");

    if (kernelFile)
        return str.substr(0, i1);
    else
        return str.substr(i0+1, i1-i0-1);
}

//****************************************************************************
// Function:  convertToLatLon
//
// Purpose:
//   Convert XYZ to Lat Lon Rad.
//
// Programmer:  Dave Pugmire
// Creation:    November 12, 2014
//
// Modifications:
//
//   Dave Pugmire, Wed Dec  3 16:45:27 EST 2014
//   Fix conversion....
//
//****************************************************************************

static inline void
convertToLatLon(double x, double y, double z, double &nx, double &ny, double &nz)
{
    const double twopi = 2.0*M_PI;
    const double toDeg = 180./M_PI;
    const double earthRad = 6371.0; //6371.0; // km

    double R = sqrt(x*x + y*y + z*z);
    double lon = atan2(y,x);
    double lat = acos(z/R);

    if (lon < 0.0)
        lon += twopi;

    nx = lon * toDeg;
    ny = lat * toDeg;
    //nz = (R-1.0) * earthRad;
    nz = R * earthRad;

    ny = -ny + 90.0;

    //normalize it....
    //nx /= 360.;
    //ny /= 180.;
    //nz = (R - 1.0) * 6.371;

    /*

    nx = R;
    ny = acos(z/R);
    nz = atan2(y, x);
    if (nz < 0.0)
        nz += twopi;

    //nx *= 6371.0; //Convert to km
    ny *= toDeg;
    nz *= toDeg;

    ny = -ny + 90.0;
    //ny -= 90.0;
    nz -= 180.0;
    */

/*
    if (nz > 340.0)
        nz = -(360.0-nz);
*/
}
