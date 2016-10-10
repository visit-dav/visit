Rendering Options
-----------------

VisIt provides support for setting various global rendering options that improve quality and realism of the plots in the visualization. Specifically, VisIt provides controls that let you smooth the appearance of lines, add specular highlights, and add shadows to plots in your visualizations. The controls for setting these options are located in the
**Rendering Options Window**
(see
) and they will be covered here while other controls in that window will be covered in the
**Preferences**
chapter. To open the
**Rendering Options Window**
, click on the
**Rendering**
option in the
**Main Window's Preferences**
menu.

Making lines look smoother
~~~~~~~~~~~~~~~~~~~~~~~~~~

Computer monitors contain an array of millions of tiny rectangular pixels that light up to form patterns which your eyes perceive as images. Lines tend to look blocky on computer monitors because they are drawn using a relatively small set of pixels. Lines can be made to look better by
blending the edges of the line with the color of the background image. This is a form of antialiasing that VisIt can use to make plots which use lines, such as the Mesh plot, look better (see
). If you want to enable antialiasing, which is off by default, you check the
**Antialiasing**
check box in the
**Rendering Options Window**
. When antialiasing is enabled, all lines drawn in a visualization window are blended with the background image so that they look smoother.

Specular lighting
~~~~~~~~~~~~~~~~~

VisIt supports specular lighting, which results in bright highlights on surfaces that reflect a lot of incident light from VisIt's light sources. Specular lighting is not handled in the
**Lighting Window**
because specular lighting is best described as a property of the material reflecting the light. The controls for specular lighting don't control any lights but instead control the amount of specular highlighting to caused by the plots. Specular lighting is not enabled by default. To
enable specular lighting, click the
**Specular lighting**
check box in the
**Rendering Options Window**
.

Once specular lighting is enabled, you can change the strength and sharpness properties of the material reflecting the light. The strength, which you can set using the
**Strength**
slider, influences how glossy the plots are and how much light is reflected off of the plots. The sharpness, which is set using the
**Sharpness**
slider, controls the locality of the reflections. Higher the sharpness values result in smaller specular highlights. Specular highlights are a crucial component of lighting models and including specular lighting in your visualizations enhances their appearance by making them more realistic. Compare and contrast the plots in
. The plot on the left side has no specular highlights and the plot on the right side has specular highlights.

Shadows
~~~~~~~

VisIt supports shadows when scalable rendering is being used. Shadows can be useful for increasing the realism of your visualization. The controls to turn on shadows can be found in the
**Rendering Options Window**
. To turn on shadows, you must currently turn on scalable rendering by clicking on the
**Always**
radio button under the
**Use scalable rendering**
label. Once scalable rendering has been turned on, the Shadows controls become enabled. The default shadow strength is 50%. If you desire a stronger or weaker shadow, adjust the
**Strength**
slider until you are satisfied with the amount of shadow that appears in the visualization. The same plot is shown with and without shadows in
.
