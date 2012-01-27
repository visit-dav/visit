#include <MaterialProperties.h>
#include <Dyna3DFile.h>

#include <sstream>

// ****************************************************************************
// Method: MaterialProperties::MaterialProperties
//
// Purpose: 
//   Constructor
//
// Programmer: Brad Whitlock
// Creation:   Mon Mar  9 16:04:53 PDT 2009
//
// Modifications:
//   
// ****************************************************************************

MaterialProperties::MaterialProperties() : 
    enabled(true),
    materialNumber(0),
    materialName(), eosName(), 
    density(0.), strength(0.), equivalentPlasticStrain(0.), materialLines()
{
    memset(materialInformation, 0, sizeof(double) * MATERIALPROPERTIES_NUM_ITEMS);
}

// ****************************************************************************
// Method: MaterialProperties::RequiresEOS
//
// Purpose: 
//   Returns whether this material properties object requires EOS. Taken from
//   pp.165-166 of Dyna3D.pdf.
//
// Arguments:
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 26 15:28:10 PST 2012
//
// Modifications:
//   
// ****************************************************************************

bool
MaterialProperties::RequiresEOS() const
{
    const int modelsWithEOS[] = {10,11,15,16,17,45,64,65};
    int materialType = int(this->materialInformation[0]);
    for(int i = 0; i < 8; ++i)
        if(modelsWithEOS[i] == materialType)
            return true;
    return false;
}

static int GetI(char *line, int c0, int c1)
{
    int value = 0;
    char tmp;
    tmp = line[c1];
    line[c1] = '\0';
    sscanf(line + c0 - 1, "%d", &value);
    line[c1] = tmp;
    return value;
}

static double GetD(char *line, int c0, int c1)
{
    double value = 0;
    char tmp;
    tmp = line[c1];
    line[c1] = '\0';
    sscanf(line + c0 - 1, "%lg", &value);
    line[c1] = tmp;
    return value;
}

static void GetD10(char *line, double *v, int n)
{
    for(int i = 0; i < n; ++i)
        v[i] = GetD(line, i*10+1, (i+1)*10);
}

static void GetD16(char *line, double *v, int n)
{
    for(int i = 0; i < n; ++i)
        v[i] = GetD(line, i*16+1, (i+1)*16);
}

// ****************************************************************************
// Method: MaterialProperties::ReadCard
//
// Purpose: 
//   Read the material information from a Dyna3D format material card.
//
// Arguments:
//   readline : A callback function that can get a line of text.
//   cbdata   : Data for the callback function.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 26 12:21:38 PST 2012
//
// Modifications:
//   
// ****************************************************************************

void
MaterialProperties::ReadCard(void (*readline)(char *, int, void *), void *cbdata)
{
    char line[1024];

    if(readline == NULL)
        return;

    //
    // The structure of the material card is taken from DYNA3D.pdf section 4.4.
    // I found that file in /usr/gapps/mdg/doc. We're just reading a small
    // subset of material types presently.
    //
    readline(line, 1024, cbdata);
    this->materialNumber         = GetI(line, 1, 5);
    this->materialInformation[0] = GetI(line, 6, 10); // Material type
    this->materialInformation[1] = GetD(line, 11,20); // Density
    this->density = materialInformation[1];
    this->materialInformation[2] = GetI(line, 21,25); // EOS type
    this->materialInformation[3] = GetI(line, 26,30); // Hourglass stabilization method
    this->materialInformation[4] = GetD(line, 31,40); // Hourglass stabilization coefficient
    this->materialInformation[5] = GetI(line, 41,45); // Bulk viscosity type for shock capture
    this->materialInformation[6] = GetD(line, 46,55); // Quadratic bulk viscosity coefficient
    this->materialInformation[7] = GetD(line, 56,65); // Linear bulk viscosity coefficient
    this->materialInformation[8] = GetI(line, 66,70); // Element type for which this material model is valid
    this->materialInformation[9] = GetI(line, 71,75); // unused
    this->materialInformation[10]= GetI(line, 76,80); // Element formulation.

    // Read the name line of the material card and strip out
    // extra spaces from the name.
    readline(line, 1024, cbdata);
    char matNameBuf[1024];
    memset(matNameBuf, 0, 1024);
    char *s = matNameBuf, *ptr = line;
    while(*ptr != '\0' && ((s - matNameBuf) < 1024))
    {
        if(*ptr == ' ')
        {
           *s++ = *ptr;
           while(*ptr == ' ')
               ptr++;
        }
        else if(*ptr != '\n')
        {
           *s++ = *ptr++;
        }
    }
    this->materialName = matNameBuf;
    if(this->materialName == " ")
        this->materialName = std::string();

    int materialType = int(this->materialInformation[0]);
    double *v = this->materialInformation + 11;
    if(materialType <= 1)
    {
        // Material type 1 has the following format:
        for(int i = 0; i < 6; ++i)
        {
            readline(line, 1024, cbdata);
            GetD10(line, v, 8);
            v += 8;
        }
    }
    else if(materialType == 11)
    {
        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 5);
        v += 5;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 7);
        v += 7;

