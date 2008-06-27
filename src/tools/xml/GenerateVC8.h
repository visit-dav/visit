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

#ifndef GENERATE_VC8_H
#define GENERATE_VC8_H

// ****************************************************************************
//  File:  Generate_VC8
//
//  Purpose:
//    VC8 specific code.
//
//  Modifications:
//  
// ****************************************************************************
    void WriteProject_TOP_LEVEL_Version8(ostream &out)
    {
        QString configs[] = {"Release", "Debug", "Purify"};

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"8.00\"" << endl;
        out << "\tName=\"" << name << "\"" << endl;
        out << "\tProjectGUID=\"\"" << endl;
        out << "\tKeyword=\"MakeFileProj\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<ToolFiles>" << endl;
        out << "\t</ToolFiles>" << endl;
        out << "\t<Configurations>" << endl;
        for (int i = 0; i < 3; i++)
        {
            out << "\t\t<Configuration" << endl;
            out << "\t\t\tName=\"" << configs[i] << "|Win32\"" << endl;
            out << "\t\t\tOutputDirectory=\".\\" << configs[i] << "\"" << endl;
            out << "\t\t\tIntermediateDirectory=\".\\" << configs[i] 
                << "\"" << endl;
            out << "\t\t\tConfigurationType=\"10\"" << endl;
            out << "\t\t\tInheritedPropertySheets=\"..\\NO_SEC_DEP.vsprops\"" << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"false\">" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] 
                << "\\" << name << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
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


    void WriteProjectSolution_Version8(ostream &out, 
                                        const vector<QString> &projects)
    {
        QString solutionKey(CreateKey());
        vector<QString> keys;
        QString configs[] = {"Release", "Debug" , "Purify"};

        size_t i;
        for(i = 0; i < projects.size(); ++i)
            keys.push_back(CreateKey());

        out << "Microsoft Visual Studio Solution File, Format Version 9.00" 
            << endl;
        out << "# Visual Studio 2005" << endl;
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
        out << "\tGlobalSection(SolutionConfigurationPlatforms) = preSolution" 
            << endl;
        out << "\t\tRelease|Win32 = Release|Win32" << endl;
        out << "\t\tDebug|Win32 = Debug|Win32" << endl;
        out << "\t\tPurify|Win32 = Purify|Win32" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ProjectConfigurationPlatforms) = postSolution" 
            << endl;
    
        for(i = 0; i < keys.size(); ++i)
        {
            for (int j = 0; j < 3; j++)
            {
                out << "\t\t" << keys[i] << "." << configs[j] << "|Win32"
                    << ".ActiveCfg = " << configs[j] << "|Win32" << endl;
                out << "\t\t" << keys[i] << "." << configs[j] << "|Win32"
                     << ".Build.0 = " << configs[j] << "|Win32" << endl;
            }
        }
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(SolutionProperties) = preSolution" << endl;
        out << "\t\tHideSolutionNode= FALSE" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "EndGlobal" << endl;
    }

    void WriteProjectHelper_Version8(ostream &out, const QString &pluginType, 
             char pluginComponent, const QString &exports, const QString &libs,
             const vector<QString> &srcFiles)
    {
        const char *suffix = (pluginComponent == 'E') ? "_ser" : "";

        QString configs[] = {"Release", "Debug", "Purify"};
        QString debug[] = {"NDEBUG", "_DEBUG", "_DEBUG"};
        int optims[] = {2, 0, 0};
        int rtl[] = {2, 3, 3};
        int brc[] = {0, 3, 0};
        int dif[] = {0, 4, 3};
        int li[] = {1, 2, 0};
        QString pType("Operator");
        if(pluginType == "plots")
            pType = "Plot";

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"8.00\"" << endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << endl;
        out << "\t>" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<ToolFiles>" << endl;
        out << "\t</ToolFiles>" << endl;
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
            out << "\t\t\tInheritedPropertySheets=\"..\\NO_SEC_DEP.vsprops\"" << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"false\"" << endl;
            out << "\t\t\tCharacterSet=\"2\">" << endl;

            if(pluginComponent == 'G')
            {
                QString winType("PlotWindow");
                if(pluginType == "operators")
                    winType = "Window";
                out << "\t\t\t<Tool" << endl;
                out << "\t\t\t\tName=\"VCPreBuildEventTool\"" << endl;
                out << "\t\t\t\tCommandLine=\""
                    << "$(QTDIR)\\bin\\moc.exe "
                    << pluginBase;
                if (withinDevDir)
                    out << "\\" << pluginType << "\\" << name;
                out << "\\Qvis" << name << winType << ".h "
                    << "-o "
                    << pluginBase;
                if (withinDevDir)
                    out << "\\" << pluginType << "\\" << name;
                out << "\\Qvis" << name << winType << "_moc.C";
                if (customwfiles)
                {
                    for (size_t i = 0; i < wfiles.size(); ++i)
                    {
                        if (wfiles[i].right(2) == ".h")
                        {
                            out << "\n$(QTDIR)\\bin\\moc.exe "
                                << pluginBase;
                            if (withinDevDir)
                                out << "\\" << pluginType << "\\" << name;
                            out << "\\" << wfiles[i] << " -o " << pluginBase;
                            if (withinDevDir)
                                out << "\\" << pluginType << "\\" << name;
                            out << "\\" << wfiles[i].left(wfiles[i].length()-2)
                                << "_moc.C";
                        }
                    }
                }
                out << "\"/>" << endl;
            }
            else
            {
                out << "\t\t\t<Tool" << endl;
                out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            }


            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"" << endl;
            out << "\t\t\t/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"" << endl;
            out << "\t\t\t/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"" << endl;
            out << "\t\t\t/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] 
                << "\"" << endl;
            out << "\t\t\t\tMkTypLibCompatible=\"true\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"true\"" << endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[j] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\tOptimization=\"" << optims[j] << "\"" << endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
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
            out << "\"" << endl;
            if (configs[j] == "Release")
                out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
            else 
                out << "\t\t\t\tBasicRuntimeChecks=\"" << brc[j] 
                    << "\"" << endl;
            out << "\t\t\t\tRuntimeLibrary=\"" << rtl[j] << "\"" << endl;
            out << "\t\t\t\tEnableFunctionLevelLinking=\"true\"" << endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"0\"" << endl;
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
            out << "\t\t\t\tName=\"VCManagedResourceCompilerTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[j] << "\"" 
                << endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
            if (configs[j] == "Purify")
                out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << endl;
            out << "\t\t\t\tAdditionalDependencies=\"" << libs << tpLibs 
                << "\"" << endl;
            out << "\t\t\t\tOutputFile=\"" << binBase << "\\" << configs[j] 
                << "\\" << pluginType << "\\lib" << pluginComponent << name 
                << pType << suffix << ".dll\"" << endl;
            out << "\t\t\t\tLinkIncremental=\"" << li[j] << "\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"true\"" << endl;
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
            out << "\t\t\t\tName=\"VCALinkTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCManifestTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXDCMakeTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCBscMakeTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCFxCopTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCAppVerifierTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
            out << "\t\t</Configuration>" << endl;
        }
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"C;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
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
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }


    /***************************************************************************
     ********************** BEGIN DATABASE PROJECT CODING **********************
     **************************************************************************/

    void WriteDatabaseProject_Version8(ostream &out, char pluginComponent,
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
            pluginDefs += ";MDSERVER_PLUGIN_EXPORTS;MDSERVER";
        }
        else if(pluginComponent == 'E')
        {
            pluginDefs += ";ENGINE_PLUGIN_EXPORTS";
            pluginSuffix = "Database_ser";
        }

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"8.00\"" << endl;
        out << "\tName=\"" << name << pluginComponent << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginComponent << "\"" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<ToolFiles>" << endl;
        out << "\t</ToolFiles>" << endl;
        out << "\t<Configurations>" << endl;
        {
        QString debug[] = {"NDEBUG", "_DEBUG", "_DEBUG"};
        int rtl[] = {2, 3, 3};
        int li[] = {1, 2, 0};
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
            out << "\t\t\tInheritedProperySheets=\"..\\NO_SEC_DEP.vsprops\"" 
                << endl;
            out << "\t\t\tUseOfMFC=\"0\"" << endl;
            out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"false\"" << endl;
            out << "\t\t\tCharacterSet=\"2\">" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXMLDataGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebServiceProxyGeneratorTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << endl;
            out << "\t\t\t\tMkTypLibCompatible=\"true\"" << endl;
            out << "\t\t\t\tSuppressStartupBanner=\"true\"" << endl;
            out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
            out << "\t\t\t\tTypeLibraryName=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\" 
                << name << pluginComponent << ".tlb\"" << endl;
            out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\tOptimization=\"" << optims[i] << "\"" << endl;
            if (configs[i] == "Release")
                out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
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
            out << "\t\t\t\tEnableFunctionLevelLinking=\"true\"" << endl;
            out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
            out << "\t\t\t\tUsePrecompiledHeader=\"0\"" << endl;
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
            out << "\t\t\t\tName=\"VCManagedResourceCompilerTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
            out << "\t\t\t\tPreprocessorDefinitions=\"" << debug[i] 
                << "\"" << endl;
            out << "\t\t\t\tCulture=\"1033\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
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
            out << "\t\t\t\tSuppressStartupBanner=\"true\"" << endl;
            out << "\t\t\t\tAdditionalLibraryDirectories=\""
                << libBase << "\\ThirdParty;"
                << libBase << "\\" << configs[i];
            out << "\"" << endl;
            if (configs[i] != "Release")
                out << "\t\t\t\tGenerateDebugInformation=\"true\"" << endl;
            out << "\t\t\t\tProgramDatabaseFile=\".\\" << configs[i] 
                << "\\" << name << pluginComponent << "\\" 
                << name << pluginComponent << ".pdb\"" << endl;
            out << "\t\t\t\tImportLibrary=\".\\" << configs[i] << "\\" 
                << name << pluginComponent << "\\lib" 
                << pluginComponent << name << pluginSuffix << ".lib\"" << endl;
            out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCALinkTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCManifestTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCXDCMakeTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCBscMakeTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCFxCopTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCAppVerifierTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCWebDeploymentTool\"/>" << endl;
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
            out << "\t\t</Configuration>" << endl;
        } // end for
        } // end new scope
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"C;cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
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
