// Copyright (c) Lawrence Livermore National Security, LLC and other VisIt
// Project developers.  See the top-level LICENSE file for dates and other
// details.  No copyright assignment is required to contribute to VisIt.

#include "avtOpenFOAMMetaDataHelper.h"

#include <vtkDirectory.h>

#include <DebugStream.h>
#include <FileFunctions.h>

#include <vtksys/SystemTools.hxx>

#include <zlib.h>

#include <algorithm>
#include <cctype>
#include <cerrno>
#include <cstddef>
#include <cstdlib>
#include <fstream>
#include <map>
#include <set>
#include <sstream>
#include <vector>

using std::string;

namespace
{
static const size_t HEADER_READ_LIMIT = 64 * 1024;
static const size_t BUFFER_SIZE = 8192;

bool
EndsWith(const string &value, const string &suffix)
{
    return value.size() >= suffix.size() &&
           value.compare(value.size() - suffix.size(), suffix.size(), suffix) == 0;
}

string
StripComments(const string &text)
{
    string result;
    result.reserve(text.size());

    bool inLineComment = false;
    bool inBlockComment = false;
    bool inString = false;
    char stringDelimiter = '\0';

    for (size_t i = 0; i < text.size(); ++i)
    {
        const char c = text[i];
        const char next = (i + 1 < text.size() ? text[i + 1] : '\0');

        if (inLineComment)
        {
            if (c == '\n')
            {
                inLineComment = false;
                result.push_back(c);
            }
            continue;
        }

        if (inBlockComment)
        {
            if (c == '*' && next == '/')
            {
                inBlockComment = false;
                ++i;
            }
            continue;
        }

        if (inString)
        {
            result.push_back(c);
            if (c == stringDelimiter && (i == 0 || text[i - 1] != '\\'))
            {
                inString = false;
            }
            continue;
        }

        if (c == '/' && next == '/')
        {
            inLineComment = true;
            ++i;
            continue;
        }

        if (c == '/' && next == '*')
        {
            inBlockComment = true;
            ++i;
            continue;
        }

        if (c == '"' || c == '\'')
        {
            inString = true;
            stringDelimiter = c;
        }

        result.push_back(c);
    }

    return result;
}

class TokenStream
{
  public:
    TokenStream(const string &text_) : text(text_), pos(0), havePeek(false) { }

    bool Next(string &token)
    {
        if (havePeek)
        {
            token = peekToken;
            havePeek = false;
            return true;
        }
        return ReadToken(token);
    }

    bool Peek(string &token)
    {
        if (!havePeek)
        {
            if (!ReadToken(peekToken))
            {
                return false;
            }
            havePeek = true;
        }

        token = peekToken;
        return true;
    }

  private:
    bool ReadToken(string &token)
    {
        SkipWhitespace();
        if (pos >= text.size())
        {
            return false;
        }

        const char c = text[pos];
        if (c == '(' || c == ')' || c == '{' || c == '}' || c == ';')
        {
            token.assign(1, c);
            ++pos;
            return true;
        }

        if (c == '"' || c == '\'')
        {
            const char delimiter = c;
            size_t start = pos++;
            while (pos < text.size())
            {
                if (text[pos] == delimiter && text[pos - 1] != '\\')
                {
                    ++pos;
                    break;
                }
                ++pos;
            }
            token = text.substr(start, pos - start);
            return true;
        }

        const size_t start = pos;
        while (pos < text.size())
        {
            const char current = text[pos];
            if (isspace(static_cast<unsigned char>(current)) ||
                current == '(' || current == ')' ||
                current == '{' || current == '}' || current == ';')
            {
                break;
            }
            ++pos;
        }

        token = text.substr(start, pos - start);
        return !token.empty();
    }

    void SkipWhitespace()
    {
        while (pos < text.size() &&
               isspace(static_cast<unsigned char>(text[pos])))
        {
            ++pos;
        }
    }

