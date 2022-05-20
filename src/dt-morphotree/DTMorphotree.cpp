#include "dt-morphotree/DTMorphotree.hpp"
#include "dt-morphotree/DTComputer.hpp"

#include <morphotree/adjacency/adjacency8c.hpp>

#include <iostream>
#include <stack>

#include <chrono>


DTMorphotree::DTMorphotree(const Box &domain, const std::vector<uint8> &f)
  : domain_{domain},
    tree_{morphotree::MorphoTreeType::MaxTree}
{
  init(f);
}


void DTMorphotree::init(const std::vector<uint8> &f)
{
  using std::chrono::steady_clock;
  using std::chrono::duration_cast;
  using std::chrono::milliseconds;


  using morphotree::buildMaxTree;
  using morphotree::Adjacency8C;


  steady_clock::time_point start = steady_clock::now();
  const int L = 256;

  DTComputer dtComputer;
  dt_.reserve(L);
  
  for(int l = 0; l < L; l++) {
    //std::cout << "computing DT for level: " << l << "\n";
    std::vector<bool> bimg(domain_.numberOfPoints(), false);

    for (int pidx = 0; pidx < domain_.numberOfPoints(); pidx++)
      bimg[pidx] = f[pidx] >= l;

    dt_.push_back(dtComputer.compute(domain_, bimg));   
  }
  steady_clock::time_point end = steady_clock::now();
  std::cout << "Elapsed time: " << duration_cast<milliseconds>(end-start).count() << " ms\n";

  tree_ = buildMaxTree(f, std::make_unique<Adjacency8C>(domain_));
}

std::vector<float> DTMorphotree::dt(uint32 nodeId) const
{
  using NodePtr = typename MTree::NodePtr;

  NodePtr node = tree_.node(nodeId);
  const std::vector<float> &ldt = dt_[node->level()];
  std::vector<float> nodeDT(domain_.numberOfPoints(), 0.0f);

  std::stack<NodePtr> s;
  s.push(node);

  while (!s.empty())
  {
    NodePtr snode = s.top();
    s.pop();

    for (uint32 pidx : snode->cnps()) {
      nodeDT[pidx] = ldt[pidx];
    }

    for (NodePtr c : snode->children()) {
      s.push(c);
    }
  }
  
  return nodeDT;
}

std::vector<morphotree::uint8> DTMorphotree::dtImage(uint32 nodeId) const
{
  std::vector<float> nodeDT = dt(nodeId);
  std::vector<uint8> dtImg(domain_.numberOfPoints(), 0);

  for (uint32 pidx = 0; pidx < domain_.numberOfPoints(); pidx++)
    dtImg[pidx] = nodeDT[pidx] * 255;

  return dtImg;
}

std::vector<float> DTMorphotree::dtSmallComponent(I32Point p) const
{
  return dt(tree_.smallComponent(domain_.pointToIndex(p))->id());
}

std::vector<morphotree::uint8> DTMorphotree::dtSmallComponentImage(I32Point p) const
{
  return dtImage(tree_.smallComponent(domain_.pointToIndex(p))->id());
}

std::map<morphotree::uint8, float> DTMorphotree::dtPoint(I32Point p) const
{
  using NodePtr = MTree::NodePtr;
  std::map<uint8, float> pdt;
  uint32 pidx = domain_.pointToIndex(p);
  NodePtr node = tree_.smallComponent(pidx);

  for (int level=node->level(); level > 0; level--) {
    pdt[static_cast<uint8>(level)] = dt_[level][pidx];
  }

  return pdt;
}

std::vector<morphotree::uint8> DTMorphotree::nodeImage(uint32 nodeIdx) const
{
  std::vector<bool> bimg = tree_.reconstructNode(nodeIdx, domain_);
  std::vector<uint8> f(domain_.numberOfPoints(), 0);
  
  for (uint32 pidx = 0; pidx < domain_.numberOfPoints(); pidx++) {
    f[pidx] = (1 - bimg[pidx]) * 255;
  }
  
  return f;
}