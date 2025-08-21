# LegacyEditor Project Documentation

My goal is to write the code necessary to **convert saves between all "Minecraft Legacy Console Edition" platforms and versions**.

This will include:
- Full world conversion
- Full player data conversion
- Easy scripting support for block and NBT editing
- Many libraries specific to managing niche console things...

**If you find any issues, please report them!**  
It helps me develop faster 🙂

## Consoles Supported

| **Console**                  | **Reading From** | **Writing To**                           |
|------------------------------|------------------|------------------------------------------|
| **Xbox 360 (.dat)**          | ✔️               | ❌                                        |
| **Xbox 360 (.bin)**          | ✔️               | ❌                                        |
| \>\> **Xenia Emulator**      | ✔️               | ❌                                        |
| **WiiU**                     | ✔️               | ✔️                                       |
| \>\> **Cemu Emulator**       | ✔️               | ✔️                                       |
| **PS Vita**                  | ✔️               | ✔️                                       |
| \>\> **Vita3K Emulator**     | ✔️               | ✔️                                       |
| **PS3**                      | ✔️               | ⚠️ *(Partial - cannot resign PARAM.PFD)* |
| \>\> **RPCS3 Emulator**      | ✔️               | ✔️                                       |
| **Xbox One**                 | ❌                | ❌                                        |
| \>\> **Windurango Emulator** | ✔️               | ❌                                        |
| **Switch**                   | ✔️               | ✔️                                       |
| **PS4**                      | ✔️               | ❌                                        |
| \>\> **ShadPS4 Emulator**    | ✔️               | ✔️                                       |

## Versions Support

| Version           | Reading From | Writing To |
|-------------------|--------------|------------|
| Pre-Release       | ✅            | ❌          |
| Pistons           | ✅            | ❌          |
| Generation        | ✅            | ❌          |
| Adventure         | ✅            | ❌          |
| Potions           | ✅            | ✅          |
| Horse             | ✅            | ❌          |
| Bountiful         | ✅            | ❌          |
| Elytra            | ✅            | ✅          |
| Aquatic           | ✅            | ✅          |
| Village & Pillage | ✅            | ❌          |

More coming soon!

## Setup

To properly set up this project, you must run:

```bash
git init
git submodule add https://github.com/lce-resources/lceLIB.git include/lce
git submodule update --init
```

## Building

- ``CLion`` - Have the IDE auto-detect the ``CMakeLists.txt``.
- ``Windows`` - Run ``build.bat``.
- ``Linux`` - Run ``build.sh``. I have not tested this one.

## Usage

### BatchConverter

To use BatchConverter.exe, supply the full filepath of your savefiles

Information on extracting save files: [Google Docs Guide](https://docs.google.com/document/d/1HUoeH9YcIwqYPYMx9ps0Ui3YF0x_g9QkluADAx_fTJQ)

### Code Examples

Refer to the `tests/` directory to see different ways the code can be used.
For unit testing, edit the folder locations in `code/unit_tests.cpp` to the directory that contains your saves (e.g., `tests/`).

## Dependencies

- [`gulrak/filesystem`](https://github.com/gulrak/filesystem) – Cross-platform filesystem operations
- [`stb`](http://nothings.org/stb) – Image loading and rendering utilities
- [`jibsen/tinf`](https://github.com/jibsen/tinf) – Tiny inflate/deflate compression
- **LZX** – Jed Wing <jedwin@ugcs.caltech.edu>
- **TINF** – Joergen Ibsen
- [`SFO`](https://github.com/hippie68/sfo) – Save file metadata handler by hippie68

## License

Please refer to [`LICENSE.md`](LICENSE.md) for information on the licensing of this code and its usage permissions.

---
