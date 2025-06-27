function bv_vtk_initialize
{
    info "bv_vtk_initialize"
    export DO_VTK="yes"
}

function bv_vtk_enable
{
    info "bv_vtk_enable"
    DO_VTK="yes"

    if [[ "$DO_MESAGL" == "no" ]] ; then
        if [[ "$OPSYS" == "Linux" ]]; then
            bv_osmesa_enable
        fi
    fi
}

function bv_vtk_disable
{
    DO_VTK="no"
}


function bv_vtk_depends_on
{
    depends_on="cmake zlib"

    if [[ "$DO_PYTHON" == "yes" ]]; then
        depends_on="${depends_on} python"
    fi

    if [[ "$DO_MESAGL" == "yes" ]]; then
        depends_on="${depends_on} mesagl glu"
    elif [[ "$DO_OSMESA" == "yes" ]]; then
        depends_on="${depends_on} osmesa"
    fi

    if [[ "$DO_OSPRAY" == "yes" ]]; then
        depends_on="${depends_on} ospray"
    fi

    if [[ "$DO_ANARI" == "yes" ]]; then
        depends_on="${depends_on} anari"
    fi

    # Only depend on Qt if we're not doing server-only builds.
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        if [[ "$DO_ENGINE_ONLY" != "yes" ]]; then
            if [[ "$DO_SERVER_COMPONENTS_ONLY" != "yes" ]]; then
                depends_on="${depends_on} qt"
            fi
        fi
    fi

    echo ${depends_on}
}

function bv_vtk_info
{
    info "setting up vtk for version 9.5.0"
    export VTK_VERSION=${VTK_VERSION:-"9.5.0"}
    export VTK_SHORT_VERSION=${VTK_SHORT_VERSION:-"9.5"}
    export VTK_SHA256_CHECKSUM="04ae86246b9557c6b61afbc534a6df099244fbc8f3937f82e6bc0570953af87d"
    export VTK_URL=""
    export VTK_FILE=${VTK_FILE:-"VTK-${VTK_VERSION}.tar.gz"}
    export VTK_COMPATIBILITY_VERSION=${VTK_SHORT_VERSION}
    export VTK_BUILD_DIR=${VTK_BUILD_DIR:-"VTK-${VTK_VERSION}"}
    export VTK_INSTALL_DIR=${VTK_INSTALL_DIR:-"vtk"}
}

function bv_vtk_print
{
    printf "%s%s\n" "VTK_FILE=" "${VTK_FILE}"
    printf "%s%s\n" "VTK_VERSION=" "${VTK_VERSION}"
    printf "%s%s\n" "VTK_BUILD_DIR=" "${VTK_BUILD_DIR}"
}

function bv_vtk_print_usage
{
    printf "%-20s %s\n" "--vtk" "Build VTK"
}

function bv_vtk_host_profile
{
    echo >> $HOSTCONF
    echo "##" >> $HOSTCONF
    echo "## VTK" >> $HOSTCONF
    echo "##" >> $HOSTCONF

    echo "SETUP_APP_VERSION(VTK $VTK_VERSION)" >> $HOSTCONF
    echo "VISIT_OPTION_DEFAULT(VISIT_VTK_DIR \${VISITHOME}/${VTK_INSTALL_DIR}/\${VTK_VERSION}/\${VISITARCH})" >> $HOSTCONF
}

function bv_vtk_initialize_vars
{
    info "initalizing vtk vars"
}

function bv_vtk_ensure
{
    if [[ "$DO_VTK" == "yes" ]] ; then
        ensure_built_or_ready $VTK_INSTALL_DIR $VTK_VERSION $VTK_BUILD_DIR $VTK_FILE $VTK_URL
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi
}


