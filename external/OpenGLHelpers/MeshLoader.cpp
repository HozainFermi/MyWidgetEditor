#include "MeshLoader.hpp"


namespace Helpers {

    MeshLoader::MeshLoader(std::span<float> verticies, short stride)
        : points(verticies.begin(), verticies.end()), stride(stride)
	{				
		glGenVertexArrays(1, &VAO);		
		glGenBuffers(1, &VBO);
		
        glBindVertexArray(VAO);  
        glBindBuffer(GL_ARRAY_BUFFER, VBO);  
        
        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(points[0]), points.data(), GL_STATIC_DRAW);

		//glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
		//glEnableVertexAttribArray(0);
	}

    MeshLoader::MeshLoader(unsigned int VBO, short stride, std::span<float> verticies)
        :VBO(VBO), 
        stride(stride),
        points(verticies.begin(), verticies.end())
    {
        glGenVertexArrays(1, &VAO);        

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
    }

    MeshLoader::MeshLoader(unsigned int VBO, short stride, std::span<float> verticies, std::span<float> inds)
        :VBO(VBO),
        stride(stride),
        points(verticies.begin(), verticies.end()),
        indices(inds.begin(), inds.end())
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);
    }

    MeshLoader::MeshLoader(std::span<float> verticies, std::span<int> inds)
        : points(verticies.begin(), verticies.end()),
          indices(inds.begin(), inds.end())
    {
        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);
        glGenBuffers(1, &EBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(points[0]), points.data(), GL_STATIC_DRAW);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size()*sizeof(indices[0]), indices.data(), GL_STATIC_DRAW);

       //glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
       //glEnableVertexAttribArray(0);
    }

	MeshLoader::MeshLoader(std::string& filepath)
	{
        std::ifstream file(filepath);
        if (!file.is_open()) {
            throw std::runtime_error("Cannot open file: " + filepath);
        }

        std::string line;
        while (std::getline(file, line)) {
            // Проверяем, что строка не пустая и не содержит только пробелы
            if (line.empty() || std::all_of(line.begin(), line.end(), ::isspace)) {
                continue; // Пропускаем пустые строки
            }

            std::istringstream iss(line);
            std::string token;

            while (std::getline(iss, token, ',')) {
                try {
                    // Удаляем пробелы вокруг числа
                    token.erase(std::remove_if(token.begin(), token.end(), ::isspace), token.end());

                    if (!token.empty()) {
                        float value = std::stof(token);
                        points.push_back(value);
                    }
                }
                catch (const std::exception& e) {
                    std::cerr << "Warning: Cannot parse float from token: " << token
                        << " in line: " << line << std::endl;
                    // Можно выбрасывать исключение или игнорировать
                }
            }
        }

        file.close();

        glGenVertexArrays(1, &VAO);
        glGenBuffers(1, &VBO);

        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glBufferData(GL_ARRAY_BUFFER, points.size() * sizeof(points[0]), points.data(), GL_STATIC_DRAW);
       // glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), nullptr);
       // glEnableVertexAttribArray(0);

	}
	MeshLoader::MeshLoader()
	{

	}
	MeshLoader::~MeshLoader()
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}
    void MeshLoader::AddAttribPointer(GLint size, GLsizei stride, const void* offset)
    {
        glBindVertexArray(VAO);
        glBindBuffer(GL_ARRAY_BUFFER, VBO);

        glVertexAttribPointer(++attribCount, size, GL_FLOAT, GL_FALSE, stride, offset);
        glEnableVertexAttribArray(attribCount);

    }
    void MeshLoader::Draw()
	{
        glBindVertexArray(VAO);
        
        if (EBO == 0) {
		glDrawArrays(GL_TRIANGLES, 0, points.size()/stride);
        }
        else {
        glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
        }		             
	}
}