# LCE DECOMPRESSED CHUNKS
### Written by UtterEvergreen1 (Daniel)

## Copyright and Usage Notice

**Copyright © 2023 UTTEREVERGREEN1 (DANIEL)**

All rights reserved. This document is the intellectual property of UTTEREVERGREEN1 (DANIEL) and is protected by copyright law.

### Terms of Use

1. **License Grant**: You are hereby granted a non-exclusive, revocable, and limited license to use this document for personal or non-commercial purposes, subject to the following conditions.

2. **Prohibitions**: You may not:
  - Redistribute or republish this document in its entirety without explicit written consent from the author.
  - Modify or create derivative works based on this document without explicit written consent from the author.

3. **Fair Use and Fair Dealing**: This document may be used in accordance with the fair use or fair dealing provisions of copyright law in your jurisdiction for purposes such as criticism, commentary, news reporting, education, or research.

4. **Contact for Permissions**: For any requests to redistribute, modify, or use this document in ways not explicitly allowed by this license, please contact UtterEvergreen1 on Discord, Instagram, or X  for permission.

### Disclaimer

This document is provided "as-is" without any warranties, express or implied. The author shall not be liable for any damage or loss arising from its use.

### Modification and Termination

The author reserves the right to modify the terms of this license or terminate it at any time. Revised versions of this license will apply to documents published after the effective date of the change.

By using this document, you agree to abide by these terms. Your use of the document indicates your acceptance of this license.


---
# This document will present you with a step-by-step guide to understanding and building a parser for decompressed LCE chunks

---
# Format of this document

## [A...B] = bytes from offset of 'A' going to 'B' (inclusive) from the start
- Example1: `[0x0...0x19]` would be from offset 0 going to byte 0x19 (25)
- Example2: `[0x0...0x11/0x19]` would be from offset 0 going to byte 0x11 (17) or 0x19 (25)
depending on the circumstances

## Every value is parseLayer in big endian format unless specified

### `Liquid data` was introduced in the Aquatic update

---

# 1. Chunk Header `[0x0...0x11/0x19]`

## 1.1 Format ID `[0x0...0x1]`
- The first two bytes as a `(short/int16_t)` represent the chunk's format ID. Here are the values:
- ### `= 0xc (12) -> Aquatic Format`
- ### `= 0xb (11) -> Elytra Format`
- ### `= 0xa (10) -> NBT Format`
- ### `= 8, 9 -> Bountiful Format?`
- If the chunk is NBT format, don't go any further from `(1.1)`, parse the rest as NBT

## 1.2 Chunk Location `[0x2...0x9]`
### 1.2.1 Chunk Location X `[0x2...0x5]`
- The following 4 bytes as a `(int/int32_t)` is the chunk's X position.
### 1.2.2 Chunk Location Z `[0x6...0x9]`
- The following 4 bytes as a `(int/int32_t)` is the chunk's Z position.

## 1.3 Misc Chunk Data `[0xa...0x11/0x19]`
### 1.3.1 Last Update `[0xa...0x11]`
- The following 4 bytes as a `(long/int64_t)` is the chunk's Last Update.
### 1.3.2 Inhabited Time `[0x12...0x19?]`
- The following 4 bytes as a `(long/int64_t)` is the chunk's Inhabited Time. Will not exist on format ID `8` `(see 1.1)`

# Section 1 Example
```
# Chunk Header
00 0c 00 00 00 00 00 00 00 02 00 00 00 00 00 04 54 1c 00 00 00 00 00 01 24 17

Let's break this down:
(1.1) [0x0...0x1] Format ID = 0xc (12); therefore, the format is Aquatic.
(1.2.1) [0x2...0x5] Chunk X = 0
(1.2.2) [0x6...0x9] Chunk Z = 2
(1.3.1) [0xa...0x11] Last Update = 0x4541c (283676)
(1.3.2) [0x12...0x19?] Inhabited Time = 0x12417 (74775)

Given this header example you can parse it and get the values of:
Format ID = 12
Chunk X = 0
Chunk Z = 0
Last Update = 283676
Inhabited Time = 74775
```
---

