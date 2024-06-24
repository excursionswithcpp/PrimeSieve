#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <chrono>

#include <charconv>

using namespace std;


#include "optionparser.h"

struct Arg : public option::Arg
{
	static option::ArgStatus Required(const option::Option& option, bool msg)
	{
		if (option.arg != 0)
			return option::ARG_OK;

		if (msg)
			cerr << "Option '" << option.name << "' needs an argument" << endl;

		return option::ARG_ILLEGAL;
	}
};

enum  optionIndex { UNKNOWN, SIEVES, MEMORY, ROUNDS, PRIME, HELP, LOGFILE };

const option::Descriptor usage[] = {
{ UNKNOWN,  0, "",  "",			Arg::None, "USAGE: primesieve [options]\n\n"
										  "Options:" },
{ HELP,     0, "h", "help",		Arg::None,    "  -h \t--help  \tPrint usage and exit." },
{ PRIME,    0, "p", "prime",	Arg::Required,"  -p <arg>, \t--prime=<arg>"
										  "  \tFind primes less than this number (default: 1.000.000)\n"
										  " \t\tcan be expressed as floating point (1.0e6)\n"
},
{ UNKNOWN,  0, "", "",			Arg::None,
 "\nExamples:\n"
 "  primesieve -p 1000000\n"
 "  primesieve -p 1e10\n"
},
{ 0, 0, 0, 0, 0, 0 } }; // End of table

// Define the number type for primes and indices, eg. differentiating between 32 and 64 bit architectures
typedef unsigned long long int itype;

itype numPrimes(itype maxNum);

int main(int argc, char* argv[])
{
	// Imbue with thousand separators because we have big numbers
	std::cout.imbue(std::locale("en_US"));
	// std::cout.imbue(std::locale(std::locale, new numpunct<char>()));

	argc -= 1;
	argv += 1; // skip program name argv[0]

	option::Stats  stats(usage, argc, argv);
	std::vector<option::Option> options(stats.options_max);
	std::vector<option::Option> buffer(stats.buffer_max);
	option::Parser parse(true, usage, argc, argv, &options[0], &buffer[0], 2);

	for (option::Option* opt = options[UNKNOWN]; opt; opt = opt->next())
		std::cout << "Unknown option: " << std::string(opt->name, opt->namelen) << "\n";

	for (int i = 0; i < parse.nonOptionsCount(); ++i)
		std::cout << "Non-option #" << i << ": " << parse.nonOption(i) << std::endl;

	if (parse.error() || options[HELP] || options[UNKNOWN] || parse.nonOptionsCount()) {
		option::printUsage(std::cout, usage, 40);
		return 1;
	}

	itype maxNum = 1000000;
	if (options[PRIME])
	{
		// Allow parameter to be in float format, eg. 1E8
		double dMaxnum;
		const char* parg = options[PRIME].arg;
		std::from_chars(parg, parg + strlen(parg), dMaxnum);

		itype maxnum = static_cast<itype>(dMaxnum);

		if (maxnum > 0)
			maxNum = maxnum;
	}

	cout << "Counting primes up to " << maxNum << endl;
	auto t0 = std::chrono::system_clock::now();
	cout << numPrimes(maxNum) << " primes found" << endl;
	cout << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now() - t0).count() / 1000.0 << " s used" << endl;
	return 0;
}

itype numPrimes(itype maxNum)
{
	auto numbers = new char[maxNum+1] {0};
	numbers[0] = numbers[1] = 1;

	itype sqrtMaxNum = static_cast<itype>(std::sqrt(maxNum));

	itype p = 2;
	do
	{
		for (itype i = p * p; i <= maxNum; i += p)
			numbers[i] = 1;

		p += 1;
		while (p <= sqrtMaxNum && numbers[p] == 1)
			p++;

	} while (p <= sqrtMaxNum);

	itype result = 0;
	for (itype i = 2; i < maxNum; i++)
		if (numbers[i] == 0)
			result++;

	return result;
}