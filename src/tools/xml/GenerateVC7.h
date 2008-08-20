/*****************************************************************************
*
* Copyright (c) 2000 - 2008, Lawrence Livermore National Security, LLC
* Produced at the Lawrence Livermore National Laboratory
* LLNL-CODE-400142
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
// ****************************************************************************

    void WriteProject_TOP_LEVEL_Version7(ostream &out)
    {
        QString configs[] = {"Release", "Debug", "Purify"};
        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\"" << endl;
        out << "\tKeyword=\"MakeFileProj\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        for (int i = 0; i < 3; i++)
        {
            out << "\t\t<Configuration" << endl;
            out << "\t\t\tName=\"" << configs[i] << "|Win32\"" << endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[i] << "\"" << endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[i] 
                << "\"" << endl;
            out << "\t\t\tConfigurationType=\"10\"" << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] 
                << "\\" << name << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            out << "\t\t</Configuration>" << endl;
        }
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    void WriteProjectSolution_Version7(ostream &out, 
                                        const vector<QString> &projects)
    {
        QString solutionKey("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}");
        vector<QString> keys;
        QString configs[] = {"Release", "Debug" , "Purify"};

        size_t i;
        for(i = 0; i < projects.size(); ++i)
            keys.push_back(CreateKey());

        out << "Microsoft Visual Studio Solution File, Format Version 8.00" 
            << endl;
        out << "Project(\"" << solutionKey << "\") = \"" << projects[0] 
            << "\", \"" << projects[0] << ".vcproj\", \"" << keys[0] << "\"" 
            << endl;
        out << "\tProjectSection(ProjectDependencies) = postProject" << endl;
        for(i = 1; i < keys.size(); ++i)
            out << "\t\t" << keys[i] << " = " << keys[i] << endl;

        out << "\tEndProjectSection" << endl;
        out << "EndProject" << endl;

        for(i = 1; i < keys.size(); ++i)
        {
            out << "Project(\"" << solutionKey << "\") = \"" << projects[i] 
                << "\", \"" << projects[i] << ".vcproj\", \"" << keys[i] 
                << "\"" << endl;
            out << "\tProjectSection(ProjectDependencies) = postProject" 
                << endl;
            out << "\tEndProjectSection" << endl;
            out << "EndProject" << endl;
        }

        out << "Global" << endl;
        out << "\tGlobalSection(SolutionConfiguration) = preSolution" << endl;
        out << "\t\tRelease = Release" << endl;
        out << "\t\tDebug = Debug" << endl;
        out << "\t\tPurify = Purify" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ProjectConfiguration) = postSolution" << endl;
    
        for(i = 0; i < keys.size(); ++i)
        {
            for (int j = 0; j < 3; j++)
            {
                out << "\t\t" << keys[i] << "." << configs[j] 
                    << ".ActiveCfg = " << configs[j] << "|Win32" << endl;
                out << "\t\t" << keys[i] << "." << configs[j] 
                     << ".Build.0 = " << configs[j] << "|Win32" << endl;
            }
        }

        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ExtensibilityGlobals) = postSolution" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ExtensibilityAddIns) = postSolution" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "EndGlobal" << endl;
    }

    void WriteProjectHelper_Version7(ostream &out, const QString &pluginType, 
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

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        for (int j = 0; j < 3; ++j)
        {
            out << "\t\t<Configuration" << endl;
            out << "\t\t\tName=\"" << configs[j] << "|Win32\"" << endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\tConfigurationType=\"2\"" << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
            out << "\t\t\tCharacterSet=\"2\">" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\tOptimization=\"" << optims[j] << "\"" << endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
            out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
            out << "\t\t\t\tAdditionalIncludeDirectories=\""
                << pluginBase;
            if (withinDevDir)
                out << "\\" << pluginType << "\\" << name;
            out << ";" << visitIncludes;
            out << ";" << includeBase << "\\VisIt"
                << ";" << includeBase << "\\VTK"
                << ";" << tpIncludes
                << "\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\""
                << preproc  << ";" << debug[j]
                << ";GENERAL_PLUGIN_EXPORTS";
            if (exports != "")
                out << ";" << exports;
            if (pluginComponent == 'E' && hasEngineSpecificCode )
                out << ";ENGINE"; 
            out << "\"" << endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
            else 
                out << "\t\t\t\tBasicRuntimeChecks=\"" << brc[j] 
                    << "\"" << endl;
            out << "\t\t\t\tRuntimeLibrary=\"" << rtl[j] << "\"" << endl;
            out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
            out << "\t\t\t\tPrecompiledHeaderFile=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pch\"" << endl;
            out << "\t\t\t\tAssemblerListingLocation=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\t\tObjectFile=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\t\tProgramDataBaseFileName=\".\\" << configs[j] 
                << "\\" << name << pluginComponent << "\\\"" << endl;
            if (configs[j] == "Release")
            {
                out << "\t\t\t\tWarningLevel=\"3\"" << endl;
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
            }
            else
            {
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
                out << "\t\t\t\tDebugInformationFormat=\"" << dif[j] 
                    << "\"/>" << endl;
            }
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
            if (configs[j] == "Purify")
                out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << endl;
            out << "\t\t\t\tAdditionalDependencies=\"" << libs << tpLibs << "\"" << endl;
            out << "\t\t\t\tOutputFile=\"" << binBase << "\\" << configs[j] 
                << "\\" << pluginType << "\\lib" << pluginComponent << name 
                << pType << suffix << ".dll\"" << endl;
            out << "\t\t\t\tLinkIncremental=\"" << li[j] << "\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
            out << "\t\t\t\tAdditionalLibraryDirectories=\""
                << libBase << "\\ThirdParty;"
                << libBase << "\\" << configs[j]
                << ";" << tpLibDir;
            out << "\"" << endl;
            if (configs[j] != "Release")
                out << "\t\t\t\tGenerateDebugInformation=\"TRUE\""  << endl;
            out << "\t\t\t\tProgramDatabaseFile=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pdb\"" << endl;
            out << "\t\t\t\tImportLibrary=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\lib" << pluginComponent  
                << name << pType << suffix << ".lib\"" << endl;
            out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] 
                << "\"" << endl;
            out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] 
                << "\"" << endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" 
                << endl;
            out << "\t\t</Configuration>" << endl;
        }
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        for(size_t i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
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
            out << "\">" << endl;
            for (int j = 0; j < 3; j++)
            {
                out << "\t\t\t\t<FileConfiguration" << endl;
                out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << endl;
                out << "\t\t\t\t\t<Tool" << endl;
                out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
                out << "\t\t\t\t\t\tOptimization=\"" << optims[j] 
                    << "\"" << endl;
                out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" 
                    << endl;
                if (configs[j] == "Debug")
                    out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
                else if (configs[j] == "Purify")
                    out << "\t\t\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
                out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
                out << "\t\t\t\t</FileConfiguration>" << endl;
            }
            out << "\t\t\t</File>" << endl;
        }
        out << "\t\t</Filter>" << endl;

        if(pluginComponent == 'G' && hdrFiles.size() > 0)
        {
            out << "\t\t<Filter" << endl;
            out << "\t\t\tName=\"Header Files\"" << endl;
            out << "\t\t\tFilter=\"h;hxx\">" << endl;
    
            for(size_t i = 0; i < hdrFiles.size(); ++i)
            {
                out << "\t\t\t<File" << endl;
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
                out << "\">" << endl;
                for (int j = 0; j < 3; j++)
                {
                    out << "\t\t\t\t<FileConfiguration" << endl;
                    out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << endl;
                    out << "\t\t\t\t\t<Tool" << endl;
                    out << "\t\t\t\t\t\tName=\"VCCustomBuildTool\"" << endl;
                    out << "\t\t\t\t\t\tDescription=\"Moc&apos;ing " 
                        << hdrFiles[i] << " ...\"" << endl;
                    out << "\t\t\t\t\t\tCommandLine=\"$(QTDIR)\\bin\\moc.exe "
                        << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << hdrFiles[i] << " -o " << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << mocFiles[i] << "\"" << endl;
                    out << "\t\t\t\t\t\tAdditionalDependencies="
                        << "\"$(QTDIR)\\bin\\moc.exe\"" << endl;
                    out << "\t\t\t\t\t\tOutputs=\"" << pluginBase;
                    if (withinDevDir)
                        out << "\\" << pluginType << "\\" << name;
                    out << "\\" << mocFiles[i] << "\"/>" << endl;
                    out << "\t\t\t\t</FileConfiguration>" << endl;
                }
                out << "\t\t\t</File>" << endl;
            }
            out << "\t\t</Filter>" << endl;
            out << "\t\t<Filter" << endl;
            out << "\t\t\tName=\"Generated MOC Files\"" << endl;
            out << "\t\t\tFilter=\"\">" << endl;
    
            for(size_t i = 0; i < mocFiles.size(); ++i)
            {
                out << "\t\t\t<File" << endl;
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
                out << "\">" << endl;
                for (int j = 0; j < 3; j++)
                {
                    out << "\t\t\t\t<FileConfiguration" << endl;
                    out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << endl;
                    out << "\t\t\t\t\t<Tool" << endl;
                    out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
                    out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
                    out << "\t\t\t\t</FileConfiguration>" << endl;
                }
                out << "\t\t\t</File>" << endl;
            }
            out << "\t\t</Filter>" << endl;
        }
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }


    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject_Version7(ostream &out, char pluginComponent,
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

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        {
        QString debug[] = {"NDEBUG", "_DEBUG", "_DEBUG"};
        int rtl[] = {2, 3, 3};
        int li[] = {1, 2, 1};
        int dif[] = {0, 4, 3};
        for (int i = 0; i < 3; ++i)
        {
            out << "\t\t<Configuration" << endl;
            out << "\t\t\tName=\""<< configs[i]<< "|Win32\"" << endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\tConfigurationType=\"2\"" << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
            out << "\t\t\tCharacterSet=\"2\">" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\tOptimization=\"" << optims[i] << "\"" << endl;
            if (configs[i] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
            out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
            out << "\t\t\t\tAdditionalIncludeDirectories=\""
                << pluginBase;
            if (withinDevDir)
                out << "\\databases\\" << name;
            out << ";" << visitIncludes;
            out << ";" << includeBase << "\\VisIt"
                << ";" << includeBase << "\\VTK";
            if (pluginComponent != 'I')
                out << tpIncludes;
            out << "\"" << endl;
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
            out << "\"" << endl;
            if (configs[i] == "Release")
            {
                out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
            }
            else
            {
                out << "\t\t\t\tBasicRuntimeChecks=\"" << brc[i] 
                    << "\"" << endl;
            }
            out << "\t\t\t\tRuntimeLibrary=\"" << rtl[i] << "\"" << endl;
            out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
            out << "\t\t\t\tPrecompiledHeaderFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pch\"" << endl;
            out << "\t\t\t\tAssemblerListingLocation=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\t\tObjectFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << endl;
            out << "\t\t\t\tProgramDataBaseFileName=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\\"" << endl;
            if (configs[i] == "Release")
            {
                out << "\t\t\t\tWarningLevel=\"3\"" << endl;
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
            }
            else
            {
                out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
                out << "\t\t\t\tDebugInformationFormat=\"" << dif[i] 
                    << "\"/>" << endl;
            }
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
            if (configs[i] == "Purify")
                out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << endl;
            out << "\t\t\t\tAdditionalDependencies=\""
                << libs
                << " vtkCommon.lib";
            if (pluginComponent != 'I')
                out << " vtkFiltering.lib" << tpLibs;
            out << "\"" << endl;
            out << "\t\t\t\tOutputFile=\"" << binBase << "\\" << configs[i]
                << "\\databases\\lib" << pluginComponent << name << pluginSuffix 
                << ".dll\"" << endl;
            out << "\t\t\t\tLinkIncremental=\"" << li[i] << "\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
            out << "\t\t\t\tAdditionalLibraryDirectories=\""
                << libBase << "\\ThirdParty;"
                << libBase << "\\" << configs[i];
            out << "\"" << endl;
            if (configs[i] != "Release")
                out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
            out << "\t\t\t\tProgramDatabaseFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pdb\"" << endl;
            out << "\t\t\t\tImportLibrary=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\lib" 
                << pluginComponent << name << pluginSuffix << ".lib\"" << endl;
            out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << endl;
            out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCManagedWrapperGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCAuxiliaryManagedWrapperGeneratorTool\"/>" 
                << endl;
            out << "\t\t</Configuration>" << endl;
        } // end for
        } // end new scope
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        for(size_t i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
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
            out << "\">" << endl;

            for (int j = 0; j < 3; ++j)
            {
                out << "\t\t\t\t<FileConfiguration" << endl;
                out << "\t\t\t\t\tName=\"" << configs[j] << "|Win32\">" << endl;
                out << "\t\t\t\t\t<Tool" << endl;
                out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
                out << "\t\t\t\t\t\tOptimization=\"" << optims[j] 
                    << "\"" << endl;
                out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" 
                    << endl;
                if (configs[j] != "Release")
                    out << "\t\t\t\t\t\tBasicRuntimeChecks=\"" << brc[j] 
                        << "\"" << endl;
                out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
                out << "\t\t\t\t</FileConfiguration>" << endl;
            }
            out << "\t\t\t</File>" << endl;
        }
    
        out << "\t\t</Filter>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }


    /***************************************************************************
     *********************** END DATABASE PROJECT CODING ***********************
     **************************************************************************/
#endif
