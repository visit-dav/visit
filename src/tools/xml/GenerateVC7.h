/*****************************************************************************
*
* Copyright (c) 2000 - 2009, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400124
* All rights reserved.
*
* This file is  part of VisIt. For  details, see https://visit.llnl.gov/.  The
* full copyright notice is contained in the file COPYRIGHT located at the root
* of the VisIt distribution or at http://www.llnl.gov/visit/copyright.html.
*
* Redistribution  and  use  in  source  and  binary  forms,  with  or  without
* modification, are permitted provided that the following conditions are met:
*
*  - Redistributions of  source code must  retain the above  copyright notice,
*    this list of conditions and the disclaimer below.
*  - Redistributions in binary form must reproduce the above copyright notice,
*    this  list of  conditions  and  the  disclaimer (as noted below)  in  the
*    documentation and/or other materials provided with the distribution.
*  - Neither the name of  the LLNS/LLNL nor the names of  its contributors may
*    be used to endorse or promote products derived from this software without
*    specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT  HOLDERS AND CONTRIBUTORS "AS IS"
* AND ANY EXPRESS OR  IMPLIED WARRANTIES, INCLUDING,  BUT NOT  LIMITED TO, THE
* IMPLIED WARRANTIES OF MERCHANTABILITY AND  FITNESS FOR A PARTICULAR  PURPOSE
* ARE  DISCLAIMED. IN  NO EVENT  SHALL LAWRENCE  LIVERMORE NATIONAL  SECURITY,
* LLC, THE  U.S.  DEPARTMENT OF  ENERGY  OR  CONTRIBUTORS BE  LIABLE  FOR  ANY
* DIRECT,  INDIRECT,   INCIDENTAL,   SPECIAL,   EXEMPLARY,  OR   CONSEQUENTIAL
* DAMAGES (INCLUDING, BUT NOT  LIMITED TO, PROCUREMENT OF  SUBSTITUTE GOODS OR
* SERVICES; LOSS OF  USE, DATA, OR PROFITS; OR  BUSINESS INTERRUPTION) HOWEVER
* CAUSED  AND  ON  ANY  THEORY  OF  LIABILITY,  WHETHER  IN  CONTRACT,  STRICT
* LIABILITY, OR TORT  (INCLUDING NEGLIGENCE OR OTHERWISE)  ARISING IN ANY  WAY
* OUT OF THE  USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
* DAMAGE.
*
*****************************************************************************/

#ifndef GENERATE_VC7_H
#define GENERATE_VC7_H
#include <QTextStream>

