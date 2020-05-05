
#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>
#include <utility>
#include <random>
#include <numeric>
#include <algorithm>
#include <functional>
#include <iterator>
#include <chrono>

#include "BinaryDSC.h"
#include "Definitions.h"


void main()
{
	FACTOR_GRAPH fg;
	//std::ifstream fs("alist_N36,dv2,dc3.txt");
	std::ifstream fs("Dv2Dc3_G26_N1602.txt");

	std::pair<uint, uint> edge;

	std::string strLine;

	fg.uiNumEdges = 0;
	fg.edges.clear();
	edge.first = 0;
	edge.second = 0;

	if (fs.is_open()) {
		fs >> fg.uiNumVariableNodes;
		fs >> fg.uiNumCheckNodes;
		std::getline(fs, strLine);

		for (edge.second = 0; edge.second < fg.uiNumVariableNodes; edge.second++) {
			std::getline(fs, strLine);
			std::istringstream lineStream(strLine);
			while (lineStream >> edge.first) {
				edge.first--;          // indices start from 1 in the file
				fg.edges.push_back(edge);

				fg.uiNumEdges++;
			}
		}
	}

	std::cout << "Factor graph:" << std::endl;
	std::cout << fg.uiNumCheckNodes << ", " << fg.uiNumVariableNodes << ", " << fg.uiNumEdges << std::endl;
	std::cout << std::string(60, '=') << std::endl;

	CBinaryDSC dsc;
	dsc.InitializeGraph(fg);

	std::vector<uint8_t> x(fg.uiNumVariableNodes, 0);
	std::vector<uint8_t> y(fg.uiNumVariableNodes, 0);
	std::vector<uint8_t> s(fg.uiNumCheckNodes, 0);
	std::vector<uint8_t> xhat(fg.uiNumVariableNodes, 0);

	double Px_y[2][2] = { {0.95,0.05},{0.05,0.95} };

	std::random_device rd;
	std::mt19937 mt(rd());
	std::uniform_int_distribution<> dist(0, 1);
	std::discrete_distribution<> dist_e0({ Px_y[0][0], Px_y[1][0] });
	std::discrete_distribution<> dist_e1({ Px_y[0][1], Px_y[1][1] });

	uint maxIter = 100;
	double dElapsedTime = 0.0;
	for (uint iter = 0; iter < maxIter; iter++) {
		// generate random x and y
		std::generate(y.begin(), y.end(), [&] {return dist(mt); });
		std::transform(y.begin(), y.end(), x.begin(), [&](uint8_t v) {return (v == 0) ? dist_e0(mt) : dist_e1(mt); });

		auto start = std::chrono::steady_clock::now();
		// encoding
		dsc.Encode(x, s);

		// decoding
		dsc.Decode(s, y, Px_y, xhat, 50);
		auto end = std::chrono::steady_clock::now();
		auto diff = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
		dElapsedTime += diff;
		// find error
		double e = 0;
		for (size_t n = 0; n < x.size(); n++)
			e += (x[n] ^ xhat[n]);

		std::cout << "number of errors = " << e << std::endl;
	}

	std::cout << std::string(60, '=') << std::endl;
	std::cout << "Size of parity check matrix: " << fg.uiNumVariableNodes << " x " << fg.uiNumCheckNodes << std::endl;
	std::cout << "Number of runs: " << maxIter << ", average run time: " << dElapsedTime/100 << " micro-sec" << std::endl;
}