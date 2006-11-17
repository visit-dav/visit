<?xml version="1.0"?>
<Object name="TemplateOptions">
    <Field name="VERSION" type="string">1.5.5</Field>
    <Field name="TEMPLATEFILE" type="string">slice3.py</Field>
    <Field name="TITLE" type="string">Slice 3D </Field>
    <Field name="DESCRIPTION" type="string">This template displays a 3D plot and slices through it along the X,Y,Z axes at various points in time.</Field>
    <Field name="PREVIEWIMAGE" type="string">slice3_preview.bmp</Field>
    <Object name="VIEWPORTS">
        <Object name="Viewport 1">
            <Field name="coordinates" type="floatArray" length="4">0. 0. 1. 1.</Field>
            <Field name="compositing" type="int">0</Field>
            <Field name="opacity" type="float">1.</Field>
            <Field name="replaceColor" type="intArray" length="3">0 0 0</Field>
            <Field name="dropShadow" type="bool">false</Field>
            <Field name="sequenceList" type="stringVector">"SEQUENCE_1" "FadeToBlack" </Field>
        </Object>
    </Object>
    <Object name="SEQUENCEDATA">
        <Object name="SEQUENCE_1">
            <Field name="sequenceType" type="string">Scripting</Field>
            <Field name="uiFile" type="string">slice3_00.ui</Field>
            <Field name="DATABASE" type="string"></Field>
            <Field name="PLOT_VAR" type="string">scalar</Field>
            <Field name="NSLICE_SEQUENCES" type="int">3</Field>
            <Field name="NFRAMES_PER_SLICE" type="int">30</Field>
            <Field name="GRADIENT_BGCOLOR1" type="intArray" length="4">100 100 100 255</Field>
            <Field name="GRADIENT_BGCOLOR2" type="intArray" length="4">0 0 0 255</Field>
            <Field name="CLASSIFICATION_TEXT" type="string">Unclassified</Field>
            <Field name="CLASSIFICATION_TEXTCOLOR" type="intArray" length="4">255 0 0 255</Field>
            <Field name="TITLE" type="string">Visualization Title</Field>
            <Field name="XAXIS_TEXT" type="string">X-Axis</Field>
            <Field name="YAXIS_TEXT" type="string">Y-Axis</Field>
            <Field name="ZAXIS_TEXT" type="string">Z-Axis</Field>
        </Object>
        <Object name="FadeToBlack">
            <Field name="sequenceType" type="string">Fade</Field>
            <Field name="input1" type="string">Frames</Field>
            <Field name="color1" type="intArray" length="3">255 0 0 </Field>
            <Field name="input2" type="string">Color</Field>
            <Field name="color2" type="intArray" length="3">0 0 0 </Field>
            <Field name="nFrames" type="int">15</Field>
        </Object>
    </Object>
</Object>
