/*
 * Copyright 2012-2015 Nedim Srndic, University of Tuebingen
 *
 * This file is part of Hidost.
 *
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
 * NPPFFile.cpp
 *  Created on: Oct 4, 2012
 */

#include "NPPFFile.h"

InNPPFFile::InNPPFFile(const char *fname) :
        in(fname, std::ifstream::binary) {
    static const char HEADER[] = "NPPF\0\0";
    static const unsigned int BUF_LEN = 6 + 1; // length of header including two null bytes
    char buffer[BUF_LEN];

    // Extract file header
    if (in.good() and in.peek() != EOF) {
        // Read the header terminated by a newline
        in.get(buffer, BUF_LEN);
        // Extract the newline character
        in.get();
    } else {
        throw INNPPFFILE_CLASS_NAME": Bad input file.\n";
    }

    // Verify file header
    for (unsigned int i = 0U; i < BUF_LEN; i++) {
        if (buffer[i] != HEADER[i]) {
            throw INNPPFFILE_CLASS_NAME": Bad file header.\n";
        }
    }
}

InNPPFFile::const_iterator::reference InNPPFFile::const_iterator::operator*() {
    std::streampos oldpos(f->in.tellg());
    if (oldpos != cachepos) {
        cache = get_pdfpath_string(f->in);
        f->in.seekg(oldpos, f->in.beg);
        cachepos = oldpos;
    }
    return cache;
}

InNPPFFile::const_iterator& InNPPFFile::const_iterator::operator++() {
    get_pdfpath_string(f->in);
    if (f->in.peek() == '\n') {
        f->in.get();
    }
    if (f->in.peek() == EOF) {
        f = 0U;
    }
    return *this;
}
