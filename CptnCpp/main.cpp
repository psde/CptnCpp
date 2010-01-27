#include "Gosu.hpp"

using namespace std;

class Window : public Gosu::Window
{
	private:
		Gosu::Font *font;
		Gosu::fpsCounter *fps;

		Map *map;
		CptnRuby *cptnruby;
		Gosu::ShadowGeometry *foo;

	public:
		Window()
		 :	Gosu::Window(800, 600, false)
		{
			glewInit();
			this->font = new Gosu::Font(graphics(), Gosu::defaultFontName(), 20);
			this->fps = new Gosu::fpsCounter(&graphics(), 800-210, 10, 200, 100);
			this->map = new Map(graphics(), L"data/CptnRuby Map.txt");
			this->cptnruby = new CptnRuby(graphics(), this->map, 400, 100);

			this->foo = new Gosu::ShadowGeometry();
		}

		void draw()
		{

			float screenx = (float)this->cptnruby->getX() - 400;
			float screeny = (float)this->cptnruby->getY() - 300;

			this->cptnruby->draw(screenx, screeny);
			this->map->draw(screenx, screeny);

			fps->updateFPS();
			fps->draw();
		}

		void update()
		{
			int move = 0;
			if(input().down(Gosu::kbLeft)) move -= 5;
			if(input().down(Gosu::kbRight)) move += 5;

			this->cptnruby->update(move);
		}

		void buttonDown(Gosu::Button button)
		{
			if(button == Gosu::kbUp) this->cptnruby->tryToJump();
			if(button == Gosu::kbEscape) close();
		}
};

int main(int argc, char* argv[])
{

    Window win;
    win.show();

	return 0;
}
