================
HIDOST
================

------------------------------------------------------------------
Toolset for extracting document structures from PDF and SWF files
------------------------------------------------------------------

Copyright 2014 Nedim Srndic, University of Tuebingen
nedim.srndic@uni-tuebingen.de


Installation and Setup
===============================

Please consult the ``INSTALL.rst`` file.

Usage
===================

Hidost consists of two parts: one for PDF and another one for SWF
files. These two parts are used to extract features from these two
different file types and they both provide data files (in
LibSVM format) as output. However, here we will describe them
separately as their implementations and ways of use have little
in common.

Extracting Features from PDF Files
-------------------------------------

The PDF part was written in C++11 and consists of a toolchain of
executables. It requires as input a text file with a list of paths to
benign PDF files, one per line, and another text file with malicious
PDF files. We will refer to these files as ``bpdfs.txt`` and
``mpdfs.txt`` in this description. The output of this toolchain is a
file (``data.libsvm``) in LibSVM input format that contains the feature
vectors of both benign and malicious PDF files listed in ``bpdfs.txt``
and ``mpdfs.txt``.

Follow these steps to obtain the data file:

  1) Prepare the files ``bpdfs.txt`` and ``mpdfs.txt``.
  2) Position in the directory where Hidost has been built,
     as detailed in ``INSTALL.rst``::

       cd build/

  3) In order to avoid having to extract tree structures from PDF
     files multiple times, we will cache the extracted structures
     in cache directories, for benign and malicious files separately::

       mkdir cache-ben cache-mal
       ./src/cacher -i bpdfs.txt --compact --values -c cache-ben/ \
       -t10 -m256
       ./src/cacher -i mpdfs.txt --compact --values -c cache-mal/ \
       -t10 -m256

     We will need the absolute paths of all non-empty cached PDF
     structures in the following steps::

       find $PWD/cache-ben -name '*.pdf' -not -empty >cached-bpdfs.txt
       find $PWD/cache-mal -name '*.pdf' -not -empty >cached-mpdfs.txt
       cat cached-bpdfs.txt cached-mpdfs.txt >cached-pdfs.txt

  4) Now we will count in how many PDF files each of the PDF
     structural paths occur::

       ./src/pathcount -i cached-pdfs.txt -o pathcounts.bin

  5) The next step is feature selection. We will only take into account
     structural paths present in at least 1,000 PDF files in our
     dataset::

       ./src/feat-select -i pathcounts.bin -o features.nppf -m1000

  6) Finally, we will extract the selected features from all files and
     store the result in the output file ``data.libsvm``::

       ./src/feat-extract -b cached-bpdfs.txt -m cached-mpdfs.txt \
       -f features.nppf --values -o data.libsvm

The output file ``data.libsvm`` can now be used for learning and
classification.

Extracting Features from SWF Files
-------------------------------------

The SWF part was written in Python 2.7 and Java. It requires as input
a text file with a list of paths to benign PDF files, one per line,
and another text file with malicious PDF files. We will refer to these
files as ``bpdfs.txt`` and ``mpdfs.txt`` in this description. The output
of this toolchain is a file (``data.libsvm``) in LibSVM input format
that contains the feature vectors of both benign and malicious PDF files
listed in ``bpdfs.txt`` and ``mpdfs.txt``.

Follow these steps to obtain the data file:

  1) Prepare the files ``bpdfs.txt`` and ``mpdfs.txt``.
  2) Position in the directory with Hidost Python source code,
     ``hidost/hidost/``::

       cd hidost/

  3) Use the Python script ``feat_extract.py`` to extract all
     features from all SWF files. The features will be pickled to
     a file called ``features.pickle`` and the feature vectors will be
     saved in the output file ``data.libsvm``::

       python feat_extract.py -b bswfs.txt -m mswfs.txt \
       -s features.pickle -o data.libsvm

The output file ``data.libsvm`` can now be used for learning and
classification.

Licensing
=================

Hidost is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Hidost is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with Hidost.  If not, see <http://www.gnu.org/licenses/>.
