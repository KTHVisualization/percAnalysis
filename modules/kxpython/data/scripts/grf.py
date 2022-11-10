import numpy as np
from inviwopy.data import *
from inviwopy.glm import dvec2

# Adapted from https://mathematica.stackexchange.com/questions/4829/efficiently-generating-n-d-gaussian-random-fields

def fftIndices(n):
    a = list(range(0, int(n/2)+1))
    b = list(range(1, int(n/2)))
    b.reverse()
    b = [-i for i in b]
    return a + b

#powerspectrum Pk k^-alpha
def Pk(k, alpha):
    return k**(-alpha)

def Pk2(kx, ky, alpha):
    if kx == 0 and ky == 0:
        return 0.0
    return np.sqrt(Pk(np.sqrt(kx**2 + ky**2), alpha))

def Pk3(kx, ky, kz, alpha):
    if kx == 0 and ky == 0 and kz==0:
        return 0.0
    return np.sqrt(Pk(np.sqrt(kx**2 + ky**2 + kz**2), alpha))

def grf2d(alpha, size):
    noise = np.fft.fft2(np.random.normal(size = (size, size)))
    amplitude = np.zeros((size,size))
    for i, kx in enumerate(fftIndices(size)):
        for j, ky in enumerate(fftIndices(size)):            
            amplitude[i, j] = Pk2(kx, ky, alpha)
    return np.fft.ifft2(noise * amplitude)

def grf3d(alpha, size):
    print("test3d")
    noise = np.fft.fftn(np.random.normal(size = (size, size, size)))
    amplitude = np.zeros((size,size,size))
    for i, kx in enumerate(fftIndices(size)):
        for j, ky in enumerate(fftIndices(size)):
            for k, kz in enumerate(fftIndices(size)):           
                amplitude[i, j, k] = Pk3(kx, ky, kz, alpha)
    return np.fft.ifftn(noise * amplitude)

data = volume.data
size = processor.volSize.value 
seed = processor.seed.value
np.random.seed(seed)

alpha = processor.alpha.value
if (processor.dimType.value == 0):
    grf = grf2d(alpha, size)
    for (index,v) in np.ndenumerate(data):
        volume.data[index[0],index[1],index[2]] = grf.real[index[0],index[1]]
else:
    grf = grf3d(alpha, size)
    for (index,v) in np.ndenumerate(data):
        volume.data[index[0],index[1],index[2]] = grf.real[index[0],index[1],index[2]]

xMin = np.min(grf.real)
xMax = np.max(grf.real)
volume.dataMap.dataRange = dvec2(xMin,xMax)
volume.dataMap.valueRange = dvec2(xMin,xMax)