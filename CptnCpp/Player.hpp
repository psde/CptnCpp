#ifndef PLAYER_HPP
#define PLAYER_HPP

#include "Gosu.hpp"

enum Dir
{
	Right,
	Left
};

class CptnRuby
{
	private:
		int x, y, vy;
		Dir dir;
		Map *map;

		vector<Gosu::Image*> playerset;
		Gosu::Image *curImg;

		int animationTick;

		bool wouldFit(int offs_x, int offs_y)
		{
			return !(this->map->isSolid(this->x + offs_x, this->y + offs_y) && this->map->isSolid(this->x + offs_x, this->y + offs_y - 45));
		}

	public:
		CptnRuby(Gosu::Graphics &graphics, Map *map, int x, int y)
		{
			this->animationTick = this->vy = 0;
			this->map = map;
			this->x = x;
			this->y = y;
			Gosu::imagesFromTiledBitmap(graphics, L"data/media/CptnRuby.png", 50, 50, false, this->playerset);
			this->curImg = this->playerset[0];
		}

		int getX() { return this->x; }
		int getY() { return this->y; }

		void draw(float screenx, float screeny)
		{
			int offs_x = 25;
			int factor = -1;
			if(this->dir == Left)
			{
				offs_x = -25;
				factor = 1;
			}
			this->curImg->draw(this->x - screenx + offs_x, this->y - screeny - 45, 3, factor);
		}

		void update(int move)
		{
			if(move > 0)
			{
				this->dir = Right;
				for(int i=0;i<move;i++) if(this->wouldFit(1, 0)) this->x++;
			}
			if(move < 0)
			{
				this->dir = Left;
				for(int i=0;i<-move;i++) if(this->wouldFit(-1, 0)) this->x--;
			}

			this->vy++;
			if(vy > 0)
			{
				for(int i=0;i<vy;i++) if(this->wouldFit(0, 1)) this->y++; else vy = 0;
			}
			if(vy < 0)
			{
				for(int i=0;i<-vy;i++) if(this->wouldFit(0, -1)) this->y--; else vy = 0;
			}

		
			if(move == 0)
			{
				this->curImg = this->playerset[0];
			}
			else
			{
				animationTick++;
				if(animationTick > 7) animationTick = 0;

				if(animationTick == 0) this->curImg = (Gosu::milliseconds() / 250 % 2 == 0 ? this->playerset[1] : this->playerset[2]); //(animationTick % 50) ? this->playerset[1] : this->playerset[2];
			}
			if(vy > 0)
			{
				this->curImg = this->playerset[3];
			}
		}

		void tryToJump()
		{
			if(this->map->isSolid(this->x, this->y+1))
			{
				this->vy = -20;
			}
		}
};

#endif