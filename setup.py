import inspect
import os
import os.path as osp
import platform
import re
import subprocess
import sys

from setuptools import setup, Extension
from setuptools.command.build_ext import build_ext
from setuptools.command.install import install
from setuptools.command.test import test
from distutils.version import LooseVersion


class lazy_dict(dict):
    """When the value associated to a key is a function, then returns
    the function call instead of the function.
    """

    def __getitem__(self, item):
        value = dict.__getitem__(self, item)
        if inspect.isfunction(value):
            return value()
        return value


def get_sphinx_command():
    """Lazy load of Sphinx distutils command class
    """
    from sphinx.setup_command import BuildDoc

    return BuildDoc


class CMakeExtension(Extension):
    def __init__(self, name, sourcedir=""):
        Extension.__init__(self, name, sources=[])
        self.sourcedir = osp.abspath(sourcedir)


class CMakeBuild(build_ext, object):
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

        if platform.system() == "Windows":
            cmake_version = LooseVersion(
                re.search(r"version\s*([\d.]+)", out.decode()).group(1)
            )
            if cmake_version < "3.1.0":
                raise RuntimeError("CMake >= 3.1.0 is required on Windows")

        for ext in self.extensions:
            self.build_extension(ext)

    def build_extension(self, ext):
        extdir = osp.abspath(osp.dirname(self.get_ext_fullpath(ext.name)))
        cmake_args = [
            "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY=" + extdir,
            "-DEXTLIB_FROM_SUBMODULES=ON",
            "-DSONATA_PYTHON=ON",
            "-DCMAKE_BUILD_TYPE=",
            "-DSONATA_CXX_WARNINGS=OFF",
            '-DPYTHON_EXECUTABLE=' + sys.executable
        ]

        optimize = "OFF" if self.debug else "ON"
        build_args = ["--config", optimize, "--target", self.target]

        if platform.system() == "Windows":
            cmake_args += [
                "-DCMAKE_LIBRARY_OUTPUT_DIRECTORY_{}={}".format(
                    optimize.upper(), extdir
                )
            ]
            if sys.maxsize > 2 ** 32:
                cmake_args += ["-A", "x64"]
            build_args += ["--", "/m"]
        else:
            build_args += ["--", "-j"]

        env = os.environ.copy()
        env["CXXFLAGS"] = '{} -DVERSION_INFO=\\"{}\\"'.format(
            env.get("CXXFLAGS", ""), self.distribution.get_version()
        )
        if not osp.exists(self.build_temp):
            os.makedirs(self.build_temp)
        subprocess.check_call(
            ["cmake", ext.sourcedir] + cmake_args, cwd=self.build_temp, env=env
        )
        subprocess.check_call(
            ["cmake", "--build", "."] + build_args, cwd=self.build_temp
        )


class PkgTest(test):
    """Custom disutils command that acts like as a replacement
    for the "test" command.
    """

    new_commands = [('test_ext', lambda self: True), ('test_doc', lambda self: True)]
    sub_commands = test.sub_commands + new_commands

    def run(self):
        super(PkgTest, self).run()
        self.run_command('test_ext')
        self.run_command('test_doc')

install_requirements = [
    "numpy>=1.12.0",
]

with open('VERSION') as versionf:
    version = versionf.readline().strip()

setup(
    name="libsonata",
    description='SONATA files reader',
    author="BlueBrain Project, EPFL",
    author_email="bbp-ou-nse@groupes.epfl.ch",
    version=version,
    classifiers=[],
    ext_modules=[CMakeExtension("libsonata")],
    cmdclass=lazy_dict(
        build_ext=CMakeBuild,
        test_ext=CMakeBuild,
        test=PkgTest,
        test_doc=get_sphinx_command,
    ),
    zip_safe=False,
    install_requires=install_requirements,
    setup_requires=["setuptools_scm"],
)
