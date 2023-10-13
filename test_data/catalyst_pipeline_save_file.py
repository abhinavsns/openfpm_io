from paraview.simple import *
from paraview import print_info

view = CreateRenderView()

producer = TrivialProducer(registrationName="grid")

def catalyst_execute(info):
    global producer

    fname = "output-%d.pvti" % info.timestep

    # save data
    SaveData(fname, proxy=producer, PointDataArrays=['scalar','vector'],DataMode='Binary')

    # save screenshot
    print_info("time=%f, saving file: %s", info.time, fname)

