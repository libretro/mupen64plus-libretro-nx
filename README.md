# Mupen64Plus-Next

Mupen64Plus-Next is [mupen64plus](https://github.com/mupen64plus/mupen64plus-core) + [GLideN64](https://github.com/gonetz/GLideN64) + [libretro](http://www.libretro.com/)

It is also the successor of the old Mupen64Plus libretro core.

#### How is this different from [Parallel-N64](https://github.com/libretro/parallel-n64)?

Parallel-N64 implements multiple Graphics plugins. There are also code modifications that make it different than standalone mupen64plus.

Mupen64Plus-Next uses GLideN64 (a graphics plugin that is not available in Parallel-N64). The emulator code itself is identical to standalone mupen64plus.

By choosing one graphics plugin (GLideN64), we will be able to keep the code in line with upstream, and maintaining the code will be much simpler.

#### Acknowledgments

A special thanks to the mupen64plus team, the libretro team, and gonetz and those that have worked on GLideN64.
