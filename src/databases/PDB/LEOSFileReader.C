#include <InvalidVariableException.h>
#include <UnexpectedValueException.h>

#include <vtkDoubleArray.h>
#include <vtkFloatArray.h>
#include <vtkPolyData.h>
#include <vtkPointData.h>
#include <vtkRectilinearGrid.h>
#include <vtkStructuredGrid.h>

#include <avtDatabaseMetaData.h>

#include <Utility.h>
#include <DebugStream.h>

// This header file is last because it includes "scstd.h" (indirectly
// through "pdb.h"), which defines min and max, which conflict with
// "limits.h" on tru64 and aix systems.  On tru64 systems the conflict
// occurs with gcc-3.0.4 and on aix systems the conflict occurs with
// gcc-3.1.
#include <LEOSFileReader.h>

using std::string;
using std::map;

// valid chracters in a variable name
static const int validChars[128] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  //  0 - 15
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,  // 16 - 31
    1, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 0, 1, 0,  // 32 - 47
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1,  // 48 - 63
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 64 - 79
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 80 - 95
    0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1,  // 96 - 111
    1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0}; //112 - 127 


// ****************************************************************************
// Function: RemoveSpaces
//
// Purpose: Compress spaces from a character string 
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

static string 
RemoveSpaces(const char *str)
{
    string retval;

    int i = 0;
    while (str[i] != 0)
    {
        if (str[i] != ' ')
            retval += str[i];
        i++;
    }

    return retval;
}

// ****************************************************************************
// Function: FixMatName 
//
// Purpose: Fixes any problems with characters in matName 
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************
static bool 
FixMatName(char *matName, int maxLen)
{
    // make sure matName contains valid characters
    // note that VisIt interprets special meaning for '(' and ')' in a
    // variable name, so if we see them, change them to '[' and ']'
    int i = 0;
    int lastNonSpace = 0;
    while ((matName[i] != 0) && (i < maxLen))
    {
        if (matName[i] < 0)
            return false;
        if (matName[i] == '(')
            matName[i] = '[';
        if (matName[i] == ')')
            matName[i] = ']';
        if (validChars[matName[i]] == 0)
            matName[i] = '?';
        if (matName[i] != ' ')
            lastNonSpace = i;
        i++;
    }

    if (i == maxLen)
        return false;

    // strip trailing space chars by pinching at last non space char
    if (lastNonSpace < i - 1)
        matName[lastNonSpace+1] = 0;

    return true;
}

// ****************************************************************************
// Method: LEOSFileReader::LEOSFileReader
//
// Purpose: 
//   Constructor for the LEOSFileReader class.
//
// Arguments:
//   pdb : The pdb file to use.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

LEOSFileReader::LEOSFileReader(const char *filename) : PDBReader(filename)
{
    tryHardLevel = 0;
    topDirs = 0;
    numTopDirs = 0;
    BuildVarInfoMap();
}

LEOSFileReader::LEOSFileReader(PDBFileObject *pdb) : PDBReader(pdb)
{
    tryHardLevel = 0;
    topDirs = 0;
    numTopDirs = 0;
    BuildVarInfoMap();
}

// ****************************************************************************
// Method: LEOSFileReader::~LEOSFileReader
//
// Purpose: 
//   Destructor for the LEOSFileReader class.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

LEOSFileReader::~LEOSFileReader()
{
    if (topDirs != 0)
        SFREE(topDirs);
}

// ****************************************************************************
// Method: ThrowInvalidVariableException 
//
// Purpose: Generate a useful error message and handle re-tries for certain
// errors involving expediency assumptions.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

