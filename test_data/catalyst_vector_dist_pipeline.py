# script-version: 2.0
# Catalyst state generated using paraview version 5.11.1-1889-ge9d6cfcddb
import paraview
paraview.compatibility.major = 5
paraview.compatibility.minor = 11

#### import the simple module from the paraview
from paraview.simple import *
#### disable automatic camera reset on 'Show'
paraview.simple._DisableFirstRenderCameraReset()

# ----------------------------------------------------------------
# setup views used in the visualization
# ----------------------------------------------------------------

# Create a new 'Render View'
renderView1 = CreateView('RenderView')
renderView1.ViewSize = [1248, 701]
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [1.0, 1.0, 1.0]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [6.407338041347557, 2.4224459328701355, -1.3001175492369867]
renderView1.CameraFocalPoint = [0.4220476159958419, 0.8479647449941072, 1.2458434096234228]
renderView1.CameraViewUp = [-0.18527777360981507, 0.9689621393402568, 0.16365976637934837]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 1.7320508075688772
renderView1.LegendGrid = 'Legend Grid Actor'

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1248, 701)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'XML PolyData Reader'
particles = TrivialProducer(registrationName='particles')
particles.PointArrayStatus = ['scalar', 'tensor_0_0', 'tensor_0_1', 'tensor_0_2', 'tensor_1_0', 'tensor_1_1', 'tensor_1_2', 'tensor_2_0', 'tensor_2_1', 'tensor_2_2', 'vector']
particles.TimeArray = 'None'

# create a new 'Glyph'
glyph = Glyph(registrationName='Glyph', Input=particles,
    GlyphType='Arrow')
glyph.OrientationArray = ['POINTS', 'vector']
glyph.ScaleArray = ['POINTS', 'scalar']
glyph.ScaleFactor = 0.04
glyph.GlyphTransform = 'Transform2'

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from glyph
glyphDisplay = Show(glyph, renderView1, 'GeometryRepresentation')

# get 2D transfer function for 'tensor_1_1'
tensor_1_1TF2D = GetTransferFunction2D('tensor_1_1')

# get color transfer function/color map for 'tensor_1_1'
tensor_1_1LUT = GetColorTransferFunction('tensor_1_1')
tensor_1_1LUT.TransferFunction2D = tensor_1_1TF2D
tensor_1_1LUT.RGBPoints = [0.0, 0.231373, 0.298039, 0.752941, 2.0, 0.865003, 0.865003, 0.865003, 4.0, 0.705882, 0.0156863, 0.14902]
tensor_1_1LUT.ScalarRangeInitialized = 1.0

# trace defaults for the display properties.
glyphDisplay.Representation = 'Surface'
glyphDisplay.ColorArrayName = ['POINTS', 'tensor_1_1']
glyphDisplay.LookupTable = tensor_1_1LUT
glyphDisplay.SelectTCoordArray = 'None'
glyphDisplay.SelectNormalArray = 'None'
glyphDisplay.SelectTangentArray = 'None'
glyphDisplay.OSPRayScaleArray = 'scalar'
glyphDisplay.OSPRayScaleFunction = 'Piecewise Function'
glyphDisplay.Assembly = ''
glyphDisplay.SelectOrientationVectors = 'None'
glyphDisplay.ScaleFactor = 0.21802423987537622
glyphDisplay.SelectScaleArray = 'None'
glyphDisplay.GlyphType = 'Arrow'
glyphDisplay.GlyphTableIndexArray = 'None'
glyphDisplay.GaussianRadius = 0.010901211993768811
glyphDisplay.SetScaleArray = ['POINTS', 'scalar']
glyphDisplay.ScaleTransferFunction = 'Piecewise Function'
glyphDisplay.OpacityArray = ['POINTS', 'scalar']
glyphDisplay.OpacityTransferFunction = 'Piecewise Function'
glyphDisplay.DataAxesGrid = 'Grid Axes Representation'
glyphDisplay.PolarAxes = 'Polar Axes Representation'
glyphDisplay.SelectInputVectors = ['POINTS', 'vector']
glyphDisplay.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
glyphDisplay.ScaleTransferFunction.Points = [0.1538461538461539, 0.0, 0.5, 0.0, 5.794871794871795, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
glyphDisplay.OpacityTransferFunction.Points = [0.1538461538461539, 0.0, 0.5, 0.0, 5.794871794871795, 1.0, 0.5, 0.0]

# setup the color legend parameters for each legend in this view

# get color legend/bar for tensor_1_1LUT in view renderView1
tensor_1_1LUTColorBar = GetScalarBar(tensor_1_1LUT, renderView1)
tensor_1_1LUTColorBar.Title = 'tensor_1_1'
tensor_1_1LUTColorBar.ComponentTitle = ''

# set color bar visibility
tensor_1_1LUTColorBar.Visibility = 1

# show color legend
glyphDisplay.SetScalarBarVisibility(renderView1, True)

# ----------------------------------------------------------------
# setup color maps and opacity maps used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get opacity transfer function/opacity map for 'tensor_1_1'
tensor_1_1PWF = GetOpacityTransferFunction('tensor_1_1')
tensor_1_1PWF.Points = [0.0, 0.0, 0.5, 0.0, 4.0, 1.0, 0.5, 0.0]
tensor_1_1PWF.ScalarRangeInitialized = 1

# ----------------------------------------------------------------
# setup animation scene, tracks and keyframes
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get the time-keeper
timeKeeper1 = GetTimeKeeper()

# initialize the timekeeper

# get time animation track
timeAnimationCue1 = GetTimeTrack()

# initialize the animation track

# get animation scene
animationScene1 = GetAnimationScene()

# initialize the animation scene
animationScene1.ViewModules = renderView1
animationScene1.Cues = timeAnimationCue1
animationScene1.AnimationTime = 0.0

# initialize the animation scene

# ----------------------------------------------------------------
# setup extractors
# ----------------------------------------------------------------

# create extractor
pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
# trace defaults for the extractor.
pNG1.Trigger = 'Time Step'

# init the 'PNG' selected for 'Writer'
pNG1.Writer.FileName = 'catalyst_vector_dist.png'
pNG1.Writer.ImageResolution = [1920, 1080]
pNG1.Writer.Format = 'PNG'

# ----------------------------------------------------------------
# restore active source
SetActiveSource(pNG1)
# ----------------------------------------------------------------

# ------------------------------------------------------------------------------
# Catalyst options
from paraview import catalyst
options = catalyst.Options()
options.GlobalTrigger = 'Time Step'
options.CatalystLiveTrigger = 'Time Step'

# ------------------------------------------------------------------------------
if __name__ == '__main__':
    from paraview.simple import SaveExtractsUsingCatalystOptions
    # Code for non in-situ environments; if executing in post-processing
    # i.e. non-Catalyst mode, let's generate extracts using Catalyst options
    SaveExtractsUsingCatalystOptions(options)
