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
renderView1.ViewSize = [1248, 685]
renderView1.AxesGrid = 'Grid Axes 3D Actor'
renderView1.CenterOfRotation = [0.4789473684210527, 0.4789473684210527, 0.4789473684210527]
renderView1.StereoType = 'Crystal Eyes'
renderView1.CameraPosition = [-1.2505517874463687, 1.9030560542325916, 2.7710893338799387]
renderView1.CameraFocalPoint = [0.4789473684210527, 0.4789473684210527, 0.4789473684210527]
renderView1.CameraViewUp = [-0.3249269691336084, 0.6737008919693314, -0.6637390849493306]
renderView1.CameraFocalDisk = 1.0
renderView1.CameraParallelScale = 0.8295611762566729
renderView1.LegendGrid = 'Legend Grid Actor'

SetActiveView(None)

# ----------------------------------------------------------------
# setup view layouts
# ----------------------------------------------------------------

# create new layout object 'Layout #1'
layout1 = CreateLayout(name='Layout #1')
layout1.AssignView(0, renderView1)
layout1.SetSize(1248, 685)

# ----------------------------------------------------------------
# restore active view
SetActiveView(renderView1)
# ----------------------------------------------------------------

# ----------------------------------------------------------------
# setup the data processing pipelines
# ----------------------------------------------------------------

# create a new 'XML Partitioned Polydata Reader'
particles = TrivialProducer(registrationName='particles')
particles.PointArrayStatus = ['scalar', 'vector']
particles.TimeArray = 'None'

# create a new 'Glyph'
glyph = Glyph(registrationName='glyph', Input=particles,
    GlyphType='Arrow')
glyph.OrientationArray = ['POINTS', 'vector']
glyph.ScaleArray = ['POINTS', 'scalar']
glyph.ScaleFactor = 0.09578947368421054
glyph.GlyphTransform = 'Transform2'

# ----------------------------------------------------------------
# setup the visualization in view 'renderView1'
# ----------------------------------------------------------------

# show data from glyph
glyphDisplay = Show(glyph, renderView1, 'GeometryRepresentation')

# get 2D transfer function for 'scalar'
scalarTF2D = GetTransferFunction2D('scalar')
scalarTF2D.ScalarRangeInitialized = 1
scalarTF2D.Range = [0.0, 10.0, 0.0, 1.0]

# get color transfer function/color map for 'scalar'
scalarLUT = GetColorTransferFunction('scalar')
scalarLUT.TransferFunction2D = scalarTF2D
scalarLUT.RGBPoints = [0.0, 0.231373, 0.298039, 0.752941, 1.436842105263158, 0.865003, 0.865003, 0.865003, 2.873684210526316, 0.705882, 0.0156863, 0.14902]
scalarLUT.ScalarRangeInitialized = 1.0

# trace defaults for the display properties.
glyphDisplay.Representation = 'Surface'
glyphDisplay.ColorArrayName = ['POINTS', 'scalar']
glyphDisplay.LookupTable = scalarLUT
glyphDisplay.SelectTCoordArray = 'None'
glyphDisplay.SelectNormalArray = 'None'
glyphDisplay.SelectTangentArray = 'None'
glyphDisplay.OSPRayScaleArray = 'scalar'
glyphDisplay.OSPRayScaleFunction = 'Piecewise Function'
glyphDisplay.Assembly = ''
glyphDisplay.SelectOrientationVectors = 'None'
glyphDisplay.ScaleFactor = 0.12148346658796072
glyphDisplay.SelectScaleArray = 'None'
glyphDisplay.GlyphType = 'Arrow'
glyphDisplay.GlyphTableIndexArray = 'None'
glyphDisplay.GaussianRadius = 0.006074173329398036
glyphDisplay.SetScaleArray = ['POINTS', 'scalar']
glyphDisplay.ScaleTransferFunction = 'Piecewise Function'
glyphDisplay.OpacityArray = ['POINTS', 'scalar']
glyphDisplay.OpacityTransferFunction = 'Piecewise Function'
glyphDisplay.DataAxesGrid = 'Grid Axes Representation'
glyphDisplay.PolarAxes = 'Polar Axes Representation'
glyphDisplay.SelectInputVectors = ['POINTS', 'vector']
glyphDisplay.WriteLog = ''

# init the 'Piecewise Function' selected for 'ScaleTransferFunction'
glyphDisplay.ScaleTransferFunction.Points = [0.06842105263157895, 0.0, 0.5, 0.0, 2.805263157894737, 1.0, 0.5, 0.0]

# init the 'Piecewise Function' selected for 'OpacityTransferFunction'
glyphDisplay.OpacityTransferFunction.Points = [0.06842105263157895, 0.0, 0.5, 0.0, 2.805263157894737, 1.0, 0.5, 0.0]

# setup the color legend parameters for each legend in this view

# get color legend/bar for scalarLUT in view renderView1
scalarLUTColorBar = GetScalarBar(scalarLUT, renderView1)
scalarLUTColorBar.Title = 'scalar'
scalarLUTColorBar.ComponentTitle = ''

# set color bar visibility
scalarLUTColorBar.Visibility = 1

# show color legend
glyphDisplay.SetScalarBarVisibility(renderView1, True)

# ----------------------------------------------------------------
# setup color maps and opacity maps used in the visualization
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get opacity transfer function/opacity map for 'scalar'
scalarPWF = GetOpacityTransferFunction('scalar')
scalarPWF.Points = [0.0, 0.0, 0.5, 0.0, 2.873684210526316, 1.0, 0.5, 0.0]
scalarPWF.ScalarRangeInitialized = 1

# ----------------------------------------------------------------
# setup animation scene, tracks and keyframes
# note: the Get..() functions create a new object, if needed
# ----------------------------------------------------------------

# get time animation track
timeAnimationCue1 = GetTimeTrack()

# initialize the animation scene

# get the time-keeper
timeKeeper1 = GetTimeKeeper()

# initialize the timekeeper

# initialize the animation track

# get animation scene
animationScene1 = GetAnimationScene()

# initialize the animation scene
animationScene1.ViewModules = renderView1
animationScene1.Cues = timeAnimationCue1
animationScene1.AnimationTime = 0.0

# ----------------------------------------------------------------
# setup extractors
# ----------------------------------------------------------------

# create extractor
pNG1 = CreateExtractor('PNG', renderView1, registrationName='PNG1')
# trace defaults for the extractor.
pNG1.Trigger = 'Time Step'

# init the 'PNG' selected for 'Writer'
pNG1.Writer.FileName = 'catalyst_vector_dist.png'
pNG1.Writer.ImageResolution = [1248, 685]
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