# 2. AQUATIC Sections Header `[0x1a..0x4b]`
- This section will only focus on Aquatic format as Elytra is different `see (1.1)`

## 2.0 What is a Section?
- Sections are intervals of 16 newBlocks on the Y level of the full chunk
- It is used in order to break down and make accessing and saving the newBlocks faster

## 2.1 Sections' Size `[0x1a..0x1b]`
- The first two bytes of the section header as a `(short/int16_t)` represent the chunk's sections' size (not in bytes)
- The sections' size in bytes is obtained by multiplying the parseLayer value by 0x100 (256)
## 2.2 Section Jump Table `[0x1c...0x3b]`
- The following is an array of 16 `short/int16_t` values that represent the sections' jump table
- The section table is used to jump a certain section
- The section jump corresponds to the index in the array (index 0 will be the first section `(Y0->Y15)` while
index 15 will be the last section `(Y240->Y256)`)
- If the value at an index is equal to the sections' size in bytes `(see 2.1 point 2)` then there is no more sections to parse after that
## 2.3 Section Size Table `[0x3c...0x4b]`
- The following is an array of 16 `byte/int8_t` values that represent the sections' size table
- The size table can be used to pre-allocate enough space for the chunk
- The parseLayer value must be multiplied by 0x100 (256) in order to get the size in bytes, this will be discussed later why
- If the size is `0` that does NOT mean it's the last chunk to parse,
there could be sections above it that are not empty

<p>
NOTE: IF A SECTION SIZE TABLE VALUE IS 0 ONLY MEANS THAT SECTION IS EMPTY, AND DOES NOT MEAN THE REST IS EMPTY.
ONLY EVER EXIT FROM A LOOP OF INCREMENTALLY PARSING ALL THE CHUNKS IF A SECTION JUMP TABLE VALUE `(SEE 2.2)`
EQUALS THE sections' size IN BYTES (SEE 2.1 POINT 2).
NEVER USE THE SECTION SIZE TABLE TO ASSUME THERE IS NO MORE CHUNKS TO PARSE
</p>

# Section 2 Example
```
# AQUATIC Sections Header
00 23 00 00 06 00 0b 00 10 00 14 00 18 00 1b 00 20 00 23 00 23 00 23 00 23 00 23 00 23 00 23 00 23 00
06 05 05 04 04 03 05 03 00 00 00 00 00 00 00 00

Let's break this down:
(2.1) [0x1a..0x1b] sections' size = 0x23 (35)

(2.2) [0x1c...0x3b] Section Jump Table = { 0, 0x600, 0xb00, 0x1000, 0x1400, 0x1800, 0x1b00, 0x2000, 0x2300, 0x2300,
0x2300, 0x2300, 0x2300, 0x2300, 0x2300, 0x2300 }

(2.3) [0x3c...0x4b] Section Size Table = { 6, 5, 5, 4, 4, 3, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0 }

Given this section header example you can parse it and get the values of:
sections' size = 35
sections' size in bytes = 35 * 256 = 8960
Section Jump Table = { 0, 1536, 2816, 4096, 5120, 6144, 6912, 8192, 8960, 8960, 8960, 8960, 8960, 8960, 8960, 8960 }
Section Size Table = { 6, 5, 5, 4, 4, 3, 5, 3, 0, 0, 0, 0, 0, 0, 0, 0 }
```
---
# 3. Sections' Data `[0x4c...*+0x4b]`
- `*` is the sections' size in bytes `(see 2.1 point 2)`
- `+ 0x4c` (-1 for inclusive) is the size of the first two sections `(1.1 to 2.3)`
- If at some point you want to skip the sections' data: skip the amount stored in
the sections' size in bytes `(see 2.1 point 2)`

# 3.0.1 Block Overview
- Individual newBlocks are stored in two bytes as a `short/int16_t` <span style="color:red"> in little endian format </span>
- The most significant bit of an individual block determines if the block is waterlogged
- The following 11 significant bits are the ID of the block
- The last 4 significant bits is the data of the block
- Keep in mind that this is little endian format, reading in big endian will be this instead:
  - The 4 most significant bits of the parseLayer value is the 4 least significant bits of the block ID
  - The following 4 significant bits is the data of the block
  - The following bit determines if the block is waterlogged
  - The last 7 bits are the most significant bits of the block ID

