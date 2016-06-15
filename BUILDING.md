#### Getting the code

clone this repository, for instance:

```git clone https://github.com/loganmc10/GLupeN64.git```

Get the submodules:

```git submodule update --init --depth=1```

#### Building the project

```make -j4```

Or if you are using a Raspberry Pi:

```platform=rpi3 make -j4```

That will create a file named **mupen64plus_libretro.so**

You can run a ROM like this:

retroarch -L mupen64plus_libretro.so /path/to/rom.n64
