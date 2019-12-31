#include "loader.hpp"

#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/matrix_decompose.hpp>
#include <fstream>
#include <limits>
#include <sstream>
#include <tiny_gltf.h>

namespace {
    void throwLineError(const std::string& error, size_t lineNum)
    {
        char err[80];
        snprintf(err, 80, "%s on line %zu", error.c_str(), lineNum);
        throw std::runtime_error(std::string(err));
    }

    std::vector<Texture> loadTextures(const tinygltf::Model& gltfModel)
    {
        std::vector<Texture> textures;
        for (const auto& gltfTexture : gltfModel.textures)
            textures.emplace_back(gltfModel.images[gltfTexture.source]);
        return textures;
    }

    std::vector<Material> loadMaterials(const tinygltf::Model& gltfModel, const std::vector<Texture>& textures)
    {
        std::vector<Material> materials;
        for (const auto& gltfMaterial : gltfModel.materials) {
            Material material;
            if (const auto& elem = gltfMaterial.values.find("baseColorTexture");
                elem != gltfMaterial.values.end()) {
                material.baseColor = &textures[elem->second.TextureIndex()];
                assert(elem->second.TextureTexCoord() == 0);
            }
            if (const auto& elem = gltfMaterial.values.find("metallicRoughnessTexture");
                elem != gltfMaterial.values.end()) {
                material.metallicRoughness = &textures[elem->second.TextureIndex()];
                assert(elem->second.TextureTexCoord() == 0);
            }
            if (const auto& elem = gltfMaterial.additionalValues.find("normalTexture");
                elem != gltfMaterial.additionalValues.end()) {
                material.normal = &textures[elem->second.TextureIndex()];
                assert(elem->second.TextureTexCoord() == 0);
            }
            if (const auto& elem = gltfMaterial.values.find("baseColorFactor");
                elem != gltfMaterial.values.end()) {
                material.baseColorFactor = glm::make_vec4(elem->second.ColorFactor().data());
            }
            if (const auto& elem = gltfMaterial.values.find("metallicFactor");
                elem != gltfMaterial.values.end()) {
                material.metallicFactor = elem->second.Factor();
            }
            if (const auto& elem = gltfMaterial.values.find("roughnessFactor");
                elem != gltfMaterial.values.end()) {
                material.roughnessFactor = elem->second.Factor();
            }
            materials.push_back(std::move(material));
        }
        return materials;
    }

