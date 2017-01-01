/* Copyright 2016 Kristofer Björnson
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

/** @file DPropertyExtractor.cpp
 *
 *  @author Kristofer Björnson
 */

#include "../../../include/PropertyExtractors/APropertyExtractor/APropertyExtractor.h"
#include "Functions.h"
#include "Streams.h"

using namespace std;

namespace TBTK{

namespace{
	complex<double> i(0,1);
}

APropertyExtractor::APropertyExtractor(ArnoldiSolver *aSolver){
	this->aSolver = aSolver;
}

APropertyExtractor::~APropertyExtractor(){
}

Property::EigenValues* APropertyExtractor::getEigenValues(){
	int size = aSolver->getNumEigenValues();
	const complex<double> *ev = aSolver->getEigenValues();

	Property::EigenValues *eigenValues = new Property::EigenValues(size);
	for(int n = 0; n < size; n++)
		eigenValues->data[n] = real(ev[n]);

	return eigenValues;
}

Property::DOS* APropertyExtractor::calculateDOS(){
	const complex<double> *ev = aSolver->getEigenValues();

	Property::DOS *dos = new Property::DOS(lowerBound, upperBound, energyResolution);
	for(int n = 0; n < aSolver->getNumEigenValues(); n++){
		int e = (int)(((real(ev[n]) - lowerBound)/(upperBound - lowerBound))*energyResolution);
		if(e >= 0 && e < energyResolution){
			dos->data[e] += 1.;
		}
	}

	return dos;
}

Property::LDOS* APropertyExtractor::calculateLDOS(
	Index pattern,
	Index ranges
){
	//hint[0] is an array of doubles, hint[1] is an array of ints
	//hint[0][0]: upperBound
	//hint[0][1]: lowerBound
	//hint[1][0]: resolution
	//hint[1][1]: spin_index
	hint = new void*[2];
	((double**)hint)[0] = new double[2];
	((int**)hint)[1] = new int[1];
	((double**)hint)[0][0] = upperBound;
	((double**)hint)[0][1] = lowerBound;
	((int**)hint)[1][0] = energyResolution;

	ensureCompliantRanges(pattern, ranges);

	int lDimensions;
	int *lRanges;
	getLoopRanges(pattern, ranges, &lDimensions, &lRanges);
	Property::LDOS *ldos = new Property::LDOS(
		lDimensions,
		lRanges,
		lowerBound,
		upperBound,
		energyResolution
	);

	calculate(
		calculateLDOSCallback,
		(void*)ldos->data,
		pattern,
		ranges,
		0,
		1
	);

	return ldos;
}

Property::SpinPolarizedLDOS* APropertyExtractor::calculateSpinPolarizedLDOS(
	Index pattern,
	Index ranges
){
	//hint[0] is an array of doubles, hint[1] is an array of ints
	//hint[0][0]: upperBound
	//hint[0][1]: lowerBound
	//hint[1][0]: resolution
	//hint[1][1]: spin_index
	hint = new void*[2];
	((double**)hint)[0] = new double[2];
	((int**)hint)[1] = new int[2];
	((double**)hint)[0][0] = upperBound;
	((double**)hint)[0][1] = lowerBound;
	((int**)hint)[1][0] = energyResolution;

	((int**)hint)[1][1] = -1;
	for(unsigned int n = 0; n < pattern.size(); n++){
		if(pattern.at(n) == IDX_SPIN){
			((int**)hint)[1][1] = n;
			pattern.at(n) = 0;
			ranges.at(n) = 1;
			break;
		}
	}
	if(((int**)hint)[1][1] == -1){
		Streams::err << "Error in PropertyExtractor::calculateSpinPolarizedLDOS: No spin index indicated.\n";
		delete [] ((double**)hint)[0];
		delete [] ((int**)hint)[1];
		delete [] (void**)hint;
		return NULL;
	}

	ensureCompliantRanges(pattern, ranges);

	int lDimensions;
	int *lRanges;
	getLoopRanges(pattern, ranges, &lDimensions, &lRanges);
	Property::SpinPolarizedLDOS *spinPolarizedLDOS = new Property::SpinPolarizedLDOS(
		lDimensions,
		lRanges,
		lowerBound,
		upperBound,
		energyResolution
	);

	calculate(
		calculateSpinPolarizedLDOSCallback,
		(void*)spinPolarizedLDOS->data,
		pattern,
		ranges,
		0,
		1
	);

	delete [] ((double**)hint)[0];
	delete [] ((int**)hint)[1];
	delete [] (void**)hint;

	return spinPolarizedLDOS;
}

void APropertyExtractor::calculateLDOSCallback(
	PropertyExtractor *cb_this,
	void *ldos,
	const Index &index,
	int offset
){
	APropertyExtractor *pe = (APropertyExtractor*)cb_this;

	const complex<double> *eigenValues = pe->aSolver->getEigenValues();

	double u_lim = ((double**)pe->hint)[0][0];
	double l_lim = ((double**)pe->hint)[0][1];
	int resolution = ((int**)pe->hint)[1][0];

	double step_size = (u_lim - l_lim)/(double)resolution;

	for(int n = 0; n < pe->aSolver->getNumEigenValues(); n++){
		if(real(eigenValues[n]) > l_lim && real(eigenValues[n]) < u_lim){
			complex<double> u = pe->aSolver->getAmplitude(n, index);

			int e = (int)((real(eigenValues[n]) - l_lim)/step_size);
			if(e >= resolution)
				e = resolution-1;
			((double*)ldos)[resolution*offset + e] += real(conj(u)*u);
		}
	}
}

void APropertyExtractor::calculateSpinPolarizedLDOSCallback(
	PropertyExtractor *cb_this,
	void *sp_ldos,
	const Index &index,
	int offset
){
	APropertyExtractor *pe = (APropertyExtractor*)cb_this;

	const complex<double> *eigenValues = pe->aSolver->getEigenValues();

	double u_lim = ((double**)pe->hint)[0][0];
	double l_lim = ((double**)pe->hint)[0][1];
	int resolution = ((int**)pe->hint)[1][0];
	int spin_index = ((int**)pe->hint)[1][1];

	double step_size = (u_lim - l_lim)/(double)resolution;

	Index index_u(index);
	Index index_d(index);
	index_u.at(spin_index) = 0;
	index_d.at(spin_index) = 1;
	for(int n = 0; n < pe->aSolver->getNumEigenValues(); n++){
		if(real(eigenValues[n]) > l_lim && real(eigenValues[n]) < u_lim){
			complex<double> u_u = pe->aSolver->getAmplitude(n, index_u);
			complex<double> u_d = pe->aSolver->getAmplitude(n, index_d);

			int e = (int)((real(eigenValues[n]) - l_lim)/step_size);
			if(e >= resolution)
				e = resolution-1;
			((complex<double>*)sp_ldos)[4*resolution*offset + 4*e + 0] += conj(u_u)*u_u;
			((complex<double>*)sp_ldos)[4*resolution*offset + 4*e + 1] += conj(u_u)*u_d;
			((complex<double>*)sp_ldos)[4*resolution*offset + 4*e + 2] += conj(u_d)*u_u;
			((complex<double>*)sp_ldos)[4*resolution*offset + 4*e + 3] += conj(u_d)*u_d;
		}
	}
}

};	//End of namespace TBTK
