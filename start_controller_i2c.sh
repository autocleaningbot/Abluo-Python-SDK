#!/bin/sh
export PYTHONPATH="${PYTHONPATH}:`pwd`"
python3 logitech_controller/controller_i2c.py