# Block Parsing Example
```
Example: 0x55 0x83 bytes in little endian would result in value 0x8355
Lets break down 0x8355 into bits and their meaning:
1 0 0 0
0 0 1 1
0 1 0 1
0 1 0 1

1 - the first most significant bit represents if it is waterlogged and in this case it is
0 0 0   - these three lines of bits are the block ID bits
0 0 1 1 
0 1 0 1
0 1 0 1 - the last 4 significant bits is the data value of the block

In this example:
Waterlogged = true
BlockID = 53 (oak-wood stairs)
Data = 5 (facing west and upside down)
So we can conclude from the two bytes of 0x55 0x83 that it is
an oak-wood stairs block facing west and upside down and waterlogged
```


# 3.0.2 Individual Section Format Overview
- Each section has a header of 0x80 (128) bytes which is the grid index table
- Following the header is the data (if any)

# 3.1 Individual Section Grid Index Table `[X+0x4c...X+Y+0x4c]`
- `X` is any value in the jump table index `(see 2.2)`
- `Y` is the value in the corresponding index in the size table `(see 2.3)`
- `+ 0x4c` is the size of the first two sections `(1.1 to 2.3)`
- The block storage can be broken down even further from sections to grids which are 4x4x4 areas of a section
- The first 0x80 (128) bytes at the start of each section is the grid index table
- The grid indices occur in order of Y first then Z then X
- `Example: gridIndex = X + ((gridX * 16) + (gridZ * 4)`

## 3.1.1 A Grid Index
- Each grid index in the table is 2 bytes as a `short/int16_t` <span style="color:red"> in little endian format </span>
- The 4 most significant bits of the parseLayer value is the format ID
- The following 8 bits is the offset where the grid is stored (multiplied by 4)
- The formats are as follows:

| Format ID | Size | Description                                                                                                  | Block Storage Format                                                                                                                                              | Bits per block |
|-----------|------|--------------------------------------------------------------------------------------------------------------|-------------------------------------------------------------------------------------------------------------------------------------------------------------------|----------------|
| 0x0       | 1    | The grid contains only <span style="color:green"> 1 </span> block                                            | The single block is stored the in the grid index                                                                                                                  | N/A            |
| 0x2       | 12   | The grid contains only <span style="color:green"> 2 </span> different newBlocks                                 | The first 4 bytes as `short/int16_t` values is the block palette and the following 8 bytes are the block positions                                                | 1              |
| 0x3       | 20   | The grid contains only <span style="color:green"> 2 </span> different newBlocks along with `Liquid data`        | The first 4 bytes as `short/int16_t` values is the block palette, the following 8 bytes are the block positions and the following 8 bytes is the `Liquid data`    | 1              |
| 0x4       | 24   | The grid contains only <span style="color:green"> up to 4 </span> different newBlocks                           | The first 8 bytes as `short/int16_t` values is the block palette and the following 16 bytes are the block positions                                               | 2              |
| 0x5       | 40   | The grid contains only <span style="color:green"> up to 4 </span> different newBlocks along with `Liquid data`  | The first 8 bytes as `short/int16_t` values is the block palette, the following 16 bytes are the block positions and the following 16 bytes is the `Liquid data`  | 2              |
| 0x6       | 40   | The grid contains only <span style="color:green"> up to 8 </span> different newBlocks                           | The first 16 bytes as `short/int16_t` values is the block palette and the following 24 bytes are the block positions                                              | 3              |
| 0x7       | 64   | The grid contains only <span style="color:green"> up to 8 </span> different newBlocks along with `Liquid data`  | The first 16 bytes as `short/int16_t` values is the block palette, the following 24 bytes are the block positions and the following 24 bytes is the `Liquid data` | 3              |
| 0x8       | 64   | The grid contains only <span style="color:green"> up to 16 </span> different newBlocks                          | The first 32 bytes as `short/int16_t` values is the block palette and the following 32 bytes are the block positions                                              | 4              |  
| 0x9       | 96   | The grid contains only <span style="color:green"> up to 16 </span> different newBlocks along with `Liquid data` | The first 32 bytes as `short/int16_t` values is the block palette, the following 32 bytes are the block positions and the following 32 bytes is the `Liquid data` | 4              |
| 0xe       | 128  | The newBlocks in this grid is stored in <span style="color:green"> full </span>                                 | The full 128 bytes stored are the newBlocks                                                                                                                          | N/A            |
| 0xf       | 256  | The newBlocks in this grid is stored in <span style="color:green"> full </span> along with `Liquid data`        | The full 128 bytes stored are the newBlocks and the following 128 bytes is the `Liquid data`                                                                         | N/A            |

