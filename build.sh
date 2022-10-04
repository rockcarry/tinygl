#!/bin/sh

gcc -Wall -Ofast -flto -D_TEST_TEXTURE_ -o test_texture texture.c
gcc -Wall -Ofast -flto -D_TEST_MODEL_   -o test_model   texture.c model.c
gcc -Wall -Ofast -flto -D_TEST_WINGDI_  -o test_wingdi  wingdi.c -lgdi32
gcc -Wall -Ofast -flto -D_TEST_TINYGL_  -o test_tinygl  vector.c matrix.c texture.c triangle.c model.c shader.c wingdi.c tinygl.c -lgdi32

