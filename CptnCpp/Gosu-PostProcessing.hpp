#ifndef GOSU_EXT_POSTPROCESSING_HPP
#define GOSU_EXT_POSTPROCESSING_HPP

#include <Gosu/Math.hpp>
#include <Gosu/IO.hpp>
#include <Gosu/Graphics.hpp>
#include <Gosu/Directories.hpp>

/* Include glew and add the lib */
#include <GL/glew.h>
#ifdef _MSC_VER
	#pragma comment(lib, "glew32.lib")
#endif

#include <vector>
#include <map>
#include <iostream>

namespace Gosu
{
	/* Gets real screen width from the graphics */
	inline unsigned int realWidth(Graphics &graphics)
	{
		return round(graphics.width() * graphics.factorX());
	}

	/* Gets real screen height from the graphics */
	inline unsigned int realHeight(Graphics &graphics)
	{
		return round(graphics.height() * graphics.factorY());
	}


	/* Holds opengl ids to shader program, vertex and fragment shader */
	struct ShaderProgram
	{
		GLint program, vertex, fragment;
	};

	struct Kernel3x3
	{
		float texCoordOffsets[18];
	};

	/* Handles custom fragment shader and multiple-pass rendering 
	 * PostProcessing only uses fragment shaders, vertex shaders are not much use 
	 * The class can handle to run multiple shaders per draw(), but for each shader-run the frame buffer gets copied to texture */
	// Not true anymore, this class can handly vertex shader aswell, if the user wishes to do so.
	class PostProcessing
	{
		private:
			Graphics *graphics;

		public:
			PostProcessing(Graphics &graphics)
			{
				/* Set our local graphics reference to passed argument */
				this->graphics = &graphics;

				static bool glewInitialized = false;

				/* Is glew already initialized? */
				if(!glewInitialized)
				{
					/* Init glew */
					glewInit();

					glewInitialized = true;
				}

				/* Check if shaders are supported by GPU */
				if (!available())
					return;

				/* Init the texture */
				internalTexture(graphics);
			}

			static GLuint internalTexture(Graphics& graphics)
			{
				static bool initialized = false;
				static GLuint texture = -1;
				static int textureWidth, textureHeight;

				int width = realWidth(graphics);
				int height = realHeight(graphics);

				/* Is the texture uninitialized or did the screen size change? */
				if (!initialized || textureWidth != width || textureHeight != height)
				{
					/* If texture already exists, delete it first */
					if(texture != -1)
						glDeleteTextures(1, &texture);

					/* Create a texture of screen size */
					glGenTextures(1, &texture);
					glBindTexture(GL_TEXTURE_2D, texture);
					glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
					glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
					glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB8, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0);
					glBindTexture(GL_TEXTURE_2D, 0);

					/* Save the width and height */
					textureWidth = width;
					textureHeight = height;

					initialized = true;
				}

				return texture;
			}

			void /*Kernel3x3*/ kernelTextureCoordOffsets(ShaderProgram program, char *uniform)
			{
				Kernel3x3 texKernel;
				GLint i, j;
				GLfloat xInc, yInc;

				xInc = 1.0f / (float)realWidth(*this->graphics);
				yInc = 1.0f / (float)realHeight(*this->graphics);

				for (i = 0; i < 3; i++)
				{
					for (j = 0; j < 3; j++)
					{
						texKernel.texCoordOffsets[(((i*3)+j)*2)+0] = (-1.0f * xInc) + ((GLfloat)i * xInc);
						texKernel.texCoordOffsets[(((i*3)+j)*2)+1] = (-1.0f * yInc) + ((GLfloat)j * yInc);
					}
				}

				glUseProgram(program.program);
				glUniform2fv(glGetUniformLocation(program.program, uniform), 9, texKernel.texCoordOffsets);
				glUseProgram(0);

				//return texKernel;
			}

			/* Check if shaders are supported by GPU */
			bool available()
			{
				if(glewIsSupported("GL_VERSION_2_0")) return true;
				return false;
			}

			std::string loadShaderFile(std::wstring file) // this is not even working, I dont get gosu buffers, wstrings or anything related to gosu reading/writing files
			{
				Buffer buf;
				loadFile(buf, file);
				std::string data(reinterpret_cast<char*>(buf.data()), buf.size());
				//const GLchar *test = data.c_str();
				//std::cout << test << std::endl;
				return data;
			}

