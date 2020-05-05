
#include "BinaryDSCwrapper.h"
#include <boost/python/stl_iterator.hpp>

// Boost::Python needs the translators
void TranslateArgumentError(const ArgumentError& e)
{
	PyErr_SetString(PyExc_RuntimeError, e.what());
}

CBinaryDSCwrapper::CBinaryDSCwrapper()
{
}


CBinaryDSCwrapper::~CBinaryDSCwrapper()
{
}


uint CBinaryDSCwrapper::Initialize(uint uiNumCheckNodes, uint uiNumVariableNodes, const bn::ndarray& rowH, const bn::ndarray& colH)
{
	// check for the size and type of the input sequence
	if ((rowH.get_dtype() != bn::dtype::get_builtin<uint32_t>()) || 
		(colH.get_dtype() != bn::dtype::get_builtin<uint32_t>()))
		throw ArgumentError("Sprase representation of H should of data type uint32.");

	if (rowH.shape(0) != colH.shape(0))
		throw ArgumentError("Sizes of the row and column indices of non-zero elements in H are not equal.");

	m_fg.uiNumCheckNodes = uiNumCheckNodes;
	m_fg.uiNumVariableNodes = uiNumVariableNodes;
	m_fg.uiNumEdges = rowH.shape(0);
	m_fg.edges.resize(m_fg.uiNumEdges);

	auto ptr_chk = reinterpret_cast<uint32_t*>(rowH.get_data());
	auto ptr_var = reinterpret_cast<uint32_t*>(colH.get_data());


	for (uint n = 0; n < m_fg.uiNumEdges; n++)
	{
		m_fg.edges[n].first = ptr_chk[n];
		m_fg.edges[n].second = ptr_var[n];
	}

	return m_codec.InitializeGraph(m_fg);
}

bn::ndarray CBinaryDSCwrapper::Encode(const bn::ndarray& x)
{
	// check for the size and type of the input sequence
	if (x.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to encode should be of data type uint8.");

	if (x.squeeze().shape(0) != m_fg.uiNumVariableNodes)
		throw ArgumentError("Size of the input vector to encode is not consistent with the provided LDPC matrix.");

	// create output numpy array
	bn::ndarray coded = bn::empty(bp::make_tuple(m_fg.uiNumCheckNodes),
		bn::dtype::get_builtin<uint8_t>());

	// copy from input numpy array to std::vector
	auto ptr_x = reinterpret_cast<uint8_t*>(x.get_data());
	std::vector<uint8_t> vec_x(ptr_x, ptr_x + m_fg.uiNumVariableNodes);
	std::vector<uint8_t> vec_s(m_fg.uiNumCheckNodes, 0);

	// encode the string
	m_codec.Encode(vec_x, vec_s);

	// copy from std::vector to python array
	std::copy(vec_s.begin(), vec_s.end(), reinterpret_cast<uint8_t*>(coded.get_data()));

	return coded;
}

bn::ndarray CBinaryDSCwrapper::Decode(const bn::ndarray& s, const bn::ndarray& y, const bn::ndarray& Px_y, uint uiMaxIter)
{
	// check for the sizes and types of input arguments
	if (s.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to decode should be of data type uint8.");

	if (s.squeeze().shape(0) != m_fg.uiNumCheckNodes)
		throw ArgumentError("Size of input vector to decode is not consistent with the provided LDPC matrix.");

	if (y.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input side-information sequence should be of data type uint8.");

	if (y.squeeze().shape(0) != m_fg.uiNumVariableNodes)
		throw ArgumentError("Size of input side-information is not consistent with the provided LDPC matrix.");

	if ((Px_y.get_dtype() != bn::dtype::get_builtin<double>()) || (Px_y.shape(0) != 2) || (Px_y.shape(1) != 2))
		throw ArgumentError("Conditional probabiltity distribution should be 2x2 double matrix.");

	// create the output numpy array
	bn::ndarray x_dec = bn::empty(bp::make_tuple(m_fg.uiNumVariableNodes),
		bn::dtype::get_builtin<uint8_t>());

	// create 2D array for p(x|y)
	double adPx_y[2][2];
	Py_intptr_t const * strides = Px_y.get_strides();
	for (int i = 0; i < 2; i++)
		for (int j = 0; j < 2; j++)
			adPx_y[i][j] = *reinterpret_cast<double *>(Px_y.get_data() + i * strides[0] + j * strides[1]);

	// copy from python arrays to std::vector
	uint8_t* ptr;
	ptr = reinterpret_cast<uint8_t*>(s.get_data());
	std::vector<uint8_t> vec_s(ptr, ptr + m_fg.uiNumCheckNodes);

	ptr = reinterpret_cast<uint8_t*>(y.get_data());
	std::vector<uint8_t> vec_y(ptr, ptr + m_fg.uiNumVariableNodes);

	std::vector<uint8_t> vec_xdec(m_fg.uiNumVariableNodes, 0);

	// decode
	m_codec.Decode(vec_s, vec_y, adPx_y, vec_xdec, uiMaxIter);

	// copy from std::vector to python array
	std::copy(vec_xdec.begin(), vec_xdec.end(), reinterpret_cast<uint8_t*>(x_dec.get_data()));

	return x_dec;
}