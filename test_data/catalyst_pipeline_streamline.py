from paraview.simple import *
from paraview import print_info

view = CreateRenderView()

producer = TrivialProducer(registrationName="grid")

#Show(producer)

def catalyst_execute(info):
    global producer

    fname = "output_streamline-%d.png" % info.timestep

    # ensure camera is setup correctly
    ResetCamera()


    view.CameraPosition = [-3.684715854397197, 4.120473016714019, -2.6194882519971405]
    view.CameraFocalPoint = [1.0000000000000007, 0.9999999999999999, 1.0]
    view.CameraViewUp = [-0.7140437014118702, -0.44673962513363724, 0.5390410928765016]
    view.CameraParallelScale = 1.7320508075688772

    # create a new 'Stream Tracer'
    streamTracer1 = StreamTracer(registrationName='StreamTracer1',Input=producer,SeedType='Line')
    streamTracer1.Vectors = ['POINTS', 'vector']
    streamTracer1.MaximumStreamlineLength = 2.0

    # init the 'High Resolution Line Source' selected for 'SeedType'
    streamTracer1.SeedType.Point1 = [0.5, 0.5, 0.5]
    streamTracer1.SeedType.Point2 = [0.0, 0.0, 0.1]

    streamTracer1Display = Show(streamTracer1, view, 'GeometryRepresentation')

    # get active source.
    vox8vtu = GetActiveSource()

    vox8vtuDisplay = GetDisplayProperties(vox8vtu, view=view)
    vox8vtuDisplay.SetScalarBarVisibility(view, True)
    vox8vtuDisplay.SetRepresentationType('Surface')

#    vox8vtuDisplay.VolumeRenderingMode = 'GPU Based'

    # set scalar coloring
    ColorBy(vox8vtuDisplay, ('POINTS', 'vector'))

    # get color transfer function/color map for 'RTData'
    rTDataLUT = GetColorTransferFunction('vector')

    # Rescale transfer function
    rTDataLUT.RescaleTransferFunction(0.0, 20.0)


    # show data in view
    # pointDatatoCellData1Display = Show(pointDatatoCellData1, view, 'UniformGridRepresentation')

    # save screenshot
    #print_info("time=%f, saving file: %s", info.time, fname)
    SaveScreenshot(fname,view,ImageResolution=[1920, 1080])

