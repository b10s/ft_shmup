#include <stdlib.h>
#include <unistd.h>
#include <curses.h>

#define MAP_W 200
#define MAP_H 100
#define SCREEN_H 50
#define SCREEN_W 100
#define LASER_RANGE 10

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
	new_position.x += delta.x*2;
	new_position.y += delta.y;

	if (new_position.x < MAP_W - SCREEN_W/2 && new_position.x > SCREEN_W/2) {
		p->cur_position.x = new_position.x;
	}
	if (new_position.y < MAP_H - SCREEN_H/2 && new_position.y > SCREEN_H/2) {
		p->cur_position.y = new_position.y;
	}
}


void draw_screen(t_player player, char map[MAP_H][MAP_W]) {
	// damage when meet enemy
	// TODO: blink screen in red
	//if (map[player.cur_position.y][player.cur_position.x]) {
	//#define LASER_RANGE (SCREEN_W/3)
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
	for (int i = 0; i < 120; i++) {
		map[rand()%MAP_H][rand()%MAP_W] = 'E';
	}

	// draw side walls
	for (int y = 0; y < MAP_H; y++){
		for (int x = 0; x < MAP_W; x++) {
			if (x < SCREEN_W/2 || x > MAP_W - SCREEN_W/2 
					|| y < SCREEN_H/2 || y > MAP_H - SCREEN_H/2) {
				map[y][x] = 'W';
			}
		}
	}

	for (; player.health > 0;) {
		int	ch = getch();
		clear();
		printw("hp=%d, ch=%c, pos [x:%d, y:%d]", player.health, ch, player.cur_position.x, player.cur_position.y);
		switch (ch) {
			case ' ':
				//fire by laser
				for (int i = 0; i < LASER_RANGE; i++) {
					t_point dir_pnt = get_next_step(player);
					mvprintw(SCREEN_H/2+dir_pnt.y*i, SCREEN_W/2+dir_pnt.x*i*2, "+");
					//TODO: kill enemies, score points?
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
		// TODO: make space for health, coordinates, etc
		mvprintw(0, 0, "hp=%d, ch=%c, pos [x:%d, y:%d]", player.health, ch, player.cur_position.x, player.cur_position.y);
	}
	// player dead
	// draw dead msg
	return 0;
}
