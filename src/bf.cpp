#include "dt-morphotree/DTComputer.hpp"

#include <iostream>
#include <vector>
#include <algorithm>

#include <chrono>

#include <morphotree/core/box.hpp>
#include <morphotree/tree/mtree.hpp>
#include <morphotree/adjacency/adjacency8c.hpp>

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
  using morphotree::buildMaxTree;
  using morphotree::Adjacency8C;

  using MTree = morphotree::MorphologicalTree<uint8>;
  using NodePtr = typename MTree::NodePtr;

  using std::chrono::steady_clock;
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;

  int width, height, channels;
  unsigned char *data = stbi_load(argv[1], &width, &height, &channels, 1);

  Box domain = Box::fromSize(UI32Point{
    static_cast<uint32>(width),
    static_cast<uint32>(height)
  });

  std::vector<uint8> f{data, data + domain.numberOfPoints() };

  MTree tree = buildMaxTree(f, std::make_unique<Adjacency8C>(domain));
  DTComputer dtComputer;

  steady_clock::time_point start = steady_clock::now();
  tree.tranverse([&dtComputer, &domain](NodePtr node){
    dtComputer.compute(domain, node->reconstruct(domain));
  });
  steady_clock::time_point end = steady_clock::now();
  std::cout << "Time elapsed: " << duration_cast<milliseconds>(end-start).count() << " ms\n";

  return 0;
} 