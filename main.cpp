#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

#define MAP_W 100
#define MAP_H 100
#define SCREEN_H 50
#define SCREEN_W 50

typedef struct s_point {
	int x;
	int y;
} t_point;

typedef enum e_direction {
	NORTH = 0,
	EAST = 1,
	SOUTH = 2,
	WEST = 3,
} t_direction;

typedef struct s_player {
	int health;
	t_point cur_position;
	t_direction cur_direction;
} t_player;

/*
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
*/

void	init_ncurses() {
	initscr();
	noecho();
	timeout(42);
}

t_point get_next_step(t_player p) {
	t_point point;
	int dx, dy;
	dx = 0;
	dy = 0;
	switch (p.cur_direction) {
		case NORTH:
			dx = 0;
			dy = -1;
			break;
		case EAST:
			dx = 1;
			dy = 0;
			break;
		case SOUTH:
			dx = 0;
			dy = 1;
			break;
		case WEST:
			dx = -1;
			dy = 0;
			break;
	}
	point.x = dx;
	point.y = dy;
	return point;
}

void move(t_player *p) {
	t_point delta = get_next_step(*p);
	t_point new_position;
	new_position = p->cur_position;
	new_position.x += delta.x;
	new_position.y += delta.y;
	if (new_position.x < MAP_W && new_position.x > 0) {
		p->cur_position.x = new_position.x;
	}
	if (new_position.y < MAP_H && new_position.y > 0) {
		p->cur_position.y = new_position.y;
	}
}


void draw_screen(t_player player, char map[MAP_H][MAP_W]) {
	// damage when meet enemy
	// TODO: blink screen in red
	//if (map[player.cur_position.y][player.cur_position.x]) {
		//player.health -= 1;
	//}

	// draw borders
	for (int y = 0; y <= SCREEN_H; y++) {
		for (int x = 0; x <= SCREEN_W; x++) {
			if (y == SCREEN_H || x == SCREEN_W) {
				mvprintw(y, x, "%c", '*');
			}
		}
	}

	// TODO: slide - move with space: fire and move without changing the direction
	// draw current screen
	// draw player at the center and his direction
	
	int start_x = player.cur_position.x - SCREEN_W/2;
	int start_y = player.cur_position.y - SCREEN_H/2;
	if ( start_x < 0 ) {
		start_x = 0;
	}
	if (start_y < 0) {
		start_y = 0;
	}

	// draw enemies
	for (int y = 0; y < SCREEN_H; y++) {
		for (int x = 0; x < SCREEN_W; x++) {
			mvprintw(y, x, "%c", map[start_y + y][start_x + x]);
			//mvprintw(y, x*2, "%c", OR(map[y+player.pos.y-SCREEN_H/2][x+player.pos.x-SCREEN_W/2], ' '));
		}
	}

	// draw player
	mvprintw(SCREEN_H/2, SCREEN_W/2, "o");
	t_point delta = get_next_step(player);
	mvprintw(SCREEN_H/2+delta.y, SCREEN_W/2+delta.x, "%c", "^>v<"[player.cur_direction]);
}

int	main() {
	init_ncurses();
	t_player player;
	player.health = 3;
	// type cast is ok?
	player.cur_position.x = MAP_W/2;
	player.cur_position.y = MAP_H/2;
	player.cur_direction = EAST;
	char	map[MAP_H][MAP_W] = {};
	// enemy count is 120
	// TODO: put breaks at the edges of map
	for (int i = 0; i < 120; i++) {
		map[rand()%MAP_H][rand()%MAP_W] = 'E';
	}

	for (; player.health > 0;) {
		int	ch = getch();
		clear();
		printw("hp=%d, ch=%c, pos [x:%d, y:%d]", player.health, ch, player.cur_position.x, player.cur_position.y);
		switch (ch) {
			case ' ':
				//fire by laser
				for (int i = 1; i < 20; i++) {
					//mvprintw(SCREEN_H/2+player.diry()*i, SCREEN_W/2+player.dirx()*i*2, "+");
				}
				break;
			case 'a':
				player.cur_direction = WEST;
				break;
			case 'd':
				player.cur_direction = EAST;
				break;
			case 'w':
				player.cur_direction = NORTH;
				break;
			case 's':
				player.cur_direction = SOUTH;
				break;
			case 'q':
				//exit game
				endwin();
				return 0;
		}
		move(&player);
		draw_screen(player, map);
	}
	// player dead
	// draw dead msg
	return 0;
}
