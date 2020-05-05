
#include "BinaryDSCwrapper.h"

namespace bp = boost::python;

BOOST_PYTHON_MODULE(binaryDSC)
{
	Py_Initialize();
	bn::initialize();

	// register exception translators
	bp::register_exception_translator<ArgumentError>(&TranslateArgumentError);

	bp::class_<CBinaryDSCwrapper>("BinaryDSC")
		.def("initialize", &CBinaryDSCwrapper::Initialize, (bp::arg("M"), bp::arg("N"),
			bp::arg("row_index"), bp::arg("col_index")), "Initialize the binary source "
			"coding with side-infromation, using DISUCS")
		.def("decode", &CBinaryDSCwrapper::Decode, (bp::arg("input_code"), bp::arg("si_sequence"),
			bp::arg("px_y"), bp::arg("max_iter")=20), "Iterative message passing to decode the input "
			"code with the give side information and conditional probability")
		.def("encode", &CBinaryDSCwrapper::Encode, (bp::arg("input")),
			"Encode the input sequence")
		;
}