void
LEOSFileReader::ThrowInvalidVariableException(bool ignorable, const char *varClass,
    const char *assumedVal, const char *actualVal, const char *fullVarName)
{
    char errMsg[2048];

    char *errMsgIgnore = "For expediency, the LEOS reader plugin makes certain "
        "assumptions about eos variables. When these assumptions turn out "
        "to be incorrect, this error condition is generated.\n\n"
        "In this case, the value for \"%s\" was assumed to be \"%s\" but in reality "
        "is \"%s\". You can ignore this error by hitting the draw button a second "
        "time. However, the values displayed for \"%s\" will be wrong.\n\nAlternatively, "
        "you can exit VisIt and set an environment variable, VISIT_LEOS_TRY_HARDER, "
        "to a value of 2 and restart VisIt. It will take VisIt longer to load the "
        "LEOS database but this error will not appear.";

    char *errMsgRestart = "For expediency, the LEOS reader plugin makes certain "
        "assumptions about eos variables. When these assumptions turn out "
        "to be incorrect, this error condition is generated.\n\n"
        "To work-around this problem, you can exit VisIt and set an "
        "environment variable, VISIT_LEOS_TRY_HARDER, "
        "to a value of 2 and restart VisIt. It will take VisIt longer to load the "
        "LEOS database but this error will not appear.";

    // check if we've already issued an exception for this variable
    bool haveSeenBefore = true;
    if (invalidVarMap.find(fullVarName) == invalidVarMap.end())
    {
        haveSeenBefore = false;
        invalidVarMap[fullVarName] = true;
    }

    // ignore it if we've seen it before
    if (haveSeenBefore && ignorable)
        return;

    if (ignorable)
    {
        sprintf(errMsg, errMsgIgnore, varClass, assumedVal, actualVal, varClass);
        EXCEPTION1(VisItException, errMsg);
    }
    else
    {
        EXCEPTION1(VisItException, errMsgRestart);
    }
}

// ****************************************************************************
// Method: ValidateVariableInfoAssumptions 
//
// Purpose: Generate a useful error message and handle re-tries for certain
// errors involving expediency assumptions.
//
// Note: currently we need not test for errors in vName, xSize or ySize
// because they are self-correcting.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

void
LEOSFileReader::ValidateVariableInfoAssumptions(eosVarInfo_t &mapInfo,
    eosVarInfo_t &fileInfo, const char *fullVarName)
{
    // we cannot ignore invalid assumptions in ndims. Always error in this case 
    if (mapInfo.ndims != fileInfo.ndims)
    {
       char s1[10],s2[10];
       sprintf(s1,"%d", mapInfo.ndims);
       sprintf(s2,"%d", fileInfo.ndims);
       ThrowInvalidVariableException(false, "table dimensions", s1, s2,
           fullVarName);
    }

    if (mapInfo.vUnits != fileInfo.vUnits)
       ThrowInvalidVariableException(true, "table units",
            mapInfo.vUnits.c_str(), fileInfo.vUnits.c_str(), fullVarName);

    if (mapInfo.xName != fileInfo.xName)
        ThrowInvalidVariableException(true, "x-axis label",
            mapInfo.xName.c_str(), fileInfo.xName.c_str(), fullVarName);

    if (mapInfo.xUnits != fileInfo.xUnits)
        ThrowInvalidVariableException(true, "x-axis units",
            mapInfo.xUnits.c_str(), fileInfo.xUnits.c_str(), fullVarName);

    if (mapInfo.yName != fileInfo.yName)
        ThrowInvalidVariableException(true, "y-axis label",
            mapInfo.yName.c_str(), fileInfo.yName.c_str(), fullVarName);

    if (mapInfo.yUnits != fileInfo.yUnits)
        ThrowInvalidVariableException(true, "y-axis units",
            mapInfo.yUnits.c_str(), fileInfo.yUnits.c_str(), fullVarName);
}

// ****************************************************************************
// Method: BuildVarInfoMap
//
// Purpose: 
//   Builds a mapping of an equation-of-state (eos) variable name to meta data
//   about that variable. This is essentially a set of assumptions about LEOS
//   eos variables that enable us to quickly open an LEOS file. Without this
//   information, we can't take the shortcuts to PopulateDatabaseMetaData 
//   quickly. Nonetheless, when we later go to read a variable, if we find that
//   the assumptions made here are wrong, we'll detect that and throw an 
//   exception.
//
//   If we later encounter eos variables that are not in this list, upon the
//   first encounter of the new variable, we'll assume what we learn about it
//   is true for this new variable ON ALL MATERIALS it is defined and append
//   to this list.
//
//   The reason for the funny look to this code and SET_MAP_ENTRY is that it
//   evolved from using initializer structs and macros.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

void SET_MAP_ENTRY(LEOSFileReader::eosVarInfo_t &info, int ndims,
    string vName, string vUnits,
    string xName, string xUnits, string xSize,
    string yName, string yUnits, string ySize)
{
    info.ndims = ndims;
    info.vName = vName;
    info.vUnits = vUnits;
    info.xName = xName;
    info.xUnits = xUnits;
    info.xSize = xSize;
    info.yName = yName;
    info.yUnits = yUnits;
    info.ySize = ySize;
}

