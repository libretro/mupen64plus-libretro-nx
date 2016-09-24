# GLupeN64

**Try this out on the [RetroArch Web Player](https://buildbot.libretro.com/web/)!**

**Minimum RetroArch version: v1.3.4**

<b>[Binary Builds](http://loganbuildbot.s3-website-us-east-1.amazonaws.com/)</b>

GLupeN64 is [mupen64plus](https://github.com/mupen64plus/mupen64plus-core) + [RSP-HLE](https://github.com/mupen64plus/mupen64plus-rsp-hle) + [GLideN64](https://github.com/gonetz/GLideN64) + [libretro](http://www.libretro.com/)

#### How is this different from [mupen64plus-libretro](https://github.com/libretro/mupen64plus-libretro)?

mupen64plus-libretro implements multiple RSP and Graphics plugins. There are also code modifications that make it different than standalone mupen64plus.

GLupeN64 uses RSP-HLE and GLideN64 (a graphics plugin that is not available in mupen64plus-libretro). The emulator code itself is identical to standalone mupen64plus.

By choosing one RSP implementation (rsp-hle) and one graphics plugin (GLideN64), we will be able to keep the code in line with upstream, and maintaining the code will be much simpler.

#### How is it organized?

The modules (mupen64plus-core, rsp-hle, GLideN64, and libretro-common) are identical to the upstream repositories, allowing us to easily update the codebase. When modifications have to be made, they are put in the "custom" directory, overriding their vanilla counterparts.

We will try to stay as close to the upstream code as possible, if there are improvements to be made, they should be submitted upstream.

Most of the modifications have involved removing dependencies on SDL, and modifying the plugin architecture to get it running inside libretro.
#### What is not supported:

Cheats

32-bit Windows, iOS. I don't have those types of systems so I can't test them. I am more than willing to accept pull requests however.

#### Acknowledgments

A special thanks to the mupen64plus team, the libretro team, and gonetz and those that have worked on GLideN64. I haven't really written any code for this project, I'm just putting together the work by other people.
