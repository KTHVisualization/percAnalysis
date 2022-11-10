import inviwopy
from inviwopy.glm import ivec3, dvec2
from inviwopy.properties import FloatProperty, IntProperty
from inviwopy.data import VolumeInport
from inviwopy.data import Volume
import numpy as np
import matplotlib.pyplot as plt
from matplotlib import rc

"""
The PythonScriptProcessor will run this script on construction and whenever this
it changes. Hence one needs to take care not to add ports and properties multiple times.
The PythonScriptProcessor is exposed as the local variable 'self'.
"""

if not "min" in self.properties:
	self.addProperty(FloatProperty("min", "Min", 0, 0, 10))

if not "max" in self.properties:
	self.addProperty(FloatProperty("max", "Max", 401.372, 0, 1000))

if not "numBins" in self.properties:
	self.addProperty(IntProperty("numBins", "Bins", 100, 10, 1000))

if not "fontSize" in self.properties:
	self.addProperty(IntProperty("fontSize", "Font Size", 10, 0, 50))


if not "inport" in self.inports:
	self.addInport(VolumeInport("inport"))

def process(self):
	"""
	The PythonScriptProcessor will call this process function whenever the processor process 
	function is called. The argument 'self' represents the PythonScriptProcessor.
	"""
	volume = self.inports.inport.getData()
	volumeData = volume.data.flatten()
	print(np.max(volumeData))
	print(np.min(volumeData))

	# Plot settings
	fontSize = self.properties.fontSize.value
	minValue = self.properties.min.value
	maxValue = self.properties.max.value
	numBins = self.properties.numBins.value

	rc("font", size=fontSize)

	print(volume.data.shape)

	fig, ax = plt.subplots()

	#plt.hist(volumeData, bins=1000, histtype="step")
	plt.hist(volumeData, bins=numBins, range=(minValue, maxValue))
	ax.set_xlabel('$f(\mathbf{x})$')
	ax.set_ylabel('Voxels')
	ax.spines['top'].set_visible(False)
	ax.spines['right'].set_visible(False)
	plt.ticklabel_format(axis='y', style='sci', scilimits=(-2,2))
	plt.xlim(minValue, maxValue)
	plt.tight_layout()
	plt.show()

def initializeResources(self):
	pass

# Tell the PythonScriptProcessor about the 'initializeResources' function we want to use
self.setInitializeResources(initializeResources)

# Tell the PythonScriptProcessor about the 'process' function we want to use
self.setProcess(process)