			/* Check if shader is already compiled and returns pointer to compiled ShaderProgram */
			ShaderProgram compile(std::wstring fsfile, std::wstring vsfile = L"")
			{
				/* Map that stores compiled shader programs */
				static std::map<std::wstring, ShaderProgram> shaders;
				if(shaders.find(fsfile) == shaders.end())
				{
					/* Load the fragment shader file, the ugly way */
					//const GLchar *fragmentShader = loadShaderFile(fsfile).c_str();
					Buffer buf;
					loadFile(buf, fsfile);
					std::string data(reinterpret_cast<char*>(buf.data()), buf.size());
					const GLchar *fragmentShader = data.c_str();


					/* Create shader program */
					ShaderProgram program;
					program.program = glCreateProgram();

					/* 'Fixed functionality' vertex shader, this is hard coded to simplify post processing */
					// Think about why this is hardcoded. Would it make more sense to actually ship a fixedfunc.vs?
					// Could be usefull for people. Or not. 
					const GLchar* ffVertexShader = "void main(void)\r\n{\r\ngl_Position = ftransform();\r\ngl_TexCoord[0] = gl_MultiTexCoord0;\r\n}\r\n";

					int success;
					/* Attach vertex shader */
					program.vertex = glCreateShader(GL_VERTEX_SHADER);
					if(vsfile != L"")
					{
						Buffer buf1;
						loadFile(buf1, vsfile);
						std::string data1(reinterpret_cast<char*>(buf1.data()), buf1.size());
						const GLchar* vertexShader = data1.c_str();
						glShaderSource(program.vertex, 1, &vertexShader, NULL);
					}else{
						glShaderSource(program.vertex, 1, &ffVertexShader, NULL);
					}
					glCompileShader(program.vertex);
					glGetShaderiv(program.vertex, GL_COMPILE_STATUS, &success);
					if(!success)
					{
						std::cout << "vertex shader error" << std::endl;
						GLchar log[32768];
						glGetShaderInfoLog(program.vertex, 32768, NULL, log);
						std::cout << log << std::endl;
						//throw std::runtime_error(log);
					}
					glAttachShader(program.program, program.vertex);

					/* Attach fragment shader */
					program.fragment = glCreateShader(GL_FRAGMENT_SHADER);
					glShaderSource(program.fragment, 1, &fragmentShader , NULL);
					glCompileShader(program.fragment);					
					glGetShaderiv(program.fragment, GL_COMPILE_STATUS, &success);
					if(!success)
					{
						std::cout << "fragment shader error" << std::endl;
						GLchar log[32768];
						glGetShaderInfoLog(program.fragment, 32768, NULL, log);
						std::cout << log << std::endl;
						//throw std::runtime_error(log);
					}
					glAttachShader(program.program, program.fragment);

					/* Compile shader program */
					glLinkProgram(program.program);

					/* Add compiled program to our shader map */
					shaders[fsfile] = program;
				}
				return shaders[fsfile];
			}
			
			// Think about passing stuff to the shader, maybe we should not wrap around glUniformXY
			// (And even if we do, implement stuff like uniform2f/3f for vector stuff, as they are quite common)
			/* Sets integer uniform */
			void uniform(ShaderProgram program, char *uniform, int value)
			{
				glUseProgram(program.program);
				glUniform1i(glGetUniformLocation(program.program, uniform), value);
				glUseProgram(0);
			}

			/* Sets float uniform */
			void uniform(ShaderProgram program, char *uniform, float value)
			{
				glUseProgram(program.program);
				glUniform1f(glGetUniformLocation(program.program, uniform), value);
				glUseProgram(0);
			}

			/* Sets 3x3 Kernel Tex uniform */
			// Decided against this, if someone uses a kernel tex they should do it on their own. kinda. 
			/*void uniform(ShaderProgram program, char* uniform, Kernel3x3 value)
			{
				glUseProgram(program.program);
				glUniform2fv(glGetUniformLocation(program.program, uniform), 9, value.texCoordOffsets);
				glUseProgram(0);
			}*/

			/* Applies shader to frame buffer contents */
			// Think about stuff like texture units. Maybe the user wants to do something more "special"
			// like multitexturing. He should be able to do so without replicating all the gl stuff
			// (As we do at the normalmapping class at the moment)
			void run(ShaderProgram program)
			{
				int width = realWidth(*this->graphics);
				int height = realHeight(*this->graphics);

				/* If we don't have shader support don't do anything */
				if(!available()) return;

				/* Force the Gosu drawOpQueue to flush */
				graphics->beginGL();

				/* Copy frame buffer contents to internal texture */
				glBindTexture(GL_TEXTURE_2D, PostProcessing::internalTexture(*this->graphics));
				glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, width, height, 0);

				/* Activate shader program */
				glUseProgram(program.program);

				/* Clears screen */
				glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
				glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

