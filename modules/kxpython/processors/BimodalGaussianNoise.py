# Name: BimodalGaussianNoise 

import inviwopy as ivw
from inviwopy.glm import size3_t, dvec2
import numpy as np

class BimodalGaussianNoise(ivw.Processor):
    def __init__(self, id, name):

        ivw.Processor.__init__(self, id, name)
        self.outport = ivw.data.VolumeOutport("outVolume")
        self.addOutport(self.outport, owner=False)

        self.volSize = ivw.properties.IntSize3Property("size", "Size", size3_t(128,128,1), size3_t(1,1,1))
        #self.slider = ivw.properties.IntProperty("slider", "slider", 0, 0, 100, 1)
        self.addProperty(self.volSize, owner=False)

        self.mu_a = ivw.properties.FloatProperty("mu_a", "Mean A", 0, -10, 10, 0.1)
        self.sigma_a = ivw.properties.FloatProperty("sigma_a", "Sigma A", 1, 0, 10, 0.1)
        self.addProperty(self.mu_a, owner=False)
        self.addProperty(self.sigma_a, owner=False)

        self.mu_b = ivw.properties.FloatProperty("mu_b", "Mean B", 0, -10, 10, 0.1)
        self.sigma_b = ivw.properties.FloatProperty("sigma_b", "Sigma B", 1, 0, 10, 0.1)
        self.addProperty(self.mu_b, owner=False)
        self.addProperty(self.sigma_b, owner=False)

        self.percentage = ivw.properties.FloatProperty("percentage", "Percentage A", 1, 0, 1, 0.01)
        self.addProperty(self.percentage, owner=False)

        self.seed = ivw.properties.Size_tProperty("seed", "Seed", 42, 0 ,1000,1)
        self.addProperty(self.seed, owner=False)

    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.BimodalGaussianNoise", 
    		displayName = "BimodalGaussianNoise",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return BimodalGaussianNoise.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        dims = self.volSize.value
        num_elements = dims[0] * dims[1] * dims[2]

        num_a = round(self.percentage.value * num_elements)
        num_b = num_elements - num_a

        np.random.seed(seed=self.seed.value)

        data_a = np.random.normal(self.mu_a.value, self.sigma_a.value, num_a)
        data_b = np.random.normal(self.mu_b.value, self.sigma_b.value, num_b)

        data = np.concatenate((data_a, data_b))
        np.random.shuffle(data)
        data = np.around(data, decimals=4)
        data = data.astype('float32').reshape((dims[0], dims[1], dims[2]))

        volume = ivw.data.Volume(data)
        # Set data and value range
        minValue = np.min(data)
        maxValue = np.max(data)
        volume.dataMap.dataRange = dvec2(minValue, maxValue)
        volume.dataMap.valueRange = dvec2(minValue, maxValue)

        self.outport.setData(volume)