#if 0
// This is how it is described in the book.
        for(int i = 0; i < 2; ++i)
        {
            readline(line, 1024, cbdata);
            GetD16(line, v, 5);
            v += 5;
        }
#else
// This is how I've seen it in a file.
        readline(line, 1024, cbdata);
        GetD16(line, v, 1);
        v ++;

        readline(line, 1024, cbdata);
        GetD16(line, v, 1);
        v ++;
#endif
    }
    else if(materialType == 64)
    {
        // Material type 64 has the following format:
        for(int i = 0; i < 5; ++i)
        {
            readline(line, 1024, cbdata);
            GetD10(line, v, 8);
            v += 8;
        }
        for(int i = 0; i < 2; ++i)
        {
            readline(line, 1024, cbdata);
            GetD16(line, v, 5);
            v += 5;
        }

        this->strength = materialInformation[12];
        this->equivalentPlasticStrain = materialInformation[19];
    }
    else
    {
        // Assume that the material has 8 cards. We've read 2.
        for(int i = 0; i < 6; ++i)
        {
            readline(line, 1024, cbdata);
            this->materialLines.push_back(std::string(line));
        }
    }

    int eosType = int(this->materialInformation[2]);
    if(eosType > 0)
    {
        readline(line, 1024, cbdata);
        this->eosName = std::string(line);

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;
        readline(line, 1024, cbdata);
        v[0] = GetD(line, 1,10);
    }
}

// ****************************************************************************
// Method: MaterialProperties::WriteCard
//
// Purpose: 
//   Write the material information as a Dyna3D format material card.
//
// Arguments:
//   writeline : A callback function that can write a line of text.
//   cbdata    : Data for the callback function.
//
// Returns:    
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Thu Jan 26 12:21:38 PST 2012
//
// Modifications:
//   
// ****************************************************************************


void
MaterialProperties::WriteCard(void (*writeline)(const char *, void *), void *cbdata) const
{
    // Make a mutable copy of the materialInformation so we can override some values.
    double mvalues[MATERIALPROPERTIES_NUM_ITEMS];
    memcpy(mvalues, this->materialInformation, sizeof(double) * MATERIALPROPERTIES_NUM_ITEMS);
    mvalues[1] = this->density;

    char line[200];
    sprintf(line,
        "%5d" "%5d" "%1.4lE" "%5d" "%5d" "%1.4lE" "%5d" "%1.4lE" "%1.4lE" "%5d" "%5d" "%5d",
        this->materialNumber,
        int(mvalues[0]),   //I
        mvalues[1],        //D
        int(mvalues[2]),   //I
        int(mvalues[3]),   //I
        mvalues[4],        //D
        int(mvalues[5]),   //I
        mvalues[6],        //D
        mvalues[7],        //D
        int(mvalues[8]),   //I
        int(mvalues[9]),   //I
        int(mvalues[10])   //I
        );
    writeline(line, cbdata);

    writeline(this->materialName.c_str(), cbdata);

    int materialType = int(mvalues[0]);
    const double *v = mvalues + 11;
    if(materialType <= 1)
    {
        for(int i = 0; i < 6; ++i)
        {
            sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
            writeline(line, cbdata);
            v += 8;
        }
    }
    else if(materialType == 11)
    {
        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        writeline(line, cbdata);
        v += 8;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4]);
        writeline(line, cbdata);
        v += 5;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        writeline(line, cbdata);
        v += 8;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
        writeline(line, cbdata);
        v += 7;
#if 0
// This is how it is described in the book
        for(int i = 0; i < 2; ++i)
        {
            sprintf(line, 
                "% 1.9lE"
                "% 1.9lE"
                "% 1.9lE"
                "% 1.9lE"
                "% 1.9lE",
                 v[0], v[1], v[2], v[3], v[4]);
            writeline(line, cbdata);
            v += 5;
        }
#else
// This is what I've seen in practice.
        sprintf(line, "% 1.9lE", v[0]);
        writeline(line, cbdata);
        v ++;
        sprintf(line, "% 1.9lE", v[0]);
        writeline(line, cbdata);
        v ++;