## 3.2 Parsing newBlocks from a grid index

## 3.2.1 Parsing Single Block (Format ID 0x0)
- The block is the 2 bytes as a `short/int16_t` stored in the grid index
- To parse single block format, copy the `short/int16_t` 64 times into the output buffer


## 3.2.2 Parsing a Palette (Format ID 0x2-0x9)

### 3.2.2.1 What is a palette?
- There are 2 components to parsing formats 2-9
- The first component is the `block palette` which stores what newBlocks are used in the grid
  - `block palette` will contain 2 `0xff` bytes for each unused block in the palette (for example if only 3 of 4 is used)
- The second component is the `pointer array` which stores what block in the palette is at an index in the grid
  - `bits per block` in the format table `(see 3.1.1)` determines how many bits are needed per block in `pointer array`
  - The more newBlocks there are in the palette, the more bits are need in order to sufficiently point to the block

| Palette Size | Bits per block |
|--------------|----------------|
| 2            | 1              |
| 4            | 2              |
| 8            | 3              |
| 16           | 4              |

#### Explanation
- Palette size of 2 only needs 1 bit because it will either point to index 0 or 1
- Palette size of 8 only needs 3 bits because 3 bits can point between index 0 to 7 inclusive which is all that is needed
- Therefore, the bits per block is determined by the number of palette newBlocks in the grid when writting the grid
- On the other hand `block palette` size can also be acquired by `bits per block`
by bit shifting left 1 by the number of bits per block (1 << `bits per block`)

### 3.2.2.2 How Does the `pointer array` Work?
- The `pointer array` can be broken down into `segments` of 8 bytes
- There are as many `segments` as the number of bits per block
- Each `segment` holds 64 bits (as 8 bytes are 64 bits)
- Each bit in the first `segment` is the least significant bit for the palette index in any given index of `segment`
- Each bit in additional `segments` appends onto the palette index in any given index of `segment`

### 3.2.2.3 How are Palettes Parsed?
- First step is to parseLayer the `palette` into an array by getting the size of the palette with (1 << `bits per block`)
  - Each `palette` entry is a `short/int16_t` type
- Second step is to create a loop for each `bits per block`
- Third step is to loop 64 times (for each bit) inside the `bits per block` loop
  - This loop should pass all the 64 bits into an array of `indexes` from the X<sup>th</sup> `segment` (8 bytes) in the `pointer array`
  - Each iteration after the first should append the bits on top of the existing value
- The end result will be an array of palettes of the newBlocks used and an array of `indexes` that should point to an index in the palette

### 3.2.2.4 How is the `palette` and `indexes` Used?
- `Indexes` array should be 64 in size and each points to a block in the `palette`
- `Indexes` now represents the grid block values
- `Indexes` is in the order of YZX in the grid `((x * 16) | (z * 4) | y)`
- Fill the `grid` with the newBlocks from `Indexes` associated with the block in palette array

## 3.2.3 Parsing Full Blocks (Format ID 0xe-0xf)
- Copy the number of bytes in `Size` coloumn of the format table `(see 3.1)`
- This should either be 128 bytes for format ID `0xe` or 256 bytes for format ID `0xf`

## 3.3 What to do with Grids?
- The grid is now filled with 64 newBlocks
- The grid is in YZX order
- Parsing grids will likely be moved to a full block array, if that is the case, take the corresponding YZX values
in the grid and place them in the associated places of the full block array

