
#include "CodecWrapper.h"
#include "../ArithmeticCoding/FrequencyTable.h"
#include "../ArithmeticCoding/ArithmeticEncoder.h"
#include "../ArithmeticCoding/ArithmeticDecoder.h"

// Boost::Python needs the translators
void TranslateArgumentError(const ArgumentError& e)
{
	PyErr_SetString(PyExc_RuntimeError, e.what());
}

bn::ndarray AdaptiveEncoder(const bn::ndarray& sequence, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to encode should be of type uint8.");


	uint32_t  uiLen = bp::len(sequence);
	uint8_t*  pInput = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  coded = new uint8_t[uiLen];

	CFrequencyTable freqTable;
	CArithmeticEncoder encoder;

	freqTable.Initialize(uiAlphabetSize);
	encoder.Initialize(coded);

	for (uint32_t n = 0; n < uiLen; n++) {
		encoder.Encode(freqTable, pInput[n]);
		freqTable.IncrementFreq(pInput[n]);
	}

	encoder.Finish();

	uint32_t codeLen;
	codeLen = encoder.GetCodeLength();

	bn::ndarray pyCoded = bn::empty(bp::make_tuple(codeLen), bn::dtype::get_builtin<uint8_t>());
	memcpy(pyCoded.get_data(), coded, codeLen * sizeof(uint8_t));

	delete[] coded;

	return pyCoded;
}

bn::ndarray AdaptiveDecoder(const bn::ndarray& sequence, uint32_t decLen, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to decode should be of type uint8.");

	bn::ndarray pyDecoded = bn::zeros(bp::make_tuple(decLen), bn::dtype::get_builtin<uint8_t>());

	uint8_t*  coded = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  decoded = reinterpret_cast<uint8_t *>(pyDecoded.get_data());

	CFrequencyTable freqTable;
	CArithmeticDecoder decoder;

	freqTable.Initialize(uiAlphabetSize);
	decoder.Initialize(coded, bp::len(sequence));

	for (uint32_t n = 0; n < decLen; n++) {
		decoded[n] = decoder.Decode(freqTable);
		freqTable.IncrementFreq(decoded[n]);
	}

	return pyDecoded;
}

bp::tuple AdaptiveEncoderWithFreq(const bn::ndarray& sequence, bn::ndarray& freq, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to encode should be of type uint8.");

	if (freq.get_dtype() != bn::dtype::get_builtin<uint64_t>())
		throw ArgumentError("Initial frequecy table should be of type uint64.");

	if (freq.squeeze().shape(0) != uiAlphabetSize)
		throw ArgumentError("Length of input frequency table should be the same as the alphabet size.");

	uint32_t  uiLen = bp::len(sequence);
	uint8_t*  pInput = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  coded = new uint8_t[uiLen];
	uint64_t* pFreq = reinterpret_cast<uint64_t *>(freq.get_data());

	CFrequencyTable freqTable;
	CArithmeticEncoder encoder;

	freqTable.Initialize(pFreq, uiAlphabetSize);
	encoder.Initialize(coded);

	for (uint32_t n = 0; n < uiLen; n++) {
		encoder.Encode(freqTable, pInput[n]);
		freqTable.IncrementFreq(pInput[n]);
	}

	encoder.Finish();

	uint32_t codeLen;
	codeLen = encoder.GetCodeLength();
	pFreq = freqTable.GetFrequencyTable();

	bn::ndarray pyCoded = bn::empty(bp::make_tuple(codeLen), bn::dtype::get_builtin<uint8_t>());
	bn::ndarray pyFreq = bn::empty(bp::make_tuple(uiAlphabetSize), bn::dtype::get_builtin<uint64_t>());

	memcpy(pyCoded.get_data(), coded, codeLen * sizeof(uint8_t));
	memcpy(pyFreq.get_data(), pFreq, uiAlphabetSize * sizeof(uint64_t));

	delete[] coded;
	return bp::make_tuple(pyCoded, pyFreq);
}