void
LEOSFileReader::BuildVarInfoMap()
{
    SET_MAP_ENTRY(eosVarInfoMap["Cs"  ] , 2, "table", "cm/s",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["E2p" ] , 2, "table", "erg/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Ec"  ] , 1, "table", "erg/g",
        "rho", "g/cc", "numrho", ""    , "" , "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Ecp" ] , 2, "table", "erg/electron",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Ee"  ] , 2, "table", "erg/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Ei"  ] , 2, "table", "erg/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Et"  ] , 2, "table", "erg/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Kp"  ] , 2, "table", "cm2/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Kr"  ] , 2, "table", "cm2/g",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["P2p" ] , 2, "table", "erg/cc",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Pc"  ] , 1, "table", "erg/cc",
        "rho", "g/cc", "numrho", ""    , "" , "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Pe"  ] , 2, "table", "erg/cc",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Pi"  ] , 2, "table", "erg/cc",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Pt"  ] , 2, "table", "erg/cc",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["S2p" ] , 2, "table", "erg/g-K",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["St"  ] , 2, "table", "erg/g-K",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Tm"  ] , 1, "table", "K",
        "rho", "g/cc", "numrho", ""    , "" , "numtemp");

    SET_MAP_ENTRY(eosVarInfoMap["Zeff"] , 2, "table", "none",
        "rho", "g/cc", "numrho", "temp", "K", "numtemp");
}

// ****************************************************************************
// Method : AddVariableAndMesh
//
// Purpose: 
//   Adds a variable and its mesh to the metadata 
//
//   If the material formula is not known, here AND if tryHardeLevel is 1 or
//   more, it will attempt to ob
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

bool 
LEOSFileReader::AddVariableAndMesh(avtDatabaseMetaData *md, const char *matDirName, 
    const char *matName, const char *matForm, const char *varDirName)
{

    map<string, eosVarInfo_t>::const_iterator i;
    eosVarInfo_t varInfo;

    // if the material formula isn't now known, read it
    string materialFormula;
    if ((matForm == 0) && (tryHardLevel > 0))
    {
        // collect the material formula (e.g. 'H2O' for water)
        char tmpStr[256];
        char *resultStr = 0;
        sprintf(tmpStr,"/%smaterial_info/formula", matDirName);
        if (pdb->GetString(tmpStr, &resultStr))
        {
           materialFormula = RemoveSpaces(resultStr);
           matForm = materialFormula.c_str();
           delete [] resultStr;
        }
    }

    // compute mesh and variable names to be served up to VisIt
    char *varName = CXX_strdup(varDirName);
    varName[strlen(varName)-1] = 0;
    string meshBaseName;
    if (matForm == 0)
        meshBaseName = string(matName);
    else
        meshBaseName = string(matName) + " {" + string(matForm) + "}";
    string mdVarName  = meshBaseName + '/' + string(varName);
    string mdMeshName = mdVarName + "_mesh";

    // if we're really trying hard, just read the variable info from file
    // and put it into the eosVarInfoMap 
    if (tryHardLevel > 1)
    {
        ReadVariableInfo(matDirName, varDirName, varInfo); 
        eosVarInfoMap[mdVarName] = varInfo;
        i = eosVarInfoMap.find(mdVarName);
    }
    else // otherwise, look up the variable info in our info map
    {
        i = eosVarInfoMap.find(varName);

        // if we didn't find the variable in the map, and we should
        // try harder, add info about it to the eosVarInfoMap
        if ((i == eosVarInfoMap.end()) && (tryHardLevel > 0))
        {
            ReadVariableInfo(matDirName, varDirName, varInfo);
            eosVarInfoMap[varName] = varInfo; 
            i = eosVarInfoMap.find(varName);
        }
    }

    // get this variable's information
    varInfo = i->second; 

    // add the mesh
    avtMeshMetaData *mmd = new avtMeshMetaData(mdMeshName, 1, 0, 0, varInfo.ndims,
                                   varInfo.ndims, AVT_RECTILINEAR_MESH);

    // we use a trick and include the axis lables in the units
    mmd->xUnits = varInfo.xName + '[' + varInfo.xUnits + ']';
    if ((varInfo.yName != "") && (varInfo.yUnits != ""))
        mmd->yUnits = varInfo.yName + '[' + varInfo.yUnits + ']';
    md->Add(mmd);

    // add the variable on this mesh
    avtScalarMetaData *smd = new avtScalarMetaData(mdVarName, mdMeshName, AVT_NODECENT);
    md->Add(smd);

    // add an entry in the dir map for this variable 
    matDirMap[mdVarName] = matDirName;

    delete [] varName;

    return true;
}