    std::vector<Mesh> loadMeshes(const tinygltf::Model& gltfModel, const std::vector<Material>& materials)
    {
        std::vector<Mesh> meshes;
        for (const auto& gltfMesh : gltfModel.meshes) {
            Mesh mesh;
            for (const auto& gltfPrimitive : gltfMesh.primitives) {
                // TODO: Support modes other than triangle
                assert(gltfPrimitive.mode == -1 || gltfPrimitive.mode == 4);

                Primitive primitive;
                // TODO: These are also in the position accessor
                primitive.min = glm::vec3(std::numeric_limits<float>::max());
                primitive.max = glm::vec3(std::numeric_limits<float>::min());
                primitive.positions = [&]{
                    const auto& attribute = gltfPrimitive.attributes.find("POSITION");
                    // All primitives should have position data
                    assert(attribute != gltfPrimitive.attributes.end());

                    const auto& accessor = gltfModel.accessors[attribute->second];
                    const auto& view = gltfModel.bufferViews[accessor.bufferView];
                    const uint8_t* data = gltfModel.buffers[view.buffer].data.data();

                    const size_t start = accessor.byteOffset + view.byteOffset;
                    const size_t dataSize = accessor.count * 3;

                    std::vector<glm::vec3> positions;
                    for (size_t i = 0; i < dataSize - 2; i += 3) {
                        positions.emplace_back(glm::make_vec3(
                            &reinterpret_cast<const float*>(&data[start])[i]
                        ));
                        primitive.min = glm::min(primitive.min, positions.back());
                        primitive.max = glm::max(primitive.max, positions.back());
                    }

                    return positions;
                }();
                primitive.normals = [&]{
                    const auto& attribute = gltfPrimitive.attributes.find("NORMAL");
                    // We might not have normals
                    if (attribute == gltfPrimitive.attributes.end())
                        return std::vector<glm::vec3>();

                    const auto& accessor = gltfModel.accessors[attribute->second];
                    const auto& view = gltfModel.bufferViews[accessor.bufferView];
                    const uint8_t* data = gltfModel.buffers[view.buffer].data.data();

                    const size_t start = accessor.byteOffset + view.byteOffset;
                    const size_t dataSize = accessor.count * 3;

                    // Normals should already be normalized
                    std::vector<glm::vec3> normals;
                    for (size_t i = 0; i < dataSize - 2; i += 3) {
                        normals.emplace_back(glm::make_vec3(
                            &reinterpret_cast<const float*>(&data[start])[i]
                        ));
                    }

                    return normals;
                }();
                primitive.tangents = [&]{
                    const auto& attribute = gltfPrimitive.attributes.find("TANGENT");
                    // We might not have tangents
                    if (attribute == gltfPrimitive.attributes.end())
                        return std::vector<glm::vec4>();

                    const auto& accessor = gltfModel.accessors[attribute->second];
                    const auto& view = gltfModel.bufferViews[accessor.bufferView];
                    const uint8_t* data = gltfModel.buffers[view.buffer].data.data();

                    const size_t start = accessor.byteOffset + view.byteOffset;
                    const size_t dataSize = accessor.count * 4;

                    std::vector<glm::vec4> tangents;
                    for (size_t i = 0; i < dataSize - 3; i += 4) {
                        tangents.emplace_back(glm::make_vec4(
                            &reinterpret_cast<const float*>(&data[start])[i]
                        ));
                    }

                    return tangents;
                }();
                primitive.texCoord0s = [&]{
                    const auto& attribute = gltfPrimitive.attributes.find("TEXCOORD_0");
                    // We might not have texCoord0s
                    if (attribute == gltfPrimitive.attributes.end())
                        return std::vector<glm::vec2>();

                    const auto& accessor = gltfModel.accessors[attribute->second];
                    const auto& view = gltfModel.bufferViews[accessor.bufferView];
                    const uint8_t* data = gltfModel.buffers[view.buffer].data.data();

                    const size_t start = accessor.byteOffset + view.byteOffset;
                    const size_t dataSize = accessor.count * 2;

                    std::vector<glm::vec2> texCoord0s;
                    for (size_t i = 0; i < dataSize - 1; i += 2) {
                        texCoord0s.emplace_back(glm::make_vec2(
                            &reinterpret_cast<const float*>(&data[start])[i]
                        ));
                    }

                    return texCoord0s;
                }();

                primitive.tris = [&] {
                    assert(gltfPrimitive.indices > -1);

                    const auto& accessor = gltfModel.accessors[gltfPrimitive.indices];
                    const auto& view = gltfModel.bufferViews[accessor.bufferView];
                    const uint8_t* data = gltfModel.buffers[view.buffer].data.data();

                    const size_t start = accessor.byteOffset + view.byteOffset;

                    std::vector<uint32_t> is;
                    if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_INT) {
                        is = std::vector(
                            reinterpret_cast<const uint32_t*>(&data[start]),
                            reinterpret_cast<const uint32_t*>(&data[start]) + accessor.count
                        );
                    } else if (accessor.componentType == TINYGLTF_COMPONENT_TYPE_UNSIGNED_SHORT) {
                        is.resize(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i)
                            is[i] = reinterpret_cast<const uint16_t*>(&data[start])[i];
                    } else {
                        is.resize(accessor.count);
                        for (size_t i = 0; i < accessor.count; ++i)
                            is[i] = reinterpret_cast<const uint8_t*>(&data[start])[i];
                    }

                    std::vector<TriIndices> tris;
                    for (size_t i = 0; i < is.size() - 2; i += 3)
                        tris.emplace_back(is[i], is[i + 1], is[i + 2]);

                    return tris;
                }();

                assert(gltfPrimitive.material != -1);

                primitive.material = &materials[gltfPrimitive.material];

                mesh.primitives.push_back(std::move(primitive));
            }

            mesh.min = glm::vec3(std::numeric_limits<float>::max());
            mesh.max = glm::vec3(std::numeric_limits<float>::min());
            for (const auto& primitive : mesh.primitives) {
                mesh.min = glm::min(mesh.min, primitive.min);
                mesh.max = glm::max(mesh.max, primitive.max);
            }

            meshes.push_back(std::move(mesh));
        }

