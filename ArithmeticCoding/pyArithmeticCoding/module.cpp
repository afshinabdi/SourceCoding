
#include "CodecWrapper.h"


BOOST_PYTHON_MODULE(arithmetic_codec)
{
	bn::initialize();

	// register exception translators
	bp::register_exception_translator<ArgumentError>(&TranslateArgumentError);

	// define functions
	def("adaptive_decoder", AdaptiveDecoder, (bp::arg("input"), bp::arg("decode_len"), 
		bp::arg("alphabet_size")=2), "Adaptive arithmetic decoder");
	def("adaptive_encoder", AdaptiveEncoder, (bp::arg("input"), bp::arg("alphabet_size") = 2),
		"Adaptive arithmetic encoder");

	def("adaptive_decoder", AdaptiveDecoderWithFreq, (bp::arg("input"), bp::arg("init_frequency"),
		bp::arg("decode_len"), bp::arg("alphabet_size") = 2),  "Adaptive arithmetic decoder");
	def("adaptive_encoder", AdaptiveEncoderWithFreq, (bp::arg("input"), bp::arg("init_frequency"),
		bp::arg("alphabet_size") = 2), "Adaptive arithmetic encoder");

	def("fixed_decoder", FixedDecoder, (bp::arg("input"), bp::arg("frequency"),
		bp::arg("decode_len"), bp::arg("alphabet_size") = 2), "Arithmetic decoder with fixed input frequency table");
	def("fixed_encoder", FixedEncoder, (bp::arg("input"), bp::arg("frequency"),
		bp::arg("alphabet_size") = 2), "Arithmetic encoder with fixed input frequency table");
}