#ifndef LOADER_HPP
#define LOADER_HPP

#include <string>

#include "mesh.hpp"
#include "world.hpp"

Mesh loadOBJ(const std::string& path);
World loadGLTF(const std::string& path);

#endif // LOADER_HPP
