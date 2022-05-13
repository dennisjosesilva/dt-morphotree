#include "dt-morphotree/DTComputer.hpp"
#include <iostream>
#include <vector>
#include <algorithm>

#include <morphotree/core/box.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define STB_IMAGE_WRITE_IMPLEMENTATION
#include <stb_image_write.h>

int main(int argc, char *argv[])
{
  using morphotree::Box;
  using morphotree::I32Point;
  using morphotree::UI32Point;
  using morphotree::uint32;

  int width, height, channels;
  unsigned char *data = stbi_load(argv[1], &width, &height,
    &channels, 1);

  Box domain = Box::fromSize(UI32Point{
    static_cast<uint32>(width), 
    static_cast<uint32>(height)});

  std::vector<bool> bimg(domain.numberOfPoints(), true);

  for (int i = 0; i < domain.numberOfPoints(); i++) {
    if (data[i] > 0)
      bimg[i] = false;
  }
  stbi_image_free(data);

  std::vector<float> dt = DTComputer().compute(domain, bimg);

  std::vector<unsigned char> out(dt.size(), 0);
  for (int i = 0; i < domain.numberOfPoints(); ++i) {
    out[i] = dt[i] * 255;
  }

  stbi_write_png(argv[2], width, height, 1, (void*)out.data(), 0);

  return 0;
}