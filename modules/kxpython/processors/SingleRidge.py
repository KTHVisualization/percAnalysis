# Name: SingleRidge 

import inviwopy as ivw
import numpy as np
from inviwopy.glm import size2_t, mat4, vec4

def single_ridge(x,y):
    return 0.5*np.exp(-(10*np.power(x - np.sin(2*y)/2, 2) + 1 * np.power(y, 2)))

class SingleRidge(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.size = ivw.properties.IntSize2Property("size", "Size", size2_t(256,128), size2_t(1,1), size2_t(2049,1024))
        self.addProperty(self.size, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.SingleRidge", 
    		displayName = "SingleRidge",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return SingleRidge.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        size_x = self.size.value.x
        size_y = self.size.value.y
        x = np.linspace(-2, 2, size_x, dtype='float32')
        y = np.linspace(-3, 3, size_y, dtype='float32')

        xv, yv = np.meshgrid(x, y)
        ridge_data = single_ridge(xv,yv)
        #print(ridge_data.dtype)
        data = ridge_data.reshape((size_x, size_y,1))
        #data = np.flip(data,0)

        reversedData = data[::,::-1]
        volume = ivw.data.Volume(data)

        # Set data and value range
        minValue = np.min(data)
        maxValue = np.max(data)
        volume.dataMap.dataRange = dvec2(minValue, maxValue)
        volume.dataMap.valueRange = dvec2(minValue, maxValue)

        modelMatrix = np.zeros((4,4))
        volume.modelMatrix = mat4(4,0,0,0,0,6,0,0,0,0,2,0,-2,-3,-1,1)
        self.outport.setData(volume)
