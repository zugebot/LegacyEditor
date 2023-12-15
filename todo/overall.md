Todo:

Non-complete v12Chunk::writeBlockData:
- currently, it does not writeData submerged blocks back.

Find correct compression functions for xbox360 and ps3.
- PS3 will be easier to do than xbox360.

- An issue with converting to and from switch, is that entities
are stored by dimension, not chunk, so all chunks would need to
be appended with the correct NBT

- get rid of windows only imports (#include <corecrt.h>)