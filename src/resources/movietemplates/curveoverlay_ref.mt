<?xml version="1.0"?>
<Object name="TemplateOptions">
    <Field name="VERSION" type="string">1.5.3</Field>
    <Field name="TEMPLATEFILE" type="string">curveoverlay_ref.py</Field>
    <Field name="TITLE" type="string">Reflected Curve Overlay 2D </Field>
    <Field name="DESCRIPTION" type="string">This template displays two reflected 2D plots in one viewport while showing a 1D curve in another viewport.</Field>
    <Field name="PREVIEWIMAGE" type="string">curveoverlay_ref_preview.bmp</Field>
    <Object name="VIEWPORTS">
        <Object name="Viewport 1">
            <Field name="coordinates" type="floatArray" length="4">0. 0. 1. 1.</Field>
            <Field name="compositing" type="int">0</Field>
            <Field name="opacity" type="float">1.</Field>
            <Field name="replaceColor" type="intArray" length="3">0 0 0</Field>
            <Field name="dropShadow" type="bool">false</Field>
            <Field name="sequenceList" type="stringVector">"SEQUENCE_1"</Field>
        </Object>
        <Object name="Viewport 2">
            <Field name="coordinates" type="floatArray" length="4">0. 0.04 1. 0.27</Field>
            <Field name="compositing" type="int">3</Field>
            <Field name="opacity" type="float">1.</Field>
            <Field name="replaceColor" type="intArray" length="3">0 0 0</Field>
            <Field name="dropShadow" type="bool">false</Field>
            <Field name="sequenceList" type="stringVector">"SEQUENCE_2"</Field>
        </Object>
    </Object>
    <Object name="SEQUENCEDATA">
        <Object name="SEQUENCE_1">
            <Field name="sequenceType" type="string">Scripting</Field>
            <Field name="uiFile" type="string">curveoverlay_ref_00.ui</Field>
            <Field name="DATABASE1" type="string"></Field>
            <Field name="PLOT_TYPE1" type="int">2</Field>
            <Field name="PLOT_VAR1" type="string">scalar</Field>
            <Field name="DATABASE2" type="string"></Field>
            <Field name="PLOT_TYPE2" type="int">0</Field>
            <Field name="PLOT_VAR2" type="string">material</Field>
            <Field name="GRADIENT_BGCOLOR1" type="intArray" length="4">100 100 100 255</Field>
            <Field name="GRADIENT_BGCOLOR2" type="intArray" length="4">0 0 0 255</Field>
            <Field name="CLASSIFICATION_TEXT" type="string">Unclassified</Field>
            <Field name="CLASSIFICATION_TEXTCOLOR" type="intArray" length="4">255 0 0 255</Field>
            <Field name="TITLE" type="string">Visualization Title</Field>
            <Field name="XAXIS_TEXT" type="string">X-Axis</Field>
            <Field name="YAXIS_TEXT" type="string">Y-Axis</Field>
        </Object>
        <Object name="SEQUENCE_2">
            <Field name="sequenceType" type="string">Scripting</Field>
            <Field name="uiFile" type="string">curveoverlay_ref_01.ui</Field>
            <Field name="CURVE_DATABASE" type="string"></Field>
            <Field name="CURVE_VARIABLE" type="string">curve</Field>
            <Field name="CURVE_TITLE" type="string">Curve Title</Field>
        </Object>
    </Object>
</Object>
