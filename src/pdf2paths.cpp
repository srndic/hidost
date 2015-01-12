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
 * pdf2paths.cpp
 *  Created on: Nov 21, 2013
 */

/*
 * This program extracts and prints structural paths from PDF files.
 */

#include <iostream>
#include <cstdlib>
#include <cstring>
#include <map>
#include <set>
#include <utility>
#include <queue>

#include <boost/regex.hpp>
#include <poppler/GlobalParams.h>
#include <poppler/PDFDoc.h>

#include "pdfpath.h"

#define PROG_NAME "pdf2paths: "

void exit_error(const char *e) {
    std::cerr << PROG_NAME << e << std::endl;
    std::exit(EXIT_FAILURE);
}

// True if path compaction is to be done
bool do_compact = false;

void printPath(const pdfpath &path, bool printAll = false) {
    static std::map<std::string, unsigned int> paths;
    if (printAll) {
        // Print path
        for (const auto &p : paths) {
            std::cout << p.first << ' ' << p.second << std::endl;
        }
    } else {
        // Insert path
        std::string pathstr;
        if (do_compact) {
            pathstr = compact_pdfpath(path);
        } else {
            pathstr = pdfpath_to_string(path);
        }
        if (pathstr.size() < 2) {
            // Remove empty paths (consisting of 2 null-bytes)
            return;
        }
        auto it = paths.find(pathstr);
        if (it == paths.end()) {
            paths.insert({pathstr, 1U});
        } else {
            it->second += 1U;
        }
    }
}

// Required to put Ref into std::map
bool operator<(const Ref& lhs, const Ref& rhs) {
    if (lhs.gen < rhs.gen) {
        return true;
    } else {
        return lhs.num < rhs.num;
    }
}

XRef *xref;
void bfs() {
    typedef std::map<std::string, int> keymap;
    typedef std::pair<Object *, pdfpath> bfsnode;
    static const std::string noname("<nn>");
    Object *root = new Object();
    xref->getCatalog(root);
    if (root->isNull()) {
        exit_error("Malformed Catalog dictionary.");
    }
    std::queue<bfsnode> unvisited;
    unvisited.push(bfsnode(root, pdfpath()));

    while (unvisited.size() > 0) {
        bfsnode node = unvisited.front();
        Object *o = node.first;
        pdfpath &path = node.second;

        switch (o->getType()) {
        case objArray: {
            Array *a = o->getArray();
            for (int i = 0; i < a->getLength(); i++) {
                Object *op = new Object();
                a->getNF(i, op);
                switch (op->getType()) {
                case objDict:
                case objStream:
                case objArray:
                case objRef:
                    unvisited.push(bfsnode(op, path));
                    break;
                case objEOF:
                case objError:
                case objNone:
                case objCmd:
                    std::cerr << PROG_NAME"Unexpected error in array.\n";
                    break;
                default:
                    // A simple PDF type or objUint
                    printPath(path);
                    delete op;
                }
            }
            if (a->getLength() == 0) {
                // Empty array
                printPath(path);
            }
        }
            break;
        case objDict: {
            keymap keys;
            Dict *d = o->getDict();
            // Sort keys
            for (int i = 0; i < d->getLength(); i++) {
                keys.insert({d->getKey(i), i});
            }
            for (const auto &key : keys) {
                path.push_back(key.first.size() ? key.first : noname);
                Object *op = new Object();
                d->getValNF(key.second, op);
                unvisited.push(bfsnode(op, path));
                path.pop_back();
            }
            if (d->getLength() == 0) {
                // Empty dict
                printPath(path);
            }
        }
            break;
        case objStream: {
            keymap keys;
            Dict *d = o->getStream()->getDict();
            // Sort keys
            for (int i = 0; i < d->getLength(); i++) {
                keys.insert({d->getKey(i), i});
            }
            for (const auto &key : keys) {
                path.push_back(key.first.size() ? key.first : noname);
                Object *op = new Object();
                d->getValNF(key.second, op);
                unvisited.push(bfsnode(op, path));
                path.pop_back();
            }
            if (d->getLength() == 0) {
                // Empty stream
                printPath(path);
            }
        }
            break;
        case objRef: {
            static std::set<Ref> printedRefs;
            Ref r = o->getRef();
            if (printedRefs.count(r) == 0) {
                printedRefs.insert(r);
                Object *op = new Object();
                xref->fetch(r.num, r.gen, op);
                unvisited.push(bfsnode(op, path));
            }
            printPath(path);
        }
            break;
        case objError:
            std::cerr << PROG_NAME"objError\n";
            break;
        case objEOF:
            std::cerr << PROG_NAME"objEOF\n";
            break;
        case objNone:
            std::cerr << PROG_NAME"objNone\n";
            break;
        case objCmd:
            std::cerr << PROG_NAME"objCmd\n";
            break;
        default:
            // Simple type or objUint
            printPath(path);
            break;
        }
        delete o;
        unvisited.pop();
    }
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    if (argc != 3) {
        exit_error("Wrong arguments. Usage: pdf2paths file_name (y|n)");
    }
    if (strncmp(argv[2], "y", 1) == 0) {
        do_compact = true;
    } else if (strncmp(argv[2], "n", 1) == 0) {
        do_compact = false;
    } else {
        exit_error("Last argument must be 'y' or 'n'.");
    }

    globalParams = new GlobalParams();
    PDFDoc *pdfdoc = new PDFDoc(new GooString(argv[1]));
    if (!pdfdoc->isOk()) {
        exit_error("Error in the PDF document.");
    }

    xref = pdfdoc->getXRef();
    if (!xref->isOk()) {
        exit_error("Error getting XRef.");
    }

    bfs();

    // Prints all paths sorted
    printPath(pdfpath(), true);
    return EXIT_SUCCESS;
}
