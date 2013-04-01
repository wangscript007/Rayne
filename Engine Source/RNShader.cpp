//
//  RNShader.cpp
//  Rayne
//
//  Copyright 2013 by Überpixel. All rights reserved.
//  Unauthorized use is punishable by torture, mutilation, and vivisection.
//

#include "RNShader.h"
#include "RNKernel.h"
#include "RNPathManager.h"

namespace RN
{
	RNDeclareMeta(Shader)
	
	Shader::Shader()
	{
		_supportedPrograms = 0;
		AddDefines();
	}
	
	Shader::Shader(const std::string& shader)
	{
		_supportedPrograms = 0;
		AddDefines();
		
		SetVertexShader(PathManager::PathForName(shader + ".vsh"));
		SetFragmentShader(PathManager::PathForName(shader + ".fsh"));
		
#if GL_GEOMETRY_SHADER
		std::string path = PathManager::PathForName(shader + ".gsh");
		if(path.length() > 0)
			SetGeometryShader(path);
#endif
		
		ProgramOfType(ShaderProgram::TypeNormal);
	}
	
	Shader::~Shader()
	{
		for(auto i=_programs.begin(); i!=_programs.end(); i++)
		{
			ShaderProgram *program = i->second;
			
			glDeleteProgram(program->program);
			delete program;
		}
	}
	
	Shader *Shader::WithFile(const std::string& file)
	{
		Shader *shader = new Shader(file);
		return shader->Autorelease();
	}
	
	
	// ---------------------
	// MARK: -
	// MARK: Defines
	// ---------------------
	
	void Shader::AddDefines()
	{
#ifdef DEBUG
		Define("DEBUG", 1);
#endif
#ifdef NDEBUG
		Define("NDEBUG", 1);
#endif
		
#if RN_TARGET_OPENGL
		Define("OPENGL", 1);
#endif
#if RN_TARGET_OPENGL_ES
		Define("OPENGLES", 1);
#endif
	}
	
	void Shader::Define(const std::string& define)
	{
		ShaderDefine def;
		def.name = define;
		def.value = "";
		
		_defines.AddObject(def);
	}
	
	void Shader::Define(const std::string& define, const std::string& value)
	{
		ShaderDefine def;
		def.name = define;
		def.value = value;
		
		_defines.AddObject(def);
	}
	
	void Shader::Define(const std::string& define, int32 value)
	{
		char buffer[32];
		sprintf(buffer, "%i", value);
		
		Define(define, buffer);
	}
	
	void Shader::Define(const std::string& define, float value)
	{
		char buffer[32];
		sprintf(buffer, "%f", value);
		
		Define(define, buffer);
	}	
	
	void Shader::Undefine(const std::string& name)
	{
		for(machine_uint i=0; i<_defines.Count(); i++)
		{
			ShaderDefine& define = _defines[(int)i];
			if(define.name == name)
			{
				_defines.RemoveObjectAtIndex(i);
				return;
			}
		}
	}
	
	// ---------------------
	// MARK: -
	// MARK: Program creation
	// ---------------------
	
	std::string Shader::PreProcessedShaderSource(const std::string& source)
	{
		std::string data = source;
		size_t index = data.find("#version");
		
		if(index != std::string::npos)
		{
			index += 8;
			
			do {
				index ++;
			} while(data[index] != '\n');
			
			index ++;
		}
		else
		{
			index = 0;
		}
		
		for(machine_uint i=0; i<_defines.Count(); i++)
		{
			ShaderDefine& define = _defines[(int)i];
			std::string exploded = "#define " + define.name + " " + define.value + "\n";
			
			data.insert(index, exploded);
			index += exploded.length();
		}
		
		return data;
	}
	
