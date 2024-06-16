#include <cstdio>
#include <curses.h>
#include <stdexcept>
#include <vector>

#define MAP_W 200
#define MAP_H 50
#define SCREEN_H 50
#define SCREEN_W 50

struct	Point {
	int x;
	int y;
	Point(int y, int x): x(x), y(y) {}
};

enum e_direction {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
};

class Player {
public:
	int	hp;
	Point pos;
	e_direction dir;

	Player(int y, int x): hp(3), pos(y, x), dir(EAST) {}
	~Player() {}

	static Point dir_to_delta(e_direction dir)
	{
		if (dir == NORTH)
			return Point(-1, 0);
		else if (dir == SOUTH)
			return Point(+1, 0);
		else if (dir == EAST)
			return Point(0, +1);
		else if (dir == WEST)
			return Point(0, -1);
		return Point(0, 0);
	}

	void	slide(int dy, int dx)
	{
		if (1 || (0 < pos.x + dx && pos.x + dx < MAP_W))
			pos.x += dx;
		if (0 < pos.y + dy && pos.y + dy < MAP_H)
			pos.y += dy;
	}
	void	rotate(e_direction newdir)
	{
		dir = newdir;
	}
	void	move(e_direction dir)
	{
		Point delta = dir_to_delta(dir);
		slide(delta.y, delta.x);
		//rotate(dir);
	}
	int	dirx() const
	{
		return dir_to_delta(dir).x;
	}
	int	diry() const
	{
		return dir_to_delta(dir).y;
	}
};

class Gamemap {
	typedef char value_type;
	value_type arr[MAP_H][MAP_W];
public:
	Gamemap(): arr() {}

	const value_type& at(int y, int x) const
	{
		return arr[y % MAP_H][x % MAP_W];
	}
	value_type& at(int y, int x)
	{
		if (0 <= y && y < MAP_H && 0 <= x && x < MAP_W)
			return arr[y][x];
		return arr[y % MAP_H][x % MAP_W];
	}
};

void	init()
{
	initscr();
	noecho();
	timeout(42);
}

#define OR(a, b) (a ? a : b)

int	main()
{
	init();
	Player	player(getmaxy(stdscr)/2, 0);
	Gamemap map;
	for (int i = 0; i < 120; i++) {
		map.at(rand() % MAP_H, rand() % MAP_W & ~1) = 'E';
	}

	for (; player.hp > 0;) {
		int	ch = getch();
		clear();
		printw("hp=%d ", player.hp);
		if (ch == ' ') {
			for (int i = 1; i < 20; i++) {
				mvprintw(player.pos.y + player.diry() * i, player.dirx() * i*2, "+");
				if (map.at(player.pos.y + player.diry() * i, player.pos.x + player.dirx() * i) == 'E')
					map.at(player.pos.y + player.diry() * i, player.pos.x + player.dirx() * i) = '#';
			}
		}
		else if (ch == 'a')
			player.move(WEST);
		else if (ch == 'd')
			player.move(EAST);
		else if (ch == 'w')
			player.move(NORTH);
		else if (ch == 's')
			player.move(SOUTH);
		else if (ch == 'q')
			break;
		else
			player.move(player.dir);

		if (map.at(player.pos.y, player.pos.x) == 'E')
			player.hp -= 1;

		for (int y = 0; y < SCREEN_H; y++) {
			for (int x = 0; x < SCREEN_W; x++) {
				mvprintw(y, x*2, "%c", map.at(y, x+player.pos.x));
			}
		}
		mvprintw(0, 0, "hp=%d x=%d y=%d ", player.hp, player.pos.x, player.pos.y);
		mvprintw(player.pos.y, 0, "o");
		mvprintw(player.pos.y + player.diry(), 0 + player.dirx(), "%c", "^>v<"[player.dir]);
	}
	endwin();
}
