# Name: QRCodeForPercolation 

import inviwopy as ivw
from inviwopy.glm import size3_t, dvec2, ivec2
import numpy as np

class QRCodeForPercolation(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport, owner=False)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)
        self.seed = ivw.properties.Size_tProperty("seed", "Seed", 42, 0 ,1000,1)
        self.addProperty(self.seed, owner=False)
        self.resolution = ivw.properties.IntVec2Property("resolution", "Resolution", ivec2(18,18))
        self.addProperty(self.resolution, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.QRCodeForPercolation", 
    		displayName = "QRCodeForPercolation",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return QRCodeForPercolation.processorInfo()

    def initializeResources(self):
        print("init")

    def process(self):
        # Resample binary image such that 
        # black 0 is mapped to 0.5 and 1
        # white 1 is mapped to 0
        print("D")
        volume = self.inport.getData()
        dims = volume.dimensions
        print("Volume of dimensions", dims)
        if ():
        	print("Input does not seem to be an image")
        	return
        volumedata = volume.data
        # Slice the volume, sub sample
        res = self.resolution.value 
        repX = round(dims[0]/res[0])
        repY = round(dims[1]/res[1])
        print("X", repX, "Y", repY)
        volumedataslice = volumedata[int(repX/2)::repX,int(repY/2)::repY,:,0]
        print("Value at 0,0,0:", volumedataslice[0,0,0])        
        
        np.random.seed(seed=self.seed.value)
        data = np.random.uniform(0.5, 1.0, volumedataslice.shape)
        data = data.astype('float32')
        print(data.shape)
        data[volumedataslice!=0] = 0
        print(np.count_nonzero(volumedataslice!=0))
        print(data[0,0,0])

        # Upsample again
        data = np.repeat(data, repX, axis=0)
        data = np.repeat(data, repY, axis=1)

        volume = ivw.data.Volume(data)
        # Set data and value range
        minValue = np.min(data)
        maxValue = np.max(data)
        volume.dataMap.dataRange = dvec2(minValue, maxValue)
        volume.dataMap.valueRange = dvec2(minValue, maxValue)

        self.outport.setData(volume)