// ****************************************************************************
// Method: LEOSFileReader::GetTopDirs
//
// Purpose: 
//   Reads the top-level directory names in the PDB file 
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

void
LEOSFileReader::GetTopDirs()
{
    if (topDirs == 0)
    {
        PDBfile *pdbPtr = pdb->filePointer();
        topDirs = PD_ls(pdbPtr, 0 /*path*/, "Directory", &numTopDirs);
    }
}

// ****************************************************************************
// Method: LEOSFileReader::IdentifyFormat
//
// Purpose: 
//   Identifies the file as an LEOS file.
//
// Returns:    True if the PDB file is an LEOS file; false otherwise.
//
// Notes: Earlier LEOS files don't contain all the symbols in the master
// directory we first try here. So, if that fails, we do it a harder way.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

bool
LEOSFileReader::IdentifyFormat()
{
    bool have_version;
    bool have_date;
    bool have_contents;
    bool validFile = false;

    // Make sure the file has entry "master/version" 
    have_version = pdb->SymbolExists("/master/version"); 

    // Make sure the file has entry "master/date"
    have_date = pdb->SymbolExists("/master/date");

    // Make sure the file has entry "master/contents"
    have_contents = pdb->SymbolExists("/master/contents");

    validFile = have_version && have_date && have_contents; 

    if (validFile)
        return validFile;
   
    //
    // If the above checks fail, it may be an older LEOS file
    //
    GetTopDirs();

    // Look for tell-tale signs that its an LEOS file 
    int i, numFalseAlarms = 0;
    for (i = 0; i < numTopDirs; i++)
    {
        string symStr;
        bool hasOtherSymbols = true;

        symStr = "/" + string(topDirs[i]) + "material_info/bulkmod";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        symStr = "/" + string(topDirs[i]) + "material_info/eosnum";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        symStr = "/" + string(topDirs[i]) + "material_info/rho0";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        symStr = "/" + string(topDirs[i]) + "material_info/rhocrit";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        symStr = "/" + string(topDirs[i]) + "material_info/t0";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        symStr = "/" + string(topDirs[i]) + "material_info/tcrit";
        hasOtherSymbols = hasOtherSymbols && pdb->SymbolExists(symStr.c_str());

        if (hasOtherSymbols)
        {
            //
            // Examine this dir for sub-dirs of known LEOS variable names 
            //
            int numKnownLEOSVars = 0; 
            map<string, eosVarInfo_t>::const_iterator j;
            for (j = eosVarInfoMap.begin(); j != eosVarInfoMap.end(); j++)
            {
                string varDir = "/" + string(topDirs[i]) + j->first + "/";
                if (pdb->SymbolExists(varDir.c_str()))
                   numKnownLEOSVars++;
            }

            // arbitrary test if we see more than 1/3 of the known variables,
            // it must be an eos file
            if (numKnownLEOSVars > eosVarInfoMap.size() / 3)
            {
                validFile = true;
                break;
            }
        }

        //
        // We do this to avoid scanning an entire file only to conclude
        // that we don't think its an LEOS file. If more than 5% of the
        // tested directories DO NOT have any known LEOS variables, we
        // terminate
        //
        numFalseAlarms++;
        if (numFalseAlarms > numTopDirs / 20)
        {
            validFile = false;
            break;
        }
    }

    return validFile;
}

// ****************************************************************************
// Method: LEOSFileReader::ReadMaterialInfo
//
// Purpose: 
//   Reads key information for the material specified in the dirName 
//
// Arguments:
//   matDirName : The PDB directory entry name for the material
//   matName    : returned string containing the material name
//   matForm    : returned string containing the material formula
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

