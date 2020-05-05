
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

#include "NonbinaryDSC.h"
#include "Definitions.h"


void main()
{
	uint q = 3;
	uint Q = 1U << q;

	std::random_device rd;
	std::mt19937 mt(rd());

	FACTOR_GRAPH fg;
	//std::ifstream fs("alist_N36,dv2,dc3.txt");
	//std::ifstream fs("Dv2Dc50_G8_N2500.txt");
	std::ifstream fs("Dv2Dc3_G26_N1602.txt");

	std::pair<uint, uint> edge;

	std::string strLine;

	fg.uiNumEdges = 0;
	fg.edges.clear();
	fg.Hvalues.clear();
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

	fg.Hvalues.resize(fg.uiNumEdges);

	std::uniform_int_distribution<> dist_h(1, Q - 1);
	auto genRandH = std::bind(dist_h, mt);
	std::generate(fg.Hvalues.begin(), fg.Hvalues.end(), genRandH);

	std::cout << "Factor graph:" << std::endl;
	std::cout << fg.uiNumCheckNodes << ", " << fg.uiNumVariableNodes << ", " << fg.uiNumEdges << std::endl;
	std::cout << std::string(60, '=') << std::endl;

	CNonbinaryDSC dsc;
	dsc.InitializeGraph(fg, q);

	std::vector<DType> x(fg.uiNumVariableNodes, 0);
	std::vector<DType> y(fg.uiNumVariableNodes, 0);
	std::vector<DType> e_xy(fg.uiNumVariableNodes, 0);
	std::vector<DType> s(fg.uiNumCheckNodes, 0);
	std::vector<DType> xhat(fg.uiNumVariableNodes, 0);

	// model the error as symmetric channel
	double *Px_y;
	double *Pe;

	Px_y = new double[Q*Q];
	Pe = new double[Q];
	Pe[0] = 0.9;
	for (DType ve = 1; ve < Q; ve++)
		Pe[ve] = 0.1 / (Q - 1);

	for (DType vx = 0; vx < Q; vx++)
		for (DType vy = 0; vy < Q; vy++)
			Px_y[vx*Q + vy] = Pe[abs(vx-vy)];

	std::uniform_int_distribution<> dist_y(0, Q - 1);
	std::discrete_distribution<> dist_e(Pe, Pe + Q);

	uint maxIter = 100;
	double dElapsedTime = 0.0;
	for (uint iter = 0; iter < maxIter; iter++) {

		auto genRandY = std::bind(dist_y, mt);
		auto genRandE = std::bind(dist_e, mt);

		// generate random x and y
		std::generate(y.begin(), y.end(), [&] {return dist_y(mt); });
		std::generate(e_xy.begin(), e_xy.end(), [&] {return dist_e(mt); });
		
		std::transform(y.begin(), y.end(), e_xy.begin(), x.begin(), [](DType &vy, DType &ve) {return (vy ^ ve); });

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
			e += (x[n] == xhat[n]) ? 0 : 1;

		std::cout << "number of mismatches between x & y = " << x.size() - std::count(e_xy.begin(), e_xy.end(), 0);
		std::cout << ", number of errors = " << e << std::endl;
	}

	std::cout << std::string(60, '=') << std::endl;
	std::cout << "Size of parity check matrix: " << fg.uiNumVariableNodes << " x " << fg.uiNumCheckNodes << std::endl;
	std::cout << "Number of runs: " << maxIter << ", average run time: " << dElapsedTime / 100 << " micro-sec" << std::endl;

	delete[] Px_y;
	delete[] Pe;
}