    const string &text;
    size_t        pos;
    bool          havePeek;
    string        peekToken;
};

void
SkipBalanced(TokenStream &tokens, const char openChar, const char closeChar)
{
    int depth = 1;
    string token;
    while (depth > 0 && tokens.Next(token))
    {
        if (token.size() == 1 && token[0] == openChar)
        {
            ++depth;
        }
        else if (token.size() == 1 && token[0] == closeChar)
        {
            --depth;
        }
    }
}

string
Unquote(const string &value)
{
    if (value.size() >= 2 &&
        ((value[0] == '"' && value[value.size() - 1] == '"') ||
         (value[0] == '\'' && value[value.size() - 1] == '\'')))
    {
        return value.substr(1, value.size() - 2);
    }

    return value;
}

bool
ResolveReadablePath(const string &path,
                    bool tryGzipSuffix,
                    string &resolvedPath,
                    bool &isGzip)
{
    if (vtksys::SystemTools::FileExists(path.c_str(), true))
    {
        resolvedPath = path;
        isGzip = EndsWith(path, ".gz");
        return true;
    }

    if (tryGzipSuffix && !EndsWith(path, ".gz"))
    {
        const string gzPath(path + ".gz");
        if (vtksys::SystemTools::FileExists(gzPath.c_str(), true))
        {
            resolvedPath = gzPath;
            isGzip = true;
            return true;
        }
    }

    return false;
}

bool
ReadFilePrefix(const string &path, bool isGzip, size_t limit, string &contents)
{
    contents.clear();

    if (isGzip)
    {
        gzFile file = gzopen(path.c_str(), "rb");
        if (file == NULL)
        {
            return false;
        }

        char buffer[BUFFER_SIZE];
        int remaining = static_cast<int>(limit);
        while (remaining > 0)
        {
            const int request = std::min<int>(remaining, static_cast<int>(BUFFER_SIZE));
            const int count = gzread(file, buffer, request);
            if (count <= 0)
            {
                break;
            }
            contents.append(buffer, count);
            remaining -= count;
        }

        gzclose(file);
        return !contents.empty();
    }

    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input)
    {
        return false;
    }

    std::vector<char> buffer(limit);
    input.read(&buffer[0], static_cast<std::streamsize>(limit));
    contents.assign(&buffer[0], static_cast<size_t>(input.gcount()));
    return !contents.empty();
}

bool
ReadWholeFile(const string &path, bool isGzip, string &contents)
{
    contents.clear();

    if (isGzip)
    {
        gzFile file = gzopen(path.c_str(), "rb");
        if (file == NULL)
        {
            return false;
        }

        char buffer[BUFFER_SIZE];
        for (;;)
        {
            const int count = gzread(file, buffer, static_cast<unsigned int>(BUFFER_SIZE));
            if (count <= 0)
            {
                break;
            }
            contents.append(buffer, count);
        }

        gzclose(file);
        return true;
    }

    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input)
    {
        return false;
    }

    std::ostringstream stream;
    stream << input.rdbuf();
    contents = stream.str();
    return true;
}

bool
ExtractFoamFileBlock(const string &text, string &foamFileBlock)
{
    const string cleaned(StripComments(text));
    const size_t foamFilePos = cleaned.find("FoamFile");
    if (foamFilePos == string::npos)
    {
        return false;
    }

    const size_t openBrace = cleaned.find('{', foamFilePos);
    if (openBrace == string::npos)
    {
        return false;
    }

    int depth = 1;
    for (size_t i = openBrace + 1; i < cleaned.size(); ++i)
    {
        if (cleaned[i] == '{')
        {
            ++depth;
        }
        else if (cleaned[i] == '}')
        {
            --depth;
            if (depth == 0)
            {
                foamFileBlock = cleaned.substr(openBrace + 1, i - openBrace - 1);
                return true;
            }
        }
    }

    return false;
}

