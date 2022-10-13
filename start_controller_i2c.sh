#!/bin/sh
export PYTHONPATH="${PYTHONPATH}:`pwd`"
python3 logitech-controller/controller_i2c.py
