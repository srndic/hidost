#include "pdfpath.h"

#include <cstdio>

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
