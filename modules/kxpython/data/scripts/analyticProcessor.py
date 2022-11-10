import inviwopy
from inviwopy.glm import ivec3, vec3, dvec2, vec4, mat4
from inviwopy.properties import IntVec3Property, CompositeProperty, FloatVec3Property
from inviwopy.data import VolumeOutport
from inviwopy.data import Volume
import numpy as np

"""
This script is meant to be used together with a general PythonScriptProcessor
A PythonScriptProcessor with the path of this script as its script property will run 
this script on construction and whenever this it changes. Hence one needs to take care 
not to add ports and properties multiple times.
The PythonScriptProcessor is exposed as the local variable 'self'.
"""

def analyticFunction(Dims, BBoxMin, BBoxMax):
	"""
	Fill a 3D volume based on dimensions and bounding box.
	"""
	# Create a numpy volume with the dimensions from the property

	ResVolume = np.empty([Dims[0], Dims[1], Dims[2]], dtype='f', order='F')

	# Example: Take the product of all coordinates

	xAxis = np.linspace(BBoxMin[0], BBoxMax[0], Dims[0])
	yAxis = np.linspace(BBoxMin[1], BBoxMax[1], Dims[1])
	zAxis = np.linspace(BBoxMin[2], BBoxMax[2], Dims[2])

	for k in range(Dims[2]):
		for j in range(Dims[1]):
			for i in range(Dims[0]):
				x = xAxis[i]
				y = yAxis[j]
				z = zAxis[k]
				# The actual formula of the scalar field
				# Could be anything else
				ResVolume[i][j][k] = x*y*z

	minValue = np.min(ResVolume)
	maxValue = np.max(ResVolume)

	return ResVolume, minValue, maxValue

def setModelMatrix(BBoxMin, BBoxMax, volume):
	#~ The basis fills a 3x3 matrix
	worldMatrixLine0 = vec4(BBoxMax[0] - BBoxMin[0], 0, 0, 0)
	worldMatrixLine1 = vec4(0, BBoxMax[1] - BBoxMin[1], 0, 0)
	worldMatrixLine2 = vec4(0, 0, BBoxMax[2] - BBoxMin[2], 0)
	worldMatrixLine3 = vec4(BBoxMin[0], BBoxMin[1], BBoxMin[2], 1)
	#~ The offset is the lower left corner and is within the last line (here: columns first)
	volume.modelMatrix = mat4(worldMatrixLine0, worldMatrixLine1, worldMatrixLine2, worldMatrixLine3)

"""
Add properties and ports
"""

# Add dimension property
if not "dim" in self.properties:
	self.addProperty(IntVec3Property("dim", "Dimensions", ivec3(5), ivec3(0), ivec3(20)))

if not "bBox" in self.properties:
	self.addProperty(CompositeProperty("bBox", "Bounding Box"))
	self.properties.bBox.addProperty(FloatVec3Property("bBoxMin", "Min", vec3(-1.0), vec3(-20.0), vec3(20)))
	self.properties.bBox.addProperty(FloatVec3Property("bBoxMax", "Max", vec3(1), vec3(-20.0), vec3(20)))

# Add volume output
if not "volumeOutport" in self.outports:
	self.addOutport(VolumeOutport("volumeOutport"))


"""
The PythonScriptProcessor needs a process function similar to a regular processor.
"""

def process(self):
	"""
	The PythonScriptProcessor will call this process function whenever the processor process 
	function is called. The argument 'self' represents the PythonScriptProcessor.
	"""
	Dims = self.properties.dim.value; # Why a ; here??
	BBoxMin = self.properties.bBox.bBoxMin.value;
	BBoxMax = self.properties.bBox.bBoxMax.value;

	ResVolume, minValue, maxValue = analyticFunction(Dims, BBoxMin, BBoxMax)

	volume = Volume(ResVolume)
	volume.dataMap.dataRange = dvec2(minValue, maxValue)
	volume.dataMap.valueRange = dvec2(minValue, maxValue)

	setModelMatrix(BBoxMin, BBoxMax, volume)

	# Set as the outport
	self.outports.volumeOutport.setData(volume)

def initializeResources(self):
	pass

# Tell the PythonScriptProcessor about the 'initializeResources' function we want to use
self.setInitializeResources(initializeResources)

# Tell the PythonScriptProcessor about the 'process' function we want to use
self.setProcess(process)