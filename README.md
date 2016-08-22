# GLupeN64

<b>Please read the [Wiki](https://github.com/loganmc10/GLupeN64/wiki), it has information on how to fix common 'gotchas'</b>

GLupeN64 is [mupen64plus](https://github.com/mupen64plus/mupen64plus-core) + [RSP-HLE](https://github.com/mupen64plus/mupen64plus-rsp-hle) + [GLideN64](https://github.com/gonetz/GLideN64) + [libretro](http://www.libretro.com/)

#### How is this different from [mupen64plus-libretro](https://github.com/libretro/mupen64plus-libretro)?

mupen64plus-libretro tries to emulate the complete mupen64plus experience (think multiple graphic and RSP plugins). They also make modifications to the code as they see fit.

Because mupen64plus is built to be modular it is difficult to "convert" that project to a libretro core, since you end up with functions with the same name (for instance multiple functions named "PluginStartup").

Many modifications had to be made to the mupen64plus-libretro code to get it to work inside libretro. As a result, it differs quite greatly from vanilla mupen64plus (there is no up to date GLideN64 implmentation for example).

By choosing one RSP implentation (rsp-hle) and one graphics plugin (GLideN64), we will be able to keep the code in line with upstream, and maintaining the code will be much simpler.

#### How is it organized?

The modules (core, rsp-hle, GLideN64, and libretro-common) are brought in as git submodules, allowing us to easily update the codebase. When modifications have to be made, they are put in the "custom" directory, overriding their vanilla counterparts.

We will try to stay as close to the upstream code as possible, if there are improvements to be made, they should be submitted upstream.

Most of the modifications have involved removing dependencies on SDL, and modifying the plugin architecture to get it running inside libretro.
#### What is not supported:

Cheats

Windows, iOS. I don't have those types of systems so I can't test them. I am more than willing to accept pull requests however.

#### Acknowledgments

A special thanks to the mupen64plus team, the libretro team, and gonetz and those that have worked on GLideN64. I haven't really written any code for this project, I'm just putting together the work by other people.
