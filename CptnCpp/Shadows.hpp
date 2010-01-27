#ifndef GOSU_EXT_SHADOWS_HPP
#define GOSU_EXT_SHADOWS_HPP

#include <Gosu/Gosu.hpp>

#include <GL/glew.h>
#ifdef _MSC_VER
	#pragma comment(lib, "glew32.lib")
#endif

#include <vector>

namespace Gosu
{

	struct Vertex 
	{
		float x, y, z;
		Vertex(float x = 0.0f, float y = 0.0f, float z = 0.0f) : x(x), y(y), z(z) {};
		void set(float x = 0.0f, float y = 0.0f, float z = 0.0f) {this->x = x; this->y = y; this->z = z; };
	};


	// If you want the light to scroll with the screenx/y coords, you need to set it to dynamic = true
	class Light
	{
	private:
		static long& identCount() { static long instance = 0; return instance; }
		long identifier;
		int x, y, range, lastModified;
		Gosu::Color color;
		bool dynamic;

	public:
		Light(int x, int y, int range, Gosu::Color color, bool dynamic = false)
			: x(x), y(y), range(range), color(color), dynamic(dynamic)
		{
			this->identifier = identCount()++;
			this->lastModified = Gosu::milliseconds();
		}

		int getX() { return this->x; }
		int getY() { return this->y; }
		int getRange() { return this->range; }
		Gosu::Color getColor() { return this->color; }
		bool isDynamic() { return this->dynamic; }

		int getLastModified() { return this->lastModified; }
		long getIdentifier() { return this->identifier; }
	};

	class CachedLight
	{
	private:
		GLuint texture;
		long identifier;
		int x, y, screenx, screeny, generated;

	public:
		CachedLight(Gosu::Light &light, int x, int y, int screenx, int screeny)
			: generated(light.getLastModified()), identifier(light.getIdentifier()), x(x), y(y), screenx(screenx), screeny(screeny)
		{

		}

		long getIdentifier() { return this->identifier; }

		bool hasChanged(Gosu::Light &light, int x, int y, int screenx, int screeny)
		{
			return !(light.getLastModified() == this->generated && this->x == x && this->y == y && this->screenx == screenx && this->screeny == screeny);
		}

		void updateLight(Gosu::Light &light, int x, int y, int screenx, int screeny)
		{
			this->x = x;
			this->y = y;
			this->screenx = screenx;
			this->screeny = screeny;
			this->generated = light.getLastModified();
		}

		GLuint getTexture() { return this->texture; }
		void setTexture(GLuint texture) { this->texture = texture; }
	};

	class ShadowCaster
	{
	private:
		std::vector<Vertex> vertices;
		GLuint vbo;

	public:
		ShadowCaster()
		{
			this->vbo = 0;
		}

		void addVertex(Vertex vertex)
		{
			this->vertices.push_back(vertex);
		}

		std::vector<Vertex>& getVerticesRef() { return this->vertices; }

		// I dont even know anymore if VBOs make sense here - not used right now
		void build()
		{
			if(this->vbo == 0)
			{
				glGenBuffers(1, &this->vbo);
			}
			glBindBuffer(GL_ARRAY_BUFFER, this->vbo);
			//glBufferData(GL_ARRAY_BUFFER, this->vertices.size()*3*sizeof(float), data, GL_DYNAMIC_DRAW);
			glBindBuffer(GL_ARRAY_BUFFER, 0);
		}

		GLuint getVbo() { return this->vbo; }
	};

	class ShadowGeometry
	{
	private:
		static long& identCount() { static long instance = 0; return instance; }

		long identifier;
		int lastModified;

		std::vector<Gosu::ShadowCaster> caster;

	public:
		int getLastModified() { return this->lastModified; }
		long getIdentifier() { return this->identifier; }

		std::vector<Gosu::ShadowCaster>& getCasterRef() { return this->caster; }

		ShadowGeometry()
		{
			this->identifier = identCount()++;
			this->lastModified = Gosu::milliseconds();
		}

		void addObject(ShadowCaster caster)
		{
			this->caster.push_back(caster);
		}

		void build()
		{
			for(std::vector<Gosu::ShadowCaster>::iterator it = this->caster.begin(); it != this->caster.end(); ++it)
			{
				(*it).build();
			}

			this->lastModified = Gosu::milliseconds();
		}
	};
	
	class Shadows
	{
	private:
		static std::vector<CachedLight>& cachedLightsRef() { static std::vector<CachedLight> instance; return instance; }

