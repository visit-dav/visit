#!/bin/bash

makeVisItDiscreteFromTxtDiscrete() {
    delta=$(perl -e "print int((10000000.0/(256-1)+0.5))/10000000")
    newct="cividis.ct"
    echo "<?xml version=\"1.0\"?>" > $newct
    echo "<Object name=\"ColorTable\">" >> $newct
    echo "    <Field name=\"Version\" type=\"string\">3.0.2</Field>" >> $newct
    echo "    <Object name=\"ColorControlPointList\">" >> $newct
    pos=0.0
    while read -r line; do
        if [ -n "$(echo $line | grep Scientific)" ] || [ -n "$(echo $line | grep Swatches)" ]; then
            continue
        fi
        redf=$(echo $line | cut -d';' -f1)
        red=$(perl -e "print int($redf * 256 + 0.5)")
        grnf=$(echo $line | cut -d';' -f2)
        grn=$(perl -e "print int($grnf * 256 + 0.5)")
        bluf=$(echo $line | cut -d';' -f3)
        blu=$(perl -e "print int($bluf * 256 + 0.5)")
        echo "        <Object name=\"ColorControlPoint\">" >> $newct
        echo "            <Field name=\"colors\" type=\"unsignedCharArray\" length=\"4\">$red $grn $blu 255</Field>" >> $newct
        echo "            <Field name=\"position\" type=\"float\">$pos</Field>" >> $newct
        echo "        </Object>" >> $newct
        pos=$(perl -e "print $pos+$delta")
    done < cividis.txt
    echo "    <Field name=\"tags\" type=\"stringVector\">\"CvdFriendly\" \"Viridis\" </Field>" >> $newct
    echo "    </Object>" >> $newct
    echo "</Object>" >> $newct
}

makeVisItDiscreteFromTxtDiscrete
