'''Test the containable objects'''

from apiUtils import *
import sys

def imperative(r):
    # Redmine #416
    r.assertEqual(r.c(multipleObjectData("modelbox")), {})
