/*
 * Copyright 2014 Nedim Srndic, University of Tuebingen
 *
 * This file is part of Hidost.

 * Hidost is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Hidost is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Hidost.  If not, see <http://www.gnu.org/licenses/>.
 *
 * feat-select.cpp
 *  Created on: Dec 06, 2013
 */

/*
 * This program reads a list of paths and their counts from the specified input
 * file and writes a list of paths with count greater than the specified N,
 * sorted by name, in the NPPF format in the specified output file.
 */

#include <iostream>
#include <cstdlib>
#include <fstream>

#include <boost/program_options.hpp>

#include "pdfpath.h"

namespace po = boost::program_options;

void exit_error(const char *e) {
    std::cerr << "feat-select: " << e << std::endl;
    std::exit(EXIT_FAILURE);
}

void feat_select(const char *in_name, const char *out_name,
                 unsigned int min_count) {
    std::ifstream in(in_name, std::ifstream::binary);
    std::ofstream out(out_name, std::ios::binary | std::ios::trunc);

    out << "NPPF" << '\0' << '\0' << '\n';
    pdfpath path;
    unsigned int count;
    while (in.good() and in.peek() != EOF) {
        in >> path >> count;
        in.get();
        if (count >= min_count) {
            out << path << std::endl;
        }
    }
}

po::variables_map parse_arguments(int argc, char *argv[]) {
    po::options_description desc(
            "This program reads a sorted list of paths and their counts from the specified input file and writes a list of paths with count greater than the specified N, sorted by name, in the NPPF format in the specified output file. Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("input-file,i",
                    po::value<std::string>()->required(),
                    "a file containing a list of paths and their counts, one per line")
            ("output-file,o",
                    po::value<std::string>()->required(),
                    "the NPPF output file to be created")
            ("min-count,m",
                    po::value<unsigned int>()->required(),
                    "minimal path count to be included in the output");

    po::variables_map vm;
    po::store(po::command_line_parser(argc, argv).options(desc).run(), vm);

    if (vm.count("help")) {
        std::cout << desc << std::endl;
        std::exit(EXIT_SUCCESS);
    }

    try {
        po::notify(vm);
    } catch (std::exception &e) {
        std::cerr << e.what() << std::endl << std::endl << desc << std::endl;
        std::exit(EXIT_FAILURE);
    }
    return vm;
}

int main(int argc, char *argv[]) {
    po::variables_map vm = parse_arguments(argc, argv);
    const std::string INPUT_FILE = vm["input-file"].as<std::string>();
    const std::string OUTPUT_FILE = vm["output-file"].as<std::string>();
    const unsigned int MIN_COUNT = vm["min-count"].as<unsigned int>();

    feat_select(INPUT_FILE.c_str(), OUTPUT_FILE.c_str(), MIN_COUNT);
    return EXIT_SUCCESS;
}
