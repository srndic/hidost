/*
 * Copyright 2012 Nedim Srndic, University of Tuebingen
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
 * feat-extract.cpp
 *  Created on: Dec 10, 2013
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <sstream>
#include <string>
#include <vector>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>	// boost::mutex
#include <quickly/DataAction.h>
#include <quickly/ThreadPool.h>

#include "NPPFFile.h"
#include "pdfpath.h"

namespace po = boost::program_options;

typedef std::vector<std::pair<std::string, bool> > filevector;

class DataActionImpl: public quickly::DataActionBase {
private:
    explicit DataActionImpl(unsigned int id) :
            DataActionBase(id) {
    }
    // A mutex for thread safety
    static boost::mutex mutex;
    // Full list of files to process
    static filevector all_files;
    // A set of all paths
    static std::set<std::string> features;
    // Index of the next file to process
    static unsigned int to_process;
    // The output file
    static std::ofstream out_file;
    // Class of input data
    static bool data_class;
    
    bool process(const std::string &line);
public:
    // Dummy constructor
    DataActionImpl() :
            DataActionBase(UINT_MAX) {
    }
    virtual ~DataActionImpl() {
    }
    // Virtual constructor
    virtual DataActionImpl *create(unsigned int id) {
        return new DataActionImpl(id);
    }
    
    // Static constructor
    static void init(const std::string &nppf_name, const std::string &out_file,
                     filevector all_files);
    
    // Overridden doFull() method
    virtual void doFull(std::stringstream &databuf);
};

boost::mutex DataActionImpl::mutex;
filevector DataActionImpl::all_files;
std::set<std::string> DataActionImpl::features;
std::ofstream DataActionImpl::out_file;

void DataActionImpl::init(const std::string &nppf_name,
                          const std::string &out_file, filevector all_files) {
    for (const auto &feat : InNPPFFile(nppf_name.c_str())) {
        DataActionImpl::features.insert(feat);
    }
    DataActionImpl::out_file.open(out_file, std::ios::binary | std::ios::trunc);
    DataActionImpl::all_files = all_files;
}

bool DataActionImpl::process(const std::string &line) {
    if (line.size() > 0) {
        boost::mutex::scoped_lock lock(DataActionImpl::mutex);
        DataActionImpl::out_file << line << std::endl;
    }
    return true;
}

void DataActionImpl::doFull(std::stringstream &databuf) {
    std::set<std::string>::const_iterator fi = DataActionImpl::features.begin();
    std::string path;
    std::stringstream ss;
    bool no_features = true;
    // Write file class
    ss << DataActionImpl::all_files[getId()].second << ' ';
    // Parse paths
    while (databuf.good() and databuf.peek() != EOF) {
        path = get_pdfpath_string(databuf);
        // Remove the space delimiter
        databuf.get();
        // Get the path value
        double val = 0.0;
        databuf >> val;
        // Remove trailing newline
        databuf.get();
        if (path < *fi) {
            continue;
        } else if (path > *fi) {
            do {
                fi++;
                if (fi == features.end()) {
                    goto end_of_features;
                }
            } while (path > *fi);
        }
        if (path == *fi) {
            // Write sparse positive feature
            ss << std::distance(DataActionImpl::features.begin(), fi) + 1U
               << ':' << val << ' ';
            no_features = false;
            fi++;
            if (fi == features.end()) {
                goto end_of_features;
            }
        }
    }
    end_of_features:
    
    // Write file name as comment
    ss << '#' << DataActionImpl::all_files[getId()].first;
    // Do not write empty lines
    std::string line(no_features ? std::string() : ss.str());
    while (not this->process(line)) {
    }
}

po::variables_map parse_arguments(int argc, char *argv[]) {
    po::options_description desc(
            "This program extracts boolean (path presence) features from files specified in the input file according to the feature (NPPF) file and stores them in libsvm format in the output file. Allowed options");
    desc.add_options()
            ("help", "produce help message")
            ("input-mal,m",
                    po::value<std::string>()->required(),
                    "a list of malicious path files, one per line")
            ("input-ben,b",
                    po::value<std::string>()->required(),
                    "a list of benign path files, one per line")
            ("features,f",
                    po::value<std::string>()->required(),
                    "an NPPF file containing the list of features to extract")
            ("output-file,o",
                    po::value<std::string>()->required(),
                    "the feature file to be created")
            ("vm-limit,M",
                    po::value<unsigned int>()->default_value(0U),
                    "limit the virtual memory of child processes in MB (default: no limit)")
            ("cpu-time,t",
                    po::value<unsigned int>()->default_value(0U),
                    "limit the CPU time of child processes in seconds (default: no limit)")
            ("parallel,N",
                    po::value<unsigned int>()->default_value(0U),
                    "number of child processes to run in parallel (default: number of cores minus one)");
    
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

void get_input_files(filevector &input_files, const char *in_name,
                     bool data_class) {
    std::string line;
    std::ifstream ifile(in_name, std::ios::binary);
    while (std::getline(ifile, line)) {
        input_files.push_back(std::pair<std::string, bool>(line, data_class));
    }
    ifile.close();
}

int main(int argc, char *argv[]) {
    // Parse arguments
    po::variables_map vm = parse_arguments(argc, argv);
    const std::string INPUT_MAL = vm["input-mal"].as<std::string>();
    const std::string INPUT_BEN = vm["input-ben"].as<std::string>();
    const std::string NPPF_FILE = vm["features"].as<std::string>();
    const std::string OUTPUT_FILE = vm["output-file"].as<std::string>();
    const unsigned int VM_LIMIT = vm["vm-limit"].as<unsigned int>();
    const unsigned int CPU_LIMIT = vm["cpu-time"].as<unsigned int>();
    const unsigned int PARALLEL = vm["parallel"].as<unsigned int>();
    
    // Read file list
    filevector input_files;
    get_input_files(input_files, INPUT_MAL.c_str(), true);
    get_input_files(input_files, INPUT_BEN.c_str(), false);
    DataActionImpl::init(NPPF_FILE, OUTPUT_FILE, input_files);
    
    std::vector<const char * const *> argvs;
    const char prog_name[] = "/guest/rarepos/cxx/hidost/build-debug/src/pdf2vals";
    const char *pn = prog_name;
    for (const auto &file : input_files) {
        const char **argv = (const char **) NULL;
        try {
            argv = new const char *[4] { pn, file.first.c_str(), "y", nullptr };
        } catch (...) {
            delete[] argv;
        }
        argvs.push_back(argv);
    }
    
    // Prepare the data action and perform scan
    DataActionImpl dummy;
    quickly::ThreadPool pool(pn, argvs, &dummy, PARALLEL);
    pool.setVerbosity(5U);
    pool.setVmLimit(VM_LIMIT * 1024U * 1024U);
    pool.setCpuLimit(CPU_LIMIT);
    pool.run();
    
    // Print results, delete command-line arguments
    for (unsigned int i = 0U; i < argvs.size(); i++) {
        delete[] argvs[i];
    }
    
    return EXIT_SUCCESS;
}
