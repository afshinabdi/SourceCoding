
#include "PEGDesignWrapper.h"


BOOST_PYTHON_MODULE(ldpc_design)
{
	Py_Initialize();
	bn::initialize();

	// register exception translators
	bp::register_exception_translator<ArgumentError>(&TranslateArgumentError);

	// define functions
	def("peg_design", LDPC_PEGDesign, (bp::arg("M"), bp::arg("N"), bp::arg("deg_vaue"),
		bp::arg("deg_distribution")), "Designing LDPC matrix using PEG method");
}