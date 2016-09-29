# GLupeN64

**Please read the [Wiki](https://github.com/loganmc10/GLupeN64/wiki) for an explanation of the core options**

**Try this out on the [RetroArch Web Player](https://buildbot.libretro.com/web/)!**

**[Binary Builds](http://loganbuildbot.s3-website-us-east-1.amazonaws.com/)**

GLupeN64 is [mupen64plus](https://github.com/mupen64plus/mupen64plus-core) + [RSP-HLE](https://github.com/mupen64plus/mupen64plus-rsp-hle) + [GLideN64](https://github.com/gonetz/GLideN64) + [libretro](http://www.libretro.com/)

#### How is this different from [mupen64plus-libretro](https://github.com/libretro/mupen64plus-libretro)?

mupen64plus-libretro implements multiple RSP and Graphics plugins. There are also code modifications that make it different than standalone mupen64plus.

GLupeN64 uses RSP-HLE and GLideN64 (a graphics plugin that is not available in mupen64plus-libretro). The emulator code itself is identical to standalone mupen64plus.

By choosing one RSP implementation (rsp-hle) and one graphics plugin (GLideN64), we will be able to keep the code in line with upstream, and maintaining the code will be much simpler.

#### What is not supported:

Cheats

32-bit Windows, iOS. I don't have those types of systems so I can't test them. I am more than willing to accept pull requests however.

#### Acknowledgments

A special thanks to the mupen64plus team, the libretro team, and gonetz and those that have worked on GLideN64. I haven't really written any code for this project, I'm just putting together the work by other people.

**Minimum RetroArch version: v1.3.4**
