import unittest


def test_suite():
    loader = unittest.TestLoader()
    suite = loader.discover('python/tests', pattern='test*.py')
    return suite
