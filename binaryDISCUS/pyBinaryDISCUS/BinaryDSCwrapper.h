#pragma once

#define BOOST_PYTHON_STATIC_LIB
#define BOOST_NUMPY_STATIC_LIB

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include "../BinaryDISCUS/BinaryDSC.h"

namespace bp = boost::python;
namespace bn = boost::python::numpy; 

class ArgumentError : public std::exception {
public:
	ArgumentError() : msg("Unknown error.") {};
	ArgumentError(std::string str) : msg(str) {};
	~ArgumentError() {};

	const char* what() const throw() { return msg.c_str(); }

private:
	std::string msg;
};

// translator for the boost::python
void TranslateArgumentError(const ArgumentError& e);

// wrapper for the binary source coding with side information using DISCUS
class CBinaryDSCwrapper
{
public:
	CBinaryDSCwrapper();
	~CBinaryDSCwrapper();

	uint Initialize(uint uiNumCheckNodes, uint uiNumVariableNodes, const bn::ndarray& rowH, const bn::ndarray& colH);
	bn::ndarray Encode(const bn::ndarray& x);
	bn::ndarray Decode(const bn::ndarray& s, const bn::ndarray& y, const bn::ndarray& adPx_y, uint uiMaxIter = 10);

private:
	FACTOR_GRAPH m_fg;
	CBinaryDSC   m_codec;
};

