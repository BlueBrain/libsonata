#!/usr/bin/env python

from setuptools import setup, Distribution
from setuptools.command.install import install

from libsonata import __version__


class BinaryDistribution(Distribution):
    def is_pure(self):
        return False

    def has_ext_modules(self):
        return True


# Workaround for 'Invalid binary wheel, found shared library "_sonata.so" in purelib folder' issue
# https://github.com/bigartm/bigartm/issues/840#issuecomment-342825690
class InstallPlatlib(install):
    def finalize_options(self):
        install.finalize_options(self)
        if self.distribution.has_ext_modules():
            self.install_lib = self.install_platlib


setup(
    name='libsonata',
    version=__version__,
    packages=[
        'libsonata',
    ],
    package_data={
        '': [
            '*.so*',
        ]
    },
    description='SONATA files reader',
    long_description='',
    url='ssh://bbpcode.epfl.ch/common/libsonata',
    author='Blue Brain Project',
    author_email='bbp-ou-nse@groupes.epfl.ch',
    install_requires=[
        'numpy>=1.12.0',
    ],
    distclass=BinaryDistribution,
    cmdclass={
        'install': InstallPlatlib
    },
    license="BBP-internal-confidential",
    zip_safe=False,
    classifiers=[]
)
