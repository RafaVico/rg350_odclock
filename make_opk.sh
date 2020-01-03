#!/bin/sh

OPK_NAME=clockod.opk

echo ${OPK_NAME}

# create default.gcw0.desktop
cat > default.gcw0.desktop <<EOF
[Desktop Entry]
Name=Clock OpenDingux
Comment=Clock, calendar
Exec=clockod
Terminal=false
Type=Application
StartupNotify=true
Icon=clockod
Categories=applications;
EOF

# create opk
FLIST="clockod"
FLIST="${FLIST} clockod.png"
FLIST="${FLIST} default.gcw0.desktop"

rm -f ${OPK_NAME}
mksquashfs ${FLIST} ${OPK_NAME} -all-root -no-xattrs -noappend -no-exports

cat default.gcw0.desktop
rm -f default.gcw0.desktop

