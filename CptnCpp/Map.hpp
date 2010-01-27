#ifndef MAP_HPP
#define MAP_HPP

#include "Gosu.hpp"

const int tileNothing = -1;
const int tileGrass = 0;
const int tileEarth = 1;

class Map
{
	private:		
		vector<int> tiles;
		int width, height;

		Gosu::Image *sky, *gem;
		vector<Gosu::Image*> tileset;

		vector<CollectibleGem> gemlist;
		vector<Gosu::ShadowGeometry> shadowgeometry;

		Gosu::Graphics *graphics;

		Gosu::Light *playerLight;
		vector<Gosu::Light*> lights;

	public:
		Map(Gosu::Graphics &graphics, wstring filename)
		{
			this->graphics = &graphics;

			Gosu::imagesFromTiledBitmap(graphics, L"data/media/CptnRuby Tileset.png", 60, 60, true, this->tileset);
			this->sky = new Gosu::Image(graphics, L"data/media/Sky.png", true);
			this->gem = new Gosu::Image(graphics, L"data/media/CptnRuby Gem.png", true);

			vector<string> lines;

			ifstream mapfile(filename.c_str(),ios::in);
			if(mapfile.is_open())
			{
				while(!mapfile.eof())
				{
					string line;
					getline(mapfile, line);
					lines.push_back(line);
				}
			}

			this->width = lines[0].length();
			this->height = lines.size()-1;

			this->tiles.resize(this->width * this->height);

			for(int y=0; y < this->height; y++)
			{
				string currentLine = lines[y];

				for(int x=0; x < this->width; x++)
				{
					string c = currentLine.substr(x, 1);

					this->tiles[y * this->width + x] = tileNothing;

					if(c.compare("\"") == 0)
					{
						this->tiles[y * this->width + x] = tileGrass;
					}
					else if(c.compare("#")  == 0)
					{
						this->tiles[y * this->width + x] = tileEarth;
					}
					else if(c.compare("x") == 0)
					{
						this->gemlist.push_back(CollectibleGem(this->gem, x, y));
					}
				}
			}

			int xc, yc;
			xc = yc = 0;

			Gosu::ShadowGeometry shadowGeom;
			for(int x=0; x < this->width; x++)
			{
				for(int y=0; y < this->height; y++)
				{
					int currentTile = this->getTiles()[y * this->getWidth() + x];
					if(currentTile != -1)
					{
						Gosu::ShadowCaster obj;

						float mod = 3;
						float tx = x*50 + mod;
						float ty = y*50 + mod;

						obj.addVertex(Gosu::Vertex(tx, ty));
						obj.addVertex(Gosu::Vertex(tx, ty+50+mod ));
						obj.addVertex(Gosu::Vertex(tx+50+mod, ty+50+mod));
						obj.addVertex(Gosu::Vertex(tx+50+mod, ty));

						shadowGeom.addObject(obj);
					}
				}
			}

			shadowGeom.build();

			this->shadowgeometry.push_back(shadowGeom);

			this->playerLight = new Gosu::Light(400, 300, 300, Gosu::Color(255, 255, 255), true);

			this->lights.push_back(new Gosu::Light(200, 300, 300, Gosu::Colors::red, false));

			this->lights.push_back(new Gosu::Light(250, 550, 300, Gosu::Color(200, 200, 200), false));
		}

		Map::~Map()
		{
			delete this->sky, this->gem;
		}

		vector<int> getTiles() {
			return this->tiles;
		}

		int getHeight()
		{
			return this->height;
		}

		int getWidth()
		{
			return this->width;
		}

		void draw(float screen_x, float screen_y, bool background = true)
		{
			if(background) this->sky->draw(0, 0, 0);

			for(int x=0; x < this->width; x++)
			{
				for(int y=0; y < this->height; y++)
				{
					int currentTile = this->tiles[y * this->width + x];
					if(currentTile != tileNothing)
					{
						this->tileset[currentTile]->draw(x * 50 - screen_x, y * 50 - screen_y, 1);
					}
				}
			}

			for(vector<CollectibleGem>::iterator it = this->gemlist.begin(); it != gemlist.end(); ++it)
			{
				(*it).draw(screen_x, screen_y);
			}


			Gosu::Shadows::beginnShadowpass(*graphics);

			Gosu::Shadows::drawLight(*graphics, *this->playerLight, this->shadowgeometry, (int)-screen_x, (int)-screen_y);

			for(std::vector<Gosu::Light*>::iterator it = this->lights.begin(); it != this->lights.end(); ++it)
			{
				Gosu::Shadows::drawLight(*graphics, *(*it), this->shadowgeometry, (int)-screen_x, (int)-screen_y);
			}

			Gosu::Shadows::endShadowpass(*graphics);

		}

		bool isSolid(int x, int y)
		{
			return (y < 0 || this->tiles[ ( (y+50)/50) * this->width + (x/50)] >= 0);
		}
};

#endif