#include <AMRreaderInterface.h>

const char* AMRreaderInterface::amr_grpname="AMR";
const char* AMRreaderInterface::amr_dimname="BlockDim";
const char* AMRreaderInterface::amr_time_name="SimuTime";
const char* AMRreaderInterface::amr_iter_name="Ncycles";
const char* AMRreaderInterface::amr_keyname="BlockKey";
const char* AMRreaderInterface::amr_crdname="BlockCoord";
const char* AMRreaderInterface::amr_stpname="BlockSteps";
const char* AMRreaderInterface::amr_datname="BlockData";
const char* AMRreaderInterface::amr_prename="BlockPressure";
const char* AMRreaderInterface::amr_sndname="BlockSoundSpeed";
const char* AMRreaderInterface::amr_tmpname="BlockTemperature";
const char* AMRreaderInterface::amr_tagname="BlockTag";
const char* AMRreaderInterface::amr_idealname="IdealGasEOS";
const char* AMRreaderInterface::amr_jwlname="JwlEOS";
const char* AMRreaderInterface::amr_sesamename="SesameEOStable";


const char* AMRreaderInterface::intf_grp_name="Interface";
const char* AMRreaderInterface::intf_np_name="nnode";
const char* AMRreaderInterface::intf_ne_name="nsegt";
const char* AMRreaderInterface::intf_coor_name="Coordinate";
const char* AMRreaderInterface::intf_velo_name="Velocity";
const char* AMRreaderInterface::intf_pres_name="Pressure";
const char* AMRreaderInterface::intf_segt_name="Segment";
const char* AMRreaderInterface::intf_matl_name="Material";


const int AMRreaderInterface::kid2i[8] = {0,1,0,1,0,1,0,1};
const int AMRreaderInterface::kid2j[8] = {0,0,1,1,0,0,1,1};
const int AMRreaderInterface::kid2k[8] = {0,0,0,0,1,1,1,1};

AMRreaderInterface::AMRreaderInterface()
{
}

AMRreaderInterface::~AMRreaderInterface()
{
}

// Move to another file later...
#include <AMRreader.h>
#include <AMRreaderLowRes.h>
#include <AMRreaderBlkConso.h>
#include <AMRreaderLowResBlkConso.h>

#include <DebugStream.h>
#include <cstdlib>

AMRreaderInterface *
InstantiateAMRreader()
{
    int typ=0;
    char* amrlvl=getenv("AMRLEVEL");
    if( amrlvl )
        typ = atoi(amrlvl);

    AMRreaderInterface *reader = NULL;
    switch(typ)
    {
    case -1:
        reader = new AMRreaderLowResBlkConso;
        debug1 << "Using AMRreaderLowResBlkConso...\n";
        break;
    case -2:
        reader = new AMRreaderLowRes2BlkConso;
        debug1 << "Using AMRreaderLowRes2BlkConso...\n";
        break;
    case 100:
        reader = new AMRreader;
        debug1 << "Using AMRreader...\n";
        break;
    case 101:
        reader = new AMRreaderLowRes;
        debug1 << "Using AMRreaderLowRes...\n";
        break;
    case 102:
        reader = new AMRreaderLowRes2;
        debug1 << "Using AMRreaderLowRes2...\n";
        break;
    default:
        reader = new AMRreaderBlkConso;
        debug1 << "Using AMRreaderBlkConso...\n";
    }

    return reader;
}
