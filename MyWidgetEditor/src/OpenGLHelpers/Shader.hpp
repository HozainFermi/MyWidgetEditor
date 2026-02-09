#include "../thirdparty/include/glad/gl.h"
#include <string>
#include <glm.hpp>

namespace Helpers {

    class Shader {
    public:
        GLuint program;

        Shader(const char* vertexSource, const char* fragmentSource);
        ~Shader();

        void Use() const;
        void setBool(const std::string& name, bool value) const;
        void setInt(const std::string& name, int value) const;
        void setFloat(const std::string& name, float value) const;
        void setVec2(const std::string& name, const glm::vec2& value) const;
        void setVec2(const std::string& name, float x, float y) const;
        void setVec3(const std::string& name, const glm::vec3& value) const;
        void setVec3(const std::string& name, float x, float y, float z) const;
        void setVec4(const std::string& name, const glm::vec4& value) const;
        void setVec4(const std::string& name, float x, float y, float z, float w) const;
        void setMat2(const std::string& name, const glm::mat2& mat) const;
        void setMat3(const std::string& name, const glm::mat3& mat) const;
        void setMat4(const std::string& name, const glm::mat4& mat) const;



       /* void setInt(const char* name, int value) const;
        void setFloat(const char* name, float value) const;
        void setVec2(const char* name, float x, float y) const;
        void setVec3(const char* name, float x, float y, float z) const;
        void setVec4(const char* name, float r, float g, float b, float a) const;*/

    private:
        GLuint compileShader(GLenum type, const char* source);
        GLuint linkProgram(GLuint vertexShader, GLuint fragmentShader);
};

}

