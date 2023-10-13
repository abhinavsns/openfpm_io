from paraview.simple import *
from paraview import print_info

view = CreateRenderView()

producer = TrivialProducer(registrationName="grid")

def catalyst_execute(info):
    global producer

    fname = "output_iso-%d.png" % info.timestep

    # ensure camera is setup correctly
    ResetCamera()

    # create a new 'Contour'
    contour1 = Contour(Input=producer)
    contour1.ContourBy = ['POINTS', 'scalar']
    contour1.PointMergeMethod = 'Uniform Binning'

    # Properties modified on contour1
    contour1.Isosurfaces = [5.0,10.0,15.0,20.0,25.0,30.0,35.0,40.0,45.0,50.0,55.0,60.0]

    view.CameraPosition = [-3.684715854397197, 4.120473016714019, -2.6194882519971405]
    view.CameraFocalPoint = [1.0000000000000007, 0.9999999999999999, 1.0]
    view.CameraViewUp = [-0.7140437014118702, -0.44673962513363724, 0.5390410928765016]
    view.CameraParallelScale = 1.7320508075688772

    Show(contour1, view, 'GeometryRepresentation')

    # get active source.
    vox8vtu = GetActiveSource()

    vox8vtuDisplay = GetDisplayProperties(vox8vtu, view=view)
    vox8vtuDisplay.SetScalarBarVisibility(view, True)
    vox8vtuDisplay.SetRepresentationType('Surface')

    # set scalar coloring
    ColorBy(vox8vtuDisplay, ('POINTS', 'scalar'))

    # get color transfer function/color map for 'RTData'
    rTDataLUT = GetColorTransferFunction('scalar')

    # Rescale transfer function
    rTDataLUT.RescaleTransferFunction(0.0, 70.0)

    # get opacity transfer function/opacity map for 'RTData'
    rTDataPWF = GetOpacityTransferFunction('scalar')

    # Rescale transfer function
    rTDataPWF.RescaleTransferFunction(0.0, 70.0)

    # save screenshot
    print_info("time=%f, saving file: %s", info.time, fname)
    SaveScreenshot(fname,view,ImageResolution=[1920, 1080])

