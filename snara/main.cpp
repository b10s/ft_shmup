#include <cstdio>
#include <curses.h>
#include <vector>

#define MAP_W 100
#define MAP_H 100
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
		if (0 + SCREEN_W/2 < pos.x + dx && pos.x + dx < MAP_W - SCREEN_W/2)
			pos.x += dx;
		if (0 + SCREEN_H/2 < pos.y + dy && pos.y + dy < MAP_H - SCREEN_H/2)
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
		rotate(dir);
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

class Entity {
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
	Player	player(getmaxy(stdscr)/2, getmaxx(stdscr)/2);
	char	map[MAP_H][MAP_W] = {};
	for (int i = 0; i < 120; i++) {
		map[rand()%MAP_H][rand()%MAP_W] = 'E';
	}

	for (; player.hp > 0;) {
		int	ch = getch();
		clear();
		printw("hp=%d ", player.hp);
		if (ch == ' ')
			for (int i = 1; i < 20; i++)
				mvprintw(SCREEN_H/2+player.diry()*i, SCREEN_W/2+player.dirx()*i*2, "+");
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

		if (map[player.pos.y][player.pos.x])
			player.hp -= 1;
		for (int y = 0; y < SCREEN_H; y++) {
			for (int x = 0; x < SCREEN_W; x++) {
				mvprintw(y, x*2, "%c", OR(map[y+player.pos.y-SCREEN_H/2][x+player.pos.x-SCREEN_W/2], ' '));
			}
		}
		mvprintw(SCREEN_H/2, SCREEN_W/2, "o");
		mvprintw(SCREEN_H/2+player.diry(), SCREEN_W/2+player.dirx(), "%c", "^>v<"[player.dir]);
	}
	endwin();
}
