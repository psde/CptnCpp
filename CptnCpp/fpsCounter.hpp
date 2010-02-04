#ifndef GOSU_EXT_FPSCOUNTER_HPP
#define GOSU_EXT_FPSCOUNTER_HPP

#include <Gosu/Gosu.hpp>

#include <deque>
#include <math.h>

template <class T> T clamp (T a, T min, T max) {
  T result;
  result = (a > max)? max : a;
  result = (result < min)? min : result;
  return (result);
}

struct Point
{
	Point(int x = 0, int y = 0) {
		this->x = x;
		this->y = y;
	}
	int x,y;
};

struct FpsPoint
{
	FpsPoint(double fps, bool landmark = false)
	{
		this->fps = fps;
		this->landmark = landmark;
	}

	double fps;
	bool landmark;
};

namespace Gosu
{
	class fpsCounter
	{
	private:
		Gosu::Graphics *graphics;

		int x, y, width, height;

		int logtime;

		std::deque<FpsPoint> fpslist;

		Gosu::Font *font;

		
		Gosu::Color boxColor, lineColor, lineLandmarkColor;

		float updateInterval;
		float accum;
		int frames;
		float timeleft;
		int lastFrameCompleted;
		double fps;

	public:	
		fpsCounter(Gosu::Graphics *graphics, int x, int y, int width, int height, float updateInterval = 0.03f)
			: graphics(graphics), x(x), y(y), width(width), height(height), updateInterval(updateInterval)
		{
			font = new Gosu::Font(*graphics, Gosu::defaultFontName(), 14);

			this->boxColor = Gosu::Color(255, 255, 255, 255);
			this->lineColor = Gosu::Color(220, 255, 255, 255);
			this->lineLandmarkColor = Gosu::Color(65, 255, 255, 255);

			this->timeleft = this->updateInterval;
			this->accum = 0.0f;
			this->frames = 0;
			this->lastFrameCompleted = Gosu::milliseconds();
			this->logtime = Gosu::milliseconds();
			this->fps = 0;
		}
        
        void setUpdateInterval(float interval)
        {
            this->updateInterval = interval;
        }

		void updateFPS()
		{
			// based on http://www.unifycommunity.com/wiki/index.php?title=FramesPerSecond
			float deltaTime = (float)Gosu::milliseconds() - this->lastFrameCompleted;
			this->timeleft -= deltaTime/1000.0f;
			this->accum += 1000.0f/deltaTime;
			this->frames++;
			this->fps = round(accum/frames);

			if( timeleft <= 0.0 )
			{
				bool landmark = false;
				if(Gosu::milliseconds() > (unsigned int)logtime + 1000)
				{
					logtime = Gosu::milliseconds();
					landmark = true;
				}
				this->fpslist.push_back(FpsPoint(fps, landmark));
				this->frames = 0;
				this->accum = 0;
				this->timeleft = this->updateInterval;
			}

			while(fpslist.size() > (unsigned int)this->width+1)
				fpslist.pop_front();
		}

		void draw()
		{
			this->lastFrameCompleted = Gosu::milliseconds();

			Point topLeft(x, y);
			Point downRight(x + width, y + height);

			int z = 10000;

			// Draw box 
			this->graphics->drawLine(topLeft.x, topLeft.y, boxColor, downRight.x, topLeft.y, boxColor, z); // Top 
			this->graphics->drawLine(topLeft.x - 1, downRight.y, boxColor, downRight.x, downRight.y, boxColor, z); // Bottom 
			this->graphics->drawLine(topLeft.x, topLeft.y, boxColor, topLeft.x, downRight.y, boxColor, z); // Left 
			this->graphics->drawLine(downRight.x, topLeft.y, boxColor, downRight.x, downRight.y, boxColor, z); // Left

			// Draw fps 
			font->draw(L"Cur: " + boost::lexical_cast<std::wstring>(fps), topLeft.x+4, topLeft.y+2, z+1, 1, 1, this->boxColor);

			// Draw graph, if we are on it, calc avg fps 
			double avg = 0;
			int avgCount = 0;
			if(fpslist.size() > 2)
			{
				for(unsigned int i=0; i < fpslist.size() - 1; i++)
				{
					int x = topLeft.x + (width - i) - 1;
					double val = fpslist.at(i).fps * -1 + height;
					double val2 = fpslist.at(i+1).fps * -1 + height;

					val = clamp(val, 0.0, (double)height);
					val2 = clamp(val2, 0.0, (double)height);
					
					this->graphics->drawLine(x, topLeft.y + val2, lineColor, x+1, topLeft.y + val, lineColor, z - 1);
					
					if(fpslist.at(i).landmark)
					{
						this->graphics->drawLine(x, topLeft.y + height, lineLandmarkColor, x+1, topLeft.y + val, lineLandmarkColor, z - 1);
					}

					// Ignore negative fps, only happens a few times after init
					if(fpslist.at(i).fps > 0)
					{
						avg += fpslist.at(i).fps;
						avgCount++;
					}
				}
				avg = avg / avgCount;


				font->draw(L"Avg: " + boost::lexical_cast<std::wstring>(avg).substr(0,4), topLeft.x+4, topLeft.y+14, z+1, 1, 1, this->boxColor);
			}
		}
	};
}

#endif