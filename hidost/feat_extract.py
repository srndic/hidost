#!/usr/bin/env python
# -*- coding: utf-8 -*-
# feat_extract.py
# Copyright 2014 Nedim Srndic, University of Tuebingen
#
# This file is part of Hidost.
# Hidost is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# Hidost is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with Hidost. If not, see <http://www.gnu.org/licenses/>.
#
# Created on December 3, 2014.
"""
This script extracts structural features from SWF files and
saves them in a text file in LibSVM format.
"""
from __future__ import print_function

from argparse import ArgumentParser
import ast
import collections
import multiprocessing
import os
import pickle
import re
import shelve
import subprocess
import sys
import types


def append_libsvm(X, y, fd, comment=[]):
    """
    Appends the data in X and y in LibSVM format to open file
    descriptor fd. Optional comment is appended to the end of
    the line.
    """
    fd.write('{} '.format(int(y)))
    for i, v in enumerate(X, start=1):
        if v != 0.0:
            fd.write('{}:{} '.format(i, v))
    if comment:
        fd.write('#{}'.format(comment))
    fd.write('\n')


def run_SWFExtractor(swffile):
    """
    Runs a child SWFExtractor process on a given file and returns the
    standard output as a string. If the process doesn't terminate in 11
    seconds, it gets killed.
    """
    classpath = '{}:{}'.format(os.path.dirname(__file__),
                               os.getenv('CLASSPATH', ''))
    extractor = subprocess.Popen(['timeout', '-k 1s', '40s',
                                  'java', '-cp', classpath,
                                  'SWFExtractor', swffile],
                                 stdout=subprocess.PIPE)
    (stdoutdata, stderrdata) = extractor.communicate()
    if stderrdata is not None and len(stderrdata) > 0:
        sys.stderr.write('Child (SWFExtractor) error: %s\n' % stderrdata)
        raise
    else:
        return stdoutdata

# Precompiled REs for compacting repetitive SWF structural paths
_comp1_re = re.compile(r'(DefineSprite\x00ControlTags\x00){2,}')
_comp2_re = re.compile(r'(Symbol\x00Name\x00){2,}')
# Precompiled RE for matching a line of SWFExtractor.java output
_line_re = re.compile(r'^(?P<level> *)\[.*?\]: (?P<label>\w+)( : (?P<value>\d+|true|false|[-+]?[0-9]*\.?[0-9]+|.*)|.*)?$')


def get_swf_structure(f, verbose=False, do_compact=True):
    """
    Returns a dictionary containing all structural paths and their
    values in the given SWF file. If verbose, will write a lot to
    sys.stderr. String values are discarded. Returns None on failure.
    """
    swfstr = 0
    try:
        swfstr = run_SWFExtractor(f).splitlines()
    except:
        if verbose:
            sys.stderr.write('Child process exited unexpectedly.\n')
        return None

    if not swfstr or swfstr[-1] != 'OKAY - DONE!':
        if verbose:
            sys.stderr.write('Child process exited unexpectedly.\n')
        if verbose and len(swfstr) >= 1:
            sys.stderr.write(swfstr[-1] + '\n')
        if verbose and len(swfstr) >= 2:
            sys.stderr.write(swfstr[-2] + '\n')
        return None

    s = {}  # SWF file structure
    path_maker = [''] * 1000
    level = 0
    for linenum, line in enumerate(swfstr, start=1):
        if line.find('[') == -1:
            if verbose:
                sys.stderr.write('-{:>03d}: {}\n'.format(linenum, line))
            continue
        if verbose:
            sys.stderr.write(' {:>03d}: {:<60s}'.format(linenum, line))
        match = _line_re.match(line)
        if not match:
            if verbose:
                sys.stderr.write('No match.\n')
            continue
        match = match.groupdict()
        level = len(match['level']) / 2
        path_maker[level] = match['label']
        if match['value'] is None:
            if verbose:
                sys.stderr.write('No value.\n')
        else:
            pathstr = '{}\0'.format('\0'.join(path_maker[:level + 1]))
            if do_compact:
                pathstr = _comp1_re.sub('DefineSprite\x00ControlTags\x00',
                                        pathstr)
                pathstr = _comp2_re.sub('Symbol\x00Name\x00', pathstr)

            # Deduce value type
            try:
                value = ast.literal_eval(match['value'])
            except:
                value = match['value']
                if value == 'true':
                    value = True
                elif value == 'false':
                    value = False

            # Discard strings, Nones, tuples and longs
            if (isinstance(value, str) or
                    isinstance(value, types.NoneType) or
                    isinstance(value, tuple) or
                    isinstance(value, long)):
                continue

            # Save value
            if pathstr in s:
                s[pathstr].append(value)
            else:
                s[pathstr] = [value]

            if verbose:
                sys.stderr.write('{}\n'.format(repr((match['value'], value))))

    # Convert all values of individual paths to the majority type
    for k in s.iterkeys():
        v = s[k]
        if len(v) == 1:
            continue
        c = collections.Counter(map(type, v))
        if len(c) == 1:
            continue
        else:
            try:
                s[k] = map(c.most_common(1)[0][0], v)
            except ValueError as e:
                sys.stderr.write('==> ValueError in get_swf_structure()\n')
                raise e

    return s if s else None


