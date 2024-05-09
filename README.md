# Flutter linux framebuffer embedder

This is a work in progress repository for a flutter embedder that writes output directly to fbdev. 

It uses software rendering, and there are no plans to make it use any kind of hardware acceleration. Its primary purpose is low power embedded linux devices. If you want a hardware accelerated embedder take a look at flutter-pi, ivi-homescreen or flutter-embedded-linux.

The embedder links libflutter_engine.so, so make sure to include the compiled engine library in the directory. This will be changed in the future

How to build it:
```bash
mkdir build
cd build
cmake ..
make -j`nproc`
```

This is heavily work in progress, for example there is no input handling at all.