#endif
    }
    else if(materialType == 64)
    {
        mvalues[12] = this->strength;
        mvalues[19] = this->equivalentPlasticStrain;

        for(int i = 0; i < 5; ++i)
        {
            sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
            writeline(line, cbdata);
            v += 8;
        }
        for(int i = 0; i < 2; ++i)
        {
            sprintf(line, 
                "      % 1.3lE"
                "      % 1.3lE"
                "      % 1.3lE"
                "      % 1.3lE"
                "      % 1.3lE",
                    v[0], v[1], v[2], v[3], v[4]);
            writeline(line, cbdata);
            v += 5;
        }
    }
    else
    {
        // We don't understand the material type so print out text that we 
        // stashed from the file.
        for(size_t i = 0; i < this->materialLines.size(); ++i)
            writeline(this->materialLines[i].c_str(), cbdata);
    }

    if(RequiresEOS())
    {
        writeline(this->eosName.c_str(), cbdata);
        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        writeline(line, cbdata);
        v += 8;
        sprintf(line, "% 1.3lE", v[0]);
        writeline(line, cbdata);
    }
}

////////////////////////////////////////////////////////////////////////////////

void
OverrideMaterialProperties(MaterialPropertiesVector &props,
    const MaterialPropertiesVector &overrides)
{
    for(size_t i = 0; i < props.size(); ++i)
    {
        for(size_t j = 0; j < overrides.size(); ++j)
        {
            if(overrides[j].materialNumber == props[i].materialNumber)
            {
                props[i] = overrides[j];
                break;
            }
        }
    }
}

void
SetEnabledMaterials(MaterialPropertiesVector &props, const std::string &matlist)
{
    if(!matlist.empty())
    {
        for(size_t i = 0; i < props.size(); ++i)
            props[i].enabled = false;

        std::istringstream iss(matlist);
        std::string cur;
        const char separator = ',';
        while(std::getline(iss, cur, separator))
        {
            if(iss)
            {
                int materialNumber = atoi(cur.c_str());
                for(size_t i = 0; i < props.size(); ++i)
                {
                    if(props[i].materialNumber == materialNumber)
                    {
                        props[i].enabled = true;
                        break;
                    }
                }
            }
        }
    }
}

void
PrintMaterialData(FILE *f, const MaterialPropertiesVector &props)
{
    for(size_t i = 0; i < props.size(); ++i)
    {
        const MaterialProperties &m = props[i];
        fprintf(f, "Material %d\n", m.materialNumber);
        fprintf(f, "\tname=%s\n", m.materialName.c_str());
        fprintf(f, "\tdensity=%lg\n", m.density);
        fprintf(f, "\tstrength=%lg\n", m.strength);
        fprintf(f, "\tequivalentPlasticStrain=%lg\n", m.equivalentPlasticStrain);
    }
}

// ****************************************************************************
// Method: ReadMaterialPropertiesFile
//
// Purpose: 
//   Read a material properties file and return the material properties therein.
//
// Arguments:
//   filename : The name of the file to open.
//
// Returns:    The vector of material properties.
//
// Note:       
//
// Programmer: Brad Whitlock
// Creation:   Wed Oct 6 10:17:00 PDT 2010
//
// Modifications:
//   
// ****************************************************************************

MaterialPropertiesVector
ReadMaterialPropertiesFile(const std::string &filename)
{
    MaterialPropertiesVector mats;

    // If the file extension is ".dyn" then try and read the material properties
    // from the Dyna3D file.
    bool readProps = true;
    if(filename.substr(filename.size()-3, 3) == "dyn")
    {
        Dyna3DFile matfile;
        if(matfile.Read(filename.c_str()))
        {
            for(int i = 0; i < matfile.GetNumMaterials(); ++i)
                mats.push_back(matfile.GetMaterial(i));

            readProps = false;
        }
    }

    // Try and read the file as a text file that contains the material props.
    if(readProps)
    {
        ifstream file(filename.c_str());
        if(!file.fail())
        {
            char buf[1024];
            while(!file.eof())
            {
                file.getline(buf, 1024);
                if(buf[0] != '#')
                {
                    MaterialProperties m;

                    // Get the material number
                    char tmp = buf[10];
                    buf[10] = '\0';
                    m.materialNumber = atoi(buf);
                    buf[10] = tmp;

                    // Get the string name (eliminate trailing spaces)
                    tmp = buf[60];
                    buf[60] = '\0';
                    for(int i = 60-1; i >= 10; i--)
                    {
                        if(buf[i] == ' ')
                            buf[i] = '\0';
                        else
                            break;
                    }
                    m.materialName = std::string(buf+10);
                    buf[60] = tmp;

                    sscanf(buf + 60, "%lg %lg %lg", &m.density, &m.strength,
                           &m.equivalentPlasticStrain);

                    mats.push_back(m);
                }
            }
        }
    }

    return mats;
}
