#include <MaterialProperties.h>
#include <cstring>
#include <cstdio>

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
//   Brad Whitlock, Fri Mar  9 10:24:04 PST 2012
//   I added material models 10, 15.
//
// ****************************************************************************

static void
fillspaces(char *buf, int buflen)
{
    for(int i = 0; i < buflen; ++i) 
        buf[i] = ' ';
    buf[buflen-1] = '\0';
}

void
MaterialProperties::ReadCard(void (*readline_cb)(char *, int, void *), void *cbdata)
{
    if(readline_cb == NULL)
        return;

    char line[1024];
    memset(line, 0, 1024 * sizeof(char));

#define readline fillspaces(line, 1024);readline_cb

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
    else if(materialType == 10)
    {
        // DYNA3D.pdf page 205
        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        this->strength = v[1];
        bool needCard9 = v[7] > 0.;
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        this->equivalentPlasticStrain = v[0];
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        if(needCard9)
        {
            readline(line, 1024, cbdata);
            GetD10(line, v, 4);
            v += 4;
        }
    }
    else if(materialType == 11)
    {
        readline(line, 1024, cbdata);
        this->strength = v[1]; // card3, value 2
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 5);
        v += 5;

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        readline(line, 1024, cbdata);
        this->equivalentPlasticStrain = v[2]; // card6, value 3
        GetD10(line, v, 7);
        v += 7;

#if 1
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
    else if(materialType == 15)
    {
        // DYNA3D.pdf page 225
        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        this->strength = v[1];
        v += 8;

        readline(line, 1024, cbdata);
        GetD10(line, v, 6);
        this->equivalentPlasticStrain = v[0];
        v += 6;

        readline(line, 1024, cbdata);
        GetD10(line, v, 7);
        v += 7;

        readline(line, 1024, cbdata);
        GetD10(line, v, 6);
        v += 6;

        readline(line, 1024, cbdata);
        GetD10(line, v, 4);
        v += 4;

        // Read as - - val - val
        // The dash represents a value we read from spaces but store anyway.
        // We don't write it back out that way.
        readline(line, 1024, cbdata);
        GetD10(line, v, 5);
        v += 5;
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
        // DYNA3D.pdf page 445
        readline(line, 1024, cbdata);
        this->eosName = std::string(line);

        readline(line, 1024, cbdata);
        GetD10(line, v, 8);
        v += 8;

        if(eosType == 4)
        {
        }
        else // most forms are like form 1, which has 1 more number.
        {
            readline(line, 1024, cbdata);
            v[0] = GetD(line, 1,10);
        }
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
//   Brad Whitlock, Fri Mar  9 10:26:04 PST 2012
//   I added material models 10, 15.
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
    else if(materialType == 10)
    {
        bool needCard9 = mvalues[18] > 0.;

        // Poke potentially modified values back into mvalues before we write out.
        mvalues[12] = this->strength;
        mvalues[19] = this->equivalentPlasticStrain;
        for(int i = 0; i < 6; ++i)
        {
            sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
            writeline(line, cbdata);
            v += 8;
        }
        if(needCard9)
        {
            sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3]);
            writeline(line, cbdata);
            v += 4;
        }
    }
    else if(materialType == 11)
    {
        // Poke potentially modified values back into mvalues before we write out.
        mvalues[12] = this->strength;
        mvalues[34] = this->equivalentPlasticStrain;

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
#if 1
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
    else if(materialType == 15)
    {
        // Poke potentially modified values back into mvalues before we write out.
        mvalues[12] = this->strength;
        mvalues[19] = this->equivalentPlasticStrain;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        writeline(line, cbdata);
        v += 8;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5]);
        writeline(line, cbdata);
        v += 6;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5], v[6]);
        writeline(line, cbdata);
        v += 7;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3], v[4], v[5]);
        writeline(line, cbdata);
        v += 6;

        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                v[0], v[1], v[2], v[3]);
        writeline(line, cbdata);
        v += 4;

        sprintf(line, "                    % 1.3lE          % 1.3lE",
                v[2], v[4]);
        writeline(line, cbdata);
        v += 5;
    }
    else if(materialType == 64)
    {
        // Poke potentially modified values back into mvalues before we write out.
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
        // DYNA3D.pdf page 445
        writeline(this->eosName.c_str(), cbdata);
        sprintf(line, "% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE% 1.3lE",
                    v[0], v[1], v[2], v[3], v[4], v[5], v[6], v[7]);
        writeline(line, cbdata);
        v += 8;
        int eosType = int(this->materialInformation[2]);
        if(eosType == 4)
        {
        }
        else // Form 1 and those derived from it.
        {
            sprintf(line, "% 1.3lE", v[0]);
            writeline(line, cbdata);
        }
    }
}