bool
ExtractHeaderValue(const string &foamFileBlock,
                   const string &key,
                   string &value)
{
    TokenStream tokens(foamFileBlock);
    string token;
    while (tokens.Next(token))
    {
        if (token != key)
        {
            continue;
        }

        if (!tokens.Next(value))
        {
            return false;
        }

        value = Unquote(value);
        return true;
    }

    return false;
}

bool
ParseHeader(const string &text,
            avtOpenFOAMMetaDataHelper::HeaderInfo &header)
{
    string foamFileBlock;
    if (!ExtractFoamFileBlock(text, foamFileBlock))
    {
        return false;
    }

    ExtractHeaderValue(foamFileBlock, "class", header.className);
    ExtractHeaderValue(foamFileBlock, "object", header.objectName);
    return !header.className.empty() || !header.objectName.empty();
}

bool
ExtractTopLevelEntryNames(const string &text, std::vector<string> &names)
{
    const string cleaned(StripComments(text));
    TokenStream tokens(cleaned);

    string token;
    while (tokens.Next(token))
    {
        if (token == "FoamFile")
        {
            string openBrace;
            if (tokens.Next(openBrace) && openBrace == "{")
            {
                SkipBalanced(tokens, '{', '}');
            }
            continue;
        }

        if (token != "(" && token != "{")
        {
            continue;
        }

        const char openChar = token[0];
        const char closeChar = (openChar == '(' ? ')' : '}');
        int depth = 1;
        while (depth > 0 && tokens.Next(token))
        {
            if (token.size() == 1 && token[0] == openChar)
            {
                ++depth;
                continue;
            }

            if (token.size() == 1 && token[0] == closeChar)
            {
                --depth;
                continue;
            }

            if (depth != 1 || token == ";")
            {
                continue;
            }

            string nextToken;
            if (!tokens.Peek(nextToken) || nextToken != "{")
            {
                continue;
            }

            names.push_back(Unquote(token));
            tokens.Next(nextToken);
            SkipBalanced(tokens, '{', '}');
        }

        return !names.empty();
    }

    return false;
}

bool
IsTimeDirectoryName(const string &name, double &value)
{
    if (name.empty())
    {
        return false;
    }

    char *endPtr = NULL;
    errno = 0;
    value = strtod(name.c_str(), &endPtr);
    if (endPtr == name.c_str() || *endPtr != '\0' || errno == ERANGE)
    {
        return false;
    }

    return true;
}

bool
IsEulerianBackupFile(const string &name)
{
    return !name.empty() &&
           (name[name.size() - 1] == '~' ||
            EndsWith(name, ".bak") ||
            EndsWith(name, ".BAK") ||
            EndsWith(name, ".old") ||
            EndsWith(name, ".save"));
}

string
FallbackObjectName(const string &name)
{
    if (EndsWith(name, ".gz"))
    {
        return name.substr(0, name.size() - 3);
    }
    return name;
}

bool
IsSupportedEulerianClass(const string &className,
                         bool &isCellField,
                         bool &isPointField)
{
    isCellField = false;
    isPointField = false;

    if (className == "volScalarField" ||
        className == "volVectorField" ||
        className == "volSphericalTensorField" ||
        className == "volSymmTensorField" ||
        className == "volTensorField")
    {
        isCellField = true;
        return true;
    }

    if (className == "pointScalarField" ||
        className == "pointVectorField" ||
        className == "pointSphericalTensorField" ||
        className == "pointSymmTensorField" ||
        className == "pointTensorField")
    {
        isPointField = true;
        return true;
    }

    return false;
}

bool
IsSupportedLagrangianClass(const string &className)
{
    return className == "labelField" ||
           className == "scalarField" ||
           className == "vectorField" ||
           className == "sphericalTensorField" ||
           className == "symmTensorField" ||
           className == "tensorField";
}

void
InsertField(std::vector<avtOpenFOAMMetaDataHelper::FieldInfo> &fields,
            const string &name,
            const string &className)
{
    for (size_t i = 0; i < fields.size(); ++i)
    {
        if (fields[i].name == name)
        {
            return;
        }
    }

    avtOpenFOAMMetaDataHelper::FieldInfo field;
    field.name = name;
    field.className = className;
    fields.push_back(field);
}

