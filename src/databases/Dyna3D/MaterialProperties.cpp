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
    materialName(),
    density(0.),
    strength(0.),
    equivalentPlasticStrain(0.)
{
}

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
