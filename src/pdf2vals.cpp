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
 * pdf2vals.cpp
 *  Created on: Nov 21, 2013
 */

/*
 * This program extracts and prints structural paths and their
 * values from PDF files.
 */

#include <algorithm>
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

#define PROG_NAME "pdf2vals: "

namespace b = boost;

void exit_error(const char *e) {
    std::cerr << PROG_NAME << e << std::endl;
    std::exit(EXIT_FAILURE);
}

typedef std::pair<b::regex, std::string> rp;
std::vector<rp> res;

// True if path compaction is to be done
bool do_compact = false;

void initRegex() {
    // Resource dictionaries
    res.push_back(
            rp(b::regex(
                    "\\0Resources\\0(ExtGState|ColorSpace|Pattern|Shading|XObject|Font|Properties|Para)\\0[^\\0]+"),
               "\\0Resources\\0\\1\\0Name"));
    // Page tree
    res.push_back(
            rp(b::regex(
                    "^Pages\\0(Kids\\0|Parent\\0)*(Kids$|Kids\\0|Parent\\0|Parent$)"),
               "Pages\\0"));
    // Other Kids/Parent hierarchies (AcroForm?)
    res.push_back(
            rp(b::regex(
                    "\\0(Kids\\0|Parent\\0)*(Kids$|Kids\\0|Parent\\0|Parent$)"),
               "\\0"));
    // Prev, Next, First and Last links (Outline tree)
    res.push_back(rp(b::regex("(Prev\\0|Next\\0|First\\0|Last\\0)+"), ""));
    // Name trees
    res.push_back(
            rp(b::regex(
                    "^Names\\0(Dests|AP|JavaScript|Pages|Templates|IDS|URLS|EmbeddedFiles|AlternatePresentations|Renditions)\\0(Kids\\0|Parent\\0)*Names"),
               "Names\\0\\1\\0Names"));
    res.push_back(
            rp(b::regex("^StructTreeRoot\\0IDTree\\0(Kids\\0)*Names"),
               "StructTreeRoot\\0IDTree\\0Names"));
    // Number trees (parent tree)
    res.push_back(
            rp(b::regex(
                    "^(StructTreeRoot\\0ParentTree|PageLabels)\\0(Kids\\0|Parent\\0)+(Nums|Limits)"),
               "\\1\\0\\3"));
    res.push_back(
            rp(b::regex("^StructTreeRoot\\0ParentTree\\0Nums\\0(K\\0|P\\0)+"),
               "StructTreeRoot\\0ParentTree\\0Nums\\0"));
    // Names StructTree entries
    res.push_back(
            rp(b::regex(
                    "^(StructTreeRoot|Outlines\\0SE)\\0(RoleMap|ClassMap)\\0[^\\0]+"),
               "\\1\\0\\2\\0Name"));
    // StructTree
    res.push_back(
            rp(b::regex("^(StructTreeRoot|Outlines\\0SE)\\0(K\\0|P\\0)*"),
               "\\1\\0"));
    // Top-level dictionaries containing names
    res.push_back(rp(b::regex("^(Extensions|Dests)\\0[^\\0]+"), "\\1\\0Name"));
    // Char maps
    res.push_back(
            rp(b::regex("Font\\0([^\\0]+)\\0CharProcs\\0[^\\0]+"),
               "Font\\0\\1\\0CharProcs\\0Name"));
    // Extra AcroForm resources
    res.push_back(
            rp(b::regex(
                    "^(AcroForm\\0(Fields\\0|C0\\0)?DR\\0)(ExtGState|ColorSpace|Pattern|Shading|XObject|Font|Properties)\\0[^\\0]+"),
               "\\1\\3\\0Name"));
    // Annots
    res.push_back(
            rp(b::regex("\\0AP\\0(D|N)\\0[^\\0]+"), "\\0AP\\0\\1\\0Name"));
    // Threads
    res.push_back(rp(b::regex("Threads\\0F\\0(V\\0|N\\0)*"), "Threads\\0F"));
    // StructTree info
    res.push_back(
            rp(b::regex("^(StructTreeRoot|Outlines\\0SE)\\0Info\\0[^\\0]+"),
               "\\1\\0Info\\0Name"));
    // Colorant name
    res.push_back(
            rp(b::regex("ColorSpace\\0([^\\0]+)\\0Colorants\\0[^\\0]+"),
               "ColorSpace\\0\\1\\0Colorants\\0Name"));
    res.push_back(
            rp(b::regex("ColorSpace\\0Colorants\\0[^\\0]+"),
               "ColorSpace\\0Colorants\\0Name"));
    // Collection schema
    res.push_back(
            rp(b::regex("Collection\\0Schema\\0[^\\0]+"),
               "Collection\\0Schema\\0Name"));
}

std::map<std::string, std::vector<double>> pathvals;
void insertValue(const pdfpath &path, Object &o) {
    if (not (o.isBool()
             or o.isInt()
             or o.isNum()
             or o.isReal()
             or o.isUint())) {
        return;
    }
    // Convert path object to string
    std::string pathstr(pdfpath_to_string(path));
    if (do_compact) {
        for (const auto &re : res) {
            pathstr = b::regex_replace(pathstr, re.first, re.second,
                                       b::match_default | b::format_all);
        }
    }
    if (pathstr.size() < 2) {
        // Remove empty paths (consisting of 2 null-bytes)
        return;
    }

    // Convert type
    double v = 0.0;
    if (o.isBool()) {
        v = o.getBool() ? 1.0 : 0.0;
    } else if (o.isInt()) {
        v = static_cast<double>(o.getInt());
    } else if (o.isNum()) {
        v = o.getNum();
    } else if (o.isReal()) {
        v = o.getReal();
    } else if (o.isUint()) {
        v = static_cast<double>(o.getUint());
    }

    // Insert into the map
    auto it = pathvals.find(pathstr);
    if (it == pathvals.end()) {
        pathvals.insert({pathstr, std::vector<double>{v}});
    } else {
        it->second.push_back(v);
    }
}

void printPaths() {
    /* Prints paths and their median values.
     */
    for (auto &p : pathvals) {
        unsigned int median_i = p.second.size() / 2;
        std::nth_element(std::begin(p.second),
                         std::begin(p.second) + median_i,
                         std::end(p.second));
        std::cout << p.first << ' ' << p.second[median_i] << std::endl;
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
                    insertValue(path, *op);
                    delete op;
                }
            }
            // Empty array not handled
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
                path.push_back(key.first);
                Object *op = new Object();
                d->getValNF(key.second, op);
                unvisited.push(bfsnode(op, path));
                path.pop_back();
            }
            // Empty dict not handled
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
                path.push_back(key.first);
                Object *op = new Object();
                d->getValNF(key.second, op);
                unvisited.push(bfsnode(op, path));
                path.pop_back();
            }
            // Empty stream not handled
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
            // References not handled
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
            insertValue(path, *o);
            break;
        }
        delete o;
        unvisited.pop();
    }
}

int main(int argc, char *argv[]) {
    // Parse command-line arguments
    if (argc != 3) {
        exit_error("Wrong arguments. Usage: pdf2vals file_name (y|n)");
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

    if (do_compact) {
        initRegex();
    }

    bfs();

    // Prints all paths sorted
    printPaths();
    return EXIT_SUCCESS;
}