void
InsertName(std::vector<string> &names, const string &name)
{
    if (std::find(names.begin(), names.end(), name) == names.end())
    {
        names.push_back(name);
    }
}

bool
CompareTimeInfo(const avtOpenFOAMMetaDataHelper::TimeInfo &lhs,
                const avtOpenFOAMMetaDataHelper::TimeInfo &rhs)
{
    if (lhs.value < rhs.value)
    {
        return true;
    }
    if (lhs.value > rhs.value)
    {
        return false;
    }
    return lhs.name < rhs.name;
}
}

avtOpenFOAMMetaDataHelper::avtOpenFOAMMetaDataHelper(const string &fileName,
                                                     int caseType_)
    : inputFileName(fileName),
      controlDictPath(),
      casePath(),
      metadataRootPath(),
      regionNames(),
      times(),
      error(),
      caseType(caseType_)
{
    Initialize();
}

bool
avtOpenFOAMMetaDataHelper::IsValid() const
{
    return error.empty();
}

const string &
avtOpenFOAMMetaDataHelper::GetError() const
{
    return error;
}

const std::vector<avtOpenFOAMMetaDataHelper::TimeInfo> &
avtOpenFOAMMetaDataHelper::GetTimes() const
{
    return times;
}

bool
avtOpenFOAMMetaDataHelper::Initialize()
{
    if (!ResolveControlDictAndCasePath())
    {
        return false;
    }

    if (!SelectMetadataRootPath())
    {
        return false;
    }

    if (!DiscoverTimes())
    {
        return false;
    }

    if (!DiscoverRegions())
    {
        return false;
    }

    return true;
}

bool
avtOpenFOAMMetaDataHelper::ResolveControlDictAndCasePath()
{
    const string baseName(FileFunctions::Basename(inputFileName));
    if (baseName == "controlDict")
    {
        controlDictPath = inputFileName;
    }
    else
    {
        const string dir(FileFunctions::Dirname(inputFileName));
        const string systemControlDict(dir + VISIT_SLASH_STRING +
                                       "system" + VISIT_SLASH_STRING +
                                       "controlDict");
        if (vtksys::SystemTools::FileExists(systemControlDict.c_str(), true))
        {
            controlDictPath = systemControlDict;
        }
        else
        {
            controlDictPath = dir + VISIT_SLASH_STRING + "controlDict";
        }
    }

    const string controlDir(FileFunctions::Dirname(controlDictPath));
    if (FileFunctions::Basename(controlDir) == "system")
    {
        casePath = FileFunctions::Dirname(controlDir);
    }
    else
    {
        casePath = controlDir;
    }

    if (casePath.empty())
    {
        casePath = ".";
    }

    if (casePath[casePath.size() - 1] != '/')
    {
        casePath += VISIT_SLASH_STRING;
    }

    return true;
}

bool
avtOpenFOAMMetaDataHelper::SelectMetadataRootPath()
{
    metadataRootPath = casePath;
    if (caseType != 0)
    {
        return true;
    }

    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(casePath.c_str()))
    {
        dir->Delete();
        error = "Cannot open OpenFOAM case directory: " + casePath;
        return false;
    }

    std::vector<std::pair<int, string> > processors;
    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string name(dir->GetFile(i));
        if (name.compare(0, 9, "processor") != 0)
        {
            continue;
        }

        const string procNo(name.substr(9));
        char *endPtr = NULL;
        const long value = strtol(procNo.c_str(), &endPtr, 10);
        if (endPtr == procNo.c_str() || *endPtr != '\0' || value < 0)
        {
            continue;
        }

        processors.push_back(std::make_pair(static_cast<int>(value), name));
    }
    dir->Delete();

    if (processors.empty())
    {
        error = "No processor directories were found for decomposed OpenFOAM case: " +
                casePath;
        return false;
    }

    std::sort(processors.begin(), processors.end());
    metadataRootPath += processors[0].second + VISIT_SLASH_STRING;
    return true;
}

