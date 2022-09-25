#!/bin/sh

gcc -Wall -D_TEST_BMP_ bmp.c -o test_bmp
gcc -Wall -D_TEST_MODEL_ bmp.c model.c -o test_model

