import inviwopy
import numpy as np
from scipy.special import erf
import matplotlib.pyplot as plt
from matplotlib import rc
from mpl_toolkits import axes_grid1
from matplotlib.ticker import FuncFormatter
from inviwopy.data import Buffer

#rc("font", family="Times New Roman")
#rc("font", size=8)

def erfAdapted(xValues, pc, delta):
	return (1-erf((xValues-pc)/delta))/2.0

def erfNonAdapted(xValues, pc, delta):
	return (1+erf((xValues-pc)/delta))/2.0

def getPixelPos(values, valueMin, valueMax, numPixels):
	return (values - valueMin) / (valueMax-valueMin) * (numPixels-1) + 0

def getValueFromPixelPos(values, valueMin, valueMax, numPixels):
	return values / (numPixels-1) * (valueMax-valueMin) + valueMin

def add_colorbar(im, aspect=20, pad_fraction=0.5, **kwargs):
    """Add a vertical color bar to an image plot."""
    divider = axes_grid1.make_axes_locatable(im.axes)
    width = axes_grid1.axes_size.AxesY(im.axes, aspect=1./aspect)
    pad = axes_grid1.axes_size.Fraction(pad_fraction, width)
    current_ax = plt.gca()
    cax = divider.append_axes("right", size=width, pad=pad)
    plt.sca(current_ax)
    return im.axes.figure.colorbar(im, cax=cax, **kwargs)

# Processor is exposed as processor
# Input values are in hValues, outValues, curveIters (all buffers, access with .data, .size)
H = hValues.data
value = outValues.data
pc = pcValues.data
delta = deltaValues.data

inputType = processor.inputType.value

# Single curve
if (inputType == 0):
	H = H.reshape((1, H.shape[0]))
	value = value.reshape((1, value.shape[0]))
# Multiple curves, but we choose one
elif (inputType == 1):
	iteration = processor.iteration.value
	iterData = curveIters.data
	if (iteration > np.max(iterData) or iteration < np.min(iterData)):
		print("Iteration %i does not occur in the data. Aborting.")
		exit()
	# Get indices where iterData has value of the selected iteration
	indicesForIter = np.nonzero((iterData > iteration-1) & (iterData < iteration+1) )
	H = H[indicesForIter]
	H = H.reshape((1, H.shape[0]))
	value = value[indicesForIter]
	value = value.reshape((1, value.shape[0]))
# Two curves: Non-Shuffled and shuffled
else:
	print("Non-Shuffled vs. shuffled is not implemented. Aborting.")
	exit()
	H = x.reshape((2, x.shape[0]/2))
	H = np.unique(H, axis=0)
	if (H.shape[0] != 1):
		print("Different xValues per Iteration provided. Aborting.")
		exit()
	value = y.reshape((2, y.shape[0]/2))

# Flip if we have decreasing values in H (should not make a difference for a single plot??)
if (H[0,0] > H[0,1]):
	H = np.flip(H,1)
	value = np.flip(value,1)

rc("font", size=processor.plot.fontSize.value)
fig, ax = plt.subplots()

computationOrder = processor.plot.computationOrder.value
showError = processor.plot.showError.value
showFitted = processor.plot.showFitted.value	

HMin = H[0,0]
HMax = H[0,-1]

def formatXticks(x, pos):
	return '{0:.1f}'.format(x)

def formatYTicksIters(y, pos):
	yVal = getValueFromPixelPos(y, np.min(iterations), np.max(iterations), iterations.size)
	return '%i' % yVal

def formatYTicksOverwritten(y, pos):
	yMin = processor.plot.iters.minMax.value[0]
	yMax = processor.plot.iters.minMax.value[1]
	yVal = getValueFromPixelPos(y, yMin, yMax, iterations.size)
	return '{0:.2f}'.format(yVal)

#ax.set_yticks(np.arange(iterations.size)[::25])
# ... and label them with the respective list entries
numXTicks = 6
ax.xaxis.set_major_formatter(FuncFormatter(formatXticks))
#ax.xaxis.set_ticks(np.linspace(0, H.size-1, numXTicks))


ax.spines['top'].set_visible(False)
ax.spines['right'].set_visible(False)
plt.ylim(0, 1.01)
plt.xlim(0, 1.01)