bool
LEOSFileReader::ReadMaterialInfo(const char *matDirName, string &matName,
    string &matForm)
{
    char tmpStr[256];
    char *resultStr;

    // collect the material name
    resultStr = 0;
    sprintf(tmpStr,"/%smaterial_info/material_name", matDirName);
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    if (!FixMatName(resultStr, 128))
        return false;
    matName = string(resultStr);
    delete [] resultStr;

    // collect the material formula (e.g. 'H2O' for water)
    resultStr = 0;
    sprintf(tmpStr,"/%smaterial_info/formula", matDirName);
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    matForm = RemoveSpaces(resultStr);
    delete [] resultStr;

    return true;
}

// ****************************************************************************
// Method: LEOSFileReader::ReadVariableInfo
//
// Purpose: 
//   Reads information of a specified eos variable 
//
// Arguments:
//   meshDirName : The PDB directory entry for the material
//   varDirName  : The PDB directory entry for the variable 
//   varInfo     : returned ref. to eosVarInfo_t struct populated with values
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

bool
LEOSFileReader::ReadVariableInfo(const char *matDirName,
    const char *varDirName, eosVarInfo_t &varInfo)

{
    char tmpStr[512];
    char *resultStr;
    string::size_type n;

    // get the table description, 'table_desc_0' 
    resultStr = 0;
    sprintf(tmpStr,"/%s%stable_desc_0", matDirName, varDirName);
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    string tableDesc = RemoveSpaces(resultStr);
    delete [] resultStr;

    // parse name of PDB symbol containing variable data
    n = tableDesc.find('(');
    varInfo.vName = string(tableDesc,0,n);
    tableDesc = string(tableDesc,n+1,string::npos);
 
    // parse name of PDB symbol containing size in x
    n = tableDesc.find_first_of(",)");
    varInfo.xSize = string(tableDesc,0,n);
    varInfo.ndims = 1;
 
    // parse name of PDB symbol containing size in y, if appropriate
    if (tableDesc[n] == ',')
    {
        tableDesc = string(tableDesc,n+1,string::npos);
        n = tableDesc.find(')');
        varInfo.ySize = string(tableDesc,0,n);
        varInfo.ndims = 2;
    }
    tableDesc = string(tableDesc,n+1,string::npos);

    // parse name of PDB symbol containing x data
    string tmp = '('+varInfo.xSize+')';
    string::size_type tmpn = tmp.length();
    n = tableDesc.find(tmp);
    varInfo.xName = string(tableDesc,0,n);

    // parse name of PDB symbol containing y data, if appropriate
    if (tableDesc[n+tmpn] != 0)
    {
        tableDesc = string(tableDesc,n+tmp.length(),string::npos);
        tmp = '('+varInfo.ySize+')';
        n = tableDesc.find(tmp);
        varInfo.yName = string(tableDesc,0,n);
    }

    // get variable units
    resultStr = 0;
    sprintf(tmpStr,"/%s%sunits_%s", matDirName, varDirName, varInfo.vName.c_str());
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    varInfo.vUnits = RemoveSpaces(resultStr);
    delete [] resultStr;

    // get xUnits
    resultStr = 0;
    sprintf(tmpStr,"/%s%sunits_%s", matDirName, varDirName, varInfo.xName.c_str());
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    varInfo.xUnits = RemoveSpaces(resultStr);
    delete [] resultStr;

    // get yUnits 
    if (varInfo.yName != "")
    resultStr = 0;
    sprintf(tmpStr,"/%s%sunits_%s", matDirName, varDirName, varInfo.yName.c_str());
    if (!pdb->GetString(tmpStr, &resultStr))
        return false;
    varInfo.yUnits = RemoveSpaces(resultStr);
    delete [] resultStr;

    return true;
}

// ****************************************************************************
// Method : ParseContentsAndPopulateMetaData 
//
// Purpose: 
//   Builds avtDatabaseMetaData stuff from the "master/contents" string of the
//   LEOS file. We assume the contents of varInfoMap are correct for this to
//   work. Later, when we go to read an actual variable, if we find out that
//   the assumtions made in constructing the varInfoMap were wrong, we'll 
//   throw an exception.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

