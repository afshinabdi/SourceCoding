
#include "NonbinaryDSCwrapper.h"
#include <boost/python/stl_iterator.hpp>

// Boost::Python needs the translators
void TranslateArgumentError(const ArgumentError& e)
{
	PyErr_SetString(PyExc_RuntimeError, e.what());
}

CNonbinaryDSCwrapper::CNonbinaryDSCwrapper()
{
}


CNonbinaryDSCwrapper::~CNonbinaryDSCwrapper()
{
}


uint CNonbinaryDSCwrapper::Initialize(uint q, uint uiNumCheckNodes, uint uiNumVariableNodes, const bn::ndarray& rowH, const bn::ndarray& colH, const bn::ndarray& valH)
{
	if ((rowH.get_dtype() != bn::dtype::get_builtin<uint32_t>()) ||
		(colH.get_dtype() != bn::dtype::get_builtin<uint32_t>()))
		throw ArgumentError("Sprase representation of H should of data type uint32.");
	
	if (valH.get_dtype() != bn::dtype::get_builtin<DType>())
		throw ArgumentError("Values of the sprase H is not of type DType.");
		
	if ((rowH.shape(0) != colH.shape(0)) || (rowH.shape(0) != valH.shape(0)))
		throw ArgumentError("Sizes of the row and column indices of non-zero elements in H are not equal.");

	DType h;
	std::pair<uint, uint> edge;

	m_fg.uiNumCheckNodes = uiNumCheckNodes;
	m_fg.uiNumVariableNodes = uiNumVariableNodes;
	m_fg.uiNumEdges = rowH.shape(0);

	m_fg.edges.resize(m_fg.uiNumEdges);
	m_fg.Hvalues.resize(m_fg.uiNumEdges, 1);

	auto ptr_chk = reinterpret_cast<uint32_t*>(rowH.get_data());
	auto ptr_var = reinterpret_cast<uint32_t*>(colH.get_data());
	auto ptr_val = reinterpret_cast<DType*>(valH.get_data());

	for (uint n = 0; n < m_fg.uiNumEdges; n++)
	{
		m_fg.edges[n].first = ptr_chk[n];
		m_fg.edges[n].second = ptr_var[n];
	}

	std::copy(ptr_val, ptr_val + m_fg.uiNumEdges, m_fg.Hvalues.begin());

	m_uiQ = 1U << q;

	return m_codec.InitializeGraph(m_fg, q);
}

bn::ndarray CNonbinaryDSCwrapper::Encode(const bn::ndarray& x)
{
	// check for the size and type of the input sequence
	if (x.get_dtype() != bn::dtype::get_builtin<DType>())
		throw ArgumentError("Input sequence to encode should be of data type uint8.");

	if (x.squeeze().shape(0) != m_fg.uiNumVariableNodes)
		throw ArgumentError("Size of the input vector to encode is not consistent with the provided LDPC matrix.");

	// create output numpy array
	bn::ndarray coded = bn::empty(bp::make_tuple(m_fg.uiNumCheckNodes),
		bn::dtype::get_builtin<DType>());

	// copy from input numpy array to std::vector
	auto ptr_x = reinterpret_cast<DType*>(x.get_data());
	std::vector<DType> vec_x(ptr_x, ptr_x + m_fg.uiNumVariableNodes);
	std::vector<DType> vec_s(m_fg.uiNumCheckNodes, 0);

	// encode the string
	m_codec.Encode(vec_x, vec_s);

	// copy from std::vector to python array
	std::copy(vec_s.begin(), vec_s.end(), reinterpret_cast<DType*>(coded.get_data()));

	return coded;
}

bn::ndarray CNonbinaryDSCwrapper::Decode(const bn::ndarray& s, const bn::ndarray& y, const bn::ndarray& Px_y, uint uiMaxIter)
{
	// check for the sizes and types of input arguments
	if (s.get_dtype() != bn::dtype::get_builtin<DType>())
		throw ArgumentError("Input sequence to decode should be of data type uint8.");

	if (s.squeeze().shape(0) != m_fg.uiNumCheckNodes)
		throw ArgumentError("Size of input vector to decode is not consistent with the provided LDPC matrix.");

	if (y.get_dtype() != bn::dtype::get_builtin<DType>())
		throw ArgumentError("Input side-information sequence should be of data type uint8.");

	if (y.squeeze().shape(0) != m_fg.uiNumVariableNodes)
		throw ArgumentError("Size of input side-information is not consistent with the provided LDPC matrix.");

	if ((Px_y.get_dtype() != bn::dtype::get_builtin<double>()) || (Px_y.shape(0) != m_uiQ) || (Px_y.shape(1) != m_uiQ))
		throw ArgumentError("Conditional probabiltity distribution should be 2x2 double matrix.");

	// create the output numpy array
	bn::ndarray x_dec = bn::empty(bp::make_tuple(m_fg.uiNumVariableNodes),
		bn::dtype::get_builtin<DType>());

	// create 2D array for p(x|y)
	double *adPx_y;
	adPx_y = new double[m_uiQ*m_uiQ];
	Py_intptr_t const * strides = Px_y.get_strides();
	for (int i = 0; i < m_uiQ; i++)
		for (int j = 0; j < m_uiQ; j++)
			adPx_y[i*m_uiQ + j] = *reinterpret_cast<double *>(Px_y.get_data() + i * strides[0] + j * strides[1]);

	// copy from python arrays to std::vector
	DType* ptr;
	ptr = reinterpret_cast<DType*>(s.get_data());
	std::vector<DType> vec_s(ptr, ptr + m_fg.uiNumCheckNodes);

	ptr = reinterpret_cast<DType*>(y.get_data());
	std::vector<DType> vec_y(ptr, ptr + m_fg.uiNumVariableNodes);

	std::vector<DType> vec_xdec(m_fg.uiNumVariableNodes, 0);

	// decode
	m_codec.Decode(vec_s, vec_y, adPx_y, vec_xdec, uiMaxIter);

	// copy from std::vector to python array
	std::copy(vec_xdec.begin(), vec_xdec.end(), reinterpret_cast<DType*>(x_dec.get_data()));

	delete[] adPx_y;
	return x_dec;
}