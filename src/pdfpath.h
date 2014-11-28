#ifndef PDFPATH_H_
#define PDFPATH_H_

#include <istream>
#include <iostream>
#include <sstream>
#include <string>
#include <vector>

/*
 * A PDF path.
 */
typedef std::vector<std::string> pdfpath;

/*!
 * \brief A function that parses an input stream for PDF paths.
 *
 * Not thread-safe.
 *
 * @param in the open input stream from which to parse from.
 * @param p the resulting pdfpath objects in a std::vector.
 *
 * @throws a lot of different const char[] messages if there are parsing errors.
 */
void parse_pdfpaths(std::istream &in, std::vector<pdfpath> &v);

/*!
 * \brief A function that parses an input stream for a PDF path.
 *
 * Not thread-safe.
 *
 * @param in the open input stream from which to parse from.
 * @param p the resulting pdfpath object.
 *
 * @throws a lot of different const char[] messages if there are parsing errors.
 */
void parse_pdfpath(std::istream &in, pdfpath &path);

/*!
 * \brief Overloaded operator >>, parses a pdfpath object in the NPPF format.
 *
 * @param in the input stream.
 * @param p the parsed path.
 */
std::istream &operator>>(std::istream &in, pdfpath &path);

/*!
 * \brief Overloaded operator <<, prints a pdfpath object in the NPPF format.
 *
 * @param out the output stream.
 * @param p the path to print.
 */
std::ostream &operator <<(std::ostream &out, const pdfpath &path);

/*!
 * \brief Returns a string representation of a pdfpath object.
 *
 * @param path the path.
 */
std::string pdfpath_to_string(const pdfpath &path);

/*!
 * \brief Parses a string representation of a pdfpath from the stream.
 *
 * @param stream the stream.
 */
std::string get_pdfpath_string(std::istream &stream);

#endif /*PDFPATH_H_*/
