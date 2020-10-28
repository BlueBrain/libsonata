import unittest
import os


def test_suite():
    loader = unittest.TestLoader()
    path = os.path.abspath(os.path.dirname(__file__))
    suite = loader.discover(path, pattern='test*.py')
    return suite
