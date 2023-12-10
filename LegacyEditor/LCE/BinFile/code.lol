DataManager in_ext;
in_ext.readFromFile(dir_path + R"(tests\WiiU Save\231008144148.ext)");
in_ext.data = in_ext.data + 0x100;
in_ext.ptr = in_ext.data;
in_ext.size -= 0x100;
WorldOptions options = getTagsInImage(in_ext);