#pragma once

#define epsilon   1e-8
typedef unsigned int  uint;

struct FACTOR_GRAPH {
	uint  uiNumCheckNodes;
	uint  uiNumVariableNodes;
	uint  uiNumEdges;

	std::vector<std::pair<uint, uint>> edges;
};

struct EDGE {
	double  ChktoVar;
	double  VartoChk;

	uint8_t uiDecValue;
};
