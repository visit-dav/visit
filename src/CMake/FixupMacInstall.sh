#!/bin/sh
VISIT_INSTALLED_VERSION_BIN=$1

# Wait until the binary dir exists.
function wait_for_file
{
    while true; do
        if [[ ! -e $1 ]]; then
            sleep 2
        else
            break
        fi
    done
    return 0
}


# Add some symlinks in the bundles
echo "Adding symlinks to application bundles in ${VISIT_INSTALLED_VERSION_BIN}"
pushd .
for app in gui viewer mcurvit xmledit; do
    wait_for_file ${VISIT_INSTALLED_VERSION_BIN}/${app}.app/Contents/MacOS/${app}
    cd ${VISIT_INSTALLED_VERSION_BIN}
    ls -la
    ln -s ${app}.app/Contents/MacOS/${app} .
    cd ${app}.app/Contents
    ln -s ../../../lib .
    cd MacOS 
    ln -s ../../../../../../bin/visit .
done
popd

##
## Switch the paths to be relative
##

# Switch Qt to be relative
#QtFrameworks="QtCore QtGui QtNetwork QtOpenGL QtScript QtSvg QtXml"
#QtPlots=$(ls $VISIT_INSTALLED_VERSION_PLUGINS/plot/libG* $VISIT_INSTALLED_VERSION_PLUGINS/plot/libV*)
#QtOperators=$(ls $VISIT_INSTALLED_VERSION_PLUGINS/operators/libG* $VISIT_INSTALLED_VERSION_PLUGINS/operators/libV*)
#for QtFW in $QtFrameworks; do
#    filename="$VISIT_INSTALLED_VERSION_LIB/${QtFW}.framework/${QtFW}"
#    debugFilename="${filename}_debug"
#    oldSOName=$(otool -D $filename | tail -n -1)
#    oldDebugSOName=$(otool -D $debugFilename | tail -n -1)
#    internalFWPath=$(echo $oldSOName | sed -e 's/.*\.framework\///')
#    internalDebugFWPath=$(echo $oldDebugSOName | sed -e 's/.*\.framework\///')
#    newSOName="@executable_path/../lib/${QtFW}.framework/$internalFWPath"
#    newDebugSOName="@executable_path/../lib/${QtFW}.framework/$internalDebugFWPath"
#    install_name_tool -id $newSOName $filename
#    install_name_tool -id $newDebugSOName $debugFilename
#    # Switch the Qt libraries to be relative
#    for otherQtFW in $QtFrameworks; do
#        install_name_tool -change $oldSOName $newSOName $VISIT_INSTALLED_VERSION_LIB/${otherQtFW}.framework/${otherQtFW}
#        install_name_tool -change $oldSOName $newSOName $VISIT_INSTALLED_VERSION_LIB/${otherQtFW}.framework/${otherQtFW}_debug
#    done#
#
#    # Switch the libraries/exes that use Qt to be relative
#    for lib in "gui winutil vtkqt viewer"; do
#        install_name_tool -change $oldSOName $newSOName $lib
#    done
#    for lib in $QtPlots; do
#        install_name_tool -change $oldSOName $newSOName $lib
#    done
#    for lib in $QtOperators; do
#        install_name_tool -change $oldSOName $newSOName $lib
#    done
#done

