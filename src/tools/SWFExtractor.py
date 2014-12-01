"""
Copyright 2014 Nedim Srndic, University of Tuebingen
This file is part of Hidost.

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

Created on July 22, 2014.
"""

import ast
import collections
import re
import subprocess
import sys

import numpy
from sklearn.feature_extraction.text import HashingVectorizer

def SWFExtractor(swffile):
    '''
    Runs a child SWFExtractor process on a given file and returns the standard
    output as a string. If the process doesn't terminate in 11 seconds, it
    gets killed.
    '''
    extractor = subprocess.Popen(['timeout', '-k 1s', '40s', 'java', '-cp', '/guest/bin/SWFREtools/bin/', 'de.cogsys.SWFExtractor', swffile], stdout=subprocess.PIPE)
    (stdoutdata, stderrdata) = extractor.communicate()
    if stderrdata is not None and len(stderrdata) > 0:
        sys.stderr.write('Child (SWFExtractor) error: %s\n' % stderrdata)
        raise
    else:
        return stdoutdata

# REs for compacting repetitive SWF structural paths
_compactor1_re = re.compile(r'(DefineSprite\x00ControlTags\x00){2,}')
_compactor2_re = re.compile(r'(Symbol\x00Name\x00){2,}')

def swf2map(swf_in, verbose=False, do_compact=True):
    '''
    Returns a dictionary containing all structural paths and their values
    in the given SWF file. If 'verbose', will write a lot to sys.stderr.
    Returns None on failure.
    '''
    swfout = []
    try:
        swfout = SWFExtractor(swf_in).splitlines()
    except:
        if verbose: sys.stderr.write('Child process exited unexpectedly.\n')
        return None

    if not swfout or swfout[-1] != 'OKAY - DONE!':
        if verbose: sys.stderr.write('Child process exited unexpectedly.\n')
        if verbose and len(swfout) >= 1: sys.stderr.write(swfout[-1] + '\n')
        if verbose and len(swfout) >= 2: sys.stderr.write(swfout[-2] + '\n')
        return None

    swfmap = {}
    path_maker = ['' for _ in range(0, 1000)]
    line_re = re.compile(r'^(?P<level> *)\[.*?\]: (?P<label>\w+)( : (?P<value>\d+|true|false|[-+]?[0-9]*\.?[0-9]+|.*)|.*)?$')
    linenum = 0
    level = 0
    for line in swfout:
        linenum += 1
        if line.find('[') == -1:
            if verbose: sys.stderr.write('-{:>03d}: {}\n'.format(linenum, line))
            continue
        if verbose: sys.stderr.write(' {:>03d}: {:<60s}'.format(linenum, line))
        match = line_re.match(line)
        if not match:
            if verbose: sys.stderr.write('No match.\n')
            continue
        match = match.groupdict()
        level = len(match['level']) / 2
        path_maker[level] = match['label']
        if match['value'] is None:
            if verbose: sys.stderr.write('No value.\n')
        else:
            pathstr = '{}\0'.format('\0'.join(path_maker[:level + 1]))
            if do_compact:
                pathstr = _compactor1_re.sub('DefineSprite\x00ControlTags\x00', pathstr)
                pathstr = _compactor2_re.sub('Symbol\x00Name\x00', pathstr)
            # Deduce value type
            try:
                value = ast.literal_eval(match['value'])
            except:
                value = match['value']
                if value == 'true':
                    value = True
                elif value == 'false':
                    value = False
            if pathstr in swfmap:
                swfmap[pathstr].append(value)
            else:
                swfmap[pathstr] = [value]
            if verbose: sys.stderr.write('{}\n'.format(repr((match['value'], value))))
            #if verbose: sys.stderr.write('Path [{}]: {}\n'.format(level, pathstr))

    # Convert all values of individual paths to the majority type
    for k in swfmap.iterkeys():
        v = swfmap[k]
        if len(v) == 1:
            continue
        c = collections.Counter(map(type, v))
        if len(c) == 1:
            continue
        if str in c:
            swfmap[k] = map(str, v)
        else:
            try:
                swfmap[k] = map(c.most_common(1)[0][0], v)
            except ValueError as e:
                sys.stderr.write('==> ValueError in swf2map()\n')
                raise e

    return swfmap if swfmap else None

