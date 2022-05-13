#pragma once 

#include <string>
#include <list>
#include <vector>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <morphotree/core/box.hpp>
#include <morphotree/core/point.hpp>

// #include <iostream>

class DTComputer 
{
public:
  using Box = morphotree::Box;
  using I32Point = morphotree::I32Point;
  using uint32 = morphotree::uint32;

  DTComputer();
  std::vector<float> compute(const Box &domain, 
    const std::vector<bool> &bimg);

  ~DTComputer();

private:
  void init();
  void destroy();

  std::string readShaderSource(const std::string &filepath);
  void setUpShaders();

  std::list<I32Point> traceBoundaries(const Box &domain, 
    const std::vector<bool> &bimg) const;
  bool isBoundary(const Box &domain, const std::vector<bool> &bimg,
    const I32Point &p) const;
  
  void genSplatTexture(const Box &domain);
  void setupFramebuffer(const Box &domain);

  void checkShaderErrors(uint32 id, 
    const std::string &type) const;
  
private:
  GLFWwindow *window_;

  uint32 shader_;
  uint32 vao_;
  uint32 vbo_;

  uint32 framebuffer_;
  uint32 texFramebuffer_;
  uint32 rbo_;

  uint32 splatTexture_;

  int splatDiameter_;
  int splatRadius_;
};