#pragma once

#include "Definitions.h"
#include "GaloisField.h"
#include "Node.h"

class CNonbinaryDSC
{
public:
	CNonbinaryDSC();
	~CNonbinaryDSC();

	int  InitializeGraph(FACTOR_GRAPH &graph, uint q);

	int  Decode(std::vector<DType> &s, std::vector<DType> &y, double adPx_y[],
		std::vector<DType> &dec_x, uint uiMaxIter = 10);

	int  Encode(std::vector<DType> &x, std::vector<DType> &s);

private:
	uint m_uiq;
	uint m_uiQ;
	CGaloisField m_gf;

	// variables for decoding
	std::vector<EDGE> m_aEdges;
	std::vector<CCheckNode> m_aCheckNodes;
	std::vector<CVariableNode> m_aVariableNodes;

	// the sparse matrix, used for encoding
	std::vector<std::vector<uint>>  m_uiSparseEntry;
	std::vector<std::vector<DType>> m_dtSparseValues;
};

