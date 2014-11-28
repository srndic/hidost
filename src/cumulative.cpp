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
 * cumulative.cpp
 *  Created on: Nov 26, 2013
 */

/*
 * This program prints as output the cumulative increase in the
 * count of novel PDF paths for every PDF file given as input.
 */

#include <cstdlib>
#include <fstream>
#include <iostream>
#include <set>
#include <string>

#include <boost/program_options.hpp>
#include <boost/thread.hpp>	// boost::mutex
#include <openssl/md5.h>
#include <quickly/DataAction.h>
#include <quickly/ThreadPool.h>

#include "pdfpath.h"

namespace po = boost::program_options;

std::string md5_encode(const pdfpath &path) {
	unsigned char hash[MD5_DIGEST_LENGTH];
	std::string pathstr(pdfpath_to_string(path));
	MD5((unsigned char *) pathstr.c_str(), pathstr.length(), hash);
	return std::string((char *)hash);
}

class DataActionImpl: public quickly::DataActionBase {
private:
	explicit DataActionImpl(unsigned int id) :
			DataActionBase(id) {
	}
	// A mutex for thread safety
	static boost::mutex mutex;
	// A set of all paths
	static std::set<std::string> all_paths;
	// A vector of path counts
	static std::vector<int> path_counts;
	// Index of the next file to process
	static unsigned int to_process;

	bool process(const std::set<std::string> &newpaths);
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
	static void init(unsigned int path_count) {
		path_counts.resize(path_count, -1U);
	}
	static std::vector<int> *getPathCounts() {
		return &path_counts;
	}

	// Overridden doFull() method
	virtual void doFull(std::stringstream &databuf);
};

boost::mutex DataActionImpl::mutex;
std::set<std::string> DataActionImpl::all_paths;
std::vector<int> DataActionImpl::path_counts;
unsigned int DataActionImpl::to_process = 0U;

bool DataActionImpl::process(const std::set<std::string> &newpaths) {
	boost::mutex::scoped_lock lock(DataActionImpl::mutex);
	if (DataActionImpl::to_process != this->getId()) {
		return false;
	}
	if (newpaths.empty()) {
		DataActionImpl::path_counts[this->getId()] = 0;
		DataActionImpl::to_process++;
		return true;
	}
	unsigned int size_pre = DataActionImpl::all_paths.size();
	DataActionImpl::all_paths.insert(newpaths.begin(), newpaths.end());
	DataActionImpl::path_counts[this->getId()] =
			DataActionImpl::all_paths.size() - size_pre;
	DataActionImpl::to_process++;
	return true;
}

void DataActionImpl::doFull(std::stringstream &databuf) {
	std::set<std::string> paths;
	pdfpath tmppath;
	// Parse paths
	while (databuf.good() and databuf.peek() != EOF) {
		databuf >> tmppath;
		// Remove the space, path count and newline characters
		while (databuf.peek() != '\n') {
			databuf.get();
		}
		databuf.get();
		paths.insert(md5_encode(tmppath));
	}
	while (not this->process(paths)) {}
}

po::variables_map parse_arguments(int argc, char *argv[]) {
	po::options_description desc("This program counts the number of novel paths per path file, in order. Allowed options");
	desc.add_options()
			("help", "produce help message")
			("input-file,i", po::value<std::string>()->required(), "a list of path files, one per line")
			("parallel,N", po::value<unsigned int>()->default_value(0U), "number of child processes to run in parallel (default: number of cores minus one)");;

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

/*
 * Main program (parent executable).
 */
int main(int argc, char *argv[]) {
	// Parse arguments
	po::variables_map vm = parse_arguments(argc, argv);
	const std::string INPUT_FILE = vm["input-file"].as<std::string>();
	const unsigned int PARALLEL = vm["parallel"].as<unsigned int>();


	// Read file list
	std::vector<std::string> input_files;
	{
		std::string line;
		std::ifstream ifile(INPUT_FILE, std::ios::binary);
		while (std::getline(ifile, line)) {
			input_files.push_back(line);
		}
		ifile.close();
	}
	DataActionImpl::init(input_files.size());

	// Construct a vector of command-line arguments
	std::vector<const char * const *> argvs;
	const char prog_name[] = "/bin/cat";
	const char *pn = prog_name;
	for (unsigned int i = 0; i < input_files.size(); i++) {
		const char **argv = (const char **) NULL;
		try {
			argv = new const char *[3];
		} catch (...) {
			delete[] argv;
		}
		argv[0] = pn;
		argv[1] = input_files[i].c_str();
		argv[2] = (const char *) NULL;
		argvs.push_back(argv);
	}

	// Prepare the data action and perform scan
	DataActionImpl dummy;
	quickly::ThreadPool pool(pn, argvs, &dummy, PARALLEL);
	pool.setVerbosity(5U);
	pool.run();

	// Print results, delete command-line arguments
	for (unsigned int i = 0U; i < DataActionImpl::getPathCounts()->size();
			i++) {
		std::cout << DataActionImpl::getPathCounts()->operator[](i) << ' ';
		delete[] argvs[i];
	}

	return EXIT_SUCCESS;
}
