// MIT License
//
// Copyright (c) 2018 Benjamin Kiesl
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.

#include "drat2er.h"

#include <iostream>
#include <stdexcept>
#include <string>

#include "CLI11.hpp"

using std::string;
using std::make_shared;
using std::max;
using std::cout;
using std::cerr;
using std::endl;

using namespace drat2er;
using namespace drat2er::options;

int main (int argc, char *argv[])
{
  CLI::App app {
    "drat2er transforms DRAT proofs into extended-resolution proofs.\n"
    "It takes as input a propositional formula (specified in the DIMCAS\n"
    "format) together with a DRAT proof, and outputs an extended-resolution\n"
    "proof of the formula in either the TRACECHECK format or the DRAT\n"
    "format. The description of this transformation can be found in the\n"
    "paper \"Extended Resolution Simulates DRAT\" (IJCAR 2018). If no\n"
    "output file is specified, the output is written to the standard output.\n"
  };

  string input_formula_path = "";
  app.add_option("input_formula", input_formula_path,
                 "Path to a DIMCAS file of the input formula."
                )->required()->check(CLI::ExistingFile);

  string input_proof_path = "";
  app.add_option("input_proof", input_proof_path,
                 "Path to a DRAT file of the input proof."
                )->required()->check(CLI::ExistingFile);

  string output_file_path = "";
  app.add_option("output_proof", output_file_path,
                 "Path for the output proof.", false)->type_name("FILE");

  bool is_verbose = false;
  app.add_flag("-v,--verbose", is_verbose,
               "Print information about the progress.");

  bool is_quiet = false;
  app.add_flag("-q,--quiet", is_quiet, "Suppress all output");

  bool is_compressed = false;
  app.add_flag("-c,--compress", is_compressed, 
               "Reduce proof size (increases runtime).");

  string output_format = "tracecheck";
  app.add_set("-f,--format", output_format, {"drat", "tracecheck"},
              "Format of the output proof (default: tracecheck).");


  CLI11_PARSE(app, argc, argv);

  VerbosityLevel verbosity =
      is_quiet ? QUIET
               : is_verbose ? VERBOSE : NORMAL;

  try {
    TransformDRATToExtendedResolution(input_formula_path, 
                                      input_proof_path,
                                      output_file_path, 
                                      output_format == "drat", 
                                      verbosity,
                                      is_compressed);
    if (verbosity > QUIET)
    {
      cout << "c drat2er: Proof successfully transformed." << endl;
    }
    return 0;
  } catch(const std::exception& ex) {
    cerr << "Error: " << ex.what() << endl;
    return 1;
  }
}