	void Shader::CompileShader(GLenum type, GLuint *outShader)
	{
		GLuint shader = 0;
		std::string source;
		
		switch(type)
		{
			case GL_VERTEX_SHADER:
				Define("RN_VERTEXSHADER", 1);
				source = PreProcessedShaderSource(_vertexShader);
				Undefine("RN_VERTEXSHADER");
				break;
				
			case GL_FRAGMENT_SHADER:
				Define("RN_FRAGMENTSHADER", 1);
				source = PreProcessedShaderSource(_fragmentShader);
				Undefine("RN_FRAGMENTSHADER");
				break;
				
#ifdef GL_GEOMETRY_SHADER
			case GL_GEOMETRY_SHADER:
				Define("RN_GEOMETRYSHADER", 1);
				source = PreProcessedShaderSource(_geometryShader);
				Undefine("RN_GEOMETRYSHADER");
				break;
#endif
		}
		
		const GLchar *data = source.c_str();
		
		shader = glCreateShader(type);
		
		glShaderSource(shader, 1, &data, NULL);
		glCompileShader(shader);
		
		*outShader = shader;
		
		// Check the compilation status of the shader
		GLint status, length;
		glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
		if(status == GL_FALSE)
		{
			glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
			
			GLchar *log = new GLchar[length];
			glGetShaderInfoLog(shader, length, &length, log);
			glDeleteShader(shader);
			
			*outShader = 0;
			
			std::string compilerLog = std::string((char *)log) + "\nShader source:\n" + source;
			delete [] log;
			
			std::string tlog;
			
			switch(type)
			{
				case GL_VERTEX_SHADER:
					tlog = "Failed to compile " + _vertexFile + ".\n";
					break;
					
				case GL_FRAGMENT_SHADER:
					tlog = "Failed to compile " + _fragmentFile + ".\n";
					break;
					
#ifdef GL_GEOMETRY_SHADER
				case GL_GEOMETRY_SHADER:
					tlog = "Failed to compile " + _geometryFile + ".\n";
					break;
#endif
			}
			
			throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderCompilingFailed, tlog, compilerLog);
		}
	}
	
	ShaderProgram *Shader::ProgramOfType(uint32 type)
	{
		if(!SupportsProgramOfType(type))
			return 0;
		
		ShaderProgram *program = _programs[type];
		if(!program)
		{
			GLuint shader[3] = {0};
			
			program = new ShaderProgram;
			program->program = glCreateProgram();
			
			if(program->program == 0)
			{
				RN_CHECKOPENGL();
				
				delete program;
				_programs[type] = 0;
				
				throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderLinkingFailed, "Couldn't create program");
			}
			
			_programs[type] = program;
			
			// Prepare the state
			if(type & ShaderProgram::TypeInstanced)
				Define("RN_INSTANCING");
			
			if(type & ShaderProgram::TypeAnimated)
				Define("RN_ANIMATION");
			
			if(type & ShaderProgram::TypeLightning)
				Define("RN_LIGHTNING");
			
			// Compile all required shaders
			try
			{
				if(_vertexShader.length() > 0)
				{
					CompileShader(GL_VERTEX_SHADER, &shader[0]);
					glAttachShader(program->program, shader[0]);
				}
				
				if(_fragmentShader.length() > 0)
				{
					CompileShader(GL_FRAGMENT_SHADER, &shader[1]);
					glAttachShader(program->program, shader[1]);
				}
			
#ifdef GL_GEOMETRY_SHADER
				if(_geometryShader.length() > 0)
				{
					CompileShader(GL_GEOMETRY_SHADER, &shader[2]);
					glAttachShader(program->program, shader[2]);
				}
#endif
			}
			catch(ErrorException e)
			{
				Undefine("RN_INSTANCING");
				Undefine("RN_ANIMATION");
				Undefine("RN_LIGHTNING");
				
				delete program;
				_programs[type] = 0;
				
				throw e;
			}
			
			// Link the program
			glLinkProgram(program->program);
			RN_CHECKOPENGL();
			
			// Clean up
			Undefine("RN_INSTANCING");
			Undefine("RN_ANIMATION");
			Undefine("RN_LIGHTNING");
			
			for(int i=0; i<3; i++)
			{
				if(shader[i] != 0)
				{
					glDetachShader(program->program, shader[i]);
					glDeleteShader(shader[i]);
				}
			}
			
			
			// Check the programs linking status
			GLint status, length;
			
			glGetProgramiv(program->program, GL_LINK_STATUS, &status);
			if(status == GL_FALSE)
			{
				glGetProgramiv(program->program, GL_INFO_LOG_LENGTH, &length);
				
				GLchar *log = new GLchar[length];
				glGetProgramInfoLog(program->program, length, &length, log);
				glDeleteProgram(program->program);
				
				delete program;
				_programs[type] = 0;
				
				std::string tlog = std::string((char *)log);
				switch(type)
				{
					case ShaderProgram::TypeNormal:
						tlog += "\nType: Normal";
						break;
						
					case ShaderProgram::TypeInstanced:
						tlog += "\nType: Instancing";
						break;
						
					default:
						break;
				}
				
				if(type > 0)
				{
					tlog += "\nType:";
					
					if(type & ShaderProgram::TypeInstanced)
						tlog += " Instancing";
					
					if(type & ShaderProgram::TypeAnimated)
						tlog += " Animation";
					
					if(type & ShaderProgram::TypeLightning)
						tlog += " Lightning";
				}
				
				if(_vertexFile.length() > 0)
					tlog += "\nVertex Shader: " + _vertexFile;
				
				if(_fragmentFile.length() > 0)
					tlog += "\nFragment Shader: " + _fragmentFile;
				
				if(_geometryFile.length() > 0)
					tlog += "\nGeometry Shader: " + _geometryFile;
				
				delete [] log;
				
				throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderLinkingFailed, tlog);
			}
			else
			{
				
#define GetUniformLocation(uniform) program->uniform = glGetUniformLocation(program->program, #uniform)
#define GetAttributeLocation(attribute) program->attribute = glGetAttribLocation(program->program, #attribute)
				
				// Get uniforms
				GetUniformLocation(matProj);
				GetUniformLocation(matProjInverse);
				
				GetUniformLocation(matView);
				GetUniformLocation(matViewInverse);
				
				GetUniformLocation(matModel);
				GetUniformLocation(matModelInverse);
				
				GetUniformLocation(matViewModel);
				GetUniformLocation(matViewModelInverse);
				
				GetUniformLocation(matProjView);
				GetUniformLocation(matProjViewInverse);
				
				GetUniformLocation(matProjViewModel);
				GetUniformLocation(matProjViewModelInverse);
				
				GetUniformLocation(matBones);
				
				GetUniformLocation(time);
				GetUniformLocation(frameSize);
				GetUniformLocation(clipPlanes);
				GetUniformLocation(discardThreshold);
				
				GetUniformLocation(viewPosition);
				
				GetUniformLocation(lightPointCount);
				GetUniformLocation(lightPointList);
				GetUniformLocation(lightPointListOffset);
				GetUniformLocation(lightPointListData);
				
				GetUniformLocation(lightSpotCount);
				GetUniformLocation(lightSpotList);
				GetUniformLocation(lightSpotListOffset);
				GetUniformLocation(lightSpotListData);
				
				GetUniformLocation(lightDirectionalDirection);
				GetUniformLocation(lightDirectionalColor);
				GetUniformLocation(lightDirectionalCount);
				
				GetUniformLocation(lightTileSize);
				
				char string[32];
				for(machine_uint i=0; ; i++)
				{
					sprintf(string, "targetmap%i", (int)i);
					GLuint location = glGetUniformLocation(program->program, string);
					
					if(location == -1)
						break;
					
					program->targetmaplocations.AddObject(location);
				}
				
				for(machine_uint i=0; ; i++)
				{
					sprintf(string, "mTexture%i", (int)i);
					GLuint location = glGetUniformLocation(program->program, string);
					
					if(location == -1)
						break;
					
					program->texlocations.AddObject(location);
				}
				
				GetUniformLocation(depthmap);
				
				// Get attributes
				GetAttributeLocation(imatModel);
				GetAttributeLocation(imatModelInverse);
				
				GetAttributeLocation(vertPosition);
				GetAttributeLocation(vertNormal);
				GetAttributeLocation(vertTangent);
				
				GetAttributeLocation(vertTexcoord0);
				GetAttributeLocation(vertTexcoord1);
				
				GetAttributeLocation(vertColor0);
				GetAttributeLocation(vertColor1);

				GetAttributeLocation(vertBoneWeights);
				GetAttributeLocation(vertBoneIndices);
				
#if RN_PLATFORM_MAC_OS || RN_PLATFORM_WINDOWS
				do
				{
					GLint maxDrawbuffers;
					glGetIntegerv(GL_MAX_DRAW_BUFFERS, &maxDrawbuffers);
					
					for(GLint i=0; i<maxDrawbuffers; i++)
					{
						sprintf(string, "fragColor%i", i);
						GLint location = glGetFragDataLocation(program->program, string);
						
						if(location == -1)
							break;
						
						program->fraglocations.AddObject(location);
					}
				} while(0);
#endif
				
#undef GetUniformLocation
#undef GetAttributeLocation
				
				RN_CHECKOPENGL();
				glFlush();
			}
		}
		
		return program;
	}
	
	bool Shader::SupportsProgramOfType(uint32 type)
	{
		return (_supportedPrograms & type || type == 0);
	}
	
	// ---------------------
	// MARK: -
	// MARK: Setter
	// ---------------------
	
	void Shader::SetVertexShader(const std::string& path)
	{
		SetShaderForType(path, GL_VERTEX_SHADER);
	}
	
	void Shader::SetVertexShader(File *file)
	{
		SetShaderForType(file, GL_VERTEX_SHADER);
	}
	
	void Shader::SetFragmentShader(const std::string& path)
	{
		SetShaderForType(path, GL_FRAGMENT_SHADER);
	}
	
	void Shader::SetFragmentShader(File *file)
	{
		SetShaderForType(file, GL_FRAGMENT_SHADER);
	}
	
	void Shader::SetGeometryShader(const std::string& path)
	{
#ifdef GL_GEOMETRY_SHADER
		SetShaderForType(path, GL_GEOMETRY_SHADER);
#else
		throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderTypeNotSupported);
