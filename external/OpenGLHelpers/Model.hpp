#pragma once
#include "Mesh.h"
#include <filesystem>
#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <string>
#include <assimp/postprocess.h>
#include <filesystem>
#include "Shader.hpp"

namespace Helpers {

    class Model
    {
    public:
        Model(std::filesystem::path path)
        {
            loadModel(path);
        }
        void Draw(Shader& shader);
    private:
        // model data
        vector<Texture> textures_loaded;
        vector<Mesh> meshes;
        string directory;

        void loadModel(std::filesystem::path path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
            string typeName);
        unsigned int TextureFromFile(std::string path, std::string directory);
    };
}
