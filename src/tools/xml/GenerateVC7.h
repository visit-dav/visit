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

    void WriteProject_TOP_LEVEL_Version7(ostream &out)
    {
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
        // Release configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\"" << endl;
        out << "\t\t\tConfigurationType=\"10\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" << name << ".tlb\"" 
            << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        // Debug configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tConfigurationType=\"10\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << ".tlb\"" 
            << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        // Purify configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Purify|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\"" << endl;
        out << "\t\t\tConfigurationType=\"10\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" << name << ".tlb\"" 
            << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t</Configuration>" << endl;
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    QString CreateKey() const
    {
        const char *digits = "0123456789ABCDEF";
        QString s("------------------------------------");
        for(int i = 0; i < (int)s.length(); ++i)
        {
            if(i == 8 || i == 13 || i == 18 || i == 23)
                continue;
            else
            {
                int v = rand() % 16;
                s[i] = digits[v];
            }
        }

        return QString("{") + s + QString("}");
    }

    void WriteProjectWorkspace_Version7(ostream &out, 
                                        const vector<QString> &projects)
    {
        QString solutionKey("{8BC9CEB8-8B4A-11D0-8D11-00A0C91BC942}");
        vector<QString> keys;

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
        out << "\t\tDebug = Debug" << endl;
        out << "\t\tRelease = Release" << endl;
        out << "\tEndGlobalSection" << endl;
        out << "\tGlobalSection(ProjectConfiguration) = postSolution" << endl;
    
        for(i = 0; i < keys.size(); ++i)
        {
            out << "\t\t" << keys[i] << ".Release.ActiveCfg = Release|Win32" 
                << endl;
            out << "\t\t" << keys[i] << ".Release.Build.0 = Release|Win32" 
                << endl;
            out << "\t\t" << keys[i] << ".Debug.ActiveCfg = Debug|Win32" 
                << endl;
            out << "\t\t" << keys[i] << ".Debug.Build.0 = Debug|Win32" << endl;
            out << "\t\t" << keys[i] << ".Purify.ActiveCfg = Purify|Win32" 
                << endl;
            out << "\t\t" << keys[i] << ".Purify.Build.0 = Purify|Win32" 
                << endl;
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
             const vector<QString> &srcFiles)
    {
        const char *suffix = (pluginComponent == 'E') ? "_ser" : "";

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
        // Release Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << "$(QTDIR)\\include"
            << ";" << pluginBase;
        if (withinDevDir)
            out << "\\" << pluginType << "\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\Mesa"
            << ";" << includeBase << "\\Python-2.5"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc 
            << "NDEBUG;" 
            << "GENERAL_PLUGIN_EXPORTS";
        if (exports != "")
            out << ";" << exports;
        out << "\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name 
            << pluginComponent << "\\" << name << pluginComponent << ".pch\"" 
            << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"" << libs << "\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << pluginType << "\\lib" << pluginComponent << name << pType 
            << suffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << "$(QTDIR)\\lib;"
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".pdb\"" 
            << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;

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
            out << "\\Qvis" << name << winType << "_moc.C\"/>" << endl;
        }
        else
        {
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        }

        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
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
        //  Debug Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << "$(QTDIR)\\include"
            << ";" << pluginBase;
        if (withinDevDir)
            out << "\\" << pluginType << "\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\Mesa"
            << ";" << includeBase << "\\Python-2.5"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << "GENERAL_PLUGIN_EXPORTS";
        if (exports != "")
            out << ";" << exports;
        out << "\"" << endl;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name 
            << pluginComponent << "\\" << name << pluginComponent<< ".pch\"" 
            << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name 
            << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\"" << libs << "\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << pluginType << "\\lib" << pluginComponent << name << pType 
            << suffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"2\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << "$(QTDIR)\\lib;"
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;

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
           out << "\\Qvis" << name << winType << "_moc.C\"/>" << endl;
        }
        else
        {
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        }
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
        // Purify Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Purify|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" 
            << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" 
            << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << "$(QTDIR)\\include"
            << ";" << pluginBase;
        if (withinDevDir)
            out << "\\" << pluginType << "\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\Mesa"
            << ";" << includeBase << "\\Python-2.5"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << "GENERAL_PLUGIN_EXPORTS";
        if (exports != "")
            out << ";" << exports;
        out << "\"" << endl;
        out << "\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" 
            << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" 
            << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" 
            << name << pluginComponent << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"3\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\""<< endl;
        out << "\t\t\t\tAdditionalDependencies=\"" << libs << "\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << pluginType << "\\lib" << pluginComponent << name << pType 
            << suffix << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"0\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << "$(QTDIR)\\lib;"
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginComponent << "\\" 
            << name << pluginComponent << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;

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
           out << "\\Qvis" << name << winType << "_moc.C\"/>" << endl;
        }
        else
        {
            out << "\t\t\t<Tool" << endl;
            out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        }
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
                out << "..\\..\\visit\\" << pluginType 
                    << "\\" << name << "\\" << srcFiles[i];
            }
            else 
            {
                out << srcFiles[i];
            }
