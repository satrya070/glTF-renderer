#ifndef SHADER_H
#define SHADER_H

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include <glad/glad.h>

class Shader
{
public:
	unsigned int ID;
	//Constructor generates the shader on the fly
	Shader(const GLchar* vertexPath, const GLchar* fragmentPath, const GLchar* tessControlPath, const GLchar* tessEvalPath)
	{
		// 1. retrieve the vertex/fragment source code from filepath
		std::string vertexCode;
		std::string fragmentCode;
		std::string tessControlCode;
		std::string tessEvalCode;


		std::ifstream vShaderFile;
		std::ifstream fShaderFile;
		std::ifstream tcShaderFile;
		std::ifstream teShaderFile;
		// ensures ifstream objects can throw exceptions
		vShaderFile.exceptions(std::ifstream::badbit);
		fShaderFile.exceptions(std::ifstream::badbit);
		tcShaderFile.exceptions(std::ifstream::badbit);
		teShaderFile.exceptions(std::ifstream::badbit);

		try
		{
			//open files
			vShaderFile.open(vertexPath);
			fShaderFile.open(fragmentPath);
			tcShaderFile.open(tessControlPath);
			teShaderFile.open(tessEvalPath);
			std::stringstream vShaderStream, fShaderStream, tcShaderStream, teShaderStream;
			// read file buffers contents into stream
			vShaderStream << vShaderFile.rdbuf();
			fShaderStream << fShaderFile.rdbuf();
			tcShaderStream << tcShaderFile.rdbuf();
			teShaderStream << teShaderFile.rdbuf();
			// close file handlers
			vShaderFile.close();
			fShaderFile.close();
			tcShaderFile.close();
			teShaderFile.close();
			// convert stream into string
			vertexCode = vShaderStream.str();
			fragmentCode = fShaderStream.str();
			tessControlCode = tcShaderStream.str();
			tessEvalCode = teShaderStream.str();
		}
		catch (std::ifstream::failure e)
		{
			std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
		}
		
		const GLchar* vShaderCode = vertexCode.c_str();
		const GLchar* fShaderCode = fragmentCode.c_str();
		const GLchar* tcShaderCode = tessControlCode.c_str();
		const GLchar* teShaderCode = tessEvalCode.c_str();
		
		// 2. Compile Shaders
		GLuint vertex, fragment, tessControl, tessEvaluation;
		GLint success;
		GLchar infoLog[512];
		// vertex shader
		vertex = glCreateShader(GL_VERTEX_SHADER);
		glShaderSource(vertex, 1, &vShaderCode, NULL);
		glCompileShader(vertex);
		// print compile errors if any
		glGetShaderiv(vertex, GL_COMPILE_STATUS, &success);
		if (!success)
		{
			glGetShaderInfoLog(vertex, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Fragment Shader
		fragment = glCreateShader(GL_FRAGMENT_SHADER);
		glShaderSource(fragment, 1, &fShaderCode, NULL);
		glCompileShader(fragment);
		glGetShaderiv(fragment, GL_COMPILE_STATUS, &success);
		// print compile error if any
		if (!success)
		{
			glGetShaderInfoLog(fragment, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
		}

		// Tesselation Control Shader
		tessControl = glCreateShader(GL_TESS_CONTROL_SHADER);
		glShaderSource(tessControl, 1, &tcShaderCode, NULL);
		glCompileShader(tessControl);
		glGetShaderiv(tessControl, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(tessControl, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESSELATION::CONTROL::COMPILATION_FAILED:\n" << infoLog << std::endl;
		}

		// tesselation evaluation shader
		tessEvaluation = glCreateShader(GL_TESS_EVALUATION_SHADER);
		glShaderSource(tessEvaluation, 1, &teShaderCode, NULL);
		glCompileShader(tessEvaluation);
		glGetShaderiv(tessEvaluation, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(tessEvaluation, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::TESSELATION::EVALUATION::COMPILATION_FAILED:\n" << infoLog << std::endl;
		}
		
		// Shader Program
		ID = glCreateProgram();
		glAttachShader(ID, vertex);
		glAttachShader(ID, fragment);
		glAttachShader(ID, tessControl);
		glAttachShader(ID, tessEvaluation);
		glLinkProgram(ID);
		// print linking errors if any
		glGetProgramiv(ID, GL_LINK_STATUS, &success);
		if (!success)
		{
			glGetProgramInfoLog(ID, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n" << infoLog << std::endl;
		}
		// Delete the shaders as they're linked into our program now and no longer necessary
		glDeleteShader(vertex);
		glDeleteShader(fragment);
		glDeleteShader(tessControl);
		glDeleteShader(tessEvaluation);
	}

	// uses
	void use()
	{
		glUseProgram(ID);
	}

	void setVec3(const std::string& name, const glm::vec3& value) const
	{
		glUniform3fv(glGetUniformLocation(ID, name.c_str()), 1, &value[0]);
	}
	void setVec3(const std::string& name, float x, float y, float z) const
	{
		glUniform3f(glGetUniformLocation(ID, name.c_str()), x, y, z);
	}
	void setMat4(const std::string name, const glm::mat4& mat) const
	{
		glUniformMatrix4fv(glGetUniformLocation(ID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
	}

	void setFloat(const std::string name, const float value) const
	{
		glUniform1f(glGetUniformLocation(ID, name.c_str()), value);
	}

	void setInt(const std::string& name, int value) const
	{
		glUniform1i(glGetUniformLocation(ID, name.c_str()), value);
	}
};

#endif