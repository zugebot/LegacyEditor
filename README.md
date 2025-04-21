# LegacyEditor Project Documentation

My goal is to write the code necessary to convert saves between
all "Minecraft Console Edition" consoles and all their specific versions. It will also be able to handle
player conversion, and be usable in such a way to be easily scriptable
for editing blocks / nbt.

IF YOU FIND ANY ISSUES, PLEASE REPORT THEM! It helps me develop faster :)

## Supported Consoles and Formats

### Reading From:
- **WiiU**
- **PS3**
- **RPCS3 Emulator**
- **PSVita**
- **Xbox 360 (.dat format)**
- **Xbox 360 (.bin format)**

### Partially Reading From:
- **PS4** (entity conversion missing)
- **Switch** (region/entity conversion missing)

### Writing To:
- **WiiU**
- **PSVita**
- **RPCS3 Emulator** (broken, I will look into it eventually)

### Partially Writing To:
- **PS3** (PARAM.PFD not yet resignable) (broken, I will look into it eventually)

## Setup

To properly set up this project, you must run:

``
git submodule add https://github.com/lce-resources/lceLIB.git include/lce
``

``
git submodule update --init
``

## Usage

Refer to the `tests/` directory to see different ways the code can be used.
For unit testing, edit the folder locations in `code/unit_tests.cpp` to the directory that contains your saves (e.g., `tests/`).

## Dependencies

This project makes use of several external libraries, including:
- [gulrak/filesystem](https://github.com/gulrak/filesystem) for filesystem operations
- [stb](http://nothings.org/stb) by Sean Barrett
- [jibsen/tinf](https://github.com/jibsen/tinf) for data compression
- LZX - Jed Wing <jedwin@ugcs.caltech.edu>
- TINF - Joergen Ibsen
- SFO - [hippie68 @github](https://github.com/hippie68/sfo)

## Building

- ``CLion`` - Have the IDE auto-detect the ``CMakeLists.txt``.
- ``Windows`` - Run ``build.bat``.
- ``Linux`` - Run ``build.sh``. I have not tested this one.

## Outside Help

PS-VITA: https://docs.google.com/document/d/1HUoeH9YcIwqYPYMx9ps0Ui3YF0x_g9QkluADAx_fTJQ

## License

Please refer to `LICENSE.md` for detailed information on the licensing of this code and its usage permissions.

---
