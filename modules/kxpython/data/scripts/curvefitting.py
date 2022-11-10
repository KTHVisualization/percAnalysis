import inviwopy
import numpy as np
from scipy.special import erf
from scipy.optimize import curve_fit

from inviwopy.data import Buffer

# Calculate mean squared error
def errorForFitted(yValues, yValuesFitted):
	return np.sqrt(np.mean((yValues - yValuesFitted)**2))

# Returns the real-valued solution between xMin and xMax for the polynomial given by the coefficients
def solvePolynomial(coefficients, value, xMin, xMax):
	coeffs = coefficients.copy()
	# Last coefficient is the one for x^0
	coeffs[-1] -= value
	roots = np.roots(coeffs)
	# Only take real-valued roots and explude values smaller than xMax and larger than xMin
	return roots.real[(abs(roots.imag)<1e-5) & (roots.real>=xMin) & (roots.real<=xMax)]

def fitPolynomial(xValues,yValues,degree):
	# Fitting the polynomial
	coefficients = np.polyfit(xValues, yValues, degree)
	f = np.poly1d(coefficients)

	# Estimation of pc as the point of max derivative
	coefficients1stDerivative = np.polyder(coefficients, 1)
	coefficients2ndDerivative = np.polyder(coefficients, 2)
	xMin = np.min(xValues)
	xMax = np.max(xValues)
	zeroCrossings = solvePolynomial(coefficients2ndDerivative, 0.0, xMin, xMax)
	if zeroCrossings.size > 0:
		crossingValues = np.polyval(coefficients1stDerivative, zeroCrossings)
		maxIndex = np.argmax(abs(crossingValues))
		pc = zeroCrossings[maxIndex]
		if (abs(np.polyval(coefficients,pc)-(xMax+xMin)/2.0)>0.15*(xMax-xMin)):
			print("Pc's value is quite far from half of the data: |f(pc)-0.5(xMax+xMin)|=%f"%abs(np.polyval(coefficients,pc)-0.5))
		# Check that pc is is not too far of solution for 0.5 
		SolOneHalf = solvePolynomial(coefficients, 0.5, xMin, xMax)
		if SolOneHalf.size > 0:
			if (abs(pc-SolOneHalf[0])>0.15*(xMax-xMin)):
				print("Pc is quite far from the x, where f(x)=0.5: |pc-x|=%f"%abs(pc-SolOneHalf[0]))
	else:
		print("Pc could not be estimated.")
		pc = -1

	# Estimation of delta
	SolOneTenth = solvePolynomial(coefficients, 0.1, xMin, xMax)
	SolNineTenth = solvePolynomial(coefficients, 0.9, xMin, xMax)
	if ((SolOneTenth.size > 0) and (SolNineTenth.size>0) and (pc!=-1)) :
		deltaLeft = abs(pc-SolNineTenth[0])
		deltaRight = abs(pc-SolOneTenth[0])
		delta = (deltaLeft+deltaRight)/2.0
	else:
		print("Delta could not be estimated.")
		delta = -1

	# Computation of the error
	yEstimatedValues = f(xValues)
	error = errorForFitted(yValues, yEstimatedValues)
	return f, error, coefficients, pc, delta

def erfAdapted(xValues, pc, delta):
	return (1-erf((xValues-pc)/delta))/2.0

def erfNonAdapted(xValues, pc, delta):
	return (1+erf((xValues-pc)/delta))/2.0

def fitErf(xValues, yValues):
	# Initial guess through fitting polynomical of given degree
	degree = 6
	_, errorPoly, _, pcPoly, deltaPoly = fitPolynomial(xValues,yValues,degree)
	if (pcPoly == -1):
		pcPoly = (np.max(xValues) + np.min(xValues)) / 2
	if (deltaPoly == -1):
		deltaPoly = 0.1
	initialGuess = [pcPoly, deltaPoly]

	# Fit Erf function
	if (curveFitType==1):
		solErf, pcov = curve_fit(erfAdapted, xValues, yValues, p0=initialGuess)
	else:
		solErf, pcov = curve_fit(erfNonAdapted, xValues, yValues, p0=initialGuess)
	pc = solErf[0]
	delta = solErf[1]
	def erfFitted(x):
		return erfAdapted(x, pc, delta)
	f = erfFitted

	# Computation of the error
	yEstimatedValues = f(xValues)
	error = errorForFitted(yValues, yEstimatedValues)

	if (error>errorPoly):
		print("Polynomial fit of degree %i yields lower error than fit to adapted erf: %f"%(degree,errorPoly))
	return f, error, pc, delta

# Processor is exposed as processor
# Input values are in toFitInput and toFitOutput (both buffers)
x = toFitInputBuffer.data
y = toFitOutputBuffer.data
numIterations = 1