bool 
LEOSFileReader::ParseContentsAndPopulateMetaData(avtDatabaseMetaData *md,
    const char *contents)
{
    int i, j, n = 0;

    while (true)
    { 
        char dirDigits[10] = {0,0,0,0,0,0,0,0,0,0};
        char matName[128];
        char varName[32];

        // scan material directory number and name
        i = j = 0;
        bool haveSeenColon = false;
        while (contents[n] != 10)
        {
            if (contents[n] == ':')
            {
                haveSeenColon = true;
                n++;
                continue;
            }

            if (haveSeenColon)
                matName[j++] = contents[n];
            else
                dirDigits[i++] = contents[n];

            n++;
        }
        if (contents[n] == 0)
            break;
        n++;

        // tack on the null char to what we scanned 
        dirDigits[i] = 0;
        matName[j] = 0;

        // fix any bad characters in matname and remove trailing spaces
        if (!FixMatName(matName, sizeof(matName)))
            return false;

        string matDirName = RemoveSpaces(dirDigits);

        // the matDirName doesn't contain the full directory name, so try to find it
        // in the topDirs list
        bool foundDir = false;
        for (i = 0; i < numTopDirs; i++)
        {
           if (string(topDirs[i]).rfind(matDirName) != string::npos)
           {
               foundDir = true;
               break;
           }
        }
        if (!foundDir)
            return false;

        // set matDirName to actual dir name (which will contain a trailing '/')
        matDirName = string(topDirs[i]);

        // read all the variables names for this material
        // and add each variable name and its mesh
        while (contents[n] != 10)
        {
            while (contents[n] == ' ')
                n++;
            j = 0;
            while ((contents[n] != ' ') && (contents[n] != 10))
                varName[j++] = contents[n++];

            // tack on the trailing slash needed by AddVariableAndMesh
            if (j > 0)
            {
                varName[j++] = '/';
                varName[j++] = 0;

                if (!AddVariableAndMesh(md, matDirName.c_str(), matName, 0, varName))
                    return false;
            }
        }

        n++;

        if (contents[n] == 0)
            break;
    }

    return true;

}

// ****************************************************************************
// Method: LEOSFileReader::ReadFileAndPopulateDatabaseMetaData
//
// Purpose: Makes explicit reads from the file to populate database metadata
//
// Arguments:
//   md : The database metadata that we're populating.
//
// Programmer: Mark C. Miller 
// Creation:   February 10, 2004 
//
// ****************************************************************************

void
LEOSFileReader::ReadFileAndPopulateMetaData(avtDatabaseMetaData *md)
{
    int i, j;
    string matName, matForm;

    //
    // Make sure we've read the top-level dirs
    //
    GetTopDirs();

    for (i = 0; i < numTopDirs; i++)
    {
        // skip dirs known NOT to be material dirs
        if ((strcmp(topDirs[i],"&ptrs/") == 0) ||
            (strcmp(topDirs[i],"master/") == 0))
            continue;

        if (ReadMaterialInfo(topDirs[i], matName, matForm))
        {
            int numVars = 0;
            PDBfile *pdbPtr = pdb->filePointer();
            char **varList = PD_ls(pdbPtr, topDirs[i], "Directory", &numVars);

            for (j = 0; j < numVars; j++)
            {
                // skip dirs known NOT to be eos variable dirs
                if (strcmp(varList[j],"material_info/") == 0)
                    continue;

                AddVariableAndMesh(md, topDirs[i], matName.c_str(),
                    matForm.c_str(), varList[j]);
            }

            SFREE(varList);
        }
    }
}

// ****************************************************************************
// Method: LEOSFileReader::PopulateDatabaseMetaData
//
// Purpose: 
//
// Arguments:
//   md : The database metadata that we're populating.
//
// Notes: If we can, we populate metadata from the "/master/contents" info.
// Otherwise, we read and scan the file which is substantially more expensive
//
// Programmer: Mark C. Miller 
// Creation:   February 10, 2004 
//
// ****************************************************************************

