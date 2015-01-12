#include "pdfpath.h"

#include <cstdio>
#include <utility>
#include <vector>

#include <boost/regex.hpp>

namespace b = boost;

void parse_pdfpaths(std::istream &in, std::vector<pdfpath> &v) {
    static pdfpath path;
    v.clear();
    while (in.good() and in.peek() != EOF) {
        parse_pdfpath(in, path);
        v.push_back(path);
        if (in.get() != '\n' or not in.good()) {
            throw "pdfpath: Missing newline character.";
            return;
        }
    }
}

void parse_pdfpath(std::istream &in, pdfpath &path) {
    static const unsigned int BUF_LEN = 1024;
    char buffer[BUF_LEN];
    path.clear();
    while (in.good() and in.peek() != EOF) {
        // Read the name terminated by a null char
        in.get(buffer, BUF_LEN, '\0');

        // Empty names are not allowed, so an immediate second
        // null-byte signals a path end
        if (in.fail()) {
            // Clear only the failbit
            in.clear(in.rdstate() ^ std::ios_base::failbit);
            if (in.good() and in.get() == '\0') {
                // Got a new path!
                return;
            } else {
                throw "pdfpath: Missing end-of-path symbol.\n";
            }
        }
        // We have a name
        path.push_back(buffer);

        // Check for errors in the stream
        if (not (in.get() == '\0' and in.good())) {
            throw "pdfpath: Error parsing name.";
        }
    }
    throw "pdfpath: Empty input.";
}

std::istream &operator>>(std::istream &in, pdfpath &path) {
    parse_pdfpath(in, path);
    return in;
}

std::ostream &operator <<(std::ostream &out, const pdfpath &path) {
    for (pdfpath::const_iterator it = path.begin(); it != path.end(); it++) {
        out << *it << '\0';
    }
    return out << '\0';
}

std::string pdfpath_to_string(const pdfpath &path) {
    std::stringstream buf;
    for (const auto &p : path) {
        buf << p << '\0';
    }
    buf << '\0';
    return buf.str();
}

std::string get_pdfpath_string(std::istream &stream) {
    std::stringstream ss;
    while (stream.good() and stream.peek() != EOF) {
        char c = stream.get();
        ss.put(c);
        if (c == '\0' and stream.peek() == '\0') {
            ss.put(stream.get());
            return ss.str();
        }
    }
    throw;
}

/*
 * PDF path compaction regular expressions.
 */
typedef std::pair<b::regex, std::string> rp;
std::vector<rp> res;
void init_regex() {
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

std::string compact_pdfpath(const pdfpath &path) {
    static bool do_init = true;
    if (do_init) {
        init_regex();
        do_init = false;
    }
    std::string pathstr(pdfpath_to_string(path));
    for (const auto &re : res) {
        pathstr = b::regex_replace(pathstr, re.first, re.second,
                                   b::match_default | b::format_all);
    }
    return pathstr;
}
