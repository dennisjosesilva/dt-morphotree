#include "dt-morphotree/DTComputer.hpp"

#include <fstream>
#include <sstream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <iostream>

using morphotree::I32Point;

DTComputer::DTComputer()
{
  init();
}

DTComputer::~DTComputer()
{
  destroy();
}

void DTComputer::destroy()
{
  glDeleteProgram(shader_);
  shader_ = 0;
}

void DTComputer::init()
{
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  window_ = glfwCreateWindow(1, 1, "Distance Transform", nullptr, 
    nullptr);
  if (window_ == nullptr) {
    glfwTerminate();
    return;
  }

  glfwMakeContextCurrent(window_);
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress)) {
    // ERROR
    glfwTerminate();
    return;
  }

  setUpShaders();
}

void DTComputer::setUpShaders()
{
  uint32 vertex = glCreateShader(GL_VERTEX_SHADER);
  std::string vSource = readShaderSource("../shaders/vertex.glsl");
  const char *vSourcePtr = vSource.c_str();
  glShaderSource(vertex, 1, &vSourcePtr, nullptr);
  glCompileShader(vertex);
  checkShaderErrors(vertex, "VERTEX");

  uint32 geometry = glCreateShader(GL_GEOMETRY_SHADER);
  std::string gSource = readShaderSource("../shaders/geometry.glsl");
  const char *gSourcePtr = gSource.c_str();
  glShaderSource(geometry, 1, &gSourcePtr, nullptr);
  glCompileShader(geometry);
  checkShaderErrors(geometry, "GEOMETRY");

  uint32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
  std::string fSource = readShaderSource("../shaders/fragment.glsl");
  const char *fSourcePtr = fSource.c_str();
  glShaderSource(fragment, 1, &fSourcePtr, nullptr);
  glCompileShader(fragment);
  checkShaderErrors(fragment, "FRAGMENT");

  shader_ = glCreateProgram();
  glAttachShader(shader_, vertex);
  glAttachShader(shader_, geometry);
  glAttachShader(shader_, fragment);

  glLinkProgram(shader_);
  checkShaderErrors(shader_, "PROGRAM");

  glDeleteShader(vertex);
  glDeleteShader(geometry);
  glDeleteShader(fragment);
}

std::string DTComputer::readShaderSource(const std::string &filepath)
{
  std::ifstream shaderFile;
  shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

  try {
    // Read shader file 
    shaderFile.open(filepath);
    std::stringstream shaderStream;
    shaderStream << shaderFile.rdbuf();
    shaderFile.close();

    return shaderStream.str();
  }
  catch (std::ifstream::failure &e) {
    std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " 
      << e.what() << std::endl;
    return std::string();
  }
}

std::list<I32Point> DTComputer::traceBoundaries(const Box &domain,
  const std::vector<bool> &bimg) const
{
  std::list<I32Point> b;

  for (I32Point p = domain.topleft(); p.y() <= domain.bottom(); p.y()++) {
    for (p.x() = domain.left(); p.x() <= domain.right(); p.x()++) {
      if (bimg[domain.pointToIndex(p)] && isBoundary(domain, bimg, p))
        b.push_back(p);
    }
  }

  return b;
}

bool DTComputer::isBoundary(const Box &domain, const std::vector<bool> &bimg, 
  const I32Point &p) const 
{
  std::vector<I32Point> neighborhood = { {-1, 0}, {0, -1}, {1, 0}, {0, 1} };

  for (const I32Point &offset : neighborhood) {
    I32Point q = p + offset;
    if (!domain.contains(q) || !bimg[domain.pointToIndex(q)])
      return true;
  }

  return false;
}

void DTComputer::genSplatTexture(const Box &domain)
{
  int R = splatRadius_;
  int S = splatDiameter_;
  float dt[S][S];

  for (int i = 0; i < S; i++) {
    for (int j = 0; j < S; j++) {
      float dst = sqrtf32((i-R)*(i-R) + (j-R)*(j-R));
      if (dst > R) 
        dst = R;
      dt[i][j] = dst / static_cast<float>(R);
    }
  }

  glGenTextures(1, &splatTexture_);
  glBindTexture(GL_TEXTURE_2D, splatTexture_);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, S, S, 0, GL_RED,
    GL_FLOAT, dt);
  glTexEnvf(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_REPLACE);
}