function apply_vtk95_vtkRectilinearGridReader_patch
{
  # patch vtkRectilinearGridReader.cxx, per this issue:
  # https://gitlab.kitware.com/vtk/vtk/-/issues/18447
   patch -p0 << \EOF
--- IO/Legacy/vtkRectilinearGridReader.cxx.orig	2025-05-15 13:43:55.914685000 -0700
+++ IO/Legacy/vtkRectilinearGridReader.cxx	2025-05-15 13:45:09.376599000 -0700
@@ -83,8 +83,14 @@
       {
         break;
       }
+      // Have to read field data because it may be binary.
+      if (!strncmp(this->LowerCase(line), "field", 5))
+      {
+        vtkFieldData* fd = this->ReadFieldData();
+        fd->Delete();
+      }

-      if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
+      else if (!strncmp(this->LowerCase(line), "dimensions", 10) && !dimsRead)
       {
         int dim[3];
         if (!(this->Read(dim) && this->Read(dim + 1) && this->Read(dim + 2)))
@@ -116,6 +122,22 @@

         dimsRead = true;
       }
+
+      // if the coordinates have been reached, should be no reason
+      // to keep reading
+      else if (strncmp(this->LowerCase(line), "x_coordinate", 12) == 0)
+      {
+        break;
+      }
+      else if (strncmp(this->LowerCase(line), "y_coordinate", 12) == 0)
+      {
+        break;
+      }
+      else if (strncmp(this->LowerCase(line), "z_coordinate", 12) == 0)
+      {
+        break;
+      }
+
     }
   }

EOF

    if [[ $? != 0 ]] ; then
        warn "vtk patch for vtkRectilinearGridReader.cxx failed."
        return 1
    fi
}

function apply_vtk95_vtkdatawriter_patch
{
  # patch vtkDataWriter to fix a bug when writing a vtkBitArray
  # Make it use the same calculation as the reader.
   patch -p0 << \EOF
--- IO/Legacy/vtkDataWriter.cxx.orig	2025-05-15 13:43:55.914685000 -0700
+++ IO/Legacy/vtkDataWriter.cxx	2025-05-15 13:45:09.376599000 -0700
@@ -1121,7 +1121,7 @@
       else
       {
         unsigned char* cptr = static_cast<vtkBitArray*>(data)->GetPointer(0);
-        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num - 1) / 8 + 1));
+        fp->write(reinterpret_cast<char*>(cptr), (sizeof(unsigned char)) * ((num*numComp+7)/8));
       }
       *fp << "\n";
     }
EOF

    if [[ $? != 0 ]] ; then
      warn "vtk patch for vtkDataWriter.cxx failed."
      return 1
    fi
    return 0;

}