#else
            out << "..\\..\\visit\\" << pluginType 
                << "\\" << name << "\\" << srcFiles[i];
#endif
            out << "\">" << endl;
            // Release configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Debug configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Purify configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Purify|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
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

    void WriteDatabaseProject_EM_Helper_Version7(ostream &out, char pluginType,
             const vector<QString> &srcFiles, const QString &libs)
    {
        char *pluginDefs = "";
        char *pluginSuffix = "Database";

        pluginDefs = "GENERAL_PLUGIN_EXPORTS;MDSERVER_PLUGIN_EXPORTS;MDSERVER";
        if(pluginType == 'E')
        {
            pluginDefs = "GENERAL_PLUGIN_EXPORTS;ENGINE_PLUGIN_EXPORTS";
            pluginSuffix = "Database_ser";
        }

        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginType << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginType << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        // Release Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "NDEBUG;"
            << pluginDefs << "\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name 
            << pluginType << "\\" << name << pluginType << ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs 
            << " utility.lib vtkCommon.lib vtkFiltering.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name << pluginSuffix 
            << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
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
        // Debug Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << pluginDefs << "\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name 
            << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs
            << " utility.lib vtkCommon.lib vtkFiltering.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name << pluginSuffix 
            << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"2\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
        // Purify Configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Purify|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << pluginDefs << "\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name 
            << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"3\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs
            << " utility.lib vtkCommon.lib vtkFiltering.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name << pluginSuffix 
            << ".dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"0\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
                out << "..\\..\\visit\\databases\\" << name << "\\" 
                    << srcFiles[i];
            }
            else
            {
                out << srcFiles[i];
            }
#else
            out << "..\\..\\visit\\databases\\" << name << "\\" 
                << srcFiles[i];
