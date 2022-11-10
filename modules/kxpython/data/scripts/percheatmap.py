import inviwopy
import numpy as np
from scipy.special import erf
import matplotlib.pyplot as plt
from matplotlib import rc
from mpl_toolkits import axes_grid1

from matplotlib.ticker import FuncFormatter
from inviwopy.data import Buffer

#rc("font", family="Times New Roman")

def erfAdapted(xValues, pc, delta):
	return (1-erf((xValues-pc)/delta))/2.0

def erfNonAdapted(xValues, pc, delta):
	return (1+erf((xValues-pc)/delta))/2.0

def getPixelPos(values, valueMin, valueMax, numPixels):
	return (values - valueMin) / (valueMax-valueMin) * (numPixels-1) + 0

def getValueFromPixelPos(values, valueMin, valueMax, numPixels):
	return values / (numPixels-1) * (valueMax-valueMin) + valueMin

def add_colorbar(im, ax, aspect=20, pad_fraction=0.5, **kwargs):
    """Add a vertical color bar to an image plot."""
    divider = axes_grid1.make_axes_locatable(im.axes)
    width = axes_grid1.axes_size.AxesY(im.axes, aspect=1./aspect)
    pad = axes_grid1.axes_size.Fraction(pad_fraction, width)
    current_ax = ax
    cax = divider.append_axes("right", size=width, pad=pad)
    plt.sca(current_ax)
    return im.axes.figure.colorbar(im, cax=cax, **kwargs)


# Processor is exposed as processor
# Input values are in hValues, outValues, curveIters (all buffers, access with .data, .size)
H = hValues.data
value = outValues.data
iterData = curveIters.data

# Extract iterations parameter and reshape H and value accordingly
iterations = np.unique(iterData)
numIterations = iterations.size
samplesPerIter = int(H.size / numIterations)
H = H.reshape((numIterations,samplesPerIter))
H = np.unique(H, axis=0)
if (H.shape[0] != 1):
	H_round = np.unique(H.round(decimals=3), axis=0)
	print(H_round.shape)
	if (H_round.shape[0] != 1):
		print("Different xValues per Iteration provided. Aborting.")
		exit()
	else:
		print("Warning: Different xValues per Iteration provided, but within tolerance.")
		H = H[0,]
		H = H.reshape((1, H.size))

print(H.shape)

value = value.reshape((numIterations,samplesPerIter))

# Flip if we have decreasing values in H
#if (H[0,0] > H[0,1]):
#	H = np.flip(H,1)
#	value = np.flip(value,1)

transpose = processor.plot.transpose.value
if (transpose):
	value = np.transpose(value)
	copy = iterations.copy()
	iterations = H[0,]
	H = copy
	H = H.reshape((1, H.size))


rc("font", size=processor.plot.fontSize.value)
fig, ax = plt.subplots()
print(plt.rcParams['figure.figsize'])
width=6.4
height=3.2
#width  = 4.6072 #textwidth
#width = 2.22055 #twopicwidth
#width = 1.42499 #threepicwith
#scale_height = 1.0
#height = scale_height*width*1.61803398875
fig.set_size_inches(width, height)
#fig.savefig('test2png.pdf', dpi=300)

showError = processor.plot.showError.value

if (showError):
	pc = pcValues.data
	delta = deltaValues.data
	fittedValues = np.zeros(value.shape)
	# Fit values with given parameters 
	# Todo: (could be more careful with iterations parameter here)
	for index in range(iterations.size):
		fittedValues[index,] = erfAdapted(H[0,], pc[index], delta[index])
	value = np.abs(value-fittedValues)
	im = ax.imshow(value, cmap=plt.cm.inferno, aspect=0.5, interpolation='none', origin='lower', zorder=1)
else:
	im = ax.imshow(value, cmap=plt.cm.viridis, aspect=0.5, vmin=0, vmax=1, interpolation='none', origin='lower', zorder=1)

