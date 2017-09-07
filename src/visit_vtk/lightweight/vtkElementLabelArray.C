#include "vtkElementLabelArray.h"
#include "DebugStream.h"
#include "StringHelpers.h"
vtkStandardNewMacro(vtkElementLabelArray);

vtkElementLabelArray::vtkElementLabelArray()
{

}

vtkElementLabelArray::~vtkElementLabelArray()
{

}

bool vtkElementLabelArray::AddName(const std::string &name, 
                            const std::vector<int> &labelRangesBegin,
                            const std::vector<int> &labelRangesEnd,
                            const std::vector<int> &elementIdsBegin,
                            const std::vector<int> &elementIdsEnd) 
{
    typedef std::map<std::string, LabelToZoneMapping> MapType; 

    bool valid = true;
    const int arraySize = this->GetNumberOfTuples();
    const int numRanges = labelRangesBegin.size();
    //
    // Basic error checking 
    //
    if(numRanges < 1)
    {
        debug5<<"vtkElementLablesArray: Range of size < 1\n";
        valid = false;
    }
    if(labelRangesEnd.size() != numRanges)
    {
        debug5<<"vtkElementLablesArray: Mismatched number of elem ranges \n";
        valid = false;
    }
    if(elementIdsBegin.size() != numRanges)
    {
        debug5<<"vtkElementLablesArray: Mismatched number of id ranges \n";
        valid = false;
    }
    if(elementIdsEnd.size() != numRanges)
    {
        debug5<<"vtkElementLablesArray: Mismatched number of ranges \n";
        valid = false;
    }
    //
    //  Validate the ranges
    //
    for(int i = 0; i < numRanges; ++i)
    {
        if(elementIdsEnd[i] >= arraySize  ||
           elementIdsBegin[i] < 0)
        {
            debug5<<"vtkElementLablesArray: Invalid range: out of bounds \n";
            debug5<<"vtkElementLablesArray: Array size "<<arraySize
                  <<" id begin "<<elementIdsBegin[i]
                  <<" end "<<elementIdsEnd[i]<<"\n";
            valid = false;
        }

        if(elementIdsBegin[i] > elementIdsEnd[i])
        {
            debug5<<"vtkElementLablesArray: Invalid range: begin is greater than end\n";
            valid = false;
        }

        if(name == "")
        {
            debug5<<"vtkElementLablesArray: Name empty\n";
            valid = false;
        }
       
        MapType::iterator  it; 
        it = NameMapping.find(name);
        if(it != NameMapping.end())
        {
            valid = false;
            debug5<<"vtkElementLablesArray: Name already exists in named array\n";
        }
    }  // for each range

    // TODO: Check of overlaps?

    if(!valid)
    {
        debug5<<"vtkElementLabelArray: Failed to add name to named array\n";
    }
    
    //
    //  Everything should be valid at this point
    //
    LabelToZoneMapping newEntry;
    newEntry.LabelRangesBegin = labelRangesBegin;
    newEntry.LabelRangesEnd= labelRangesEnd;
    newEntry.ElementIdsBegin = elementIdsBegin;
    newEntry.ElementIdsEnd= elementIdsEnd;
    NameMapping[name] = newEntry;
    return valid;
}

bool vtkElementLabelArray::GetElementId(const std::string &name,
                                 int &id)
{
    typedef std::map<std::string, LabelToZoneMapping> MapType; 

    std::vector<std::string> tokens = StringHelpers::split(name, ' ');
    if(tokens.size() != 2)
    {
        return false;
    }  

    int localId = 0;
    bool isInt = StringHelpers::StringToInt(tokens[1], localId);
    
    if(!isInt || localId < 0)
    {
        std::cout<<"Is not a int \n"; 
        return false;
    }

    MapType::iterator  it; 
    it = NameMapping.find(tokens[0]);
    if(it == NameMapping.end())
    {
        return false;
    }
    // label starting index. E.g., Mili is one indexed
    const LabelToZoneMapping &nameMap = NameMapping[tokens[0]];
    const int numRanges = nameMap.LabelRangesBegin.size(); 
    bool found = false;
    for(int i = 0; i < numRanges; ++i)
    {
        if(localId >= nameMap.LabelRangesBegin[i] &&
           localId <= nameMap.LabelRangesEnd[i])
        {
            found = true;
            const int offset = localId - nameMap.LabelRangesBegin[i];
            id = nameMap.ElementIdsBegin[i] + offset;
            break;
        }
    }

    return found;
}

// ----------------------------------------------------------------------
void vtkElementLabelArray::Destruct(void *ptr)
{
    vtkElementLabelArray *array = reinterpret_cast<vtkElementLabelArray*>(ptr);
    if(array ==  NULL)
    {
        std::cout<<"Failed to destruct. Void pointer failed to cast to named array\n";
    }
    else
    {
        array->Delete();
    }
}

// ----------------------------------------------------------------------
void vtkElementLabelArray::PrintSelf(ostream& os, vtkIndent indent)
{
    this->Superclass::PrintSelf(os,indent);

    os << indent
       << "vtkElementLabelArray stuff\n";
    std::map<std::string, LabelToZoneMapping>::iterator it; 
    for(it = NameMapping.begin(); it != NameMapping.end(); ++it)
    {
        os<<"Range name: "<<it->first<<"\n";
        for(int i = 0; i < it->second.LabelRangesBegin.size(); ++i)
        {
            os<<"  [block "<<i<<"]\n";
            os<<"    Labels ranges "<<it->second.LabelRangesBegin[i]<<" -- "<<it->second.LabelRangesEnd[i]<<"\n";
            os<<"    Element ids   "<<it->second.ElementIdsBegin[i]<<" -- "<<it->second.ElementIdsEnd[i]<<"\n";
        }
    }

 }