bool
avtOpenFOAMMetaDataHelper::DiscoverTimes()
{
    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(metadataRootPath.c_str()))
    {
        dir->Delete();
        error = "Cannot open OpenFOAM metadata directory: " + metadataRootPath;
        return false;
    }

    std::vector<TimeInfo> discoveredTimes;
    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string name(dir->GetFile(i));
        if (!dir->FileIsDirectory(name.c_str()))
        {
            continue;
        }

        TimeInfo timeInfo;
        timeInfo.name = name;
        if (!IsTimeDirectoryName(name, timeInfo.value))
        {
            continue;
        }

        discoveredTimes.push_back(timeInfo);
    }
    dir->Delete();

    std::sort(discoveredTimes.begin(), discoveredTimes.end(), CompareTimeInfo);

    for (size_t i = 0; i < discoveredTimes.size(); ++i)
    {
        if (!times.empty() && times.back().value == discoveredTimes[i].value)
        {
            debug4 << "Ignoring duplicate OpenFOAM time directory '"
                   << discoveredTimes[i].name << "' for value "
                   << discoveredTimes[i].value << endl;
            continue;
        }
        times.push_back(discoveredTimes[i]);
    }

    if (times.empty() &&
        vtksys::SystemTools::FileIsDirectory((metadataRootPath + "constant").c_str()))
    {
        TimeInfo constantTime;
        constantTime.name = "0";
        constantTime.value = 0.0;
        times.push_back(constantTime);
    }

    if (times.empty())
    {
        error = "OpenFOAM case contains no timesteps under " + metadataRootPath;
        return false;
    }

    return true;
}

bool
avtOpenFOAMMetaDataHelper::DiscoverRegions()
{
    const string constantPath(metadataRootPath + "constant");
    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(constantPath.c_str()))
    {
        dir->Delete();
        error = "Cannot open OpenFOAM constant directory: " + constantPath;
        return false;
    }

    if (HasMeshFiles(constantPath))
    {
        regionNames.push_back("");
    }

    std::vector<string> subRegions;
    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string name(dir->GetFile(i));
        if (name == "." || name == ".." || !dir->FileIsDirectory(name.c_str()))
        {
            continue;
        }

        if (HasMeshFiles(constantPath + VISIT_SLASH_STRING + name))
        {
            subRegions.push_back(name);
        }
    }
    dir->Delete();

    std::sort(subRegions.begin(), subRegions.end());
    regionNames.insert(regionNames.end(), subRegions.begin(), subRegions.end());

    if (regionNames.empty())
    {
        error = "OpenFOAM case contains no polyMesh directories under " + constantPath;
        return false;
    }

    return true;
}

string
avtOpenFOAMMetaDataHelper::RegionPath(const string &regionName) const
{
    return (regionName.empty() ? string() : VISIT_SLASH_STRING + regionName);
}

string
avtOpenFOAMMetaDataHelper::RegionPrefix(const string &regionName) const
{
    return (regionName.empty() ? string() : regionName + VISIT_SLASH_STRING);
}

string
avtOpenFOAMMetaDataHelper::TimeRegionPath(const string &timeName,
                                          const string &regionName) const
{
    return metadataRootPath + timeName + RegionPath(regionName);
}

string
avtOpenFOAMMetaDataHelper::ConstantRegionPath(const string &regionName) const
{
    return metadataRootPath + "constant" + RegionPath(regionName);
}

