// stbimpl.cpp
//
// Single translation unit with the stb implementations used by nchat,
// compiled separately so third-party code does not inherit nchat's warning flags.

#define STB_IMAGE_IMPLEMENTATION
#define STBI_ONLY_JPEG
#define STBI_ONLY_PNG
#define STBI_ONLY_GIF
#include "stb_image.h"

#define STB_IMAGE_RESIZE_IMPLEMENTATION
#include "stb_image_resize2.h"
