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
 * merger.cpp
 *  Created on: Dec 05, 2013
 */

/*
 * This program merges structural paths and their counts from two files
 * and saves the result into a third file.
 */

#include <iostream>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <fstream>
#include <sstream>

#include <stdlib.h>	// mkstemp()
#include <unistd.h> // write(), close()

#include "pdfpath.h"

void exit_error(const char *e) {
    std::cerr << "merger: " << e << std::endl;
    std::exit(EXIT_FAILURE);
}

// If true, every path count read from the file will be treated as a one
bool count_one = false;

bool okstream(std::ifstream &s) {
    return s.good() and s.peek() != EOF;
}

void read_line(std::ifstream &f, std::string &p, unsigned int &c) {
    p = get_pdfpath_string(f);
    f >> c;
    if (count_one) {
        c = 1U;
    }
    f.get();
}

void write_line(int fd, const std::string &p, unsigned int c) {
    static std::stringstream ss;
    ss << p << ' ' << c << '\n';
    std::string buf(ss.str());
    write(fd, buf.c_str(), buf.size());
    ss.str("");
}

void copy_trailer(int fd, std::ifstream &f) {
    std::string path;
    while (okstream(f)) {
        unsigned int count;
        read_line(f, path, count);
        write_line(fd, path, count);
    }
}

void merge(const char *fname1, const char *fname2) {
    std::ifstream f1(fname1, std::ios::binary), f2(fname2, std::ios::binary);
    char tmpname[] = "/tmp/mergerXXXXXX";
    int fd = mkstemp(tmpname);
    if (fd == -1) {
        exit_error("mkstemp() problem");
    }
    std::cout << tmpname << std::endl;
    std::string p1, p2;
    unsigned int c1, c2;
    bool do_read = true;
    
    while (not do_read or (okstream(f1) and okstream(f2))) {
        if (do_read) {
            read_line(f1, p1, c1);
            read_line(f2, p2, c2);
        }
        do_read = true;
        
        if (p1 < p2) {
            write_line(fd, p1, c1);
            while (okstream(f1)) {
                read_line(f1, p1, c1);
                if (p1 < p2) {
                    write_line(fd, p1, c1);
                } else {
                    do_read = false;
                    break;
                }
            }
        } else if (p2 < p1) {
            write_line(fd, p2, c2);
            while (okstream(f2)) {
                read_line(f2, p2, c2);
                if (p2 < p1) {
                    write_line(fd, p2, c2);
                } else {
                    do_read = false;
                    break;
                }
            }
        }
        if (p1 == p2) {
            write_line(fd, p1, c1 + c2);
            do_read = true;
        }
    }
    
    copy_trailer(fd, f1);
    copy_trailer(fd, f2);
    close(fd);
}

int main(int argc, char *argv[]) {
    if (argc != 4) {
        exit_error("Wrong count of arguments.\n"
                   "Usage: merger file1 file2 (1|n)");
    }
    
    if (strncmp(argv[3], "1", 1) == 0) {
        count_one = true;
    } else if (strncmp(argv[3], "n", 1) == 0) {
        count_one = false;
    } else {
        exit_error("Third argument must be either '1' or 'n'.");
    }
    
    merge(argv[1], argv[2]);
    
    return EXIT_SUCCESS;
}
