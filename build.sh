#!/bin/sh

gcc --static -Wall -march=native -Ofast -flto -D_TEST_TEXTURE_ -o test_texture texture.c
gcc --static -Wall -march=native -Ofast -flto -D_TEST_MODEL_   -o test_model   texture.c model.c
gcc --static -Wall -march=native -Ofast -flto -D_TEST_WINGDI_  -o test_wingdi  wingdi.c -lgdi32
gcc --static -Wall -march=native -Ofast -flto -D_TEST_TINYGL_  -o test_tinygl  utils.c vector.c matrix.c texture.c triangle.c model.c shader.c wingdi.c tinygl.c -lgdi32
gcc --static -Wall -march=native -Ofast -flto -o tinygl_demo utils.c vector.c matrix.c texture.c triangle.c model.c shader.c wingdi.c tinygl.c demo.c -lgdi32
strip --strip-all *.exe

