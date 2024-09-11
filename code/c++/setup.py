from setuptools import setup, Extension
import numpy as np

module = Extension('pvzinterface', sources=['code/c++/pvzinterface.cpp', 'code/c++/pvz.cpp', 'code/c++/machinecode.cpp', 'code/c++/memoryconfig.cpp', 'code/c++/memorylocator.cpp', 'code/c++/util.cpp'],
                   include_dirs=['.', 'code/c++', np.get_include(), "build/_deps/tomlplusplus-src/include"],
                   extra_compile_args=['/std:c++20'])

setup(
    name='pvz',
    version='1.0',
    description='Python wrapper for a C module',
    ext_modules=[module]
)