#endif
            out << "\">" << endl;
            // Release Configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Debug Configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Purify Configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Purify|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            out << "\t\t\t</File>" << endl;
        }
    
        out << "\t\t</Filter>" << endl;
        out << "\t</Files>" << endl;
        out << "\t<Globals>" << endl;
        out << "\t</Globals>" << endl;
        out << "</VisualStudioProject>" << endl;
    }

    void WriteDatabaseProject_I_Version7(ostream &out, const QString &libs)
    {
        const char pluginType = 'I';
        out << "<?xml version=\"1.0\" encoding=\"Windows-1252\"?>" << endl;
        out << "<VisualStudioProject" << endl;
        out << "\tProjectType=\"Visual C++\"" << endl;
        out << "\tVersion=\"7.10\"" << endl;
        out << "\tName=\"" << name << pluginType << "\"" << endl;
        out << "\tRootNamespace=\"" << name << pluginType << "\"" << endl;
        out << "\tSccProjectName=\"\"" << endl;
        out << "\tSccLocalPath=\"\">" << endl;
        out << "\t<Platforms>" << endl;
        out << "\t\t<Platform" << endl;
        out << "\t\t\tName=\"Win32\"/>" << endl;
        out << "\t</Platforms>" << endl;
        out << "\t<Configurations>" << endl;
        // Release configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Release|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"2\"" << endl;
        out << "\t\t\t\tInlineFunctionExpansion=\"1\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "NDEBUG;"
            << "GENERAL_PLUGIN_EXPORTS\"" << endl;
        out << "\t\t\t\tStringPooling=\"TRUE\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"2\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Release\\" << name 
            << pluginType << "\\" << name << pluginType << ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Release\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tWarningLevel=\"3\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs 
            << " vtkCommon.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name 
            << "Database.dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"1\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Release\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"NDEBUG\"" << endl;
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
        // Debug configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Debug|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\VTK"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << "GENERAL_PLUGIN_EXPORTS\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name 
            << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"4\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs
            << " vtkCommon.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name 
            << "Database.dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"2\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
        // Purify configuration
        out << "\t\t<Configuration" << endl;
        out << "\t\t\tName=\"Purify|Win32\"" << endl;
        out << "\t\t\tOutputDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tIntermediateDirectory=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\tConfigurationType=\"2\"" << endl;
        out << "\t\t\tUseOfMFC=\"0\"" << endl;
        out << "\t\t\tATLMinimizesCRunTimeLibraryUsage=\"FALSE\"" << endl;
        out << "\t\t\tCharacterSet=\"2\">" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
        out << "\t\t\t\tOptimization=\"0\"" << endl;
        out << "\t\t\t\tOptimizeForProcessor=\"2\"" << endl;
        out << "\t\t\t\tAdditionalIncludeDirectories=\""
            << pluginBase;
        if (withinDevDir)
            out << "\\databases\\" << name;
        out << ";" << includeBase << "\\VisIt"
            << ";" << includeBase << "\\vtk"
            << ";" << includeBase << "\\silo"
            << ";" << includeBase << "\\zlib"
            << "\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\""
            << preproc
            << "_DEBUG;"
            << "GENERAL_PLUGIN_EXPORTS\"" << endl;;
        out << "\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
        out << "\t\t\t\tRuntimeLibrary=\"3\"" << endl;
        out << "\t\t\t\tEnableFunctionLevelLinking=\"TRUE\"" << endl;
        out << "\t\t\t\tEnableEnhancedInstructionSet=\"1\"" << endl;
        out << "\t\t\t\tUsePrecompiledHeader=\"2\"" << endl;
        out << "\t\t\t\tPrecompiledHeaderFile=\".\\Debug\\" << name 
            << pluginType << "\\" << name << pluginType<< ".pch\"" << endl;
        out << "\t\t\t\tAssemblerListingLocation=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tObjectFile=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tProgramDataBaseFileName=\".\\Debug\\" << name 
            << pluginType << "\\\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tDebugInformationFormat=\"3\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCCustomBuildTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCLinkerTool\"" << endl;
        out << "\t\t\t\tAdditionalOptions=\"/FIXED:NO\"" << endl;
        out << "\t\t\t\tAdditionalDependencies=\""
            << libs
            << " vtkCommon.lib\"" << endl;
        out << "\t\t\t\tOutputFile=\"" << binBase << "\\";
        out << "databases\\lib" << pluginType << name 
            << "Database.dll\"" << endl;
        out << "\t\t\t\tLinkIncremental=\"0\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tAdditionalLibraryDirectories=\""
            << libBase;
        out << "\"" << endl;
        out << "\t\t\t\tGenerateDebugInformation=\"TRUE\"" << endl;
        out << "\t\t\t\tProgramDatabaseFile=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".pdb\"" << endl;
        out << "\t\t\t\tImportLibrary=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".lib\"" << endl;
        out << "\t\t\t\tTargetMachine=\"1\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCMIDLTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
        out << "\t\t\t\tMkTypLibCompatible=\"TRUE\"" << endl;
        out << "\t\t\t\tSuppressStartupBanner=\"TRUE\"" << endl;
        out << "\t\t\t\tTargetEnvironment=\"1\"" << endl;
        out << "\t\t\t\tTypeLibraryName=\".\\Debug\\" 
            << name << pluginType << "\\" 
            << name << pluginType << ".tlb\"" << endl;
        out << "\t\t\t\tHeaderFileName=\"\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPostBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreBuildEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCPreLinkEventTool\"/>" << endl;
        out << "\t\t\t<Tool" << endl;
        out << "\t\t\t\tName=\"VCResourceCompilerTool\"" << endl;
        out << "\t\t\t\tPreprocessorDefinitions=\"_DEBUG\"" << endl;
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
        out << "\t</Configurations>" << endl;
        out << "\t<References>" << endl;
        out << "\t</References>" << endl;
        out << "\t<Files>" << endl;
        out << "\t\t<Filter" << endl;
        out << "\t\t\tName=\"Source Files\"" << endl;
        out << "\t\t\tFilter=\"cpp;c;cxx;rc;def;r;odl;idl;hpj;bat\">" << endl;
    
        vector<QString> srcFiles;
        srcFiles.push_back(name + "PluginInfo.C");

        for(size_t i = 0; i < srcFiles.size(); ++i)
        {
            out << "\t\t\t<File" << endl;
            out << "\t\t\t\tRelativePath=\"";
#ifdef _WIN32
            if (withinDevDir)
            {
                out << "..\\..\\visit\\databases\\" << name << "\\" 
                    << srcFiles[i];
            }
            else 
            {
                out << srcFiles[i];
            }
#else
            out << "..\\..\\visit\\databases\\" << name << "\\" 
                << srcFiles[i];
#endif
            out << "\">" << endl;
            // Release configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Release|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"2\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Debug configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Debug|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"3\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
            // Purify configuration
            out << "\t\t\t\t<FileConfiguration" << endl;
            out << "\t\t\t\t\tName=\"Purify|Win32\">" << endl;
            out << "\t\t\t\t\t<Tool" << endl;
            out << "\t\t\t\t\t\tName=\"VCCLCompilerTool\"" << endl;
            out << "\t\t\t\t\t\tOptimization=\"0\"" << endl;
            out << "\t\t\t\t\t\tPreprocessorDefinitions=\"$(Inherit)\"" << endl;
            out << "\t\t\t\t\t\tBasicRuntimeChecks=\"0\"" << endl;
            out << "\t\t\t\t\t\tCompileAs=\"2\"/>" << endl;
            out << "\t\t\t\t</FileConfiguration>" << endl;
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
