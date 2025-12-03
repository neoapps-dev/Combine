Combine Engine
=============

The Combine Engine is a modular C++ framework for rendering and scripting, featuring an Entity-Component-System.

Dependencies:
  - `dialog` utility (for `make menuconfig`), non-runtime dependency
  - Standard C++ build toolchain (`make`, compiler)
  - Lua (only if you have enabled Lua in the build configuration)
  - Squirrel (only if you have enabled Squirrel in the build configuration)
  - git (duh)

Build Instructions:
-------------------
1. Configure the engine:
   `make menuconfig`
2. Compile the engine:
   `make`

Usage:
------
Run the compiled binary from the `bin/` directory. The engine will load its configured modules and execute an initial script from the `scripts/` directory.

For detailed information, refer to the source code. because I'm too lazy to make this any bigger. lol.

License:
--------
The Combine Engine itself is licensed under the Apache 2.0 License.
Except for ChaiScript which is licensed under the BSD-3-Clause License.
Lua and Squirrel are licensed under the MIT license.
stb_image under the public domain.