		static GLuint generateShadowMap(Graphics& graphics, GLuint texture, Gosu::ShadowGeometry &objects, int x, int y, int screenx, int screeny)
		{
			GLuint glTex = texture;
			if(glTex == -1)
			{
				glGenTextures(1, &glTex);
				glBindTexture(GL_TEXTURE_2D, glTex);
				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 800, 600, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
			}

			static GLuint fbo;
			static GLuint renderbuffer;
			static bool init = false;

			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, 0);
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, 0);

			// Setup frame and render buffer, if we havent already
			if(!init)
			{
				glGenRenderbuffersEXT( 1, &renderbuffer );
				glBindRenderbufferEXT( GL_RENDERBUFFER_EXT, renderbuffer );
				glRenderbufferStorageEXT( GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT32, 800, 600 );

				glGenFramebuffersEXT( 1, &fbo );
				glBindFramebufferEXT( GL_FRAMEBUFFER_EXT, fbo );
				glFramebufferRenderbufferEXT( GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_RENDERBUFFER_EXT, renderbuffer );
				glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glTex, 0 );

				GLenum status = glCheckFramebufferStatusEXT(GL_FRAMEBUFFER);

				if (status != GL_FRAMEBUFFER_COMPLETE_EXT)
				{
					std::cout << "FBO error: " << status << std::endl; // meh
				}
				
				glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0);
				init = true;
			}

			glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, fbo); 
			glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, glTex, 0 );

			// Clear our shadow texture
			glClearColor( 0.0f, 0.0f, 0.0f, 0.0f );
			glClear( GL_COLOR_BUFFER_BIT );

			Gosu::PostProcessing local(graphics);
			static Gosu::ShaderProgram program = local.compile(L"data/shader/shadows/shadowMap.frag", L"data/shader/shadows/shadowMap.vert");
			glUseProgram(program.program);	
			
			glUniform2f(glGetUniformLocation(program.program, "LightPosition"), (GLfloat)x, (GLfloat)y);
			glUniform2f(glGetUniformLocation(program.program, "screenpos"), (GLfloat)screenx, (GLfloat)screeny);

			for (std::vector<ShadowCaster>::iterator objIt = objects.getCasterRef().begin(); objIt!=objects.getCasterRef().end(); ++objIt) {
				ShadowCaster curObj = *objIt;
				
				std::vector<Vertex> vertices = curObj.getVerticesRef();
				
				glBegin(GL_TRIANGLE_STRIP);
					for (unsigned int i=0;i<vertices.size(); i++) {
						glVertex3f(vertices[i].x, vertices[i].y, 0);
						glVertex3f(vertices[i].x, vertices[i].y, -1);
					}

					glVertex3f(vertices[0].x, vertices[0].y, 0);
					glVertex3f(vertices[0].x, vertices[0].y, -1);

				glEnd();
			}
			glUseProgram(0);

			// this is an ugly hack for cptncpp, we want all the blocks visible, so we just erase the shadow
			glDisable(GL_BLEND);
			glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
			for (std::vector<ShadowCaster>::iterator objIt = objects.getCasterRef().begin(); objIt!=objects.getCasterRef().end(); ++objIt) {
				ShadowCaster curObj = *objIt;
				
				std::vector<Vertex> vertices = curObj.getVerticesRef();
				
				glBegin(GL_QUADS);
					for (unsigned int i=0;i<vertices.size(); i++) {
						glVertex3f(vertices[i].x+screenx, vertices[i].y+screeny, 0);
					}
				glEnd();
			}
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);
			glEnable(GL_BLEND);

			glBindFramebuffer(GL_FRAMEBUFFER, 0); 
			glDisable(GL_TEXTURE_2D);

			return glTex;
		}

		static GLuint getShadowMap(Graphics &graphics, Gosu::ShadowGeometry &objects, Gosu::Light &light, int screenx, int screeny)
		{
			int x = light.getX();
			int y = light.getY();
			if(light.isDynamic())
			{
				x-=screenx;
				y-=screeny;
			}
			for(std::vector<Gosu::CachedLight>::iterator it = Shadows::cachedLightsRef().begin(); it != Shadows::cachedLightsRef().end(); ++it)
			{
				if((*it).getIdentifier() == light.getIdentifier())
				{
					if((*it).hasChanged(light, light.getX(), light.getY(), screenx, screeny))
					{
						(*it).updateLight(light, light.getX(), light.getY(), screenx, screeny);
						Shadows::generateShadowMap(graphics, (*it).getTexture(), objects, x, y, screenx, screeny);
					}
					
					return (*it).getTexture();
				}
			}
			
			CachedLight cachedStuff(light, light.getX(), light.getY(), screenx, screeny);

			cachedStuff.setTexture(Shadows::generateShadowMap(graphics, -1, objects, light.getX(), light.getY(), screenx, screeny));

			Shadows::cachedLightsRef().push_back(cachedStuff);

			return cachedStuff.getTexture();
		}

		static GLuint internalColorTexture(Graphics &graphics)
		{
			static bool initialized = false;
			static GLuint texture = -1;
			static int textureWidth, textureHeight;

			int width = realWidth(graphics);
			int height = realHeight(graphics);

			// Is the texture uninitialized or did the screen size change?
			if (!initialized || textureWidth != width || textureHeight != height)
			{
				// If texture already exists, delete it first
				if(texture != -1)
					glDeleteTextures(1, &texture);

				// Create a texture of screen size
				glGenTextures(1, &texture);
				glBindTexture(GL_TEXTURE_2D, texture);
				glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, 1);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, 0);
				glBindTexture(GL_TEXTURE_2D, 0);
				
				// Save the width and height
				textureWidth = width;
				textureHeight = height;

				initialized = true;
			}

			return texture;
		}

	public:

		static void beginnShadowpass(Graphics &graphics)
		{			
			graphics.beginGL();

			glBindTexture(GL_TEXTURE_2D, Shadows::internalColorTexture(graphics));
			glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, 0, 0, realWidth(graphics), realHeight(graphics), 0);
			glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
			glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			glViewport(0, 0, realWidth(graphics), realHeight(graphics));
			glOrtho(0, realWidth(graphics), realHeight(graphics), 0, -1, 1);

			glEnable(GL_TEXTURE_2D);
			glEnable(GL_BLEND);
			glBlendFunc(GL_SRC_ALPHA, GL_DST_ALPHA);
			
		}

		static void drawLight(Graphics &graphics, Light &light, std::vector<Gosu::ShadowGeometry> &objects, int screenx, int screeny)
		{
			int x = light.getX();
			int y = light.getY();
			if(!light.isDynamic())
			{
				x+=screenx;
				y+=screeny;
			}

			if(!(x-light.getRange() < 800 && x+light.getRange() > 0 && y-light.getRange() < 600 && y+light.getRange() > 0)) return;

			static GLuint texture = -1;
			if(texture == -1)
			{
				// Init renderbuffer/texture/thingythingthing
			}

			// combine multiple shadowmaps to one texture
			// this is needed if multiple shadow caster objects are used (=> performance)
			GLuint currentShadowmap = 0;
			for(std::vector<Gosu::ShadowGeometry>::iterator it = objects.begin(); it != objects.end(); ++it)
			{
                // yeah, not working at the moment
				currentShadowmap = Shadows::getShadowMap(graphics, (*it), light, screenx, screeny);
			}

			glEnable(GL_BLEND);

			// Set up the shader with PostProcessing library
			
			Gosu::PostProcessing local(graphics);
			static Gosu::ShaderProgram program = local.compile(L"data/shader/shadows/finalShadow.frag");
			glUseProgram(program.program);

			glUniform3f(glGetUniformLocation(program.program, "LightColor"), (float)light.getColor().red()/255, (float)light.getColor().green()/255, (float)light.getColor().blue()/255);

			glUniform2f(glGetUniformLocation(program.program, "LightPosition"), (GLfloat)x, (GLfloat)600-y);
			glUniform1f(glGetUniformLocation(program.program, "LightRange"), (GLfloat)light.getRange());

			glUniform1i(glGetUniformLocation(program.program, "texture"), 0); 
			glUniform1i(glGetUniformLocation(program.program, "lightmap"), 1);

			
			glColor4f(1.0f, 1.0f, 1.0f, 1.0f);

			// Enable texturing and activate/bind the internal texture to units
			glEnable(GL_TEXTURE_2D);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, Shadows::internalColorTexture(graphics));
			glActiveTexture(GL_TEXTURE1);
			glBindTexture(GL_TEXTURE_2D, currentShadowmap);

			// Draws screen-sized quad
			glBegin(GL_QUADS);
				glTexCoord2f(0.0f, 1.0f);
				glVertex2f(0, 0);
				glTexCoord2f(1.0f, 1.0f);
				glVertex2f((GLfloat)realWidth(graphics), 0);
				glTexCoord2f(1.0f, 0.0f);
				glVertex2f((GLfloat)realWidth(graphics), (GLfloat)realHeight(graphics));
				glTexCoord2f(0.0f, 0.0f);
				glVertex2f(0, (GLfloat)realHeight(graphics));
			glEnd();

			//Disable shader program
			glUseProgram(0);
		}

		static void endShadowpass(Graphics &graphics)
		{
			graphics.endGL();
		}
	};

}

#endif