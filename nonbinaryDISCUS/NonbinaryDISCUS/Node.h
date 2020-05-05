#pragma once

#include <cmath>
#include <vector>
#include "Definitions.h"
#include "GaloisField.h"

class CNode
{
public:
	CNode();
	~CNode();

	void ResetNode() {
		m_iDegree = 0;
		m_aEdges.clear();
	};

	void AddEdge(EDGE *pEdge) {
		m_iDegree++;
		m_aEdges.push_back(pEdge);
	};

	virtual void Initialize(uint q, CGaloisField *gf) { 
		m_uiq = q;  m_uiQ = 1U << q; m_gf = gf;
	};

	virtual DType ComputeMessage() { return 0; };

protected:
	static uint  m_uiq;
	static uint  m_uiQ;
	static CGaloisField* m_gf;

	int  m_iDegree;                    // degree of the node (number of edges)
	std::vector<EDGE *> m_aEdges;
};

// check node
class CCheckNode :
	public CNode
{
public:
	CCheckNode();
	~CCheckNode();

	void Initialize(uint q, CGaloisField *gf);

	// decoding function
	DType ComputeMessage() override;      // compute the outgoing message to each edge and return the syndorm error
	void SetSyndrome(DType &s) {
		m_dtS = s;
	};
	
private:
	void ApplyFFT(std::vector<double> &x);
	void ApplyFFT(std::vector<double> &x, std::vector<double> &y);

	DType m_dtS;                                 // syndrome
	std::vector<std::vector<double>> m_adUpc;    // FFT of the permutation of the message (probability) from variable to check node
	std::vector<std::vector<double>> m_adVtp;    // FFT of the computed message to the variable node (before permutation)
};

// variable node
class CVariableNode :
	public CNode
{
public:
	CVariableNode();
	~CVariableNode();

	void Initialize(uint q, CGaloisField *gf);

	// decoding functions
	DType ComputeMessage() override;      // compute the outgoing message to each edge and return the decoded value (0/1)
	void  SetInitialProbability(double adPx_y[], DType &y);
	DType GetValue() {
		return m_dtValue;
	};

private:
	std::vector<double> m_adLogP0;     // log(P(x|y)), initial probability
	std::vector<double> m_adTotalLL;   // total log-likelihood, sum of the all incoming messages for decoding the ML value
	DType  m_dtValue;             // the decoded value at the variable node
};

