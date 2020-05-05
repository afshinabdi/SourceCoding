#pragma once

#include <vector>
#include "Definitions.h"
#include "Node.h"

class CBinaryDSC
{
public:
	CBinaryDSC();
	~CBinaryDSC();

	uint  InitializeGraph(FACTOR_GRAPH &graph);

	uint  Decode(std::vector<uint8_t> &s, std::vector<uint8_t> &y, double adPx_y[2][2],
		std::vector<uint8_t> &dec_x, uint uiMaxIter = 10);

	uint  Encode(std::vector<uint8_t> &x, std::vector<uint8_t> &s);

private:
	// variables for decoding
	std::vector<EDGE> m_aEdges;
	std::vector<CCheckNode> m_aCheckNodes;
	std::vector<CVariableNode> m_aVariableNodes;

	// the sparse matrix, used for encoding
	std::vector<std::vector<uint>> m_uiSparseH;
};

