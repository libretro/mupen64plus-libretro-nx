# Mupen64Plus

**Please read the [Wiki](https://github.com/GLupeN64/GLupeN64/wiki) for an explanation of the core options**

**[Binary Builds](http://loganbuildbot.s3-website-us-east-1.amazonaws.com/)**

Mupen64Plus is [mupen64plus](https://github.com/mupen64plus/mupen64plus-core) + [GLideN64](https://github.com/gonetz/GLideN64) + [libretro](http://www.libretro.com/)

#### How is this different from [mupen64plus-libretro](https://github.com/libretro/mupen64plus-libretro)?

mupen64plus-libretro implements multiple Graphics plugins. There are also code modifications that make it different than standalone mupen64plus.

Mupen64Plus uses GLideN64 (a graphics plugin that is not available in mupen64plus-libretro). The emulator code itself is identical to standalone mupen64plus.

By choosing one graphics plugin (GLideN64), we will be able to keep the code in line with upstream, and maintaining the code will be much simpler.

#### Acknowledgments

A special thanks to the mupen64plus team, the libretro team, and gonetz and those that have worked on GLideN64.

**Minimum RetroArch version: v1.3.4**
