// -*- coding:utf-8; mode:c++; mode:auto-fill; fill-column:80; -*-

/// @file      line.cpp
/// @brief     Implementation of line.hpp.
/// @author    F. Ayala <19fraayala@asfg.edu.mx>
/// @date      December 11, 2018
/// @copyright (C) 2018 Nabla Zero Labs

// Related mxd header
#include "line.hpp"

// C++ Standard Library
#include <memory>
#include <string>
#include <vector>

// mxd Library
#include "program.hpp"
#include "shader.hpp"
#include "time_point.hpp"

// Third party libraries
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>

namespace {  // anonymous namespace

/// @TODO Do we really want to hard-code shader sources? What happens if we want
/// to modify a small detail? Do we need to make a full build? Aren't Shaders
/// regular source code?
const std::string vertex_shader_source =
    "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "void main(){\n"
    "  gl_Position = vec4(aPos.x, aPos.y, aPos.z, 1.0);\n"
    "}\n";

const std::string fragment_shader_source =
    "#version 330 core\n"
    "out vec4 FragColor;\n"
    "uniform vec3 color;\n"
    "void main(){\n"
    "  FragColor = vec4(color, 1.0f);\n"
    "}\n";

/// @TODO This is too ugly. Program needs a full refactoring.
auto make_program() {
  /// @TODO The Shader/Program interface feels very awkward. For instance: why
  /// do I need to pass a vector of shaders? Why do I need to compile the
  /// shaders and then compile the program? Shouldn't the program compile the
  /// shaders if it needs to? What happens if I forget to compile the shader?
  std::vector<nzl::Shader> shaders;
  shaders.emplace_back(nzl::Shader::Stage::Vertex, vertex_shader_source);
  shaders.emplace_back(nzl::Shader::Stage::Fragment, fragment_shader_source);
  for (auto& shader : shaders) {
    shader.compile();
  }

  nzl::Program program(shaders);
  program.compile();

  return program;
}
}  // anonymous namespace

namespace nzl {

/// Put all you need in the Implementation.
struct nzl::Line::LineImp {
  LineImp();
  ~LineImp() noexcept;
  nzl::Program program;  /// @TODO Why not provide a default constructor?
  unsigned int vao_id;
  unsigned int vbo_id;
  int number_of_points{0};
  glm::vec3 color;

  void load_points(std::vector<glm::vec3> points);
};

nzl::Line::LineImp::LineImp() : program{make_program()} {
  /// @TODO Add error checking! 10 minutes spent adding good error checking and
  /// error messages will save you 10 hours debugging the program in the future.
  glGenVertexArrays(1, &vao_id);
  glBindVertexArray(vao_id);
  glGenBuffers(1, &vbo_id);
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
  glDisableVertexAttribArray(0);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
  glBindVertexArray(0);
}

nzl::Line::LineImp::~LineImp() noexcept {
  /// @TODO: Add error checking!
  glDeleteVertexArrays(1, &vao_id);
  glDeleteBuffers(1, &vbo_id);
}

/// @TODO What would happen if points were a very large array? (hint: why are we
/// not passing by reference or moving; why don't I have a point array).
void nzl::Line::LineImp::load_points(std::vector<glm::vec3> points) {
  number_of_points = points.size();
  glBindBuffer(GL_ARRAY_BUFFER, vbo_id);
  glBufferData(GL_ARRAY_BUFFER, points.size() * 3 * sizeof(float),
               points.data(), GL_STATIC_DRAW);
  glBindBuffer(GL_ARRAY_BUFFER, 0);
}

// -----------------------------------------------------------------------------
//         The section below forwards API calls to the implementation
// -----------------------------------------------------------------------------

Line::Line(glm::vec3 color) : m_pimpl{std::make_shared<Line::LineImp>()} {
  m_pimpl->color = color;
}

Line::Line() : Line(glm::vec3(1.0f, 1.0f, 1.0f)) {}

Line::Line(glm::vec3 color, std::vector<glm::vec3> points) : Line(color) {
  m_pimpl->load_points(points);
}

void Line::load_points(std::vector<glm::vec3> points) noexcept {
  m_pimpl->load_points(points);
}

glm::vec3 Line::color() const noexcept { return m_pimpl->color; }

void Line::set_color(glm::vec3 color) noexcept { m_pimpl->color = color; }

/// @TODO Shouldn't this return a const reference? Why are we returning a copy?
nzl::Program Line::get_program() const noexcept { return m_pimpl->program; }

/// @TODO Mark unused variables! Compilation must be 100% clean with no
/// warnings.
void Line::do_render(TimePoint t [[maybe_unused]]) {
  auto&& program = m_pimpl->program;
  program.use();
  program.set("color", m_pimpl->color);

  /// @TODO Add error checking!
  glBindVertexArray(m_pimpl->vao_id);
  glEnableVertexAttribArray(0);
  glDrawArrays(GL_LINE_STRIP, 0, m_pimpl->number_of_points);
  glDisableVertexAttribArray(0);
  glBindVertexArray(0);
}

}  // namespace nzl