void DTComputer::setupFramebuffer(const Box &domain)
{
  glGenFramebuffers(1, &framebuffer_);
  glBindFramebuffer(GL_FRAMEBUFFER, framebuffer_);

  glGenTextures(1, &texFramebuffer_);
  glBindTexture(GL_TEXTURE_2D, texFramebuffer_);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, domain.width(), domain.height(),
    0, GL_RGB, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
    texFramebuffer_, 0);
  
  glGenRenderbuffers(1, &rbo_);
  glBindRenderbuffer(GL_RENDERBUFFER, rbo_);
  glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, domain.width(),
    domain.height());
  glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, 
    GL_RENDERBUFFER, rbo_);
}

std::vector<float> DTComputer::compute(const Box &domain, 
  const std::vector<bool> &bimg)
{
  glViewport(0, 0, domain.width(), domain.height());
  splatDiameter_ = std::min(domain.width(), domain.height());
  splatRadius_ = splatDiameter_ / 2;

  std::list<I32Point> b = traceBoundaries(domain, bimg);
  std::vector<float> boundary;
  boundary.reserve(b.size()*2);

  for (I32Point p : b) {
    boundary.push_back(p.x());
    boundary.push_back(p.y());
  }

  glUseProgram(shader_);
  genSplatTexture(domain);
  setupFramebuffer(domain);

  glEnable(GL_BLEND);
  glBlendEquation(GL_MIN);
  glBlendFunc(GL_ONE, GL_ONE);

  glGenVertexArrays(1, &vao_);
  glBindVertexArray(vao_);
  glGenBuffers(1, &vbo_);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_);
  glBufferData(GL_ARRAY_BUFFER, sizeof(float)*boundary.size(), 
    &boundary[0], GL_STATIC_DRAW);

  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2*sizeof(float),
    (void*)0);

  glUniform1i(glGetUniformLocation(shader_, "splatRadius"), splatRadius_);

  glm::mat4 projection = glm::ortho(
    static_cast<float>(domain.left()), static_cast<float>(domain.right()),
    static_cast<float>(domain.top()), static_cast<float>(domain.bottom()),
    -1.0f, 1.0f);

  glUniformMatrix4fv(glGetUniformLocation(shader_, "viewProjection"), 1, 
    GL_FALSE, &projection[0][0]);
  
  glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  glBindVertexArray(vao_);
  glBindTexture(GL_TEXTURE_2D, splatTexture_);
  glUniform1i(glGetUniformLocation(shader_, "splatTexture"), 0);
  glActiveTexture(GL_TEXTURE0);

  glDrawArrays(GL_POINTS, 0, b.size());

  std::vector<float> dt(domain.numberOfPoints(), 0.0f);
  glReadPixels(0, 0, domain.width(), domain.height(), GL_RED, 
    GL_FLOAT, &dt[0]);
  
  glDeleteFramebuffers(1, &framebuffer_);
  glDeleteTextures(1, &texFramebuffer_);
  glDeleteTextures(1, &splatTexture_);
  glDeleteRenderbuffers(1, &rbo_);
  glDeleteBuffers(1, &vbo_);

  return dt;
}

void DTComputer::checkShaderErrors(uint32 id, 
  const std::string &type) const
{
  int success;
  char infoLog[1024];
  if (type != "PROGRAM") {
    glGetShaderiv(id, GL_COMPILE_STATUS, &success);
    if (!success) {
      glGetShaderInfoLog(id, 1024, nullptr, infoLog);
      std::cerr << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n"
        << infoLog << "\n----------------------------------------------------\n";
    }
  }
  else {
    glGetProgramiv(id, GL_LINK_STATUS, &success);
    if (!success) {
      glGetProgramInfoLog(id, 1024, nullptr, infoLog);
      std::cerr << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n"
      << infoLog << "\n-------------------------------------------------\n";
    }
  }
}