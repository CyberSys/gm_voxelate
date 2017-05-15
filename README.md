gm_voxelate
====================
[![forthebadge](http://forthebadge.com/images/badges/compatibility-club-penguin.svg)](http://forthebadge.com)

Welcome to the home of gm_voxelate, the configurable voxel module for Garrysmod.

## Links
[More Information (Facepunch Thread)](http://facepunch.com/showthread.php?t=1466364)

[Development Plan (Trello)](https://trello.com/b/zrWAQgqX/gm-voxelate)

## Documentation
See the wiki in the sidebar.

## Bugs
Please report bugs on this repository's issues page!

## Building
This is designed to be built with [MetaMan](https://github.com/danielga)'s [Garry's Mod](https://github.com/danielga/garrysmod_common) and [Source SDK](https://github.com/danielga/sourcesdk-minimal) headers.

It requires a compiler/runtime that supports C++11, which can cause problems on older linux systems.

**Windows**: Should just work.

**Linux**: Should just work?

**Mac**: Unknown, probably similar to Linux.

The test_install scripts are for quickly copying the binaries into directories on my systems, and are probably broken right now.

## Technical details for using this in a gamemode

### Required libraries/dependencies

We generally don't depend on garryFuncs:tm:
You should be good to completely override `includes/init.lua`, as long as the stock luajit 5.1 funcs aren't modified, and a version of the `hook` and `timer` libraries is available for use.

### Sub entities

#### Voxelate engine based

use `gm_voxelate:RegisterSubEntity(className,classObj)` and it will internally extend the VoxelEntity class

#### Source engine based

extend `voxel_entity`

note: both the voxelate engine and source engine entities are mutually exclusive: you may only use one type at a time.

## Testing/Debugging

### Lua Hotloading

Right now, the Lua portion of gm\_voxelate is compiled into the DLL, and is unmodifiable at runtime. This can get quite annoying during extended sessions of modifying the Lua portion of gm\_voxelate exclusively. As such, you may enable lua hotloading to ease the burden of development and debugging, which directly reads files from the OS filesystem and loads them into Lua.

Hotloading can be enabled by creating a file in GarrysMod/garrysmod/data with the name `.vox_hotloading`, whose contents is the path to the `lua` folder in this repo. **NO NEWLINES, NO TRAILING SPACES!**

Secondly, you will either:
1. install [gm_luaiox](https://cdn.discordapp.com/attachments/152162730244177920/311479180141527041/gmsv_luaiox_win32.dll) (kindly provided by MetaMan)
2. compile gm_voxelate with the premake trigger `luahotloading`, which will add a dangerous function that can read any file without restrictions

Enjoy!

P.S. fair word of warning:
![warning](https://cdn.discordapp.com/attachments/152162730244177920/311486688025247744/unknown.png)

## Used In
[gm_f1atgrass](https://github.com/glua/gm_f1atgrass) - Testbed for the module
