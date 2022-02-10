import inspect
import os
import platform
import re
import subprocess
import sys

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from distutils.version import LooseVersion


MIN_CPU_CORES = 2


def get_cpu_count():
    try:
        return len(os.sched_getaffinity(0))  # linux only
    except:
        pass

    try:
        return os.cpu_count()  # python 3.4+
    except:
        return 1  # default


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = os.path.abspath(sourcedir)


class CMakeBuild(build_ext):
    user_options = build_ext.user_options + [
        ('target=', None, "specify the CMake target to build")
    ]

    def initialize_options(self):
        self.target = "sonata_python"
        super(CMakeBuild, self).initialize_options()

    def run(self):
        try:
            out = subprocess.check_output(["cmake", "--version"])
        except OSError:
            raise RuntimeError(
                "CMake must be installed to build the following extensions: "
                + ", ".join(e.name for e in self.extensions)
            )

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = os.path.abspath(os.path.dirname(self.get_ext_fullpath(ext.name)))

        build_type =  os.environ.get("SONATA_BUILD_TYPE", "Release")
        if self.debug:
            build_type = "Debug"

        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + extdir,
            "-DSONATA_TESTS={}".format(os.environ.get("SONATA_TESTS", "OFF")),
            "-DEXTLIB_FROM_SUBMODULES=ON",
            "-DSONATA_PYTHON=ON",
            "-DSONATA_VERSION=" + self.distribution.get_version(),
            "-DCMAKE_BUILD_TYPE={}".format(build_type),
            "-DSONATA_CXX_WARNINGS=OFF",
            '-DPYTHON_EXECUTABLE=' + sys.executable
        ]

        build_args = ["--config", build_type,
                      "--target", self.target,
                      "--",
                      "-j{}".format(max(MIN_CPU_CORES, get_cpu_count())),
                      ]

        if not os.path.exists(self.build_temp):
            os.makedirs(self.build_temp)

        env = os.environ.copy()
        env["CXXFLAGS"] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get("CXXFLAGS", ""), self.distribution.get_version()
        )

        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env
        )

        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp
        )


# nearly verbatim from how h5py handles is
install_requires = [
    # We only really aim to support NumPy & Python combinations for which
    # there are wheels on PyPI (e.g. NumPy >=1.17.5 for Python 3.8).
    # But we don't want to duplicate the information in oldest-supported-numpy
    # here, and if you can build an older NumPy on a newer Python
    # NumPy 1.14.5 is the first with wheels for Python 3.7, our minimum Python.
    "numpy >=1.14.5",
]

setup_requires = [
    "setuptools_scm",
]

with open('README.rst') as f:
    README = f.read()

setup(
    name="libsonata",
    description='SONATA files reader',
    author="Blue Brain Project, EPFL",
    long_description=README,
    long_description_content_type='text/x-rst',
    license="LGPLv3",
    url='https://github.com/BlueBrain/libsonata',
    classifiers=[
        "License :: OSI Approved :: GNU Lesser General Public License v3 (LGPLv3)",
    ],
    ext_modules=[CMakeExtension("libsonata._libsonata")],
    cmdclass={'build_ext': CMakeBuild,
              },
    zip_safe=False,
    setup_requires=setup_requires,
    install_requires=install_requires,
    extras_require={
        'docs': ['sphinx-bluebrain-theme'],
    },
    python_requires=">=3.7",
    use_scm_version={"local_scheme": "no-local-version",
                     },
    package_dir={"": "python"},
    packages=['libsonata',
              ],
)