        return meshes;
    }

    std::vector<Scene::Node> loadNodes(const tinygltf::Model& gltfModel, const std::vector<Mesh>& meshes)
    {
        // TODO: More complex nodes
        std::vector<Scene::Node> nodes(gltfModel.nodes.size());
        for (size_t n = 0; n < gltfModel.nodes.size(); ++n) {
            const auto& gltfNode = gltfModel.nodes[n];
            std::transform(
                gltfNode.children.begin(),
                gltfNode.children.end(),
                std::back_inserter(nodes[n].children),
                [&](int i){ return &nodes[i]; }
            );
            if (gltfNode.mesh > -1)
                nodes[n].mesh = &meshes[gltfNode.mesh];
            if (gltfNode.matrix.size() == 16) {
                // Spec defines the matrix to be decomposeable to T * R * S
                const glm::mat4 matrix = glm::make_mat4(gltfNode.matrix.data());
                glm::vec3 skew;
                glm::vec4 perspective;
                glm::decompose(
                    matrix,
                    nodes[n].scale,
                    nodes[n].rotation,
                    nodes[n].translation,
                    skew,
                    perspective
                );
            }
            if (gltfNode.translation.size() == 3)
                nodes[n].translation = glm::make_vec3(gltfNode.translation.data());
            if (gltfNode.rotation.size() == 4)
                nodes[n].rotation = glm::make_quat(gltfNode.rotation.data());
            if (gltfNode.scale.size() == 3)
                nodes[n].scale = glm::make_vec3(gltfNode.scale.data());
        }

        return nodes;
    }

    std::tuple<std::vector<Scene>, size_t> loadScenes(const tinygltf::Model& gltfModel, std::vector<Scene::Node>* nodes)
    {
        std::vector<Scene> scenes(gltfModel.scenes.size());
        for (size_t s = 0; s < gltfModel.scenes.size(); ++s) {
            const auto& gltfScene = gltfModel.scenes[s];
            std::transform(
                gltfScene.nodes.begin(),
                gltfScene.nodes.end(),
                std::back_inserter(scenes[s].nodes),
                [&](int i){ return &(*nodes)[i]; }
            );
        }

        const size_t currentScene = gltfModel.defaultScene > 0 ? gltfModel.defaultScene : 0;

        return std::make_tuple(scenes, currentScene);
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
            primitive.positions.emplace_back(v3);
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

    printf("%zu verts and %zu tris\n", primitive.positions.size(), primitive.tris.size());
    printf(
        "min (%.2f, %.2f, %.2f) max (%.2f, %.2f, %.2f)\n",
        primitive.min.x, primitive.min.y, primitive.min.z,
        primitive.max.x, primitive.max.y, primitive.max.z
    );

    return {primitive.min, primitive.max, {primitive}};
}

World loadGLTF(const std::string& path)
{
    const tinygltf::Model gltfModel = [&](){
        tinygltf::Model model;
        tinygltf::TinyGLTF loader;
        std::string warn;
        std::string err;

        const bool ret = loader.LoadASCIIFromFile(&model, &err, &warn, path);
        if (!warn.empty())
            throw std::runtime_error(warn);
        if (!err.empty())
            throw std::runtime_error(err);
        if (!ret)
            throw std::runtime_error("Parsing glTF failed");

        return model;
    }();

    World world;
    world.textures = loadTextures(gltfModel);
    world.materials = loadMaterials(gltfModel, world.textures);
    world.meshes = loadMeshes(gltfModel, world.materials);
    world.nodes = loadNodes(gltfModel, world.meshes);
    auto [scenes, currentScene] = loadScenes(gltfModel, &world.nodes);
    world.scenes = scenes;
    world.currentScene = currentScene;

    // TODO: Log resource counts per scene

    return world;
}
