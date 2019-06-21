#!/usr/bin/env python
# coding: utf-8

# Copyright (c) IPython Development Team.
# Distributed under the terms of the Modified BSD License.
from __future__ import print_function

import setuptools
from setuptools import setup
from setuptools.extension import Extension
from codecs import open
from os import path, makedirs, walk
from shutil import copytree, rmtree, copy2, move
import sysconfig
from glob import glob
from pathlib import Path
import platform
import os.path

# the name of the package
name = 'YAP4PY'

# -----------------------------------------------------------------------------
# Minimal Python version sanity check
# -----------------------------------------------------------------------------

import sys

v = sys.version_info
if v[:2] < (2, 7) or (v[0] >= 3 and v[:2] < (3, 3)):
    error = "ERROR: %s requires Python version 2.7 or 3.3 or above." % name
    print(error, file=sys.stderr)
    sys.exit(1)

PY3 = (sys.version_info[0] >= 3)

# -----------------------------------------------------------------------------
# get on with it
# -----------------------------------------------------------------------------
from codecs import open
from os import path, makedirs, walk
from shutil import copytree, rmtree, copy2, move
from glob import glob
from pathlib import Path
import platform
from os.path import dirname, join,abspath
import os
import shutil

from distutils.core import setup

here = abspath(dirname(__file__))
libpydir = abspath(sysconfig.get_path('platlib'))
#libpyauxdir = abspath(os.path.dirname('stdlib'))
#pkg_root = join(here, name)

here = path.abspath(path.dirname(__file__))

sys.path.insert(0, "/home/vsc/github/yap-6.3/packages/python/swig")

python_libdir = path.abspath(path.dirname("/usr/lib/libpython3.7m.so"))

if platform.system() == 'Windows':
    local_libs = []
    win_libs =  ['wsock32','ws2_32']
    my_extra_link_args = ['-Wl,-export-all-symbols']
elif platform.system() == 'Darwin':
    my_extra_link_args = ['-L','..','-Wl,-rpath,'+abspath(join(sysconfig.get_path('platlib'),'yap4py')),'-Wl,-rpath,/lib','-Wl,-rpath,../yap4py']
    win_libs = []
    local_libs = ['Py4YAP']
elif platform.system() == 'Linux':
    my_extra_link_args = ['-L','..','-Wl,-rpath,'+abspath(join(sysconfig.get_path('platlib'),'yap4py')),'-L','/lib','-Wl,-rpath,/lib','-Wl,-rpath,'+join('/lib','..'),'-Wl,-rpath,../yap4py']
    win_libs = []
    local_libs = ['Py4YAP']

# or dll in glob('yap/dlls/*'):
#    move(  dll ,'lib' )

native_sources = ["yap4py/yap_wrap.cxx","yap4py/yapi.cpp"]

#gmp_dir = path.abspath(path.dirname("/lib/libgmp.so"))
# Get the long description from the README file



extensions = [Extension('_yap', native_sources,
                    define_macros=[('MAJOR_VERSION', '6'),
                                       ('MINOR_VERSION', '5'),
                                       ('_YAP_NOT_INSTALLED_', '1'),
                                       ('YAP_PYTHON', '1'),
                                       ('PYTHONSWIG', '1'),
                                       ('_GNU_SOURCE', '1')],
                        runtime_library_dirs=[
                                              abspath(join(sysconfig.get_path('platlib'),'yap4py')), abspath(sysconfig.get_path('platlib')),'/lib'],
                        swig_opts=['-modern', '-c++', '-py3',
                                   '-DX_API', '-Iyap4py/include' ],
                        library_dirs=[".",'../../..','/lib'],
                        extra_link_args=my_extra_link_args,
                        libraries=['Yap','gmp']+win_libs+local_libs,
                        include_dirs=['/home/vsc/github/yap-6.3/H',
                        '/home/vsc/github/yap-6.3/H/generated',
                        '/home/vsc/github/yap-6.3/include',
                        '/home/vsc/github/yap-6.3/OPTYap',
                        '/home/vsc/github/yap-6.3/os',
                        '/home/vsc/github/yap-6.3/utf8proc',
                        '/home/vsc/github/yap-6.3/packages/python',
                        '/home/vsc/github/yap-6.3',
                        '/home/vsc/github/yap-6.3/CXX' ]
                        )]


package_data = {
    #'': '*.*'
}

data_files=[]

version_ns = {'__version__': '6.5.0', 'major-version': '6', 'minor-version': '5', 'patch': '0'}

setup_args = dict(
    name=name,
    version=version_ns['__version__'],
    scripts=glob(join('scripts', '*')),
    packages=['yap4py'],
    ext_modules=extensions,
    sources=['yap4py/yapi.py','yap4py/yap.py','yap4py/__main__.py','yap4py/__init_.py'],
#    package_data=package_data,
    include_package_data=True,
    data_files = data_files,
    # requirements=[
    # 'm2w64-gmp',
    # 'm2-msys2-keyring',
    # 'm2-msys2-launcher-git',
    # 'm2-msys2-runtime',
    # ],
    description="YAP in Python",
    author='YAP Development Team',
    author_email='vsc@dcc.fc.up.pt',
    url='http://www.dcc.fc.up/~vsc/yap',
    license='BSD',
    platforms="Linux, Mac OS X, Windows",
    keywords=['Interactive', 'Interpreter', 'Shell', 'Web'],
    classifiers=[
        'Intended Audience :: Developers',
        'Intended Audience :: System Administrators',
        'Intended Audience :: Science/Research',
        'License :: OSI Approved :: BSD License',
        'Programming Language :: Python',
        'Programming Language :: Python :: 2.7',
        'Programming Language :: Python :: 3',
    ],
)

if 'develop' in sys.argv or any(a.startswith('bdist') for a in sys.argv):
    import setuptools

setuptools_args = {}
install_requires = setuptools_args['install_requires'] = [
]

extras_require = setuptools_args['extras_require'] = {
    'test:python_version=="2.7"': ['mock'],
    'test': ['nose_warnings_filters', 'nose-timer'],
}

if 'setuptools' in sys.modules:
    setup_args.update(setuptools_args)

if __name__ == '__main__':
    setup(**setup_args)
