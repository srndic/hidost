/*
 * Copyright 2014 Nedim Srndic, University of Tuebingen
 *
 * This file is part of pdf2misc.

 * pdf2misc is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * pdf2misc is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with pdf2misc.  If not, see <http://www.gnu.org/licenses/>.
 *
 * NPPFFile.h
 *  Created on: Oct 4, 2012
 */

#ifndef NPPFFILE_H_
#define NPPFFILE_H_

#include <fstream>
#include <iterator>
#include <string>

#include "pdfpath.h"

#define INNPPFFILE_CLASS_NAME "InNPPFFile"

/*!
 * \brief A class representing a file in the Null-terminated PDF Path Format
 * (NPPF).
 *
 * An NPPF file has a header in this form: NPPF\0\0\n
 * All other lines in the file are PDF paths with individual path segments
 * (PDF names that make up the path) delimited with null-bytes (\0). There is
 * an extra null-byte at the end of each path, just before the newline (\n)
 * byte. This extra null-byte marks the end of the path. It is easily found
 * programatically as it comes right after the null-byte of the last path
 * segment of that path, so it is the only occurence of two consecutive
 * null-bytes.
 */
class InNPPFFile {
private:
    std::ifstream in;
public:
    /*!
     * \brief Constructor.
     *
     * Constructs a NPPFFile instance from an open file stream.
     *
     * @param ifile the open NPPF file stream.
     *
     * @throw A const char[] with the exception explanation if parsing goes
     * wrong.
     */
    explicit InNPPFFile(const char *fname);

    struct const_iterator: std::iterator<std::forward_iterator_tag, std::string,
            std::ptrdiff_t, const std::string*, const std::string&> {
    private:
        InNPPFFile *f;
        std::streampos cachepos;
        std::string cache;
    public:
        explicit const_iterator(InNPPFFile *f = 0U) :
                f(f), cachepos(), cache() {
        }
        
        const_iterator(const const_iterator &) = default;
        const_iterator &operator=(const const_iterator &) = default;
        ~const_iterator() = default;
        
        reference operator*();
        const_iterator& operator++();

        bool operator==(const const_iterator& other) const {
            return f == other.f;
        }
        bool operator!=(const const_iterator& other) const {
            return f != other.f;
        }
    };

    const_iterator begin() {
        return const_iterator(this);
    }
    const_iterator end() {
        return const_iterator();
    }
};

#endif /* NPPFFILE_H_ */
