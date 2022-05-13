#include "dt-morphotree/DTComputer.hpp"
#include "dt-morphotree/DTMorphotree.hpp"

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
  using morphotree::uint8;

  int width, height, channels;
  unsigned char *data = stbi_load(argv[1], &width, &height,
    &channels, 1);

  Box domain = Box::fromSize(UI32Point{
    static_cast<uint32>(width), 
    static_cast<uint32>(height)});

  std::vector<uint8> f(data, data + domain.numberOfPoints());

  DTMorphotree dtMorphotree{domain, f};

  std::vector<uint8> out = dtMorphotree.dtImage(50);
  stbi_write_png(argv[2], width, height, 1, (void*)out.data(), 0);

  std::vector<uint8> nodeImg = dtMorphotree.nodeImage(50);
  stbi_write_png(argv[3], width, height, 1, (void*)nodeImg.data(), 0);

  std::map<uint8, float> pdt = dtMorphotree.dtPoint(I32Point{186, 412});

  for (std::pair<uint8, float> levelDT : pdt) {
    std::cout << static_cast<int>(levelDT.first) << ": " << levelDT.second << "\n";
  }

  return 0;
}