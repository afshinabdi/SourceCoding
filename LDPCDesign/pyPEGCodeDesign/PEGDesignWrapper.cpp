
#include <vector>
#include <numeric>

#include "../PEGCodeDesign/PEGDesign.h"
#include "PEGDesignWrapper.h"

// Boost::Python needs the translators
void TranslateArgumentError(const ArgumentError& e)
{
	PyErr_SetString(PyExc_RuntimeError, e.what());
}


bp::tuple LDPC_PEGDesign(int M, int N, const bp::list& degValue, const bp::list &degDistribution)
{
	if (M > N)
		throw ArgumentError("Number of check nodes can not be more than variable nodes.");

	if (bp::len(degValue) != bp::len(degDistribution))
		throw ArgumentError("Lists of input degree and degree distribution should be equal.");

	int degLen = bp::len(degValue);
	std::vector<int> degSeq(N, 0);
	std::vector<int> deg(degLen, 0);
	std::vector<double> degFrac(degLen, 0);

	for (int n = 0; n < degLen; n++) {
		deg[n] = bp::extract<int>(degValue[n]);
		degFrac[n] = bp::extract<double>(degDistribution[n]);
	}

	std::partial_sum(degFrac.begin(), degFrac.end(), degFrac.begin());

	if (abs(degFrac.back() - 1.0) > 1e-6)
		throw ArgumentError("Sum of degree distribution should be 1.");

	std::transform(degFrac.begin(), degFrac.end(), degFrac.begin(), [&](double &x) {return __min(N - 1, int(N*x)); });
	degFrac[degFrac.size() - 1] = N - 1;

	int prvIdx = 0;
	int nxtIdx;

	for (int i = 0; i < degLen; i++) {
		nxtIdx = degFrac[i];
		for (int j = prvIdx; j <= nxtIdx; j++)
			degSeq[j] = deg[i];

		prvIdx = nxtIdx + 1;
	}

	CPEGDesign bigGirth;

	int numEdges = 0;
	numEdges = bigGirth.GenerateLDPCMatrix(M, N, degSeq.data(), 1, 10000, false);

	bn::ndarray rowIdx = bn::empty(bp::make_tuple(numEdges), bn::dtype::get_builtin<uint32_t>());
	bn::ndarray colIdx = bn::empty(bp::make_tuple(numEdges), bn::dtype::get_builtin<uint32_t>());
	uint32_t *row, *col;
	row = reinterpret_cast<uint32_t *>(rowIdx.get_data());
	col = reinterpret_cast<uint32_t *>(colIdx.get_data());

	bigGirth.GetSparseMatrix(row, col);

	return bp::make_tuple(rowIdx, colIdx);
}