def reduce_structure(f, label, verbose=False):
    """
    Reduces the SWF file structure by discarding information.
    All values belonging to a single path are processed according to
    their type. For integers and floats, all values except for the
    median are discarded. Booleans are reduced to their mean value.
    All returned values are converted to floating-point.
    """
    try:
        s = get_swf_structure(f)
    except:
        return None, label
    if s is None:
        # Probably an error occured in Java code
        return None, label
    for k in s.keys():
        vals = s[k]
        if verbose:
            sys.stderr.write('[{}]: {} '.format(k, repr(vals)))
        if isinstance(vals[0], bool):
            # For booleans, only keep mean
            s[k] = float(sum(vals)) / len(vals)
        elif isinstance(vals[0], int) or isinstance(vals[0], float):
            # For integers and floats, only keep median
            vl = len(vals)
            if vl % 2 == 0:
                s[k] = sum(sorted(vals)[vl / 2 - 1: vl / 2 + 1]) / 2.0
            else:
                s[k] = float(sorted(vals)[vl / 2])
        else:
            raise TypeError('Unexpected value type: {}'.format(type(vals[0])))
        if verbose:
            sys.stderr.write('{}\n'.format(s[k]))
    return s, label


def reduce_structure_fn(args):
    """
    Helper function for multiprocessing.
    """
    return reduce_structure(*args)


def extract_structures(bfs, mfs, cache, N_parallel, verbose=False):
    """
    Extracts and returns SWF file structures from files in bfs and
    mfs. Only processes files missing from cache and caches the
    obtained structures. Uses N_parallel processes.
    """
    pool_args = []

    def check_cache(flist, label):
        for i, f in enumerate(flist, start=1):
            if f not in cache:
                pool_args.append((f, label))

    check_cache(bfs, False)
    check_cache(mfs, True)
    if pool_args:
        print('Files to process (not in cache):', len(pool_args))
        pool = multiprocessing.Pool(N_parallel, maxtasksperchild=10)
        for i, (s, label) in enumerate(pool.imap(reduce_structure_fn,
                                                 pool_args)):
            fname = pool_args[i][0]
            if s is not None:
                s['f'] = fname
                s['m'] = label
                print('{:>5}: {}'.format(i, fname))
            else:
                print('ERROR {:>5}: {}'.format(i, fname))
            cache[fname] = s
        pool.close()
        pool.join()
    else:
        print('All files already cached')

    return [cache[f] for f in bfs + mfs if cache[f] is not None]


def main():
    cachef = os.path.abspath(os.path.join(os.path.dirname(__file__),
                                          'swf.cache'))
    parser = ArgumentParser(description=__doc__)
    parser.add_argument('-b', '--ben',
                        default=False,
                        help='A text file containing a list of paths to '
                        'benign SWF files, one per line.')
    parser.add_argument('-m', '--mal',
                        default=False,
                        help='A text file containing a list of paths to '
                        'malicious SWF files, one per line.')
    parser.add_argument('-o', '--out',
                        required=True,
                        help='Where to save output data file in LibSVM '
                        'format.')
    parser.add_argument('-f', '--features',
                        default=False,
                        help='A pickled list of features to extract; if '
                        'not provided all features found in the dataset '
                        'will be used.')
    parser.add_argument('-s', '--save-features',
                        default=False,
                        help='Where to save a pickled list of all features '
                        'found in the dataset.')
    parser.add_argument('-N', '--N-parallel',
                        type=int, default=1,
                        help='Number of parallel processes to run for '
                        'SWF structure extraction; expect occasional '
                        'explosion of memory use (jumps to >2GB per '
                        'process) due to invoked Java code (default: 1)')
    parser.add_argument('-c', '--cache',
                        default=False,
                        help='Where to cache extracted structures (file '
                        'name, default: {})'.format(cachef))

    args = parser.parse_args()
    bfs, mfs = [], []
    if args.ben:
        bfs = open(args.ben, 'rb').read().splitlines()
    if args.mal:
        mfs = open(args.mal, 'rb').read().splitlines()
    if not args.ben and not args.mal:
        print('At least one of -b, -m is required.')
        return 1

    if args.cache:
        cachef = args.cache
    print('Opening cache file [{}]'.format(cachef))
    cache = shelve.open(cachef)

    print('Extracting uncached SWF file structures')
    structs = extract_structures(bfs, mfs, cache, args.N_parallel)
    print('Successfully extracted {}/{} structures'.format(len(structs),
                                                           len(bfs) +
                                                           len(mfs)))
    feats = 0
    if args.features:
        feats = pickle.load(open(args.features, 'rb'))
    else:
        print('Enumerating all features in the dataset')
        feats = set()
        for s in structs:
            feats.update(s.keys())
        feats = sorted(list(feats.difference(['m', 'f'])))
        if args.save_features:
            print('Saving pickled feature list [{}]'.format(
                args.save_features))
            pickle.dump(feats, open(args.save_features, 'wb+'))
    print('Number of features:', len(feats))

    print('Generating data file [{}]'.format(args.out))
    with open(args.out, 'wb+') as of:
        for s in structs:
            append_libsvm([s.get(p, 0.0) for p in feats],
                          s['m'], of, s['f'])

    return 0

if __name__ == '__main__':
    sys.exit(main())