bool
avtOpenFOAMMetaDataHelper::HasMeshFiles(const string &path) const
{
    return vtksys::SystemTools::FileExists((path + VISIT_SLASH_STRING +
                                            "polyMesh" + VISIT_SLASH_STRING +
                                            "faces").c_str(), true) ||
           vtksys::SystemTools::FileExists((path + VISIT_SLASH_STRING +
                                            "polyMesh" + VISIT_SLASH_STRING +
                                            "faces.gz").c_str(), true) ||
           vtksys::SystemTools::FileExists((path + VISIT_SLASH_STRING +
                                            "polyMesh" + VISIT_SLASH_STRING +
                                            "boundary").c_str(), true) ||
           vtksys::SystemTools::FileExists((path + VISIT_SLASH_STRING +
                                            "polyMesh" + VISIT_SLASH_STRING +
                                            "boundary.gz").c_str(), true);
}

string
avtOpenFOAMMetaDataHelper::ResolveMeshInstance(int timeState,
                                               const string &regionName) const
{
    for (int i = timeState; i >= 0; --i)
    {
        if (HasMeshFiles(TimeRegionPath(times[i].name, regionName)))
        {
            return times[i].name;
        }
    }

    if (HasMeshFiles(ConstantRegionPath(regionName)))
    {
        return "constant";
    }

    return string();
}

bool
avtOpenFOAMMetaDataHelper::ReadHeader(const string &path,
                                      HeaderInfo &header,
                                      bool tryGzipSuffix) const
{
    string resolvedPath;
    bool isGzip = false;
    if (!ResolveReadablePath(path, tryGzipSuffix, resolvedPath, isGzip))
    {
        return false;
    }

    string prefix;
    if (!ReadFilePrefix(resolvedPath, isGzip, HEADER_READ_LIMIT, prefix))
    {
        return false;
    }

    header.className.clear();
    header.objectName.clear();
    return ParseHeader(prefix, header);
}

bool
avtOpenFOAMMetaDataHelper::ReadEntryNames(const string &path,
                                          std::vector<string> &names) const
{
    string resolvedPath;
    bool isGzip = false;
    if (!ResolveReadablePath(path, true, resolvedPath, isGzip))
    {
        return false;
    }

    string contents;
    if (!ReadWholeFile(resolvedPath, isGzip, contents))
    {
        return false;
    }

    return ExtractTopLevelEntryNames(contents, names);
}

void
avtOpenFOAMMetaDataHelper::GatherBoundaryMetaData(int timeState,
                                                  MetaData &metaData) const
{
    for (size_t regionIndex = 0; regionIndex < regionNames.size(); ++regionIndex)
    {
        const string &regionName = regionNames[regionIndex];
        const string meshInstance(ResolveMeshInstance(timeState, regionName));
        if (meshInstance.empty())
        {
            continue;
        }

        const string boundaryPath((meshInstance == "constant"
                                       ? ConstantRegionPath(regionName)
                                       : TimeRegionPath(meshInstance, regionName)) +
                                  VISIT_SLASH_STRING + "polyMesh" +
                                  VISIT_SLASH_STRING + "boundary");
        std::vector<string> boundaryNames;
        if (!ReadEntryNames(boundaryPath, boundaryNames))
        {
            continue;
        }

        InsertName(metaData.patchNames, RegionPrefix(regionName) + "internalMesh");
        for (size_t i = 0; i < boundaryNames.size(); ++i)
        {
            InsertName(metaData.patchNames,
                       RegionPrefix(regionName) + boundaryNames[i]);
        }
    }
}

void
avtOpenFOAMMetaDataHelper::GatherZoneMetaData(int timeState,
                                              MetaData &metaData) const
{
    for (size_t regionIndex = 0; regionIndex < regionNames.size(); ++regionIndex)
    {
        const string &regionName = regionNames[regionIndex];
        const string meshInstance(ResolveMeshInstance(timeState, regionName));
        if (meshInstance.empty())
        {
            continue;
        }

        const string regionPath((meshInstance == "constant"
                                     ? ConstantRegionPath(regionName)
                                     : TimeRegionPath(meshInstance, regionName)) +
                                VISIT_SLASH_STRING + "polyMesh" +
                                VISIT_SLASH_STRING);

        std::vector<string> names;
        if (ReadEntryNames(regionPath + "pointZones", names))
        {
            for (size_t i = 0; i < names.size(); ++i)
            {
                InsertName(metaData.pointZoneNames, names[i]);
            }
        }

        names.clear();
        if (ReadEntryNames(regionPath + "faceZones", names))
        {
            for (size_t i = 0; i < names.size(); ++i)
            {
                InsertName(metaData.faceZoneNames, names[i]);
            }
        }

        names.clear();
        if (ReadEntryNames(regionPath + "cellZones", names))
        {
            for (size_t i = 0; i < names.size(); ++i)
            {
                InsertName(metaData.cellZoneNames, names[i]);
            }
        }
    }
}

