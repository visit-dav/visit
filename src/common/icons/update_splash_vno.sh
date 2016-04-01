#!/bin/sh

usage() {
    echo "$0 \"<new-vno>\" \"<xcf-filename>\""
    exit 1
}

if [[ -z "$1" ]] || [[ -z "$2" ]]; then
    usage
fi

if [[ ! -e "$2" ]]; then
    echo "File \"$2\" not found"
    exit 1
fi

vno=$1
file=$(basename $2 .xcf)

# ImageMagick can *read* xcf files but cannot *write* them.
# If we get away from this gimp-specific file format,
# we can probably fully automate the splashscreent update
convert ${file}.xcf \
    \( -clone 10 \
        -fill "rgb(255,0,0)" \
        -draw "rectangle 0,0 400,220" \
        -transparent "rgb(255,0,0)" \
        -font Helvetica-Oblique -style Oblique -weight 100 -pointsize 60 -draw "gravity North fill purple text 115,170 '${vno}' " \
    \) -swap 10 +delete \
    ${file}.tif
