================
HIDOST
================

Copyright 2014 Nedim Srndic, University of Tuebingen
nedim.srndic@uni-tuebingen.de

Source code for the work published in:
Nedim Srndic, Pavel Laskov. **Detection of Malicious PDF Files Based on
Hierarchical Document Structure**. In *Proceedings of the 20th Annual
Network & Distributed System Security Symposium*, 2013.



Installation and Setup
===============================

Hidost feature extraction is programmed in C++11, learning and
classification in Python 2.7. It was developed and tested on
64-bit Ubuntu 12.04.

Required Dependencies
-----------------------

The following C++ libraries are required (default versions
from Ubuntu 12.04):

- boost_filesystem
- boost_program_options
- boost_regex
- boost_system
- boost_thread
- poppler
- libquickly (part of this distribution; see its
              README file for more information)

The following Python libraries are required:

- scikit-learn 0.14.1

IMPORTANT: Please build and install libquickly first and then Hidost.

Use the hidost/bashsrc/install.sh script for installation. The script
code contains usage instructions.


Usage
===================

Use the hidost/bashsrc/featextract.sh script for feature extraction.
The script code contains usage instructions.

Use the hidost/bashsrc/classify.sh script for classification. The
script code contains usage instructions.

Use the learned model in hidost/ndss2013-model/model.pickle.


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
