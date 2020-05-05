#pragma once

#define BOOST_PYTHON_STATIC_LIB
#define BOOST_NUMPY_STATIC_LIB

#include <boost/python.hpp>
#include <boost/python/numpy.hpp>
#include <exception>
#include <string>

#include "../ArithmeticCoding/Definitions.h"

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

bn::ndarray AdaptiveEncoder(const bn::ndarray& sequence, uint8_t uiAlphabetSize);
bn::ndarray AdaptiveDecoder(const bn::ndarray& sequence, uint32_t decLen, uint8_t uiAlphabetSize);

bp::tuple AdaptiveEncoderWithFreq(const bn::ndarray& sequence, bn::ndarray& freq, uint8_t uiAlphabetSize);
bp::tuple AdaptiveDecoderWithFreq(const bn::ndarray& sequence, bn::ndarray& freq, uint32_t decLen, uint8_t uiAlphabetSize);

bn::ndarray FixedEncoder(const bn::ndarray& sequence, const bn::ndarray& freq, uint8_t uiAlphabetSize);
bn::ndarray FixedDecoder(const bn::ndarray& sequence, const bn::ndarray& freq, uint32_t decLen, uint8_t uiAlphabetSize);
