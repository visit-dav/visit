#!/bin/bash

# CVD = color vision deficiency
#    <Field name="discrete" type="bool">true</Field>
#    <Field name="tags" type="stringVector">"AccessibleColors" </Field>
#
# Categorical pallet is 100 colors maximally disparate from previous
# colors. First, last, middle-first-half, middle-2nd-half, etc.
#
# cvd, color vision deficiency, ColorImpaired, Inclusive, CVDFriendly, ColorImpairedFriendly
#
# Discrete pallet is set to discrete and then samples from orig.
#
# mode: cyc(C), seq(Q), div(D), seq2(M), cat
# type: disc or cont
# count: for discrete
# categorical is still continus (shuffled, rand, hash, mix, )
#
# diverging maps diverge from white or black
#
# Examples
#     cvd-batlowK-seq        (main)
#     cvd-batlowK-seq-jumbled
#     cvd-batlowK-seq-10
#     cvd-batlowK-seq-25
#     cvd-batlowK-seq-50

seqnames="batlow batlowW batlowK devon lajolla bamako davos bilbao nuuk oslo grayC hawaii lapaz tokyo buda acton turku imola"
divnames="broc cork vik lisbon tofino berlin roma bam vanimo"
biseqnames="oleron bukavu fes"
cycnames="romaO bamO brocO corkO vikO"

ctfiles=$(find ~/Downloads/ScientificColourMaps7 -type d -maxdepth 1)

modeName() {
    for n in $seqnames; do
        if [ "$n" == "$1" ]; then
            echo "seq"
            return
        fi
    done
    for n in $divnames; do
        if [ "$n" == "$1" ]; then
            echo "div"
            return
        fi
    done
    for n in $biseqnames; do
        if [ "$n" == "$1" ]; then
            echo "2seq"
            return
        fi
    done
    for n in $cycnames; do
        if [ "$n" == "$1" ]; then
            echo "cyc"
            return
        fi
    done
    exit "unknown name $1"
}

#
# Scientific Colour Map Categorical Palette
# hawaii 10 Swatches
# 140   2 115     hawaii-1 #8C0273
# 146  42  89     hawaii-29 #922A59
# 150  71  66     hawaii-58 #964742
# 153  99  48     hawaii-86 #996330
# 157 131  30     hawaii-114 #9D831E
# 151 169  42     hawaii-143 #97A92A
# 128 197  95     hawaii-171 #80C55F
# 102 216 156     hawaii-199 #66D89C
# 108 235 219     hawaii-228 #6CEBDB
# 179 242 253     hawaii-256 #B3F2FD
#
makeVisItDiscreteFromTxtDiscrete() {
# Header...
#     <?xml version="1.0"?>
#     <Object name="ColorTable">
#         <Field name="Version" type="string">3.0.2</Field>
#         <Object name="ColorControlPointList">

#
# Color...
#         <Object name="ColorControlPoint">
#            <Field name="colors" type="unsignedCharArray" length="4">48 18 59 255</Field>
#            <Field name="position" type="float">0.000000</Field>
#        </Object>
#
# .
# .
# .
#
# Footer...
#
#     <Field name="discrete" type="bool">true</Field>
#     <Field name="tags" type="stringVector">\"AccessibleColors\" </Field>
#    </Object>
# </Object>
#            $f/DiscretePalettes $m $name 10

    delta=$(perl -e "print int((10000000.0/($4-1)+0.5))/10000000")
    newct="$3-$2-$4.ct"
    echo "<?xml version=\"1.0\"?>" > $newct
    echo "<Object name=\"ColorTable\">" >> $newct
    echo "    <Field name=\"Version\" type=\"string\">3.0.2</Field>" >> $newct
    echo "    <Object name=\"ColorControlPointList\">" >> $newct
    pos=0.0
    while read -r line; do
        if [ -n "$(echo $line | grep Scientific)" ] || [ -n "$(echo $line | grep Swatches)" ]; then
            continue
        fi
        red=$(echo $line | cut -d' ' -f1)
        grn=$(echo $line | cut -d' ' -f2)
        blu=$(echo $line | cut -d' ' -f3)
        echo "        <Object name=\"ColorControlPoint\">" >> $newct
        echo "            <Field name=\"colors\" type=\"unsignedCharArray\" length=\"4\">$red $grn $blu 255</Field>" >> $newct
        echo "            <Field name=\"position\" type=\"float\">$pos</Field>" >> $newct
        echo "        </Object>" >> $newct
        pos=$(perl -e "print $pos+$delta")
        pos=$(perl -e "print $pos>1?1:$pos")
    done < $1/$3$4.txt
    echo "    <Field name=\"discrete\" type=\"bool\">true</Field>" >> $newct
    echo "    <Field name=\"tags\" type=\"stringVector\">\"CvdFriendly\" \"Crameri\" </Field>" >> $newct
    echo "    </Object>" >> $newct
    echo "</Object>" >> $newct
}

for f in $ctfiles; do
    name=$(echo $f | cut -d'/' -f6)
    if [ -z "$name" ] || [ "$name" == "+TOOLS" ] || [ "$name" == "+TESTS" ]; then
        continue
    fi
    disc=""
    cat=""
    if [ -d $f/DiscretePalettes ]; then
        disc="has discrete"
    fi
    if [ -d $f/CategoricalPalettes ]; then
        cat="has categorical"
    fi
    m=$(modeName $name)
    echo "$name, mode=$m, $disc, $cat"

    # Get the main map
    rm -f $name-$m.ct
    cat $f/$name.ct | tr '\n' '@' | sed -e 's#@    </Object>@</Object>#@        <Field name="tags" type="stringVector">"CvdFriendly" "Crameri" </Field>@    </Object>@</Object>#' | tr '@' '\n' > $name-$m.ct

    # Get categorical map if present
    #git mv ${name}S-$m-jumbled.ct $name-$m-jumbled.ct
    rm -f $name-$m-jumbled.ct
    if [ -n "$cat" ]; then
        cat $f/CategoricalPalettes/${name}S.ct | tr '\n' '@' | sed -e 's#@    </Object>@</Object>#@        <Field name="tags" type="stringVector">"CvdFriendly" "Crameri" </Field>@    </Object>@</Object>#' | tr '@' '\n' > ${name}-$m-jumbled.ct
    fi

    # Make the discrete maps
    if [ -n "$disc" ]; then
        if [ -e $f/DiscretePalettes/${name}10.txt ]; then
            makeVisItDiscreteFromTxtDiscrete $f/DiscretePalettes $m $name 10
        fi
        if [ -e $f/DiscretePalettes/${name}25.txt ]; then
            makeVisItDiscreteFromTxtDiscrete $f/DiscretePalettes $m $name 25
        fi
        if [ -e $f/DiscretePalettes/${name}50.txt ]; then
            makeVisItDiscreteFromTxtDiscrete $f/DiscretePalettes $m $name 50
        fi
        if [ -e $f/DiscretePalettes/${name}100.txt ]; then
            makeVisItDiscreteFromTxtDiscrete $f/DiscretePalettes $m $name 100
        fi
    fi
done
