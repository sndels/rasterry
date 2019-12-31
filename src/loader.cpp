#include "loader.hpp"

#include <fstream>
#include <limits>
#include <sstream>

namespace {
    void throwLineError(const std::string& error, size_t lineNum)
    {
        char err[80];
        snprintf(err, 80, "%s on line %zu", error.c_str(), lineNum);
        throw std::runtime_error(std::string(err));
    }
}

Mesh loadOBJ(const std::string& path)
{
    fprintf(stderr, "Parsing obj %s\n", path.c_str());

    std::ifstream objFile;
    objFile.open(path);
    if (!objFile.is_open()) {
        throw std::runtime_error("Failed to open file");
    }
    std::stringstream objStream;
    objStream << objFile.rdbuf();
    std::string objString = objStream.str();

    Primitive primitive;
    primitive.min = glm::vec3(std::numeric_limits<float>::max());
    primitive.max = glm::vec3(std::numeric_limits<float>::min());
    size_t lineNum = 1;
    const char* buffer = objString.c_str();
    while (*buffer) {
        // Skip empty lines
        if (*buffer == '\n') {
            lineNum++;
            buffer++;
            continue;
        }

        // Parse line type
        char linetype[3];
        if (sscanf(buffer, "%2s ", linetype) != 1)
            throwLineError("Invalid linetype", lineNum);

        // Skip line type to simplify parsing
        buffer += strlen(linetype);

        // Parse line data
        if (strcmp(linetype, "v") == 0) {
            glm::vec4 v(0, 0, 0, 1);
            if (sscanf(buffer, "%f %f %f %f", &v.x, &v.y, &v.z, &v.w) < 3)
                throwLineError("Invalid vertex", lineNum);
            glm::vec3 v3(glm::vec3(v) / v.w);
            primitive.min = glm::min(primitive.min, v3);
            primitive.max = glm::max(primitive.max, v3);
            // TODO: Parse from buffers once normals etc. are supported
            Vertex vertex;
            vertex.pos = v3;
            primitive.verts.emplace_back(std::move(vertex));
        } else if (strcmp(linetype, "f") == 0) {
            TriIndices tis;
            // TODO: Mixed indices
            if (sscanf(buffer, "%zu %zu %zu", &tis.v0, &tis.v1, &tis.v2) != 3)
                throwLineError("Invalid face", lineNum);
            // OBJ indices start at 1
            tis.v0--;
            tis.v1--;
            tis.v2--;
            primitive.tris.push_back(std::move(tis));
        }

        // Seek to the end of the line
        while (*buffer++ != '\n') {
            if (!*buffer)
                break;
        }
        lineNum++;
    }

    printf("%zu verts and %zu tris\n", primitive.verts.size(), primitive.tris.size());
    printf(
        "min (%.2f, %.2f, %.2f) max (%.2f, %.2f, %.2f)\n",
        primitive.min.x, primitive.min.y, primitive.min.z,
        primitive.max.x, primitive.max.y, primitive.max.z
    );

    return {primitive.min, primitive.max, {primitive}};
}