function apply_vtk95_vtkospray_patches
{
    count_patches=3
    # patch vtkOSPRay files:

    # 1) expose vtkViewNodeFactory via vtkOSPRayPass.h
    current_patch=1
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayPass.h.orig	2025-05-15 13:50:01.170625000 -0700
+++ Rendering/RayTracing/vtkOSPRayPass.h	2025-05-15 13:52:33.330669000 -0700
@@ -39,6 +39,8 @@
 class vtkRenderPassCollection;
 class vtkSequencePass;
 class vtkVolumetricPass;
+// Used by VisIt
+class vtkViewNodeFactory;

 class VTKRENDERINGRAYTRACING_EXPORT vtkOSPRayPass : public vtkRenderPass
 {
@@ -65,6 +67,11 @@
    */
   virtual void RenderInternal(const vtkRenderState* s);

+  /**
+   * Called by VisIt
+   */
+  virtual vtkViewNodeFactory* GetViewNodeFactory();
+
   ///@{
   /**
    * Wrapper around ospray's init and shutdown that protect
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch ${current_patch}/${count_patches} for vtkOSPRayPass.h failed."
        return 1
    fi

    # 2) expose vtkViewNodeFactory via vtkOSPRayPass.cxx
    ((current_patch++))
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayPass.cxx.orig	2025-05-15 13:55:03.700624000 -0700
+++ Rendering/RayTracing/vtkOSPRayPass.cxx	2025-05-15 13:55:50.006697000 -0700
@@ -417,6 +417,12 @@
 }

 //------------------------------------------------------------------------------
+vtkViewNodeFactory* vtkOSPRayPass::GetViewNodeFactory()
+{
+  return this->Internal->Factory;
+}
+
+//------------------------------------------------------------------------------
 bool vtkOSPRayPass::IsSupported()
 {
   static bool detected = false;
EOF

    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch ${current_patch}/${count_patches} for vtkOSPRayPass.cxx failed."
        return 1
    fi

    # 3) Set the samples in the VolumeMapper
    ((current_patch++))
    patch -p0 << \EOF
--- Rendering/RayTracing/vtkOSPRayVolumeMapper.cxx.orig	2025-05-15 13:57:16.832663000 -0700
+++ Rendering/RayTracing/vtkOSPRayVolumeMapper.cxx	2025-05-15 13:58:31.661593000 -0700
@@ -61,6 +61,10 @@
   {
     this->Init();
   }
+  vtkOSPRayRendererNode::SetSamplesPerPixel(
+    vtkOSPRayRendererNode::GetSamplesPerPixel(ren), this->InternalRenderer);
+  vtkOSPRayRendererNode::SetAmbientSamples(
+    vtkOSPRayRendererNode::GetAmbientSamples(ren), this->InternalRenderer);
   this->InternalRenderer->SetRenderWindow(ren->GetRenderWindow());
   this->InternalRenderer->SetActiveCamera(ren->GetActiveCamera());
   this->InternalRenderer->SetBackground(ren->GetBackground());
EOF
    if [[ $? != 0 ]] ; then
        warn "vtk 9.5 patch $current_patch/$count_patches for vtkOSPRayVolumeMapper.cxx failed."
        return 1
    fi
}

function apply_vtk_patch
{
    if [[ ${VTK_VERSION} == 9.5.0 ]] ; then
        apply_vtk95_vtkospray_patches
        if [[ $? != 0 ]] ; then
            return 1
        fi

        apply_vtk95_vtkdatawriter_patch
        if [[ $? != 0 ]] ; then
           return 1
        fi

        # should submit a ticket to kitware
        apply_vtk95_vtkRectilinearGridReader_patch
        if [[ $? != 0 ]] ; then
            return 1
        fi
    fi

    return 0
}

function build_vtk
{
    # Extract the sources
    if [[ -d $VTK_BUILD_DIR ]] ; then
        if [[ ! -f $VTK_FILE ]] ; then
            warn "The directory VTK exists, deleting before uncompressing"
            rm -Rf $VTK_BUILD_DIR
            ensure_built_or_ready $VTK_INSTALL_DIR    $VTK_VERSION    $VTK_BUILD_DIR    $VTK_FILE
        fi
    fi

    #
    # Prepare the build dir using src file.
    #
    prepare_build_dir $VTK_BUILD_DIR $VTK_FILE
    untarred_vtk=$?
    # 0, already exists, 1 untarred src, 2 error

    if [[ $untarred_vtk == -1 ]] ; then
        warn "Unable to prepare VTK build directory. Giving Up!"
        return 1
    fi

    #
    # Apply patches
    #
    info "Patching VTK . . ."
    cd $VTK_BUILD_DIR || error "Can't cd to VTK build dir."
    apply_vtk_patch
    if [[ $? != 0 ]] ; then
        if [[ $untarred_vtk == 1 ]] ; then
            warn "Giving up on VTK build because the patch failed."
            return 1
        else
            warn "Patch failed, but continuing.  I believe that this script\n" \
                 "tried to apply a patch to an existing directory that had\n" \
                 "already been patched ... that is, the patch is\n" \
                 "failing harmlessly on a second application."
        fi
    fi

    # move back up to the start dir
    cd "$START_DIR"

    #
    # Configure VTK
    #
    info "Configuring VTK . . ."

    # Make a build directory for an out-of-source build. Change the
    # VTK_BUILD_DIR variable to represent the out-of-source build directory.
    VTK_SRC_DIR=$VTK_BUILD_DIR
    VTK_BUILD_DIR="${VTK_SRC_DIR}-build"
    if [[ ! -d $VTK_BUILD_DIR ]] ; then
        echo "Making build directory $VTK_BUILD_DIR"
        mkdir $VTK_BUILD_DIR
    fi

    #
    # Remove the CMakeCache.txt files ... existing files sometimes prevent
    # fields from getting overwritten properly.
    #
    rm -Rf ${VTK_BUILD_DIR}/CMakeCache.txt ${VTK_BUILD_DIR}/*/CMakeCache.txt

    #
    # Setup paths and libs for python for the VTK build.
    #
    if [[ "$OPSYS" == "Darwin" ]]; then
        if [[ "${VISIT_PYTHON_DIR}/lib" != "/usr/lib" ]]; then
            export DYLD_LIBRARY_PATH="${VISIT_PYTHON_DIR}/lib/:$DYLD_LIBRARY_PATH"
        fi
    else
        export LD_LIBRARY_PATH="${VISIT_PYTHON_DIR}/lib/:$LD_LIBRARY_PATH"
    fi

    export VTK_PY_LIBS="-lpthread"
    if [[ "$OPSYS" == "Linux" ]]; then
        export VTK_PY_LIBS="$VTK_PY_LIBS -ldl -lutil -lm"
    fi

    vopts=""
    vtk_build_mode="${VISIT_BUILD_MODE}"
    vtk_inst_path="${VISITDIR}/${VTK_INSTALL_DIR}/${VTK_VERSION}/${VISITARCH}"
    vtk_debug_leaks="false"

    # Some linker flags.
    lf=""
    if test "${OPSYS}" = "Darwin" ; then
        lf="-Wl,-headerpad_max_install_names"
        lf="${lf},-compatibility_version,${VTK_COMPATIBILITY_VERSION}"
        lf="${lf},-current_version,${VTK_VERSION}"
    fi
    # normal stuff
    vopts="${vopts} -DCMAKE_BUILD_TYPE:STRING=${vtk_build_mode}"
    vopts="${vopts} -DCMAKE_INSTALL_PREFIX:PATH=${vtk_inst_path}"
    if test "x${DO_STATIC_BUILD}" = "xyes" ; then
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=OFF"
    else
        vopts="${vopts} -DBUILD_SHARED_LIBS:BOOL=ON"
    fi
    vopts="${vopts} -DVTK_DEBUG_LEAKS:BOOL=${vtk_debug_leaks}"
    vopts="${vopts} -DVTK_LEGACY_REMOVE:BOOL=true"
    vopts="${vopts} -DCMAKE_C_COMPILER:STRING=${C_COMPILER}"
    vopts="${vopts} -DCMAKE_CXX_COMPILER:STRING=${CXX_COMPILER}"
    vopts="${vopts} -DCMAKE_C_FLAGS:STRING=\"${C_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_CXX_FLAGS:STRING=\"${CXX_OPT_FLAGS}\""
    vopts="${vopts} -DCMAKE_EXE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_MODULE_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DCMAKE_SHARED_LINKER_FLAGS:STRING=${lf}"
    vopts="${vopts} -DVTK_BUILD_TESTING:STRING=OFF"
    vopts="${vopts} -DVTK_BUILD_DOCUMENTATION:BOOL=OFF"
    vopts="${vopts} -DVTK_FORBID_DOWNLOADS:BOOL=ON"
    # setting this to true causes errors when building debug versions of
    # visit, so set it to false
    vopts="${vopts} -DVTK_REPORT_OPENGL_ERRORS:BOOL=false"

    if test "${OPSYS}" = "Darwin" ; then

        vopts="${vopts} -DVTK_USE_COCOA:BOOL=ON"
        vopts="${vopts} -DCMAKE_INSTALL_NAME_DIR:PATH=${vtk_inst_path}/lib"

        if test "${MACOSX_DEPLOYMENT_TARGET}" = "10.10"; then
            # If building on 10.10 (Yosemite) check if we are building with Xcode 7 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 7; then
                # Workaround for Xcode 7 not having a 10.10 SDK: Prevent CMake from linking to 10.11 SDK
                # by using Frameworks installed in root directory.
                echo "Xcode 7 on MacOS 10.10 detected: Enabling CMake workaround"
                vopts="${vopts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        elif test "${MACOSX_DEPLOYMENT_TARGET}" = "10.12"; then
            # If building on 10.12 (Sierra) check if we are building with Xcode 9 ...
            XCODE_VER=$(xcodebuild -version | head -n 1 | awk '{print $2}')
            if test ${XCODE_VER%.*} == 9; then
                # Workaround for Xcode 9 not having a 10.12 SDK: Prevent CMake from linking to 10.13 SDK
                # by using Frameworks installed in root directory.
                echo "Xcode 9 on MacOS 10.12 detected: Enabling CMake workaround"
                vopts="${vopts} -DCMAKE_OSX_DEPLOYMENT_TARGET:STRING=\"\" -DCMAKE_OSX_SYSROOT:STRING=/"
            fi
        fi
    fi

    # allow VisIt to override any of vtk's classes
    vopts="${vopts} -DVTK_ALL_NEW_OBJECT_FACTORY:BOOL=true"
    # disable downloads (also disables testing)
    vopts="${vopts} -DVTK_FORBID_DOWNLOADS:BOOL=true"

    # Turn off module groups
    vopts="${vopts} -DVTK_GROUP_ENABLE_Imaging:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_MPI:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Qt:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Rendering:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_StandAlone:STRING=DONT_WANT"
    # one of the vtk modules introduced this case for StandALone
    # Probably a mistake, but guard against it anyways as it shows up
    # in the Cache.
    vopts="${vopts} -DVTK_GROUP_ENABLE_STANDALONE:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Views:STRING=DONT_WANT"
    vopts="${vopts} -DVTK_GROUP_ENABLE_Web:STRING=DONT_WANT"

    # Turn on individual modules. dependent modules are turned on automatically
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_CommonCore:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersFlowPaths:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersHybrid:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersModeling:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersVerdict:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GeovisCore:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOEnSight:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOGeometry:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOLegacy:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOPLY:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_IOXML:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_InteractionStyle:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingAnnotation:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingFreeType:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingOpenGL2:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingVolumeOpenGL2:STRING=YES"
    vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_libxml2:STRING=YES"
    vopts="${vopts} -DVTK_ENABLE_REMOTE_MODULES:BOOL=OFF"

    # Tell VTK where to locate qmake if we're building graphical support. We
    # do not add graphical support for server-only builds.
    if [[ "$DO_QT" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_GUISupportQt:STRING=YES"
        vopts="${vopts} -DQt6_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6"
        vopts="${vopts} -DQt6CoreTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6CoreTools"
        vopts="${vopts} -DQt6GuiTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6GuiTools"
        vopts="${vopts} -DQt6WidgetsTools_DIR:PATH=${QT_INSTALL_DIR}/lib/cmake/Qt6WidgetsTools"
    fi

    # Add python wrapping
    if [[ "$DO_DBIO_ONLY" != "yes" ]]; then
        # python... but static libs and python filters are incompatible.
        if [[ "$DO_STATIC_BUILD" != "yes" ]]; then
            py="${PYTHON_COMMAND}"
            pyinc="${PYTHON_INCLUDE_DIR}"
            pylib="${PYTHON_LIBRARY}"

            vopts="${vopts} -DVTK_WRAP_PYTHON:BOOL=true"
            vopts="${vopts} -DPython3_EXECUTABLE:FILEPATH=${py}"
            vopts="${vopts} -DPython3_EXTRA_LIBS:STRING=\"${VTK_PY_LIBS}\""
            vopts="${vopts} -DPython3_INCLUDE_DIR:PATH=${pyinc}"
            vopts="${vopts} -DPython3_LIBRARY:FILEPATH=${pylib}"
        else
            warn "Forgetting python filters because we are doing a static build."
        fi
    fi

    # For now, turn off EGL (Our large-image regression tests fail)
    vopts="${vopts} -DOPENGL_EGL_INCLUDE_DIR:PATH=\"\""
    vopts="${vopts} -DOPENGL_egl_LIBRARY:FILEPATH=\"\""
    vopts="${vopts} -DVTK_OPENGL_HAS_EGL:BOOL=OFF"

    if [[ "$OPSYS" == "Linux" ]]; then
        vopts="${vopts} -DVTK_USE_X:BOOL=ON"
    fi

    # Use Mesa as GL?
    if [[ "$DO_MESAGL" == "yes" ]] ; then
        vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"
        vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING=${MESAGL_OPENGL_LIB}"
        vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
        vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH=${MESAGL_GLU_LIB}"
        # for now, until Mesa can be updated to a version that supports GLVND,
        # set LEGACY preference
        vopts="${vopts} -DOpenGL_GL_PREFERENCE:STRING=LEGACY"
        vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
        vopts="${vopts} -DOSMESA_LIBRARY:STRING=${MESAGL_OSMESA_LIB}"
        vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${MESAGL_INCLUDE_DIR}"

    #elif [[ "$DO_OSMESA" == "yes" ]] ; then
    #    # Is there a use-case where VTK-9.5 would need to build against only OSMesa?
    #    vopts="${vopts} -DOPENGL_INCLUDE_DIR:PATH="
    #    vopts="${vopts} -DOPENGL_gl_LIBRARY:STRING="
    #    vopts="${vopts} -DOPENGL_opengl_LIBRARY:STRING="
    #    vopts="${vopts} -DOPENGL_glu_LIBRARY:FILEPATH="
    #    vopts="${vopts} -DVTK_OPENGL_HAS_OSMESA:BOOL=ON"
    #    vopts="${vopts} -DOSMESA_LIBRARY:STRING=\"${OSMESA_LIB}\""
    #    vopts="${vopts} -DOSMESA_INCLUDE_DIR:PATH=${OSMESA_INCLUDE_DIR}"
    #    vopts="${vopts} -DVTK_USE_X:BOOL=OFF"
    fi

    # Use OSPRay?
    if [[ "$DO_OSPRAY" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=YES"
        if [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib/cmake/ospray-${OSPRAY_VERSION}"
        elif [[ -d ${OSPRAY_INSTALL_DIR}/ospray/lib64 ]] ; then
            vopts="${vopts} -Dospray_DIR=${OSPRAY_INSTALL_DIR}/ospray/lib64/cmake/ospray-${OSPRAY_VERSION}"
        else
            warn "Disabling ospray because its lib dir couldn't be found"
            vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingRayTracing:STRING=NO"
        fi
    fi

    # Use ANARI?
    if [[ "$DO_ANARI" == "yes" ]] ; then
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_RenderingAnari:STRING=YES"
        vopts="${vopts} -DVTK_MODULE_ENABLE_VTK_FiltersTexture:STRING=YES"
        if [[ -d ${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib64 ]] ; then
            vopts="${vopts} -Danari_DIR=${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib64/cmake/anari-${ANARI_VERSION}"
        else
            vopts="${vopts} -Danari_DIR=${VISITDIR}/anari/${ANARI_VERSION}/${VISITARCH}/lib/cmake/anari-${ANARI_VERSION}"
        fi

        if [[ "$DO_ANARI_NVTX" == "yes" ]] ; then
            vopts="${vopts} -DVTK_ANARI_ENABLE_NVTX:BOOL=ON"
        fi
    fi

    # zlib support, use the one we build
    vopts="${vopts} -DVTK_MODULE_USE_EXTERNAL_VTK_zlib:BOOL=ON"
    vopts="${vopts} -DZLIB_INCLUDE_DIR:PATH=${ZLIB_INCLUDE_DIR}"
    if [[ "$VISIT_BUILD_MODE" == "Release" ]] ; then
        vopts="${vopts} -DZLIB_LIBRARY_RELEASE:FILEPATH=${ZLIB_LIBRARY}"
    else
        vopts="${vopts} -DZLIB_LIBRARY_DEBUG:FILEPATH=${ZLIB_LIBRARY}"
    fi

    CMAKE_BIN="${CMAKE_INSTALL}/cmake"
    cd ${VTK_BUILD_DIR}

    if [[ "$DO_MESAGL" == "yes" || "$DO_OSMESA" == "yes"  ]] ; then
        export LD_LIBRARY_PATH="${LLVM_LIB_DIR}:$LD_LIBRARY_PATH"
    fi

    #
    # Several platforms have had problems with the VTK cmake configure command
    # issued simply via "issue_command".  This was first discovered on
    # BGQ and then showed up in random cases for both OSX and Linux machines.
    # Brad resolved this on BGQ  with a simple work around - we write a simple
    # script that we invoke with bash which calls cmake with all of the proper
    # arguments. We are now using this strategy for all platforms.
    #

    if test -e bv_run_cmake.sh ; then
        rm -f bv_run_cmake.sh
    fi
    echo "\"${CMAKE_BIN}\"" ${vopts} ../${VTK_SRC_DIR} > bv_run_cmake.sh
    cat bv_run_cmake.sh
    issue_command bash bv_run_cmake.sh || error "VTK configuration failed."

    #
    # Now build VTK.
    #
    info "Building VTK . . . (~20 minutes)"
    env DYLD_LIBRARY_PATH=`pwd`/bin $MAKE $MAKE_OPT_FLAGS || \
        error "VTK did not build correctly.  Giving up."

    info "Installing VTK . . . "
    $MAKE install || error "VTK did not install correctly."

    # Filter out an include that references the user's VTK build directory
    configdir="${vtk_inst_path}/lib/cmake/vtk-${VTK_SHORT_VERSION}"
    cat ${configdir}/VTKConfig.cmake | grep -v "vtkTestingMacros" > ${configdir}/VTKConfig.cmake.new
    mv ${configdir}/VTKConfig.cmake.new ${configdir}/VTKConfig.cmake

    chmod -R ug+w,a+rX ${VISITDIR}/${VTK_INSTALL_DIR}
    if [[ "$DO_GROUP" == "yes" ]] ; then
        chgrp -R ${GROUP} "$VISITDIR/${VTK_INSTALL_DIR}"
    fi
    cd "$START_DIR"
    info "Done with VTK"
    return 0
}

function bv_vtk_is_enabled
{
    if [[ $DO_VTK == "yes" ]]; then
        return 1
    fi
    return 0
}

function bv_vtk_is_installed
{
    check_if_installed "$VTK_INSTALL_DIR" $VTK_VERSION
    if [[ $? == 0 ]] ; then
        return 1
    fi
    return 0
}

function bv_vtk_build
{
    #
    # Build VTK
    #
    cd "$START_DIR"

    if [[ "$DO_VTK" == "yes" ]] ; then
        check_if_installed $VTK_INSTALL_DIR $VTK_VERSION
        if [[ $? == 0 ]] ; then
            info "Skipping VTK build.  VTK is already installed."
        else
            info "Building VTK (~20 minutes)"
            build_vtk
            if [[ $? != 0 ]] ; then
                error "Unable to build or install VTK.  Bailing out."
            fi
        fi
        info "Done building VTK"
    fi
}
