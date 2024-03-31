import unittest

from neuron_viewer import *


class TestConvertCellState(unittest.TestCase):
  def test_normal(self):
      self.assertEqual(convert_cell_state('-30'), ('background-color: #000000', 0))

  def test_outofrange(self):
      with self.assertRaises(ValueError):
          convert_cell_state('130')


if __name__ == '__main__':
  unittest.main()