#endif
	}
	
	void Shader::SetGeometryShader(File *file)
	{
#ifdef GL_GEOMETRY_SHADER
		SetShaderForType(file, GL_GEOMETRY_SHADER);
#else
		throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderTypeNotSupported);
#endif
	}
	
	void Shader::SetShaderForType(File *file, GLenum type)
	{		
		// Preprocess the shader
		std::string data = file->String();
		size_t index = 0;
		
		// Search for includes files
		while((index = data.find("#include \"", index)) != std::string::npos)
		{
			std::string::iterator iterator = data.begin() + (index + 10);
			size_t length = 0;
			while(*iterator != '"')
			{
				length ++;
				iterator ++;
			}
			
			std::string name = data.substr(index + 10, length);
			data.erase(index, length + 11);
			
			try
			{
				File *includeFile = new File(PathManager::Join(file->Path(), name));
				data.insert(index, includeFile->String());
				includeFile->Release();
			}
			catch(ErrorException e)
			{
				throw ErrorException(e.Error(), "Failed to preprocess " + file->Name() + "." + file->Extension() + "\n\n" + e.Description());
			}
		}
		
		// Check what program types the shader supports
		_supportedPrograms |= (data.find("#ifdef RN_INSTANCING") != std::string::npos) ? (ShaderProgram::TypeInstanced) : 0;
		_supportedPrograms |= (data.find("#ifdef RN_ANIMATION") != std::string::npos) ? (ShaderProgram::TypeAnimated) : 0;
		_supportedPrograms |= (data.find("#ifdef RN_LIGHTNING") != std::string::npos) ? (ShaderProgram::TypeLightning) : 0;
		_supportedPrograms |= (data.find("#ifdef RN_DISCARD") != std::string::npos) ? (ShaderProgram::TypeDiscard) : 0;
		
		switch(type)
		{
			case GL_VERTEX_SHADER:
				_vertexShader = data;
				_vertexFile   = file->Name() + "." + file->Extension();
				break;
				
			case GL_FRAGMENT_SHADER:
				_fragmentShader = data;
				_fragmentFile   = file->Name() + "." + file->Extension();
				break;
				
#ifdef GL_GEOMETRY_SHADER
			case GL_GEOMETRY_SHADER:
				_geometryShader = data;
				_geometryFile   = file->Name() + "." + file->Extension();
				break;
#endif
				
			default:
				throw ErrorException(kErrorGroupGraphics, 0, kGraphicsShaderTypeNotSupported);
				break;
		}
	}
	
	void Shader::SetShaderForType(const std::string& path, GLenum type)
	{
		File *file = new File(path);
		SetShaderForType(file, type);
		file->Release();
	}
}
