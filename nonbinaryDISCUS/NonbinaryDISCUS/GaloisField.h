#pragma once

#include "Definitions.h"

class CGaloisField
{
public:
	CGaloisField();
	~CGaloisField();

	int   Initialize(uint q = 1U);
	DType Multiply(DType a, DType b);
	DType Divide(DType a, DType b);
	DType Add(DType a, DType b) {
		return (a^b);
	}

private:
	void ReleaseMemory();

	uint m_uiQ;

	DType*  m_aLogQ;
	DType*  m_aExpQ;
};

