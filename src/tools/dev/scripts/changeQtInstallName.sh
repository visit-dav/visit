QtBaseDir=$1

QtFrameworks="QtCore QtGui QtNetwork QtOpenGL QtScript QtSvg QtXml"
for QtFW in $QtFrameworks; do
    echo $QtFW;
    filename="$QtBaseDir/lib/${QtFW}.framework/${QtFW}"
    debugFilename="${filename}_debug"
    oldSOName=$(otool -D $filename | tail -n -1)
    oldDebugSOName=$(otool -D $debugFilename | tail -n -1)
    internalFWPath=$(echo $oldSOName | sed -e 's/.*\.framework\///')
    internalDebugFWPath=$(echo $oldDebugSOName | sed -e 's/.*\.framework\///')
    newSOName="@executable_path/../lib/${QtFW}.framework/$internalFWPath"
    newDebugSOName="@executable_path/../lib/${QtFW}.framework/$internalDebugFWPath"
    install_name_tool -id $newSOName $filename
    install_name_tool -id $newDebugSOName $debugFilename
    for otherQtFW in $QtFrameworks; do
        install_name_tool -change $oldSOName $newSOName $QtBaseDir/lib/${otherQtFW}.framework/${otherQtFW}
        install_name_tool -change $oldSOName $newSOName $QtBaseDir/lib/${otherQtFW}.framework/${otherQtFW}_debug
    done
done