void
LEOSFileReader::PopulateDatabaseMetaData(avtDatabaseMetaData *md)
{
    // do an initial ls at the top-level to get a list of all the
    // directories in the file
    GetTopDirs();

    // 
    // Check if user has explicitly told VisIt to try harder.
    // Note: reading from the enviornment is unlikely to work well if the
    // engine is running remotely. This is so because while the user
    // may have thought to set the enviornment where s/he started visit,
    // s/he surely may have not thought to set it in his/her .cshrc file
    // (or whatever) where the remote engine will execute. Fortunately,
    // LEOS data isn't so large that remote engine scenarious are likely.
    //
    char *s = getenv("VISIT_LEOS_TRY_HARDER");
    if (s != 0)
        tryHardLevel = atoi(s);

    //
    // if "master/contents" exists, its faster to use that.
    //
    if (tryHardLevel <= 1)
    {
        char *resultStr;
        if (pdb->GetString("/master/contents", &resultStr))
        {
            bool succeeded = ParseContentsAndPopulateMetaData(md, resultStr);
            delete [] resultStr;

            if (succeeded)
                return;
        }
    }

    //
    // If we get here, we have to do it the hard way
    //
    ReadFileAndPopulateMetaData(md);
}

// ****************************************************************************
// Method: LEOSFileReader::GetMesh
//
// Purpose: 
//   Returns the mesh.
//
// Arguments:
//   state : The time state for which we want the mesh.
//   var   : The name of the mesh that we want.
//
// Returns:    A vtkDataSet object containing the mesh.
//
// Programmer: Mark C. Miller
// Creation:   February 10, 2004 
//
// ****************************************************************************

vtkDataSet *
LEOSFileReader::GetMesh(int state, const char *var)
{
    debug4 << "LEOSFileReader::GetMesh: state=" << state
           << ", var=" << var << endl;

    int size;
    char tmpStr[256];
    string::size_type n;
    map<string, eosVarInfo_t>::const_iterator i;

    // string off the "_mesh" from end of the variable name and
    // compute  the full variable name (e.g. "mat/eos-var") and the
    // variable name
    strcpy(tmpStr,var);
    tmpStr[strlen(tmpStr)-5] = 0;
    string fullVarName = tmpStr;
    n = fullVarName.find('/');
    string varName(fullVarName,n+1,string::npos);

    // look up the PDB material directory the eos variable lives in 
    string matDirName = matDirMap[fullVarName];

    // attempt to lookup the full variable name first, then the short name
    i = eosVarInfoMap.find(fullVarName);
    if (i == eosVarInfoMap.end())
        i = eosVarInfoMap.find(varName);

    if (i == eosVarInfoMap.end())
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    // get the variable info we have in the eosVarInfoMap
    eosVarInfo_t varInfoFromMap = i->second;

    // read the actual variable info from the file
    eosVarInfo_t varInfoFromFile;
    string varDirName = varName + '/';
    ReadVariableInfo(matDirName.c_str(), varDirName.c_str(), varInfoFromFile);

    // this will throw exceptions if file info doesn't agree with our assumptions
    ValidateVariableInfoAssumptions(varInfoFromMap, varInfoFromFile,
        fullVarName.c_str());

    //
    // from here on, we should use the info just read from the file because
    // we may have detected, but ignored, a problem with the info from the map
    //

    int dimensions[3];
    vtkFloatArray  *coords[3];
    for (int i = 0 ; i < 3 ; i++)
    {
        size = 1;
        if (i == 0)
        {
            // set name of PDB symbol containing size to read 
            sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.xSize.c_str());

            // read the size
            if (!pdb->GetInteger(tmpStr, &size))
            {
                EXCEPTION1(InvalidVariableException, var);
            }

            // set name of variable to read
            sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.xName.c_str());
        }
        else if (i == 1)
        {
            // set name of PDB symbol containing size to read 
            sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.ySize.c_str());

            // read the size
            if (!pdb->GetInteger(tmpStr, &size))
            {
                EXCEPTION1(InvalidVariableException, var);
            }

            sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.yName.c_str());
        }

        // Default number of components for an array is 1.
        coords[i] = vtkFloatArray::New();
        coords[i]->SetNumberOfTuples(size);
        dimensions[i] = size;

        if (i < varInfoFromFile.ndims)
        {
            float *fbuf = (float*) coords[i]->GetVoidPointer(0);

            TypeEnum type;
            int nelems;
            int *dims;
            int ndims;
            double *dbuf = (double *) pdb->ReadValues(tmpStr, &type, &nelems, &dims, &ndims);
            delete [] dims;

            if (ndims != 1)
                EXCEPTION2(UnexpectedValueException, 1, ndims);
            if (nelems != size)
                EXCEPTION2(UnexpectedValueException, size, nelems);
            if (type != DOUBLEARRAY_TYPE)
                EXCEPTION2(UnexpectedValueException, DOUBLEARRAY_TYPE, type);

            // copy the values
            int j;
            for (j = 0; j < size; j++)
            {
                fbuf[j] = (float) dbuf[j];
            }

            delete [] dbuf;
        }

    }

    vtkRectilinearGrid  *rGrid = vtkRectilinearGrid::New();
    rGrid->SetDimensions(dimensions);
    rGrid->SetXCoordinates(coords[0]);
    coords[0]->Delete();
    rGrid->SetYCoordinates(coords[1]);
    coords[1]->Delete();
    rGrid->SetZCoordinates(coords[2]);
    coords[2]->Delete();

    return rGrid;
}

