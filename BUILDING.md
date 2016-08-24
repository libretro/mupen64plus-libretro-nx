#### Getting the code

clone this repository, for instance:

```git clone --depth=1 https://github.com/loganmc10/GLupeN64.git```

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

### Subtrees
#### Adding Subtree

git subtree add --prefix mupen64plus-core https://github.com/mupen64plus/mupen64plus-core.git master --squash

#### Updating Subtree

git subtree pull --prefix mupen64plus-core https://github.com/mupen64plus/mupen64plus-core.git master --squash
