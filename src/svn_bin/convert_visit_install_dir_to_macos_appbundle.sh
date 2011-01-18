#!/bin/sh
visit_version=`readlink $1/current`
if [ $? != 0 ]; then
    echo "Cannot determine VisIt version number"
    exit
fi
mkdir $1/Contents
cat >$1/Contents/Info.plist <<EOF
<?xml version="1.0" encoding="UTF-8"?>
<!DOCTYPE plist PUBLIC "-//Apple//DTD PLIST 1.0//EN" "http://www.apple.com/DTDs/PropertyList-1.0.dtd">
<plist version="1.0">
<dict>
        <key>CFBundleDevelopmentRegion</key>
        <string>English</string>
        <key>CFBundleExecutable</key>
        <string>visit</string>
        <key>CFBundleGetInfoString</key>
        <string>VisIt Visualization Tool</string>
        <key>CFBundleIconFile</key>
        <string>VisItIcon.icns</string>
        <key>CFBundleIdentifier</key>
        <string>VISI</string>
        <key>CFBundleInfoDictionaryVersion</key>
        <string>6.0</string>
        <key>CFBundleLongVersionString</key>
EOF
echo "        <string>VisIt version $visit_version</string>" >> $1/Contents/Info.plist
cat >> $1/Contents/Info.plist  <<EOF
        <key>LSEnvironment</key>
        <dict>
                <key>TERM_PROGRAM</key>
                <string>Apple_Terminal</string>
                <key>VISIT_STARTED_FROM_APPBUNDLE</key>
                <string>TRUE</string>
        </dict>
        <key>CFBundleName</key>
EOF
echo "        <string>VisIt $visit_version</string>" >> $1/Contents/Info.plist
cat >>$1/Contents/Info.plist <<EOF
        <key>CFBundlePackageType</key>
        <string>APPL</string>
        <key>CFBundleShortVersionString</key>
EOF
echo "        <string>VisIt $visit_version</string>" >> $1/Contents/Info.plist
cat >>$1/Contents/Info.plist <<EOF
        <key>CFBundleSignature</key>
        <string>????</string>
        <key>CFBundleVersion</key>
EOF
echo "        <string>$visit_version</string>" >>$1/Contents/Info.plist
cat >>$1/Contents/Info.plist <<EOF
        <key>CSResourcesFileMapped</key>
        <true/>
        <key>LSRequiresCarbon</key>
        <true/>
        <key>NSHumanReadableCopyright</key>
        <string>Copyright (c) 2000 - 2011, Lawrence Livermore National Security, LLC</string>
</dict>
</plist>
EOF
mkdir $1/Contents/MacOS
ln -s ../../bin/visit $1/Contents/MacOS/visit
mkdir $1/Contents/Resources
cp $1/current/darwin-*/bin/gui.app/Contents/Resources/VisItIcon.icns $1/Contents/Resources
mv $1 `dirname $1`/VisIt.app
