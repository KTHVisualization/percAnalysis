import inviwopy
import numpy
from pathlib import Path
from scipy.io import netcdf
import sys
from inviwopy.glm import dvec2, vec4, mat4
from inviwopy.data import VolumeOutport
from inviwopy.data import Volume


"""
This script is pard of the NetCDF Source processor. 
It will be run on construction and whenever this script changes. 
The PythonScriptProcessor is exposed as the local variable 'self'.
We bind functions here to onChange for properties.
"""

def getAllNCFiles(FolderPath):
    #~ Get all files in the directory with *.nc ending
    AllFilesNames = list(Path(FolderPath).glob('*.nc'))
    NumFiles = len(AllFilesNames)
    if NumFiles == 0:
        raise Exception("No files *.nc found. Check if the correct input folder is specified.")
    return AllFilesNames, NumFiles

def printAllVariables():
    FolderPath = self.properties.folder.value; 
    try:
        AllFilesNames, _ = getAllNCFiles(FolderPath)
    except Exception as e:
        print(e, file=sys.stderr)
        return
    with netcdf.netcdf_file(str(AllFilesNames[0]), 'r', mmap=False) as f:
        AllDims = list(f.dimensions)
        for var in f.variables:
            if var not in AllDims and len(f.variables[var].dimensions) > 0:
                print(str(var) + str(f.variables[var].dimensions))



def getVolumeInfo(AllFilesNames, NumFiles):
    #~ Latitude is the y-dimension. Latitudes measure the distance from the equator. Horizontal lines on the planet.
    #~ Longitude is the x-dimensions. Longitudes are all great circles, and denote the distance from the Nullmeridian / Greenwich. Vertical lines on the planet.
    #~ From the first one, gather the amount of time steps and t_min
    #~ as well as the other dimensions
    BBoxMin = [0,0,0]
    BBoxMax = [-1,-1,-1]
    Dims = [0,0,0]
    TimeStepsPerFile = 0
    DataType = 'Kayleigh'
    with netcdf.netcdf_file(str(AllFilesNames[0]), 'r', mmap=False) as f:
        current = f.variables[self.properties.variable.value]
        DataType = current.typecode()
        lat = f.variables['lat'][:]
        lon = f.variables['lon'][:]
        time = f.variables['time'][:]    
        TimeStepsPerFile = len(time)
        Dims = [len(lon), len(lat), NumFiles * TimeStepsPerFile]
        #~ If values for longtitute are given between 0E and 360E, we would want to roll that axis such that 
        #~ 0 is centered, i.e. values are between -180E and 180E
        if(self.properties.rollLongtitude.value):
            lon = numpy.roll(lon, int(Dims[0]/2))
            lon = ((lon + 180) % 360) - 180
        BBoxMin = [lon[0], lat[0], time[0]]
        BBoxMax = [lon[-1], lat[-1], -1]


    #~ From the last one, gather t_max
    with netcdf.netcdf_file(str(AllFilesNames[-1]), 'r', mmap=False) as f:
        time = f.variables['time'][:]
        BBoxMax[2] = time[-1]
        if len(time) != TimeStepsPerFile:
            raise Exception("Number of time steps is not the same in every file.")

    #~ If values are given between 90N and -90N, we need to flip latitude values, such that they are given
    #~ as ranging between -90N and 90N
    if (self.properties.flipLatitude.value):
        oldMax = BBoxMax[1]
        BBoxMax[1] = BBoxMin[1]
        BBoxMin[1] = oldMax 

    return BBoxMin, BBoxMax, Dims, TimeStepsPerFile, DataType

def printVolumeInfo():
    FolderPath = self.properties.folder.value; 
    try:
        AllFilesNames, NumFiles = getAllNCFiles(FolderPath)
    except Exception as e:
        print(e, file=sys.stderr)
        return
    try:
        BBoxMin, BBoxMax, Dims, TimeStepsPerFile, DataType = getVolumeInfo(AllFilesNames, NumFiles)
    except Exception as e:
        print(e, file=sys.stderr)
        return
    print("Number of files: " + str(NumFiles))
    print("Number of time steps per file: " + str(TimeStepsPerFile))
    print("Bounding Box: " + str(BBoxMin) + " -- " + str(BBoxMax))
    print("Dims: " + str(Dims))

def readNetCDFFiles():
    """
    Given a folder, go through all files
    """
    FolderPath = self.properties.folder.value; # Why a ; here??
    Variable = self.properties.variable.value;

    #~ Get all files
    try:
        AllFilesNames, NumFiles = getAllNCFiles(FolderPath)
    except Exception as e:
        print(e, file=sys.stderr)
        return
    #~ From the last and first file, extract volume information
    try:
        BBoxMin, BBoxMax, Dims, TimeStepPerFile, DataType = getVolumeInfo(AllFilesNames, NumFiles)
    except KeyError as e:
    	print("%s is not a variable in the nc files in the given folder. Try printing the available variables."%e, file=sys.stderr)
    	return
    except Exception as e:
        print(e, file=sys.stderr)
        return

    #~ Read all arrays from all files into a list of arrays
    Slabs = list()
    for FileName in AllFilesNames:
        with netcdf.netcdf_file(str(FileName), 'r', mmap=False) as f:
        	#~ Assuming dimensions in the input are [time, latitute, longtitute]
        	#~ and dimensions in the output are [longtitude, latitude, time]
            current = f.variables[Variable][:]
            if (self.properties.flipLatitude.value):
                current = numpy.flip(current,1)
            if (self.properties.rollLongtitude.value):
                current = numpy.roll(current, int(Dims[0]/2), axis=2)
            Slabs.append(current.transpose())

    #~ Join all those arrays into one large array
    ResVolume = numpy.concatenate(Slabs, axis=2)

    #~ This shows the shape of things
    #print("Shape: " + str(ResVolume.shape) + "  where xdim=%d, ydim=%d, zdim=%d" % tuple(Dims) )

    # Data type in netcdf is >f4
    volume = Volume(ResVolume.astype(numpy.float32))

    minValue = numpy.min(ResVolume)
    maxValue = numpy.max(ResVolume)

    volume.dataMap.dataRange = dvec2(minValue, maxValue)
    volume.dataMap.valueRange = dvec2(minValue, maxValue)

    #~ The basis fills a 3x3 matrix
    worldMatrixLine0 = vec4(BBoxMax[0] - BBoxMin[0], 0, 0, 0)
    worldMatrixLine1 = vec4(0, BBoxMax[1] - BBoxMin[1], 0, 0)
    worldMatrixLine2 = vec4(0, 0, BBoxMax[2] - BBoxMin[2], 0)
    worldMatrixLine3 = vec4(BBoxMin[0], BBoxMin[1], BBoxMin[2], 1)
    #~ The offset is the lower left corner and is within the last line (here: columns first)
    volume.modelMatrix = mat4(worldMatrixLine0, worldMatrixLine1, worldMatrixLine2, worldMatrixLine3)

    # Set as the outport
    self.outports.outVolume.setData(volume)

# Tell the NetCDFSourceProcessor about the function we want to call in case things change

self.properties.folder.onChange(readNetCDFFiles)
self.properties.variable.onChange(readNetCDFFiles)
self.properties.flipLatitude.onChange(readNetCDFFiles)
self.properties.rollLongtitude.onChange(readNetCDFFiles)

self.properties.printVariables.onChange(printAllVariables)
self.properties.printVolumeInfo.onChange(printVolumeInfo)

#readNetCDFFiles()



