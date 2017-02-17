### Getting the code

clone this repository, for instance:

```git clone --depth=1 https://github.com/libretro/mupen64plus-libretro.git```

### Building the project

Compiling the project:

```make -j4```

Or if you are using a Raspberry Pi:

Rpi2:
```platform=rpi2 make -j4```

Rpi3:
```platform=rpi3 make -j4```

That will create a file named **mupen64plus_libretro.so**

### Building for Windows (64-bit only)

The only supported way to build this core for Windows is cross-compiling it from Linux. You may be able to compile it on Windows using something like MSYS2, but I haven't tried.

On Fedora, make sure you have these packages installed: ```mingw64-gcc-c++ mingw64-zlib```

Then run ```platform=win make -j4```

That will create a file named **mupen64plus_libretro.dll**
