# Name: Histogram 

import inviwopy as ivw
from inviwopy.glm import ivec3, dvec2
from inviwopy.properties import FloatProperty, IntProperty, BoolProperty
from inviwopy.data import VolumeInport
from inviwopy.data import Volume
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rc


class Histogram(ivw.Processor):
    def __init__(self, id, name):
        ivw.Processor.__init__(self, id, name)
        self.inport = ivw.data.VolumeInport("inport")
        self.addInport(self.inport, owner=False)

        self.min = FloatProperty("min", "Min", -500, -500, 500)
        self.addProperty(self.min, owner=False)
        self.max = FloatProperty("max", "Max", 500, -500, 500)
        self.addProperty(self.max, owner=False)
        self.numBins = IntProperty("numBins", "Bins", 100, 10, 1000)
        self.addProperty(self.numBins, owner=False)
        
        self.showSamples = BoolProperty("showSamples", "Show Samples", True)
        self.addProperty(self.showSamples, owner=False)
        self.numSamples = IntProperty("numSamples", "Samples", 10, 10, 100)
        self.addProperty(self.numSamples, owner=False)
        self.sampleMax = FloatProperty("sampleMax", "Sample Max", 500, -500, 500)
        self.addProperty(self.sampleMax, owner=False)

        self.fontSize = IntProperty("fontSize", "Font Size", 12, 6, 36)
        self.addProperty(self.fontSize, owner=False)
        self.figureWidth = FloatProperty("figureWidth", "Width (in)", 5, 1, 8.27)
        self.addProperty(self.figureWidth, owner=False)
        self.figureHeight = FloatProperty("figureHeight", "Height Scale", 0.8, 0.1, 1.5)
        self.addProperty(self.figureHeight, owner=False)



    @staticmethod
    def processorInfo():
        return ivw.ProcessorInfo(
    		classIdentifier = "org.inviwo.Histogram", 
    		displayName = "Histogram",
    		category = "Python",
    		codeState = ivw.CodeState.Stable,
    		tags = ivw.Tags.PY
        )

    def getProcessorInfo(self):
        return Histogram.processorInfo()

    def initializeResources(self):
        pass

    def process(self):
        volume = self.inport.getData()
        volumeData = volume.data.flatten()
        print(np.min(volumeData), np.max(volumeData))

        # Plot settings
        fontSize = self.fontSize.value
        minValue = self.min.value
        maxValue = self.max.value
        numBins = self.numBins.value
        width = self.figureWidth.value
        goldenRevert = 2/(1+np.sqrt(5))
        height = self.figureHeight.value * width * goldenRevert

        #print(plt.rcParams.get('figure.figsize'))
        rc("font", size=fontSize)

        if self.showSamples.value:
            fig, (ax, ax2, ax3) = plt.subplots(nrows=3, ncols=1, sharex=True, 
                                gridspec_kw={'height_ratios': [10, 1, 1], 'wspace':0.02},
                                figsize=(width, height))
        else:
            fig, ax = plt.subplots(figsize=(width, height))

        ax.hist(volumeData, bins=numBins, range=(minValue, maxValue))
        ax.set_ylabel('Number of Elements')
        ax.spines['top'].set_visible(False)
        ax.spines['right'].set_visible(False)
        ax.ticklabel_format(scilimits=(-2,2))
        ax.set_xlim([minValue, maxValue])
        
        if self.showSamples.value:
            ## Equidistant value sampling
            numSamples=self.numSamples.value
            sampleMax = self.sampleMax.value
            valueSamples = np.linspace(minValue, sampleMax, num=numSamples)
            #print(valueSamples)

            ax2.plot(valueSamples, np.full(valueSamples.shape, 1), 'o', c='C1', clip_on=False)

            ## Eqidistant voxel sampling
            minQuant = (volumeData < minValue).sum() / volumeData.size
            if (volumeData < minValue).sum() == 0:
            	minQuant = 0.0

            maxQuant = 1.0 - (volumeData > sampleMax).sum() / volumeData.size
            if (volumeData > sampleMax).sum() == 0:
            	maxQuant = 1.0
            voxelSamplesQuant = np.linspace(minQuant, maxQuant, num=numSamples)
            quantiles = np.quantile(volumeData, voxelSamplesQuant)
            #print(quantiles)
            ax3.plot(quantiles, np.full(valueSamples.shape, 1), 'o', c='C3', 
                clip_on=False)
            ax3.set_xlabel('$f(\mathbf{x})$')

            for direction in ["left", "right", "top"]:
            # hides borders
                ax2.spines[direction].set_visible(False)
                ax3.spines[direction].set_visible(False)
            ax2.get_yaxis().set_visible(False)
            ax3.get_yaxis().set_visible(False)
        else:
            ax.set_xlabel('$f(\mathbf{x})$')
            plt.tight_layout()
        plt.show()

        import tikzplotlib
        tikzplotlib.save("histogram_iso.tex")

