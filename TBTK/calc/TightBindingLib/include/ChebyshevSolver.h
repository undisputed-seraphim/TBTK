/** @package TBTKcalc
 *  @file ChebyshevSolver.h
 *  @brief Solves a Model using the Chebyshev method
 *
 *  Based on PhysRevLett.105.167006
 *
 *  @author Kristofer Björnson
 */

#ifndef COM_DAFER45_TBTK_CHEBYSHEV_SOLVER
#define COM_DAFER45_TBTK_CHEBYSHEV_SOLVER

#include "Model.h"
#include <complex>

/** The ChebyshevSolver can be used to calculate Green's function for a given
 *  Model. The implementation is based on PhysRevLett.105.167006. The
 *  ChebyshevSolver can be run on CPU, GPU, or a mixture of both. The
 *  calculation of Chebyshev coefficients scales as \f$O(n)\f$ with each of the
 *  following: dimension of the Hilbert space and number of Chebyshev
 *  coefficients. The generation of Green's functions scales as \f$O(n)\f$ with
 *  the following: Number of coefficients, energy resolution, and the number of
 *  Green's functions.
 */
class ChebyshevSolver{
public:
	/** Constructor. */
	ChebyshevSolver();

	/** Destructor. */
	~ChebyshevSolver();

	/** Set model to work on. */
	void setModel(Model *model);

	/** Calculates the Chebyshev coefficients for \f$ G_{ij}(E)\f$, where
	 *  \f$i = \textrm{to}\f$ and \f$j = \textrm{from}\f$. Runs on CPU.
	 *  @param to 'To'-index, or \f$i\f$.
	 *  @param from 'From'-index, or \f$j\f$.
	 *  @param coefficients Pointer to array able to hold numCoefficients coefficients.
	 *  @param numCoefficients Number of coefficients to calculate.
	 *  @param broadening Broadening to use in convolusion of coefficients
	 *  to remedy Gibb's osciallations.
	 */
	void calculateCoefficients(Index to, Index from, std::complex<double> *coefficients, int numCoefficients, double broadening = 0.0001);

	/** Calculates the Chebyshev coefficients for \f$ G_{ij}(E)\f$, where
	 *  \f$i = \textrm{to}\f$ is a set of indices and \f$j =
	 *  \textrm{from}\f$. Runs on GPU.
	 *  @param to vector of 'to'-indeces, or \f$i\f$'s.
	 *  @param from 'From'-index, or \f$j\f$.
	 *  @param coefficients Pointer to array able to hold
	 *  numCoefficients\f$\times\f$toIndeices.size() coefficients.
	 *  @param numCoefficients Number of coefficients to calculate for each
	 *  to-index.
	 *  @param broadening Broadening to use in convolusion of coefficients
	 *  to remedy Gibb's osciallations.
	 */
	void calculateCoefficientsGPU(std::vector<Index> &to, Index from, std::complex<double> *coefficients, int numCoefficients, double broadening = 0.0001);

	/** Calculates the Chebyshev coefficients for \f$ G_{ij}(E)\f$, where
	 *  \f$i = \textrm{to}\f$ and \f$j = \textrm{from}\f$. Runs on GPU.
	 *  @param to 'To'-index, or \f$i\f$.
	 *  @param from 'From'-index, or \f$j\f$.
	 *  @param coefficients Pointer to array able to hold numCoefficients coefficients.
	 *  @param numCoefficients Number of coefficients to calculate.
	 *  @param broadening Broadening to use in convolusion of coefficients
	 *  to remedy Gibb's osciallations.
	 */
	void calculateCoefficientsGPU(Index to, Index from, std::complex<double> *coefficients, int numCoefficients, double broadening = 0.0001);

	/** Experimental. */
	void calculateCoefficientsWithCutoff(Index to, Index from, std::complex<double> *coefficients, int numCoefficients, double componentCutoff, double broadening = 0.0001);

	/** Generate lokup table for quicker generation of multiple Green's
	 *  functions. Required if evaluation is to be performed on GPU.
	 *  @param numCoefficeints Number of coefficients used in Chebyshev
	 *  @param lowerBound Lower bound, has to be larger or equal to -1
	 *  @param upperBound Upper bound, has to be smaller or equal to 1
	 *  expansion.*/
	void generateLookupTable(int numCoefficeints, int energyResolution, double lowerBound = -1., double upperBound = 1.);

	/** Load lookup table generated by ChebyshevSolver::generateLookupTable
	 *   onto GPU. */
	void loadLookupTableGPU();

	/** Free memory allocated on GPU with
	 *  ChebyshevSolver::loadLookupTableGPU */
	void destroyLookupTableGPU();

	/** Genererate Green's function. Does not use lookup table generated by
	 *  ChebyshevSolver::generateLookupTable. Runs on CPU.
	 *  @param greensFunction Pointer to array able to hold Green's
	 *  function. Has to be able to hold energyResolution elements.
	 *  @param coefficients Chebyshev coefficients calculated by
	 *  ChebyshevSolver::calculateCoefficients.
	 *  @param numCoefficeints Number of coefficients in coefficients.
	 *  @param energyResolution Number of elements in greensFunction.
	 *  @param lowerBound Lower bound, has to be larger or equal to -1
	 *  @param upperBound Upper bound, has to be smaller or equal to 1
	 */
	void generateGreensFunction(std::complex<double> *greensFunction, std::complex<double> *coefficients, int numCoefficients, int energyResolution, double lowerBound = -1., double upperBound = 1.);

	/** Genererate Green's function. Uses lookup table generated by
	 *  ChebyshevSolver::generateLookupTable. Runs on CPU.
	 *  @param greensFunction Pointer to array able to hold Green's
	 *  function. Has to be able to hold energyResolution elements.
	 *  @param coefficients Chebyshev coefficients calculated by
	 *  ChebyshevSolver::calculateCoefficients.
	 *
	 * numCoefficients and energyResolution are here the values specified
	 * in the call to ChebyshevSolver::generateLookupTable
	 */
	void generateGreensFunction(std::complex<double> *greensFunction, std::complex<double> *coefficients);

	/** Genererate Green's function. Uses lookup table generated by
	 *  ChebyshevSolver::generateLookupTable. Runs on GPU.
	 *  @param greensFunction Pointer to array able to hold Green's
	 *  function. Has to be able to hold energyResolution elements.
	 *  @param coefficients Chebyshev coefficients calculated by
	 *  ChebyshevSolver::calculateCoefficients.
	 *
	 * numCoefficients and energyResolution are here the values specified
	 * in the call to ChebyshevSolver::generateLookupTable
	 */
	void generateGreensFunctionGPU(std::complex<double> *greensFunction, std::complex<double> *coefficients);

	void setTalkative(bool isTalkative);
private:
	/** Model to wok on. */
	Model *model;

	/** Pointer to lookup table used to speed up evaluation of multiple
	 *  Green's functions. */
	std::complex<double> **generatingFunctionLookupTable;

	/** Pointer to lookup table on GPU. */
	std::complex<double> **generatingFunctionLookupTable_device;

	/** Number of coefficients assumed in the generatino of Green's
	 *  function using the lookup tables*/
	int lookupTableNumCoefficients;

	/** Energy resolution assumed in the generation of Green's functios
	 *  using the lookup table. */
	int lookupTableResolution;

	bool isTalkative;
};

inline void ChebyshevSolver::setTalkative(bool isTalkative){
	this->isTalkative = isTalkative;
}

#endif