# Section 3 Example
```
# AQUATIC Grid Index Header
00 40 06 20 09 40 0f 40 15 20 18 20 1b 60 25 40 2b 40 31 20 34 20 37 40 3d 20 40 40 46 40 4c 20
4f 40 55 40 5b 40 61 20 64 20 67 20 6a 40 70 20 73 20 76 40 7c 40 82 40 88 40 8e 40 94 20 97 40
9d 20 a0 20 a3 60 ad 20 b0 20 b3 40 b9 60 c3 20 c6 40 cc 40 d2 40 d8 40 de 40 e4 40 ea 60 f4 60
fe 20 01 41 07 21 0a 41 10 21 13 41 19 61 10 00 23 41 29 41 2f 61 39 41 3f 41 b0 00 45 61 4f 61
# AQUATIC Palettes (this follows after grid index header)
70 00 15 00 10 00 ff ff 13 10 31 30 13 30 13 00 00 03 00 03 20 03 00 11 - index 0 grid palette
15 00 10 00 73 3f 73 bf 77 7f 77 ff - index 1 grid palette
... (skip 1176 bytes)
b0 00 10 00 15 00 11 00 - index 58 grid palette
30 00 ff ff ff ff ff ff
10 00 30 00 91 12 09 97
01 11 21 11 91 13 09 91
00 00 00 00 66 20 f6 60

Let's parse a couple of indexes:

At index 0:
00 40 becomes 0x4000 as a short/int16_t
This is the value as bits:
0 1 0 0 - Format ID
0 0 0 0 - Offset
0 0 0 0
0 0 0 0
Format ID = 4
Offset = 0 * 4 = 0

At index 1 (2 byte offset):
06 20 becomes 0x2006 as a short/int16_t
This is the value as bits:
0 0 1 0 - Format ID
0 0 0 0 - The next 3 lines of bits are the offset
0 0 0 0
0 1 1 0
Format ID = 2
Offset = 6 * 4 (don't forget to multiply by 4) = 24


At index 58 (116 byte offset):
2f 61 becomes 0x612f as a short/int16_t
This is the value as bits:
0 1 1 0 - Format ID
0 0 0 1 - The next 3 lines of bits are the offset
0 0 1 0
1 1 1 1
Format ID = 6
Offset = 303 * 4 (don't forget to multiply by 4) = 1212


Let's parse some grids!

At index 0:
Format ID = 4
Offset = 0
Skip to offset 0 from the end of the table which in this example is:
70 00 15 00 10 00 ff ff 13 10 31 30 13 30 13 00 00 03 00 03 20 03 00 11
Since we know the format ID is 4, the palette is size (1 << bits per block) = 4
Palette = { 0x7000, 0x1500, 0x1000, 0xffff } - this is our palette of newBlocks
Segments = { 0x1310313013301300, 0x0003000320030011 } - there are 2 entries because
the bits per block for this format ID is 2 (see 3.1 format table)

The segments will be used to get indexes
Lets break down segments into bits:
Segment[0] (each number represents 2 bits: 0-3):
0 0 0 1 0 0 1 1 0 0 0 1 0 0 0 0
0 0 1 1 0 0 0 1 0 0 1 1 0 0 0 0
0 0 0 1 0 0 1 1 0 0 1 1 0 0 0 0
0 0 0 1 0 0 1 1 0 0 0 0 0 0 0 0
Segment[1] (each number represents 2 bits: 0-3):
0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1
0 0 0 0 0 0 0 0 0 0 0 0 0 0 1 1
0 0 1 0 0 0 0 0 0 0 0 0 0 0 1 1
0 0 0 0 0 0 0 0 0 0 0 1 0 0 0 1

Let's loop over the 2 segment piece and add them into indexes:
Starting with segment[0]:
Indexes = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 0, 0
            0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0
            0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 0, 0
            0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0 }
            
Next, we bitwise operator OR the values from segment[1] but with each value bit shifted left 1
in order to append bits on each other, this is the end result after this step is done:
Indexes = { 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 2, 2
            0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 2, 2
            0, 0, 2, 1, 0, 0, 1, 1, 0, 0, 1, 1, 0, 0, 2, 2
            0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 2, 0, 0, 0, 2 }
Now each index in indexes corresponds to a value in the palette
0 = 0x7000
1 = 0x1500
2 = 0x1000
3 = 0xffff - this means it is unused
which can be seen that no index in the indexes array is 3


At index 1:
Format ID = 2
Offset = 24
Skip to offset 24 from the end of the table which in this example is:
15 00 10 00 73 3f 73 bf 77 7f 77 ff
Since we know the format ID is 2, the palette is size (1 << bits per block) = 2
Palette = { 0x1500, 0x1000 } - this is our palette of newBlocks
Segments = { 0x733f73bf777f77ff } - there is only one entry because
the bits per block for this format ID is 1 (see 3.1 format table)

The segments will be used to get indexes
Lets break down segments into bits:
Segment[0]:
0 1 1 1 0 0 1 1 0 0 1 1 1 1 1 1
0 1 1 1 0 0 1 1 1 0 1 1 1 1 1 1
0 1 1 1 0 1 1 1 0 1 1 1 1 1 1 1
0 1 1 1 0 1 1 1 1 1 1 1 1 1 1 1

Since this is just one segment it will simply translate over into indexes
Indexes = { 0, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1
            0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1
            0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1
            0, 1, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1, 1 }
Now each index in indexes corresponds to a value in the palette
0 = 0x1500
1 = 0x1000


At index 58:
Format ID = 6
Offset = 1212
Skip to offset 1212 from the end of the table which in this example is this:
b0 00 10 00 15 00 11 00
30 00 ff ff ff ff ff ff
10 00 30 00 91 12 09 97
01 11 21 11 91 13 09 91
00 00 00 00 66 20 f6 60
Since we know the format ID is 6, the palette is size (1 << bits per block) = 8
Palette = { 0xb000, 0x1000, 0x1500, 0x1100, 0x3000, 0xffff, 0xffff, 0xffff } - this is our palette of newBlocks
Segments = { 0x1000300091120997, 0x0111211191130991, 0x000000006620f660 } - there are 3 entries because
the bits per block for this format ID is 3 (see 3.1 format table)

The segments will be used to get indexes
Lets break down segments into bits:
Segment[0]:
0 0 0 1 0 0 0 0 0 0 0 0 0 0 0 0
0 0 1 1 0 0 0 0 0 0 0 0 0 0 0 0
1 0 0 1 0 0 0 1 0 0 0 1 0 0 1 0
0 0 0 0 1 0 0 1 1 0 0 1 0 1 1 1
Segment[1]:
0 0 0 0 0 0 0 1 0 0 0 1 0 0 0 1
0 0 1 0 0 0 0 1 0 0 0 1 0 0 0 1
1 0 0 1 0 0 0 1 0 0 0 1 0 0 1 1
0 0 0 0 1 0 0 1 1 0 0 1 0 0 0 1
Segment[2]:
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 0 0 0 0 0 0 0 0 0 0 0 0 0 0 0
0 1 1 0 0 1 1 0 0 0 1 0 0 0 0 0
1 1 1 1 0 1 1 0 0 1 1 0 0 0 0 0

Let's loop over the 2 segment piece and add them into indexes:
Starting with segment[0]:
Indexes = { 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0
            1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0
            0, 0, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1 }
            
Next, we bitwise operator OR the values from segment[1] but with each value in bit shifted left 1
in order to append bits on each other, this is the end result after this step is done:
Indexes = { 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2
            0, 0, 3, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2
            3, 0, 0, 3, 0, 0, 0, 3, 0, 0, 0, 3, 0, 0, 3, 2
            0, 0, 0, 0, 3, 0, 0, 3, 3, 0, 0, 3, 0, 1, 1, 3 }
            
            Next, we bitwise operator OR the values from segment[2] but with each value in bit shifted left 2
in order to append bits on each other, this is the end result after this step is done:
Indexes = { 0, 0, 0, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2
            0, 0, 3, 1, 0, 0, 0, 2, 0, 0, 0, 2, 0, 0, 0, 2
            3, 4, 4, 3, 0, 4, 4, 3, 0, 0, 4, 3, 0, 0, 3, 2
            4, 4, 4, 4, 3, 4, 4, 3, 3, 4, 4, 3, 0, 1, 1, 3 }
Now each index in indexes corresponds to a value in the palette
0 = 0xb000
1 = 0x1000
2 = 0x1500
3 = 0x1100
4 = 0x3000
5-7 = 0xffff - this means it is unused
which can be seen that no index in the indexes array is 5 to 7
```

