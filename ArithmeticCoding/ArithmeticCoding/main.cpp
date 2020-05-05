
#include "ArithmeticEncoder.h"
#include "ArithmeticDecoder.h"

#include <iostream>
#include <iomanip>
#include <random>
#include <functional>
#include <chrono>

void main()
{
	CArithmeticEncoder encoder;
	CArithmeticDecoder decoder;
	CFrequencyTable freq;

	const uint8_t alphabet = 2;
	const uint32_t seqLen = 10000;
	uint32_t codeLen;
	std::vector<uint8_t> input(seqLen, 0);
	std::vector<uint8_t> decoded(seqLen, 0);
	uint8_t *coded = new uint8_t[2 * seqLen];

	double dEncTime = 0, dDecTime = 0;
	std::chrono::time_point<std::chrono::steady_clock> time_start, time_end;

	uint32_t iter;
	for (iter = 0; iter < 100; iter++) {
		// generate random input sequence
		std::random_device rd;
		std::mt19937 mt(rd());
		std::discrete_distribution<> dist({ 0.3, 0.7 });

		auto genRand = std::bind(dist, mt);
		std::generate(input.begin(), input.end(), genRand);

		// encoding
		time_start = std::chrono::steady_clock::now();

		freq.Initialize(alphabet);
		encoder.Initialize(coded);

		for (auto symbol : input) {
			encoder.Encode(freq, symbol);
			freq.IncrementFreq(symbol);
		}

		encoder.Finish();        // Flush remaining code bits
		time_end = std::chrono::steady_clock::now();
		dEncTime += std::chrono::duration_cast<std::chrono::microseconds> (time_end - time_start).count();

		codeLen = encoder.GetCodeLength();

		// decoding
		time_start = std::chrono::steady_clock::now();

		freq.Initialize(alphabet);
		decoder.Initialize(coded, codeLen);
		for (uint32_t n = 0; n < seqLen; n++) {
			// Decode and write one byte
			decoded[n] = decoder.Decode(freq);
			freq.IncrementFreq(decoded[n]);
		}

		time_end = std::chrono::steady_clock::now();
		dDecTime += std::chrono::duration_cast<std::chrono::microseconds> (time_end - time_start).count();
		
		uint32_t nError = 0;
		for (uint32_t n = 0; n < seqLen; n++)
			if (decoded[n] != input[n])
				nError++;

		std::cout << "code-length (byes) = " << codeLen << std::endl;
		std::cout << "Number of mismatches = " << nError << std::endl;
		std::cout << std::setfill('=') << std::setw(40) << "" << std::endl;
	}

	std::cout << "Average encoding time: " << dEncTime / iter << " us." << std::endl;
	std::cout << "Average decoding time: " << dDecTime / iter << " us." << std::endl;

	delete[] coded;
}