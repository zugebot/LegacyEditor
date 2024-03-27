# LegacyEditor Project Documentation

My goal is to write the code necessary to convert saves between
all "Minecraft Console Edition" consoles and all their specific versions. It will also be able to handle
player conversion, and be usable in such a way to be easily scriptable
for editing blocks / nbt.

## Supported Consoles and Formats

### Reading From:
- **WiiU**
- **PS3**
- **RPCS3 Emulator**
- **PSVita**
- **Xbox 360 (.bin format)**
- **Xbox 360 (.dat format)**
- **Switch** (region/entity conversion imminent)
- **PS4** (region/entity conversion imminent)

### Writing To:
- **WiiU**
- **PSVita**
- **PS3** (METADATA not yet resignable)
- **RPCS3 Emulator** (METADATA not yet resignable)

## Usage

Refer to the `examples/` directory to see different ways the code can be used. For unit testing, edit the folder locations in `LegacyEditor/examples/unit_tests.cpp` to the directory that contains your saves (e.g., `/saves/`).

## Dependencies

This project makes use of several external libraries, including:
- [gulrak/filesystem](https://github.com/gulrak/filesystem) for filesystem operations
- [stb](http://nothings.org/stb) by Sean Barrett
- [jibsen/tinf](https://github.com/jibsen/tinf) for data compression
- LZX - Jed Wing <jedwin@ugcs.caltech.edu>
- TINF - Joergen Ibsen

## License

Please refer to `LICENSE.md` for detailed information on the licensing of this code and its usage permissions.

---