#ifndef GEM_HPP
#define GEM_HPP

#include "Gosu.hpp"

class CollectibleGem
{
	private:
		int x, y;
		Gosu::Image *img;

	public:
		CollectibleGem(Gosu::Image *img, int x, int y)
		{
			this->img = img;
			this->x = x;
			this->y = y;
		};

		void draw(float screen_x, float screen_y)
		{
			this->img->drawRot((this->x * 50) - screen_x + 25, (this->y * 50) - screen_y + 25, 2, 25 * sin((float)Gosu::milliseconds() / 1337));
		};
};


#endif