#cbar = add_colorbar(im, ax)
#cbar.ax.set_ylabel('$|P_{\mathrm{max}}(%s)-P_{\mathrm{max}}(%s)|$'%(inputString,inputString) if showError else '$P_{\mathrm{max}}(%s)$'%inputString)

HMin = H[0,0]
HMax = H[0,-1]

if (processor.plot.valueBased.value):
	inputString = "\overline{p}"
else:
	inputString = "p"

def formatXticks(x, pos):
	xVal = getValueFromPixelPos(x, HMin, HMax, H.size)
	return '{0:.2f}'.format(xVal)

def formatYTicksIters(y, pos):
	yVal = getValueFromPixelPos(y, np.min(iterations), np.max(iterations), iterations.size)
	return '%i' % yVal

def formatYTicksOverwritten(isFloat, y, pos):
	yMin = processor.plot.iters.minMax.value[0]
	yMax = processor.plot.iters.minMax.value[1]
	yVal = getValueFromPixelPos(y, yMin, yMax, iterations.size)
	return '{0:.2f}'.format(yVal) if isFloat else '%i' % yVal

#ax.set_yticks(np.arange(iterations.size)[::25])
# ... and label them with the respective list entries
numXTicks = 6
ax.xaxis.set_major_formatter(FuncFormatter(formatXticks))
ax.xaxis.set_ticks(np.linspace(0, H.size-1, numXTicks))
ax.set_xlabel("$%s$"%inputString)
if (transpose):
	ax.set_xlabel("t")
	ax.xaxis.set_major_formatter(FuncFormatter(lambda x, p: '%i' % x ))
	ax.xaxis.set_ticks(np.linspace(0, H.size-1, numXTicks))

if (processor.plot.iters.overwrite.value):
	if processor.plot.iters.isFloat.value == 0:
		ax.yaxis.set_major_formatter(FuncFormatter(lambda x, p: formatYTicksOverwritten(False,x,p)))
	else:
		ax.yaxis.set_major_formatter(FuncFormatter(lambda x, p: formatYTicksOverwritten(True,x,p)))
	ax.set_ylabel(processor.plot.iters.name.value)
else:
	ax.yaxis.set_major_formatter(FuncFormatter(formatYTicksIters))
	ax.set_ylabel("Iteration")
numYTicks = 6
# For 2d-3d
if (transpose):
	ax.set_ylabel("$%s$"%inputString)
	ax.yaxis.set_major_formatter(FuncFormatter(lambda x, p: '{0:.2f}'.format(getValueFromPixelPos(x, iterations[0], iterations[-1], iterations.size))))
print(np.linspace(0, iterations.size-3, numYTicks))
if processor.plot.iters.isFloat.value == 0:
	ax.yaxis.set_ticks(np.linspace(0, iterations.size-3, numYTicks))
else:
	ax.yaxis.set_ticks(np.linspace(0, iterations.size-1, numYTicks))

if (processor.plot.enableOverlayPc.value):
	pc = pcValues.data
	pcPixel = getPixelPos(pc, HMin, HMax, H.size)
	ax.annotate('$p_c$', xy=(pcPixel[-1], iterations[-1]), xytext=(pcPixel[-1]+H.size*0.04, iterations[-1]+int(iterations.size*0.05)),
	            arrowprops=dict(facecolor='black', arrowstyle="->"))
	plt.plot(pcPixel, iterations, zorder=2, color=('white' if showError else 'black'))
	plt.ylim(iterations[0], iterations[-1])