multipleIters = processor.multipleIters.value
if (multipleIters):
	iterData = commonPerIterBuffer.data
	iterations = np.unique(iterData)
	print(iterations)
	numIterations = iterations.size
	print(numIterations)
	samplesPerIter = int(x.size / numIterations)
	print(x.size / numIterations)
	x = x.reshape((numIterations,samplesPerIter))
	x = np.unique(x, axis=0)
	print(x)
	if (x.shape[0] != 1):
		x_round = np.unique(x.round(decimals=3), axis=0)
		print(x_round.shape)
		if (x_round.shape[0] != 1):
			print("Different xValues per Iteration provided. Aborting.")
			exit()
		else:
			print("Warning: Different xValues per Iteration provided, but within tolerance.")
		#H = H[0,].reshape((1, H[0,].size))
	# Reshape x and y so that each row contains one iteration
	y = y.reshape((numIterations,samplesPerIter))
else:
	x = x.reshape((1, x.shape[0]))
	y = y.reshape((1, y.shape[0]))
	if (np.unique(x).size != x.size):
		print("X values of curve to be fitted are not unique.")
		exit()

samples = processor.samples.value
xSamples = np.zeros((numIterations, samples))
ySamples = np.zeros((numIterations, samples))

curveFitType = processor.curveFitType.value
### Fitting the curve
# polynomial
if (curveFitType==0):
	degree = processor.degree.value
	errors = np.zeros((numIterations,1))
	pcs = np.zeros((numIterations,1))
	deltas = np.zeros((numIterations,1))
	coefficientsAll = np.zeros((numIterations,degree+1))
	for index in range(0, numIterations):
		yCurr = y[index,]
		f, error, coefficients, pc, delta = fitPolynomial(x[0,],yCurr,degree)
		errors[index] = error
		pcs[index] = pc
		deltas[index] = delta
		coefficientsAll[index,] = coefficients
		# Create samples for that function
		xSamples[index,] = np.linspace(x[0,0], x[0,-1], samples)
		ySamples[index,] = f(xSamples[index,])

	# Fill buffers with those results
	for degreeIndex in range(degree, -1, -1):
		currentCoeffBuffer = globals()["coeffBuffer_%i"%degreeIndex]
		currentCoeffBuffer.size = numIterations
	pcEstimateBuffer.size = numIterations
	deltaEstimateBuffer.size = numIterations
	rmseBuffer.size = numIterations
	if multipleIters:
		commonPerIterParamsBuffer.size = numIterations
	
	for index in range(0, numIterations):
		for degreeIndex in range(degree, -1, -1):
			currentCoeffBuffer = globals()["coeffBuffer_%i"%degreeIndex]
			currentCoeffBuffer.data[index] = coefficientsAll[index,degree-degreeIndex]
		pcEstimateBuffer.data[index] = pcs[index]
		deltaEstimateBuffer.data[index] = deltas[index]
		rmseBuffer.data[index] = errors[index]
		if multipleIters:
			commonPerIterParamsBuffer.data[index] = iterations[index]

# Adapted erf
else:
	pcBuffer.size = numIterations
	deltaBuffer.size = numIterations
	rmseBuffer.size = numIterations
	if multipleIters:
		commonPerIterParamsBuffer.size = numIterations
	for index in range(0, numIterations):
		yCurr = y[index,]
		f, error, pc, delta = fitErf(x[0,],yCurr)
		pcBuffer.data[index] = pc
		deltaBuffer.data[index] = delta
		rmseBuffer.data[index] = error
		if multipleIters:
			commonPerIterParamsBuffer.data[index] = iterations[index]
		# Create samples for that function
		xSamples[index,] = np.linspace(x[0,0], x[0,-1], samples)
		ySamples[index,] = f(xSamples[index,])

### Update properties
# Set variables based on the (last) fit
processor.estimates.pcEstimate.value = pc
processor.estimates.deltaEstimate.value = delta
processor.estimates.error.value = error

### Update output
# Results for samples are bound to buffers xSamples, ySamples
xSamplesBuffer.size = xSamples.size
ySamplesBuffer.size = ySamples.size
if (multipleIters):
	commonPerIterSamplesBuffer.size = ySamples.size

for iterIndex in range(0, numIterations):
	for sampleIndex in range(samples):
		xSamplesBuffer.data[iterIndex*samples + sampleIndex] = xSamples[iterIndex, sampleIndex]
		ySamplesBuffer.data[iterIndex*samples + sampleIndex] = ySamples[iterIndex, sampleIndex]
		if (multipleIters):
			commonPerIterSamplesBuffer.data[iterIndex*samples + sampleIndex] = iterations[iterIndex]

