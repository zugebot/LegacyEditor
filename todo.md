Todo:

Fix v12Chunk::writeBlockData:
- currently, it does not write newBlocks correctly 
if there is an empty section, with newBlocks above
said section. Writing newBlocks to the highest section
also does not work.

Add V11Chunk::write and V11Chunk::read
- This is needed for switch support

Find correct compression functions for xbox360 and ps3.
- PS3 will be easier to do than xbox360.

Refactor FileListing.
- support multiple file readings.
- add switch support.
- make files have custom data to support data like
  1. map data (index)
  2. region data (dimension, x, z)
This is so that it is easier to go from and to files,
instead of relying on filenames.
I think I can do this by using NBT.
- add support for holding entities, so:
- An issue with converting to and from switch, is that entities
are stored by dimension, not chunk, so all chunks would need to
be appended with the correct NBT