void
avtOpenFOAMMetaDataHelper::ReadEulerianFieldDirectory(
    const string &path,
    std::vector<FieldInfo> &cellFields,
    std::vector<FieldInfo> &pointFields) const
{
    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(path.c_str()))
    {
        dir->Delete();
        return;
    }

    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string fileName(dir->GetFile(i));
        if (fileName.empty() || dir->FileIsDirectory(fileName.c_str()) ||
            IsEulerianBackupFile(fileName))
        {
            continue;
        }

        HeaderInfo header;
        if (!ReadHeader(path + VISIT_SLASH_STRING + fileName, header, false))
        {
            continue;
        }

        bool isCellField = false;
        bool isPointField = false;
        if (!IsSupportedEulerianClass(header.className, isCellField, isPointField))
        {
            continue;
        }

        const string objectName(header.objectName.empty()
                                    ? FallbackObjectName(fileName)
                                    : header.objectName);
        if (isCellField)
        {
            InsertField(cellFields, objectName, header.className);
        }
        else if (isPointField)
        {
            InsertField(pointFields, objectName, header.className);
        }
    }

    dir->Delete();
}

void
avtOpenFOAMMetaDataHelper::ReadLagrangianFieldDirectory(
    const string &path,
    std::vector<FieldInfo> &lagrangianFields) const
{
    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(path.c_str()))
    {
        dir->Delete();
        return;
    }

    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string fileName(dir->GetFile(i));
        if (fileName.empty() || dir->FileIsDirectory(fileName.c_str()) ||
            IsEulerianBackupFile(fileName))
        {
            continue;
        }

        HeaderInfo header;
        if (!ReadHeader(path + VISIT_SLASH_STRING + fileName, header, false))
        {
            continue;
        }

        if (!IsSupportedLagrangianClass(header.className))
        {
            continue;
        }

        const string objectName(header.objectName.empty()
                                    ? FallbackObjectName(fileName)
                                    : header.objectName);
        InsertField(lagrangianFields, objectName, header.className);
    }

    dir->Delete();
}

void
avtOpenFOAMMetaDataHelper::GatherFieldMetaDataForTime(const string &timeName,
                                                      MetaData &metaData) const
{
    for (size_t regionIndex = 0; regionIndex < regionNames.size(); ++regionIndex)
    {
        ReadEulerianFieldDirectory(TimeRegionPath(timeName, regionNames[regionIndex]),
                                   metaData.cellFields,
                                   metaData.pointFields);
    }
}

