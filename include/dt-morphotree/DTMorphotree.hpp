#pragma once 

#include <morphotree/tree/mtree.hpp>
#include <map>


class DTMorphotree
{
public:
  using uint8 = morphotree::uint8;
  using uint32 = morphotree::uint32;
  using I32Point = morphotree::I32Point;

  using MTree = morphotree::MorphologicalTree<uint8>;
  using Box = morphotree::Box;

  DTMorphotree(const Box &domain, const std::vector<uint8> &f);

  std::vector<float> dt(uint32 nodeId) const;
  std::vector<uint8> dtImage(uint32 nodeId) const;

  std::vector<float> dtSmallComponent(I32Point p) const;
  std::vector<uint8> dtSmallComponentImage(I32Point p) const;

  std::map<uint8, float> dtPoint(I32Point p) const;

  std::vector<uint8> nodeImage(uint32 nodeIdx) const;


private:
  void init(const std::vector<uint8> &f);

private:
  MTree tree_;
  std::vector<std::vector<float>> dt_;
  Box domain_;
};