if (processor.plot.enableOverlayDelta.value):
	pc = pcValues.data
	delta = deltaValues.data
	pcMinusDelta = getPixelPos(pc-delta, HMin, HMax, H.size)
	pcPlusDelta = getPixelPos(pc+delta, HMin, HMax, H.size)
	ax.annotate('$p_c-\Delta$', xy=(pcMinusDelta[-1], iterations[-1]), xytext=(pcMinusDelta[-1]-H.size*0.1, iterations[-1]+int(iterations.size*0.05)),
	            arrowprops=dict(facecolor='black', arrowstyle="->"))
	plt.plot(pcMinusDelta, iterations, zorder=2, color=('white' if showError else 'black'))
	ax.annotate('$p_c+\Delta$', xy=(pcPlusDelta[-1], iterations[-1]), xytext=(pcPlusDelta[-1]+H.size*0.05, iterations[-1]+int(iterations.size*0.05)),
	            arrowprops=dict(facecolor='black', arrowstyle="->"))
	plt.plot(pcPlusDelta, iterations, zorder=2, color=('white' if showError else 'black'))
	plt.ylim(iterations[0], iterations[-1])
	plt.xlim(0, H.size-1)

if (processor.plot.show2dPc.value):
	pc2d = 0.5927
	if (processor.plot.computerationOrder.value == 0):
		pc2d = 1 - pc2d
	pc2d = getPixelPos(pc2d, HMin, HMax, H.size)
	plt.axvline(x=pc2d, color='k', linestyle='--', zorder=2)
	ax.annotate('$p^{2D}_c\approx 0.5927$', xy=(pc2d, iterations[4]), xytext=(pc2d-H.size*0.08, iterations[4]+int(iterations.size*0.1)),
		            arrowprops=dict(facecolor='black', arrowstyle="->"))
if (processor.plot.show3dPc.value):
	pc3d = 0.3116
	if (processor.plot.computerationOrder.value == 0):
		pc3d = 1 - pc3d
	pc3d = getPixelPos(pc3d, HMin, HMax, H.size)
	plt.axvline(x=pc3d, color='k', linestyle='--', zorder=2)
	ax.annotate('$p^{3D}_c\approx 0.3116$', xy=(pc3d, iterations[-1]), xytext=(pc3d-H.size*0.08, iterations[-1]+int(iterations.size*0.05)),
		            arrowprops=dict(facecolor='black', arrowstyle="->"))


plt.tight_layout()
plt.show()

#import tikzplotlib
#tikzplotlib.save("heatmap_new.tex")

if processor.plot.separateParams.value:
	pc = pcValues.data
	delta = deltaValues.data
	rmse = rmseValues.data
	figParams, axParams = plt.subplots()	
	plt.plot(iterations, pc, label="$p_c$", zorder=3)	
	plt.plot(iterations, delta, color="C2", label="$\Delta$", zorder=3)
	plt.plot(iterations, rmse, color="C3", label="$RMSE(P_{\mathrm{erf}})$", zorder=3)
	axParams.legend(frameon=False, loc='upper left',ncol=3,handlelength=2)
	axParams.spines['top'].set_visible(False)
	axParams.spines['right'].set_visible(False)
	#plt.ylim(0, 1)
	plt.xlim(iterations[0], iterations[-1])
	if (processor.plot.iters.overwrite.value):
		if processor.plot.iters.isFloat.value == 0:
			axParams.xaxis.set_major_formatter(FuncFormatter(lambda x, p: formatYTicksOverwritten(False,x,p)))
		else:
			axParams.xaxis.set_major_formatter(FuncFormatter(lambda x, p: formatYTicksOverwritten(True,x,p)))
		axParams.set_xlabel(processor.plot.iters.name.value)
	else:
		axParams.xaxis.set_major_formatter(FuncFormatter(formatYTicksIters))
		axParams.set_xlabel("Iteration")
	numYTicks = 6
	# For 2d-3d
	print(np.linspace(0, iterations.size-3, numYTicks))
	if processor.plot.iters.isFloat.value == 0:
		axParams.xaxis.set_ticks(np.linspace(0, iterations.size-3, numYTicks))
	else:
		axParams.xaxis.set_ticks(np.linspace(0, iterations.size-1, numYTicks))
	plt.show()