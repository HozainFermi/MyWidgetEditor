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
        Model()
        {            
        }

        void Draw(Shader& shader);
        void AddMesh(Mesh& mesh);
    private:
        // model data
        std::vector<Texture> textures_loaded;
        std::vector<Mesh> meshes;
        std::string directory;

        void loadModel(std::filesystem::path path);
        void processNode(aiNode* node, const aiScene* scene);
        Mesh processMesh(aiMesh* mesh, const aiScene* scene);
        std::vector<Texture> loadMaterialTextures(aiMaterial* mat, aiTextureType type,
            std::string typeName);
        unsigned int TextureFromFile(std::string path, std::string directory);
    };
}