if (processor.plot.valueBased.value):
	inputString = "\overline{p}"
else:
	inputString = "p"

ax.set_xlabel("$%s$"%inputString)

plt.plot(H[0,], value[0,], marker=".", linewidth=1, markersize=6, linestyle='dashed', label="$P_{\mathrm{max}}(%s)$"%inputString, zorder=6)
#plt.scatter(H[0,], value[0,], c=value[0,], marker=".", linewidth=1, label="$P_{\mathrm{max}}(%s)$"%inputString, zorder=6)

index=0
if (inputType == 1):
	paramIters = paramIters.data
	index = np.nonzero((paramIters > iteration-1) & (paramIters < iteration+1))

if (showFitted or showError):
	if (computationOrder):
		fittedValues = erfNonAdapted(H[0,], pc[index], delta[index])
	else:
		fittedValues = erfAdapted(H[0,], pc[index], delta[index])
	plt.plot(H[0,], fittedValues, linewidth=2, color="darkgray", label="$P_{\mathrm{erf}}(%s)$"%inputString, zorder=5)
	#plt.plot(H[0,], fittedValues, linewidth=2, label="$P_{erf}(%s, p_c, \Delta)$"%inputString, zorder=5)

if (showError):
	error = np.abs(value-fittedValues)
	plt.plot(H[0,], error[0,], marker=".", linewidth=1, color='red', markersize=4, linestyle='dashed', label="$|P_{\mathrm{max}}(%s)-P_{\mathrm{erf}}(%s)|$"%(inputString, inputString), zorder=3)

if (processor.plot.enableOverlayPc.value):
	plt.axvline(x=pc[index], color='k', linestyle='--', zorder=2)
	annotatePos= pc[index]-0.1 if computationOrder else pc[index]+0.1
	ax.annotate('$p_c$', xy=(pc[index], 1), xytext=(annotatePos, 1.0-0.05),
	           arrowprops=dict(facecolor='black', arrowstyle="->"))
	print("Pc", pc[index])

if (processor.plot.enableOverlayDelta.value):
	plt.axvspan(pc[index]-delta[index], pc[index]+delta[index], facecolor='gray', alpha=0.25, label="$[p_c-\Delta, p_c+\Delta]$", zorder=1)
	print("Pc-Delta" ,pc[index]-delta[index])
	print("Pc+Delta", pc[index]+delta[index])
	print("Delta", delta[index])

# Show 2D pc
if (processor.plot.show2dPc.value):
	pc2d = 0.5927
	if (computationOrder == 0):
		pc2d = 1.0 - pc2d
	plt.axvline(x=pc2d, color='k', linestyle='--', zorder=2)
	annotatePos= pc2d-0.15 if computationOrder else pc2d+0.1
	ax.annotate('$p^{2D}_c$', xy=(pc2d, 1), xytext=(annotatePos, 1.0-0.05), arrowprops=dict(facecolor='black', arrowstyle="->"))

if (processor.plot.show3dPc.value):
	pc3d = 0.3116
	if (computationOrder == 0):
		pc3d = 1.0 - pc3d
	plt.axvline(x=pc3d, color='k', linestyle='--', zorder=2)
	annotatePos= pc3d-0.15 if computationOrder else pc3d+0.1
	ax.annotate('$p^{3D}_c$', xy=(pc3d, 1), xytext=(annotatePos, 1.0-0.05), arrowprops=dict(facecolor='black', arrowstyle="->"))

if (computationOrder == 0):
	ax.legend(frameon=False, loc='lower left',ncol=1,handlelength=2)
else:
	ax.legend(frameon=False, loc='lower right',ncol=1,handlelength=2)

#fig = plt.gcf()
width  = 4.6072 #textwidth
#width = 2.22055 #twopicwidth
#width = 1.42499 #threepicwith
scale_height = 0.8
height = scale_height*width/1.61803398875
#import time
#timestr = time.strftime("%Y%m%d-%H%M%S")
#fig.savefig(r"C:\\Inviwo\\output-inviwo-dd\\apps\\inviwo\\video\\curve%s.png" %timestr, dpi=300)
fig.set_size_inches(width, height)
plt.tight_layout()
plt.show()