// ****************************************************************************
// Method: LEOSFileReader::GetVar
//
// Purpose: 
//   Returns data for the specified variable and time state.
//
// Arguments:
//   state : The time state.
//   var   : The variable that we want to read.
//
// Returns:    A data array containing the data that we want.
//
// Programmer: Mark C. Miller 
// Creation:   February 10, 2004 
//
// ****************************************************************************

vtkDataArray *
LEOSFileReader::GetVar(int state, const char *var)
{
    debug4 << "LEOSFileReader::GetVar: state=" << state
           << ", var=" << var << endl;

    char tmpStr[256];
    string::size_type n;
    map<string, eosVarInfo_t>::const_iterator i;

    // compute  the full variable name (e.g. "mat/eos-var") and the
    // variable name
    string fullVarName = var;
    n = fullVarName.find('/');
    string varName(fullVarName,n+1,string::npos);

    // look up the PDB material directory the eos variable lives in 
    string matDirName = matDirMap[fullVarName];

    // attempt to lookup the full variable name first, then the short name
    i = eosVarInfoMap.find(fullVarName);
    if (i == eosVarInfoMap.end())
        i = eosVarInfoMap.find(varName);

    if (i == eosVarInfoMap.end())
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    // get the variable info we have in the eosVarInfoMap
    eosVarInfo_t varInfoFromMap = i->second;

    // read the actual variable info from the file
    eosVarInfo_t varInfoFromFile;
    string varDirName = varName + '/';
    ReadVariableInfo(matDirName.c_str(), varDirName.c_str(), varInfoFromFile);

    // this will throw exceptions if file info doesn't agree with our assumptions
    ValidateVariableInfoAssumptions(varInfoFromMap, varInfoFromFile,
        fullVarName.c_str());

    //
    // from here on, we should use the info just read from the file because
    // we may have detected, but ignored, a problem with the info from the map
    //

    int Nx, Ny;

    // set name of PDB symbol containing size in x to read 
    sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.xSize.c_str());
    // read the size
    if (!pdb->GetInteger(tmpStr, &Nx))
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    // set name of PDB symbol containing size in y to read 
    sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
                                      varInfoFromFile.ySize.c_str());
    // read the size
    if (!pdb->GetInteger(tmpStr, &Ny))
    {
        EXCEPTION1(InvalidVariableException, var);
    }

    int size = Nx * Ny;

    // set the name of the PDB symbol containing the variable data
    sprintf(tmpStr, "/%s%s/%s", matDirName.c_str(), varName.c_str(),
        varInfoFromFile.vName.c_str());

    // allocate VTK data array for this variable
    vtkFloatArray *var_data = vtkFloatArray::New();
    var_data->SetNumberOfComponents(1);
    var_data->SetNumberOfTuples(size);
    float *fbuf = (float*) var_data->GetVoidPointer(0);

    TypeEnum type;
    int nelems;
    int *dims;
    int ndims;
    double *dbuf = (double *) pdb->ReadValues(tmpStr, &type, &nelems, &dims, &ndims);
    delete [] dims;

    if (ndims != varInfoFromFile.ndims)
        EXCEPTION2(UnexpectedValueException, varInfoFromFile.ndims, ndims);
    if (nelems != size)
        EXCEPTION2(UnexpectedValueException, size, nelems);
    if (type != DOUBLEARRAY_TYPE)
        EXCEPTION2(UnexpectedValueException, DOUBLEARRAY_TYPE, type);

    // copy the values
    int j;
    for (j = 0; j < size; j++)
        fbuf[j] = (float) dbuf[j];

    delete [] dbuf;

    return var_data;
}