// ****************************************************************************
//  File:  Generate_VC7
//
//  Purpose:
//    VC7 specific code.
//
//  Modifications:
//    Kathleen Bonnell, Mon Jun 2 18:19:27 PDT 2008
//    Code consolidation for Release/Debug/Purify configurations.
//    Consolidate database project methods into one.
//  
//    Kathleen Bonnell, Fri Jun 20 08:19:55 PDT 2008
//    Move 'CreateKey' method to parent class.
//    
//    Kathleen Bonnell, Tue Aug 19 10:32:54 PDT 2008
//    Modified how moc files are generated, to eliminate unnecessary recompile.
//    
//    Kathleen Bonnell, Wed Aug 20 10:22:17 PDT 2008
//    Add ENGINE and MDSERVER preprocessor defines when plugin has engine
//    specific mdserver specific code respectively.
// 
//    Brad Whitlock, Tue Nov 18 16:16:49 PST 2008
//    Qt 4.
//
//    Kathleen Bonnell, Wed Dec 17 15:25:45 MST 2008
//    Update moc location to reflect QT 4, Remove unnecessary blocks. 
//
// ****************************************************************************

    void WriteProject_TOP_LEVEL_Version7(QTextStream &out)
    {
        QString configs[] = {"Release", "Debug", "Purify"};
        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
        out << "<VisualStudioProject" << Endl;
        out << "\tProjectType=\"Visual C++\"" << Endl;
        out << "\tVersion=\"7.10\"" << Endl;
        out << "\tName=\"" << name << "\"" << Endl;
        out << "\tSccProjectName=\"\"" << Endl;
        out << "\tSccLocalPath=\"\"" << Endl;
        out << "\tKeyword=\"MakeFileProj\">" << Endl;
        out << "\t<Platforms>" << Endl;
        out << "\t\t<Platform" << Endl;
        out << "\t\t\tName=\"Win32\"/>" << Endl;
        out << "\t</Platforms>" << Endl;
        out << "\t<Configurations>" << Endl;
        for (int i = 0; i < 3; i++)
        {
            out << "\t\t<Configuration" << Endl;
            out << "\t\t\tName=\"" << configs[i] << "|Win32\"" << Endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[i] << "\"" << Endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[i] 
                << "\"" << Endl;
            out << "\t\t\tConfigurationType=\"10\"" << Endl;
            out << "\t\t\tUseOfMFC=\"0\"" << Endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << Endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] 
                << "\\" << name << ".tlb\"" << Endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << Endl;
            out << "\t\t</Configuration>" << Endl;
        }
        out << "\t</Configurations>" << Endl;
        out << "\t<References>" << Endl;
        out << "\t</References>" << Endl;
        out << "\t<Files>" << Endl;
        out << "\t</Files>" << Endl;
        out << "\t<Globals>" << Endl;
        out << "\t</Globals>" << Endl;
        out << "</VisualStudioProject>" << Endl;
    }

    void WriteProjectSolution_Version7(QTextStream &out, 
                                        const vector<QString> &projects)
    {
        QString solutionKey("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}");
        vector<QString> keys;
        QString configs[] = {"Release", "Debug" , "Purify"};

        size_t i;
        for(i = 0; i < projects.size(); ++i)
            keys.push_back(CreateKey());

        out << "Microsoft Visual Studio Solution File, Format Version 8.00" 
            << Endl;
        out << "Project(\"" << solutionKey << "\") = \"" << projects[0] 
            << "\", \"" << projects[0] << ".vcproj\", \"" << keys[0] << "\"" 
            << Endl;
        out << "\tProjectSection(ProjectDependencies) = postProject" << Endl;
        for(i = 1; i < keys.size(); ++i)
            out << "\t\t" << keys[i] << " = " << keys[i] << Endl;

        out << "\tEndProjectSection" << Endl;
        out << "EndProject" << Endl;

        for(i = 1; i < keys.size(); ++i)
        {
            out << "Project(\"" << solutionKey << "\") = \"" << projects[i] 
                << "\", \"" << projects[i] << ".vcproj\", \"" << keys[i] 
                << "\"" << Endl;
            out << "\tProjectSection(ProjectDependencies) = postProject" 
                << Endl;
            out << "\tEndProjectSection" << Endl;
            out << "EndProject" << Endl;
        }

        out << "Global" << Endl;
        out << "\tGlobalSection(SolutionConfiguration) = preSolution" << Endl;
        out << "\t\tRelease = Release" << Endl;
        out << "\t\tDebug = Debug" << Endl;
        out << "\t\tPurify = Purify" << Endl;
        out << "\tEndGlobalSection" << Endl;
        out << "\tGlobalSection(ProjectConfiguration) = postSolution" << Endl;
    
        for(i = 0; i < keys.size(); ++i)
        {
            for (int j = 0; j < 3; j++)
            {
                out << "\t\t" << keys[i] << "." << configs[j] 
                    << ".ActiveCfg = " << configs[j] << "|Win32" << Endl;
                out << "\t\t" << keys[i] << "." << configs[j] 
                     << ".Build.0 = " << configs[j] << "|Win32" << Endl;
            }
        }

        out << "\tEndGlobalSection" << Endl;
        out << "\tGlobalSection(ExtensibilityGlobals) = postSolution" << Endl;
        out << "\tEndGlobalSection" << Endl;
        out << "\tGlobalSection(ExtensibilityAddIns) = postSolution" << Endl;
        out << "\tEndGlobalSection" << Endl;
        out << "EndGlobal" << Endl;
    }

    void WriteProjectHelper_Version7(QTextStream &out, const QString &pluginType, 
             char pluginComponent, const QString &exports, const QString &libs,
             const vector<QString> &srcFiles, const vector<QString> &hdrFiles, 
             const vector<QString> &mocFiles)
    {
        const char *suffix = (pluginComponent == 'E') ? "_ser" : "";

        QString configs[] = {"Release", "Debug", "Purify"};
        QString debug[] = {"NDEBUG", "_DEBUG", "_DEBUG"};
        int optims[] = {2, 0, 0};
        int rtl[] = {2, 3, 3};
        int brc[] = {0, 3, 0};
        int dif[] = {0, 4, 3};
        int li[] = {1, 2, 1};
        QString pType("Operator");
        if(pluginType == "plots")
            pType = "Plot";

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
        out << "<VisualStudioProject" << Endl;
        out << "\tProjectType=\"Visual C++\"" << Endl;
        out << "\tVersion=\"7.10\"" << Endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << Endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << Endl;
        out << "\tSccProjectName=\"\"" << Endl;
        out << "\tSccLocalPath=\"\">" << Endl;
        out << "\t<Platforms>" << Endl;
        out << "\t\t<Platform" << Endl;
        out << "\t\t\tName=\"Win32\"/>" << Endl;
        out << "\t</Platforms>" << Endl;
        out << "\t<Configurations>" << Endl;
        for (int j = 0; j < 3; ++j)
        {
            out << "\t\t<Configuration" << Endl;
            out << "\t\t\tName=\"" << configs[j] << "|Win32\"" << Endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\tConfigurationType=\"2\"" << Endl;
            out << "\t\t\tUseOfMFC=\"0\"" << Endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << Endl;
            out << "\t\t\tCharacterSet=\"2\">" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << Endl;
            out << "\t\t\t\tOptimization=\"" << optims[j] << "\"" << Endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << Endl;
            out << "\t\t\t\tOptimizeForProcessor=\"2\"" << Endl;
            out << "\t\t\t\tAdditionalIncludeDirectories=\""
                << pluginBase;
            if (withinDevDir)
                out << "\\" << pluginType << "\\" << name;
            out << ";" << visitIncludes;
            out << ";" << includeBase << "\\VisIt"
                << ";" << includeBase << "\\VTK"
                << ";" << tpIncludes
                << "\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\""
                << preproc  << ";" << debug[j]
                << ";GENERAL_PLUGIN_EXPORTS";
            if (exports != "")
                out << ";" << exports;
            if (pluginComponent == 'E' && hasEngineSpecificCode )
                out << ";ENGINE"; 
            out << "\"" << Endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tStringPooling=\"TRUE\"" << Endl;
            else 
                out << "\t\t\t\tBasicRuntimeChecks=\"" << brc[j] 
                    << "\"" << Endl;
            out << "\t\t\t\tRuntimeLibrary=\"" << rtl[j] << "\"" << Endl;
            out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << Endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << Endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << Endl;
            out << "\t\t\t\tPrecompiledHeaderFile=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pch\"" << Endl;
            out << "\t\t\t\tAssemblerListingLocation=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\t\tObjectFile=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\t\tProgramDataBaseFileName=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\\"" << Endl;
            if (configs[j] == "Release")
            {
                out << "\t\t\t\tWarningLevel=\"3\"" << Endl;
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << Endl;
            }
            else
            {
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
                out << "\t\t\t\tDebugInformationFormat=\"" << dif[j] 
                    << "\"/>" << Endl;
            }
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCLinkerTool\"" << Endl;
            if (configs[j] == "Purify")
                out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << Endl;
            out << "\t\t\t\tAdditionalDependencies=\"" << libs << tpLibs << "\"" << Endl;
            out << "\t\t\t\tOutputFile=\"" << binBase << "\\" << configs[j] 
                << "\\" << pluginType << "\\lib" << pluginComponent << name 
                << pType << suffix << ".dll\"" << Endl;
            out << "\t\t\t\tLinkIncremental=\"" << li[j] << "\"" << Endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
            out << "\t\t\t\tAdditionalLibraryDirectories=\""
                << libBase << "\\ThirdParty;"
                << libBase << "\\" << configs[j]
                << ";" << tpLibDir;
            out << "\"" << Endl;
            if (configs[j] != "Release")
                out << "\t\t\t\tGenerateDebugInformation=\"TRUE\""  << Endl;
            out << "\t\t\t\tProgramDatabaseFile=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pdb\"" << Endl;
            out << "\t\t\t\tImportLibrary=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\lib" << pluginComponent  
                << name << pType << suffix << ".lib\"" << Endl;
            out << "\t\t\t\tTargetMachine=\"1\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] 
                << "\"" << Endl;
            out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << Endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << Endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << Endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] 
                << "\"" << Endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" 
                << Endl;
            out << "\t\t</Configuration>" << Endl;
        }
        out << "\t</Configurations>" << Endl;
        out << "\t<References>" << Endl;
        out << "\t</References>" << Endl;
        out << "\t<Files>" << Endl;
        out << "\t\t<Filter" << Endl;
        out << "\t\t\tName=\"Source Files\"" << Endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << Endl;
    
        for(size_t i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << Endl;
            out << "\t\t\t\tRelativePath=\"";
#ifdef _WIN32
            if (withinDevDir)
            {
                out << pluginBase << "\\" << pluginType 
                    << "\\" << name << "\\" << srcFiles[i];
            }
            else 
            {
                out << srcFiles[i];
            }
#else
            out << pluginBase << "\\" << pluginType 
                << "\\" << name << "\\" << srcFiles[i];
#endif
            out << "\">" << Endl;
            for (int j = 0; j < 3; j++)
            {
                out << "\t\t\t\t<FileConfiguration" << Endl;
                out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << Endl;
                out << "\t\t\t\t\t<Tool" << Endl;
                out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << Endl;
                out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << Endl;
                out << "\t\t\t\t</FileConfiguration>" << Endl;
            }
            out << "\t\t\t</File>" << Endl;
        }
        out << "\t\t</Filter>" << Endl;

        if(pluginComponent == 'G' && hdrFiles.size() > 0)
        {
            out << "\t\t<Filter" << Endl;
            out << "\t\t\tName=\"Header Files\"" << Endl;
            out << "\t\t\tFilter=\"h;hxx\">" << Endl;
    
            for(size_t i = 0; i < hdrFiles.size(); ++i)
            {
                out << "\t\t\t<File" << Endl;
                out << "\t\t\t\tRelativePath=\"";
#ifdef _WIN32
                if (withinDevDir)
                {
                    out << pluginBase << "\\" << pluginType 
                        << "\\" << name << "\\" << hdrFiles[i];
                }
                else 
                {
                    out << hdrFiles[i];
                }
#else
                out << pluginBase << "\\" << pluginType 
                    << "\\" << name << "\\" << hdrFiles[i];
#endif
                out << "\">" << Endl;
                for (int j = 0; j < 3; j++)
                {
                    out << "\t\t\t\t<FileConfiguration" << Endl;
                    out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" 
                        << Endl;
                    out << "\t\t\t\t\t<Tool" << Endl;
                    out << "\t\t\t\t\t\tName=\"VCCustomBuildTool\"" << Endl;
                    out << "\t\t\t\t\t\tDescription=\"Moc&apos;ing " 
                        << hdrFiles[i] << " ...\"" << Endl;
                    out << "\t\t\t\t\t\tCommandLine="
                        << "\"..\\..\\bin\\MSVC7.Net\\ThirdParty\\moc.exe "
                        << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << hdrFiles[i] << " -o " << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << mocFiles[i] << "\"" << Endl;
                    out << "\t\t\t\t\t\tAdditionalDependencies="
                        << "\"..\\..\\bin\\MSVC7.Net\\ThirdParty\\moc.exe\"" 
                        << Endl;
                    out << "\t\t\t\t\t\tOutputs=\"" << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << mocFiles[i] << "\"/>" << Endl;
                    out << "\t\t\t\t</FileConfiguration>" << Endl;
                }
                out << "\t\t\t</File>" << Endl;
            }
            out << "\t\t</Filter>" << Endl;
            out << "\t\t<Filter" << Endl;
            out << "\t\t\tName=\"Generated MOC Files\"" << Endl;
            out << "\t\t\tFilter=\"\">" << Endl;
    
            for(size_t i = 0; i < mocFiles.size(); ++i)
            {
                out << "\t\t\t<File" << Endl;
                out << "\t\t\t\tRelativePath=\"";
#ifdef _WIN32
                if (withinDevDir)
                {
                    out << pluginBase << "\\" << pluginType 
                        << "\\" << name << "\\" << mocFiles[i];
                }
                else 
                {
                    out << mocFiles[i];
                }
#else
                out << pluginBase << "\\" << pluginType 
                    << "\\" << name << "\\" << mocFiles[i];
#endif
                out << "\">" << Endl;
                for (int j = 0; j < 3; j++)
                {
                    out << "\t\t\t\t<FileConfiguration" << Endl;
                    out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << Endl;
                    out << "\t\t\t\t\t<Tool" << Endl;
                    out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << Endl;
                    out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << Endl;
                    out << "\t\t\t\t</FileConfiguration>" << Endl;
                }
                out << "\t\t\t</File>" << Endl;
            }
            out << "\t\t</Filter>" << Endl;
        }
        out << "\t</Files>" << Endl;
        out << "\t<Globals>" << Endl;
        out << "\t</Globals>" << Endl;
        out << "</VisualStudioProject>" << Endl;
    }


    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject_Version7(QTextStream &out, char pluginComponent,
             const vector<QString> &srcFiles, const QString &libs,
             const QString &tpLibs, const QString &tpIncludes,
             const QString &tpPreproc)
    {
        QString pluginDefs = "";
        char *pluginSuffix = "Database";
        QString configs[] = {"Release", "Debug", "Purify"};
        int optims[] = {2, 0, 0};
        int brc[] = {0, 3, 0};

        pluginDefs = "GENERAL_PLUGIN_EXPORTS";
        if (pluginComponent == 'M')
        {
            pluginDefs += ";MDSERVER_PLUGIN_EXPORTS";
            if (has_MDS_specific_code)
                pluginDefs += ";MDSERVER";
        }
        else if(pluginComponent == 'E')
        {
            pluginDefs += ";ENGINE_PLUGIN_EXPORTS";
            if (hasEngineSpecificCode)
                pluginDefs += ";ENGINE";
            pluginSuffix = "Database_ser";
        }

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << Endl;
        out << "<VisualStudioProject" << Endl;
        out << "\tProjectType=\"Visual C++\"" << Endl;
        out << "\tVersion=\"7.10\"" << Endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << Endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << Endl;
        out << "\tSccProjectName=\"\"" << Endl;
        out << "\tSccLocalPath=\"\">" << Endl;
        out << "\t<Platforms>" << Endl;
        out << "\t\t<Platform" << Endl;
        out << "\t\t\tName=\"Win32\"/>" << Endl;
        out << "\t</Platforms>" << Endl;
        out << "\t<Configurations>" << Endl;
        {
        QString debug[] = {"NDEBUG", "_DEBUG", "_DEBUG"};
        int rtl[] = {2, 3, 3};
        int li[] = {1, 2, 1};
        int dif[] = {0, 4, 3};
        for (int i = 0; i < 3; ++i)
        {
            out << "\t\t<Configuration" << Endl;
            out << "\t\t\tName=\""<< configs[i]<< "|Win32\"" << Endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\tConfigurationType=\"2\"" << Endl;
            out << "\t\t\tUseOfMFC=\"0\"" << Endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << Endl;
            out << "\t\t\tCharacterSet=\"2\">" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << Endl;
            out << "\t\t\t\tOptimization=\"" << optims[i] << "\"" << Endl;
            if (configs[i] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << Endl;
            out << "\t\t\t\tOptimizeForProcessor=\"2\"" << Endl;
            out << "\t\t\t\tAdditionalIncludeDirectories=\""
                << pluginBase;
            if (withinDevDir)
                out << "\\databases\\" << name;
            out << ";" << visitIncludes;
            out << ";" << includeBase << "\\VisIt"
                << ";" << includeBase << "\\VTK";
            if (pluginComponent != 'I')
                out << tpIncludes;
            out << "\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\""
                << preproc
                << ";" << debug[i] << ";"
                << pluginDefs;
            if (pluginComponent != 'I')
               out << tpPreproc;
            if (pluginComponent == 'E' && hasEngineSpecificCode)
               out << ";ENGINE";
            if (pluginComponent == 'M' && has_MDS_specific_code)
               out << ";MDSERVER";
            out << "\"" << Endl;
            if (configs[i] == "Release")
            {
                out << "\t\t\t\tStringPooling=\"TRUE\"" << Endl;
            }
            else
            {
                out << "\t\t\t\tBasicRuntimeChecks=\"" << brc[i] 
                    << "\"" << Endl;
            }
            out << "\t\t\t\tRuntimeLibrary=\"" << rtl[i] << "\"" << Endl;
            out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << Endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << Endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << Endl;
            out << "\t\t\t\tPrecompiledHeaderFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pch\"" << Endl;
            out << "\t\t\t\tAssemblerListingLocation=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\t\tObjectFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << Endl;
            out << "\t\t\t\tProgramDataBaseFileName=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << Endl;
            if (configs[i] == "Release")
            {
                out << "\t\t\t\tWarningLevel=\"3\"" << Endl;
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << Endl;
            }
            else
            {
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
                out << "\t\t\t\tDebugInformationFormat=\"" << dif[i] 
                    << "\"/>" << Endl;
            }
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCLinkerTool\"" << Endl;
            if (configs[i] == "Purify")
                out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << Endl;
            out << "\t\t\t\tAdditionalDependencies=\""
                << libs
                << " vtkCommon.lib";
            if (pluginComponent != 'I')
                out << " vtkFiltering.lib" << tpLibs;
            out << "\"" << Endl;
            out << "\t\t\t\tOutputFile=\"" << binBase << "\\" << configs[i]
                << "\\databases\\lib" << pluginComponent << name << pluginSuffix 
                << ".dll\"" << Endl;
            out << "\t\t\t\tLinkIncremental=\"" << li[i] << "\"" << Endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
            out << "\t\t\t\tAdditionalLibraryDirectories=\""
                << libBase << "\\ThirdParty;"
                << libBase << "\\" << configs[i];
            out << "\"" << Endl;
            if (configs[i] != "Release")
                out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << Endl;
            out << "\t\t\t\tProgramDatabaseFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pdb\"" << Endl;
            out << "\t\t\t\tImportLibrary=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\lib" 
                << pluginComponent << name << pluginSuffix << ".lib\"" << Endl;
            out << "\t\t\t\tTargetMachine=\"1\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << Endl;
            out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << Endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << Endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << Endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << Endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << Endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << Endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << Endl;
            out << "\t\t\t<Tool" << Endl;
            out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" 
                << Endl;
            out << "\t\t</Configuration>" << Endl;
        } // end for
        } // end new scope
        out << "\t</Configurations>" << Endl;
        out << "\t<References>" << Endl;
        out << "\t</References>" << Endl;
        out << "\t<Files>" << Endl;
        out << "\t\t<Filter" << Endl;
        out << "\t\t\tName=\"Source Files\"" << Endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << Endl;
    
        for(size_t i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << Endl;
            out << "\t\t\t\tRelativePath=\"";
#ifdef _WIN32
            if (withinDevDir)
            {
                out << pluginBase << "\\databases\\" << name << "\\" 
                    << srcFiles[i];
            }
            else
            {
                out << srcFiles[i];
            }
#else
            out << pluginBase << "\\databases\\" << name << "\\" 
                << srcFiles[i];
#endif
            out << "\">" << Endl;

            for (int j = 0; j < 3; ++j)
            {
                out << "\t\t\t\t<FileConfiguration" << Endl;
                out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << Endl;
                out << "\t\t\t\t\t<Tool" << Endl;
                out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << Endl;
                out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << Endl;
                out << "\t\t\t\t</FileConfiguration>" << Endl;
            }
            out << "\t\t\t</File>" << Endl;
        }
    
        out << "\t\t</Filter>" << Endl;
        out << "\t</Files>" << Endl;
        out << "\t<Globals>" << Endl;
        out << "\t</Globals>" << Endl;
        out << "</VisualStudioProject>" << Endl;
    }


    /***************************************************************************
     *********************** END DATABASE PROJECT CODING ***********************
     **************************************************************************/
#endif