def swf2paths(swf_in, verbose=False, do_compact=True):
    '''
    Returns a dictionary containing all structural paths and their counts
    in the given SWF file. If 'verbose', will write a lot to sys.stderr.
    Returns None on failure.
    '''
    swfout = []
    try:
        swfout = SWFExtractor(swf_in).splitlines()
    except:
        if verbose: sys.stderr.write('Child process exited unexpectedly.\n')
        return None

    if not swfout or swfout[-1] != 'OKAY - DONE!':
        if verbose: sys.stderr.write('Child process exited unexpectedly.\n')
        if verbose: sys.stderr.write(swfout[-1] + '\n')
        if verbose: sys.stderr.write(swfout[-2] + '\n')
        return None

    all_paths = {}
    path_maker = ['' for _ in range(0, 1000)]
    line_re = re.compile(r'^(?P<level> *)\[.*?\]: (?P<label>\w+)( : (?P<value>\d+|true|false|[-+]?[0-9]*\.?[0-9]+|.*)|.*)?$')
    linenum = 0
    level = 0
    for line in swfout:
        linenum += 1
        if line.find('[') == -1:
            if verbose: sys.stderr.write('-{:>03d}: {}\n'.format(linenum, line))
            continue
        if verbose: sys.stderr.write(' {:>03d}: {:<60s}'.format(linenum, line))
        match = line_re.match(line)
        if not match:
            if verbose: sys.stderr.write('No match.\n')
            continue
        match = match.groupdict()
#         if verbose: sys.stderr.write('{}'.format(repr(match)))
        level = len(match['level']) / 2
        path_maker[level] = match['label']
        if match['value'] is None:
            if verbose: sys.stderr.write('No value.\n')
        else:
            pathstr = '{}\0'.format('\0'.join(path_maker[:level + 1]))
            if do_compact:
                pathstr = _compactor1_re.sub('DefineSprite\x00ControlTags\x00', pathstr)
                pathstr = _compactor2_re.sub('Symbol\x00Name\x00', pathstr)
            if pathstr in all_paths:
                all_paths[pathstr] += 1
            else:
                all_paths[pathstr] = 1
            if verbose: sys.stderr.write('Path [{}]: {}\n'.format(level, pathstr))

    return all_paths if all_paths else None

_hv = HashingVectorizer(analyzer='char',
                       ngram_range=(3,3),
                       decode_error='replace',
                       norm='l1',
                       non_negative=True)

def swfmap2vector_nan(swfmap, all_paths):
    res = float('NaN') * numpy.empty(len(all_paths))
    for i, p in enumerate(all_paths):
        v = swfmap.get(p)
        if v is not None and type(v) != str:
                res[i] = float(v)
    return res

def swfmap2vector(swfmap, all_paths):
    res = numpy.zeros(len(all_paths))
    for i, p in enumerate(all_paths):
        v = swfmap.get(p, 0.0)
        if type(v) != str:
                res[i] = float(v)
    return res

#def swfmap2vector(swfmap, all_paths):
#    res = numpy.empty(len(all_paths))
#    for i, p in enumerate(all_paths):
#        v = swfmap.get(p)
#        if v is not None:
#            if type(v) != str:
#                res[i] = float(v)
#            else:
#                # a hack to calculate the norm of a sparse vector
#                res[i] = math.sqrt((_hv.fit_transform([v]).data ** 2).sum())
#    return res

def get_all_paths(swfmaps):
    '''
    Returns a sorted list of all paths found in the provided
    SWF map iterable.
    '''
    res = set()
    for sm in swfmaps:
        res.update(sm.keys())
    return sorted(list(res.difference(['m', 'f'])))