bp::tuple AdaptiveDecoderWithFreq(const bn::ndarray& sequence, bn::ndarray& freq, uint32_t decLen, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to decode should be of type uint8.");

	if (freq.get_dtype() != bn::dtype::get_builtin<uint64_t>())
		throw ArgumentError("Initial frequecy table should be of type uint64.");

	if (freq.squeeze().shape(0) != uiAlphabetSize)
		throw ArgumentError("Length of input frequency table should be the same as the alphabet size.");

	bn::ndarray pyDecoded = bn::zeros(bp::make_tuple(decLen), bn::dtype::get_builtin<uint8_t>());

	uint8_t*  coded = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  decoded = reinterpret_cast<uint8_t *>(pyDecoded.get_data());
	uint64_t* pFreq = reinterpret_cast<uint64_t *>(freq.get_data());

	CFrequencyTable freqTable;
	CArithmeticDecoder decoder;

	freqTable.Initialize(pFreq, uiAlphabetSize);
	decoder.Initialize(coded, bp::len(sequence));

	for (uint32_t n = 0; n < decLen; n++) {
		decoded[n] = decoder.Decode(freqTable);
		freqTable.IncrementFreq(decoded[n]);
	}

	pFreq = freqTable.GetFrequencyTable();
	bn::ndarray pyFreq = bn::empty(bp::make_tuple(uiAlphabetSize), bn::dtype::get_builtin<uint64_t>());

	memcpy(pyFreq.get_data(), pFreq, uiAlphabetSize * sizeof(uint64_t));

	return bp::make_tuple(pyDecoded, pyFreq);
}

bn::ndarray FixedEncoder(const bn::ndarray& sequence, const bn::ndarray& freq, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to encode should be of type uint8.");

	if (freq.get_dtype() != bn::dtype::get_builtin<uint64_t>())
		throw ArgumentError("Frequecy table should be of type uint64.");

	if (freq.squeeze().shape(0) != uiAlphabetSize)
		throw ArgumentError("Length of input frequency table should be the same as the alphabet size.");

	uint32_t   uiLen = bp::len(sequence);
	uint8_t*  input = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  coded = new uint8_t[uiLen];
	uint64_t* pFreq = reinterpret_cast<uint64_t *>(freq.get_data());

	CFrequencyTable freqTable;
	CArithmeticEncoder encoder;

	freqTable.Initialize(pFreq, uiAlphabetSize);
	encoder.Initialize(coded);

	for (uint32_t n = 0; n < uiLen; n++)
		encoder.Encode(freqTable, input[n]);

	encoder.Finish();

	uint32_t codeLen = encoder.GetCodeLength();
	bn::ndarray pyCoded = bn::empty(bp::make_tuple(codeLen), bn::dtype::get_builtin<uint8_t>());

	memcpy(pyCoded.get_data(), coded, codeLen * sizeof(uint8_t));

	delete[] coded;
	return pyCoded;
}

bn::ndarray FixedDecoder(const bn::ndarray& sequence, const bn::ndarray& freq, uint32_t decLen, uint8_t uiAlphabetSize)
{
	if (sequence.get_dtype() != bn::dtype::get_builtin<uint8_t>())
		throw ArgumentError("Input sequence to decode should be of type uint8.");

	if (freq.get_dtype() != bn::dtype::get_builtin<uint64_t>())
		throw ArgumentError("Initial frequecy table should be of type uint64.");

	if (freq.squeeze().shape(0) != uiAlphabetSize)
		throw ArgumentError("Length of input frequency table should be the same as the alphabet size.");

	bn::ndarray dec_sequence = bn::zeros(bp::make_tuple(decLen), bn::dtype::get_builtin<uint8_t>());
	uint8_t*  coded = reinterpret_cast<uint8_t *>(sequence.get_data());
	uint8_t*  decoded = reinterpret_cast<uint8_t *>(dec_sequence.get_data());
	uint64_t* pFreq = reinterpret_cast<uint64_t *>(freq.get_data());

	CFrequencyTable freqTable;
	CArithmeticDecoder decoder;

	freqTable.Initialize(pFreq, uiAlphabetSize);
	decoder.Initialize(coded, bp::len(sequence));

	for (uint32_t n = 0; n < decLen; n++)
		decoded[n] = decoder.Decode(freqTable);

	return dec_sequence;
}
