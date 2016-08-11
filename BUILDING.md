#### Getting the code

clone this repository, for instance:

```git clone https://github.com/loganmc10/GLupeN64.git```

Get the submodules:

```git submodule update --init```

You can combine those 2 steps into 1 by doing this:

```git clone --recursive https://github.com/loganmc10/GLupeN64.git```

#### Building the project

```make -j4```

Or if you are using a Raspberry Pi:

Rpi2:
```platform=rpi2 make -j4```

Rpi3:
```platform=rpi3 make -j4```

That will create a file named **glupen64_libretro.so**

You can run a ROM like this:

retroarch -L glupen64_libretro.so /path/to/rom.n64