bool
avtOpenFOAMMetaDataHelper::GatherLagrangianMetaDataForRegion(
    const string &timeName,
    const string &regionName,
    MetaData &metaData) const
{
    const string lagrangianRoot(TimeRegionPath(timeName, regionName) +
                                VISIT_SLASH_STRING + "lagrangian");

    vtkDirectory *dir = vtkDirectory::New();
    if (!dir->Open(lagrangianRoot.c_str()))
    {
        dir->Delete();
        return false;
    }

    bool found = false;
    std::vector<string> cloudNames;
    for (int i = 0; i < dir->GetNumberOfFiles(); ++i)
    {
        const string name(dir->GetFile(i));
        if (name == "." || name == ".." || !dir->FileIsDirectory(name.c_str()))
        {
            continue;
        }
        cloudNames.push_back(name);
    }
    dir->Delete();

    std::sort(cloudNames.begin(), cloudNames.end());
    for (size_t i = 0; i < cloudNames.size(); ++i)
    {
        const string cloudPath(lagrangianRoot + VISIT_SLASH_STRING + cloudNames[i]);
        HeaderInfo header;
        if (!ReadHeader(cloudPath + VISIT_SLASH_STRING + "positions", header, true))
        {
            continue;
        }

        if (header.objectName != "positions" ||
            header.className.find("Cloud") == string::npos)
        {
            continue;
        }

        found = true;
        InsertName(metaData.lagrangianPatches,
                   RegionPrefix(regionName) + "lagrangian" +
                   VISIT_SLASH_STRING + cloudNames[i]);
        ReadLagrangianFieldDirectory(cloudPath, metaData.lagrangianFields);
    }

    if (found)
    {
        return true;
    }

    HeaderInfo header;
    if (ReadHeader(lagrangianRoot + VISIT_SLASH_STRING + "positions", header, true) &&
        header.objectName == "positions" &&
        header.className.find("Cloud") != string::npos)
    {
        InsertName(metaData.lagrangianPatches,
                   RegionPrefix(regionName) + "lagrangian");
        ReadLagrangianFieldDirectory(lagrangianRoot, metaData.lagrangianFields);
        return true;
    }

    return false;
}

void
avtOpenFOAMMetaDataHelper::GatherLagrangianMetaDataForTime(const string &timeName,
                                                           MetaData &metaData) const
{
    for (size_t regionIndex = 0; regionIndex < regionNames.size(); ++regionIndex)
    {
        GatherLagrangianMetaDataForRegion(timeName, regionNames[regionIndex], metaData);
    }
}

void
avtOpenFOAMMetaDataHelper::GatherFieldMetaData(int timeState,
                                               MetaData &metaData) const
{
    GatherFieldMetaDataForTime(times[timeState].name, metaData);

    bool foundLagrangian = false;
    for (size_t regionIndex = 0; regionIndex < regionNames.size(); ++regionIndex)
    {
        foundLagrangian |= GatherLagrangianMetaDataForRegion(times[timeState].name,
                                                             regionNames[regionIndex],
                                                             metaData);
    }

    if (timeState == 0 && times.size() > 1)
    {
        GatherFieldMetaDataForTime(times[1].name, metaData);
        if (!foundLagrangian)
        {
            GatherLagrangianMetaDataForTime(times[1].name, metaData);
        }
    }

    std::sort(metaData.cellFields.begin(), metaData.cellFields.end(),
              [](const avtOpenFOAMMetaDataHelper::FieldInfo &lhs,
                 const avtOpenFOAMMetaDataHelper::FieldInfo &rhs)
              { return lhs.name < rhs.name; });
    std::sort(metaData.pointFields.begin(), metaData.pointFields.end(),
              [](const avtOpenFOAMMetaDataHelper::FieldInfo &lhs,
                 const avtOpenFOAMMetaDataHelper::FieldInfo &rhs)
              { return lhs.name < rhs.name; });
    std::sort(metaData.lagrangianFields.begin(), metaData.lagrangianFields.end(),
              [](const avtOpenFOAMMetaDataHelper::FieldInfo &lhs,
                 const avtOpenFOAMMetaDataHelper::FieldInfo &rhs)
              { return lhs.name < rhs.name; });
}

bool
avtOpenFOAMMetaDataHelper::ReadMetaData(int timeState,
                                        bool readZones,
                                        MetaData &metaData) const
{
    metaData = MetaData();
    if (!IsValid())
    {
        return false;
    }

    if (timeState < 0 || static_cast<size_t>(timeState) >= times.size())
    {
        return false;
    }

    GatherBoundaryMetaData(timeState, metaData);
    if (readZones)
    {
        GatherZoneMetaData(timeState, metaData);
    }
    GatherFieldMetaData(timeState, metaData);
    return true;
}