				glMatrixMode(GL_PROJECTION);
				glPushMatrix();
				glLoadIdentity();
				glViewport(0, 0, width, height);
				glOrtho(0, width, height, 0, -1, 1);

				/* Enable texturing and bind the internal texture */
				// Why are we not activating texturing nor the texture unit here?
				//glEnable(GL_TEXTURE_2D);
				//glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, PostProcessing::internalTexture(*this->graphics));

				/* Draws screen-sized quad */
				glBegin(GL_QUADS);
					glTexCoord2f(0.0f, 1.0f);
					glVertex2f(0, 0);
					glTexCoord2f(1.0f, 1.0f);
					glVertex2f((GLfloat)width, 0);
					glTexCoord2f(1.0f, 0.0f);
					glVertex2f((GLfloat)width, (GLfloat)height);
					glTexCoord2f(0.0f, 0.0f);
					glVertex2f(0, (GLfloat)height);
				glEnd();

				/* Disable shader program */
				glUseProgram(0);

				glMatrixMode(GL_PROJECTION);
				glPopMatrix();

				graphics->endGL();
			}
	};

	/* Applies 'pixelate'-shader */
	inline void pixelate(Graphics &graphics, int pixelsize = 10)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/pixelate.fs");
		local.uniform(program, "pixelSize", pixelsize);           

		int width = realWidth(graphics);
		int height = realHeight(graphics);

		local.uniform(program, "width", width);
		local.uniform(program, "height", height);
		local.run(program);
	};

	/* Applies 'radialblur'-shader */
	/* int x,y: Screen coordinates */
	/* float bluriness, brightness: [0-1] */
	inline void radialBlur(Graphics &graphics, int x = 0, int y = 0, float bluriness = 0.02f, float brightness = 1.0f, int passes = 10)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/radialblur.fs");
		local.uniform(program, "origin_x", (float)(x / graphics.width()) );
		local.uniform(program, "origin_y", ((float)(y / graphics.height())-1)*-1 ); /* Weird */ // fucking ugly hack ffs. shader needs to be rewritten!
		local.uniform(program, "BlurFactor", bluriness);
		local.uniform(program, "BrightFactor", brightness);
		local.uniform(program, "passes", passes);
		local.run(program);
	};

	/* Applies 'fade'-shader */
	/* float fade: [0-1] (Values above 1.0f brightens up the screen) */
	inline void fade(Graphics &graphics, float fade = 1.0f)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/fade.fs");
		local.uniform(program, "fade", fade);
		local.run(program);
	};

	/* Applies 'sepia'-shader */
	inline void sepia(Graphics &graphics)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/sepia.fs");
		local.run(program);
	};

	/* Applies 'blur'-shader */
	inline void blur(Graphics &graphics)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/blur.fs");
		local.kernelTextureCoordOffsets(program, "texcoordOff");
		local.run(program);
	};

	inline void sharpen(Graphics &graphics)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/sharpen.fs");
		local.kernelTextureCoordOffsets(program, "texcoordOff");
		local.run(program);
	};

	inline void invert(Graphics &graphics)
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/invert.fs");
		local.run(program);
	};

	inline void contrast(Graphics &graphics) /* NOT USABLE YET! */ // Shader is just not working as intended
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/contrast.fs");
		local.uniform(program, "contrast", 0.1f);
		local.run(program);
	};

	inline void saturation(Graphics &graphics, float alpha) 
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/saturation.fs");
		local.uniform(program, "a", alpha);
		local.run(program);
	};

	// meh, to much stuff to do, gets its own class
	/*inline void normalMapping(Graphics &graphics, float lpos[3]) 
	{
		PostProcessing local(graphics);
		static ShaderProgram program = local.compile(Gosu::sharedResourcePrefix() + L"shader/normalmapping.fs", Gosu::sharedResourcePrefix() + L"shader/normalmapping.vs");
		glUseProgram(program.program);


		glUniform3f(glGetUniformLocation(program.program, "LightPosition"), 0.0,1.0,1.0);
		glUniform3f(glGetUniformLocation(program.program, "LightPosition"), lpos[0], lpos[1], lpos[2]);
		glUniform3f(glGetUniformLocation(program.program, "SurfaceColor"), 0.7,0.6,1.0);
		glUniform1f(glGetUniformLocation(program.program, "BumpDensity"), 16.0);
		glUniform1f(glGetUniformLocation(program.program, "BumpSize"), 0.15);
		glUniform1f(glGetUniformLocation(program.program, "SpecularFactor"), 0.5);


		glUseProgram(0);
		local.run(program);
	};*/
}

#endif