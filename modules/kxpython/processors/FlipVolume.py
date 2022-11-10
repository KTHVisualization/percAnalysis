# Name: FlipVolume 

import inviwopy as ivw
import numpy as np

class FlipVolume(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport, owner=False)
        self.outport = ivw.data.VolumeOutport("outport")
        self.addOutport(self.outport, owner=False)

        self.flipX = ivw.properties.BoolProperty("flipX", "Flip X", 0)
        self.addProperty(self.flipX, owner=False)

        self.flipY = ivw.properties.BoolProperty("flipY", "Flip Y", 0)
        self.addProperty(self.flipY, owner=False)

        self.minValue = ivw.properties.FloatProperty("min", "Min", 0, -10, 10)
        self.addProperty(self.minValue, owner=False)

        self.useDataMin = ivw.properties.BoolProperty("useMin", "Use Data Min", 0)
        self.addProperty(self.useDataMin, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.FlipVolume", 
    		displayName = "FlipVolume",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return FlipVolume.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        volume = self.inport.getData()
        volumedata = volume.data
        dims = volumedata.shape

        if (self.flipX.value):
            flipped = np.fliplr(volumedata)
            volumedata = np.copy(flipped)

        if (self.flipY.value):
            flipped = np.flipud(volumedata)
            volumedata = np.copy(flipped)

       	outvolume = ivw.data.Volume(volumedata)
        minValue = np.min(volumedata)
        if (not self.useDataMin.value):
            minValue = self.minValue.value
        else:
            self.minValue.value = minValue
        maxValue = np.max(volumedata)
        outvolume.dataMap.dataRange = dvec2(minValue, maxValue)
        outvolume.dataMap.valueRange = dvec2(minValue, maxValue)

        outvolume.modelMatrix = volume.modelMatrix


        self.outport.setData(outvolume)