# 4. Light `[*+0x4c...]`
- `*` is the sections' size in bytes `(see 2.1 point 2)`
- There are 4 sections in light
  - First 2 is sky-light
  - Last 2 is block-light
  - The first light section is y0-y127 newBlocks with the second section of the same light type being y128-y255

## 4.1 Light Section size `[*+0x4c...*+0x4f]`
- The first 4 bytes of a light section header as a `(int/int32_t)` represent the light section's size (not in bytes)
- The light section's size in bytes is obtained by multiplying the parseLayer value by 0x80 (128) and then adding 128
(which is the size of the light section header)
- If the size parseLayer value is 0, that means the light values are all in the header

## 4.2 Light Section's Header `[*+X+0x50...*+X+Y+0x4f]`
- `X` is the size of the previous light sections sizes added together `(see 4.1 point 2 for size and 4 for the different sections)`
- `Y` is the light section size `(see 4.1 point 2)`
- The following 0x80 (128) bytes is the header for the lights in the section
- Each byte in the header points to the start of the light data
- Each byte in the header is an offset the light data with 2 exceptions:
  - If the parseLayer byte is `0x80 (128)` then the 128 bytes is filled with `0` values
  - If the parseLayer byte is `0x81 (129)` then the 128 bytes is filled with `0xf (15)` values
