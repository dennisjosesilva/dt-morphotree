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

#include <chrono>

int main(int argc, char *argv[])
{
  using morphotree::Box;
  using morphotree::I32Point;
  using morphotree::UI32Point;
  using morphotree::uint32;
  using morphotree::uint8;

  using std::chrono::steady_clock;
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;

  int width, height, channels;
  unsigned char *data = stbi_load(argv[1], &width, &height,
    &channels, 1);

  Box domain = Box::fromSize(UI32Point{
    static_cast<uint32>(width), 
    static_cast<uint32>(height)});

  std::vector<uint8> f(data, data + domain.numberOfPoints());

  DTMorphotree dtMorphotree{domain, f};

  steady_clock::time_point start = steady_clock::now();
  dtMorphotree.dt(dtMorphotree.numberOfNodes() / 2);
  steady_clock::time_point end = steady_clock::now();
  std::cout << "Elapsed time (node rec): " << duration_cast<milliseconds>(end-start).count() << " ms\n";

  start = steady_clock::now();
  for (int nodeId = 0; nodeId < dtMorphotree.numberOfNodes(); ++nodeId) {
    dtMorphotree.dt(nodeId);
  }
  end = steady_clock::now();
  std::cout << "Elapsed time (full rec): " << duration_cast<milliseconds>(end-start).count() << " ms\n";

  // std::vector<uint8> out = dtMorphotree.dtImage(50);
  // stbi_write_png(argv[2], width, height, 1, (void*)out.data(), 0);

  // std::vector<uint8> nodeImg = dtMorphotree.nodeImage(50);
  // stbi_write_png(argv[3], width, height, 1, (void*)nodeImg.data(), 0);

  // std::map<uint8, float> pdt = dtMorphotree.dtPoint(I32Point{186, 412});

  // for (std::pair<uint8, float> levelDT : pdt) {
  //   std::cout << static_cast<int>(levelDT.first) << ": " << levelDT.second << "\n";
  // }

  return 0;
}