- Each offset is multiplied by 128 plus 128 to get the actual offset from the start of the light header `(start of 4.2)`

- The next 0x80 bytes `[X+0x4..X+0x83]` is the light data header. Every byte is an offset pointing to where the light data should be retrieved from (each offset points to somewhere in the light data section, each taking up 0x80 bytes). Multiply the offset by `0x80` to get the actual offset in the light data.

## 4.3 Light Section's Data `[*+X+Y+0x50...*+X+Y+0xcf]`
- `X` is the size of the previous light sections sizes added together `(see 4.1 point 2 for size and 4 for the different sections)`
- `Y` is the offset `(see 4.2 point 5)`
- At each offset is 128 bytes of light data in XZY format

# Section 4 Example
```
# AQUATIC Light Header
00 00 00 14
80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80
80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80 80
00 01 02 03 04 05 06 07 08 09 0a 0b 0c 0d 0e 0f 10 11 12 13 81 81
81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81 81

Let's break this down:
(4.1) Light Section Size = 0x14 (20)
(4.2) The buffer for this light section would be:
64 (number of 0x80 bytes in the header) * 128 = 8192 bytes of value 0
then for the next 20 (number of offset values in the header) * 128 = 2560 bytes of whatever follows the header
(the offset values might not be in order, the offset (see 4.2 point 6) still needs to be seeked to for each header bytes,
but in this case the values are in order so the quick explaination applies)
then 34 (number of 0x81 bytes in the header) * 128 = 4352 bytes of value 0xf (15)
```

# 5. Additional Chunk Data `[*+X+0xd0...*+X+0x2d1]`
- `*` is the sections' size in bytes `(see 2.1 point 2)`
- `X` is the size of the all the light sections sizes added together `(see 4.1 point 2 for size and 4 for the different sections)`

## 5.1 Heightmap `[*+X+0xd0...*+X+0x1cf]`
- The first 128 bytes are the chunk's heightmap stored in ZX order

## 5.2 Terrain Populated Flags `[*+X+0x1d0...*+X+0x1d1]`
- The next 2 bytes as a `short/int16_t` represent the chunk's Terrain Populated Flags

## 5.3 Biomes `[*+X+0x1d2...*+X+0x2d1]`
- The next 128 bytes are the chunk's biomes' ID stored in ZX order

# 6. NBT Data `[*+X+0x2d2...END]`
- `*` is the sections' size in bytes `(see 2.1 point 2)`
- `X` is the size of the all the light sections sizes added together `(see 4.1 point 2 for size and 4 for the different sections)`
- The rest of the file contains NBT data of `Entities`, `TileEntities` and `TileTicks`