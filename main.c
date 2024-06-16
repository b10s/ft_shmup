#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>

#include "./ft_shmup.h"

void	init_ncurses() {
	initscr();
	noecho();
	curs_set(0);
	cbreak();
	timeout(42);
	start_color();
	// color of text, color of background
	init_pair(1, COLOR_RED, COLOR_RED);
	init_pair(2, COLOR_BLUE, COLOR_BLUE);
}

char	map[MAP_H][MAP_W] = {};
int	enemy_state[MAP_H][MAP_W] = {};
long time_taken;

clock_t start;
t_player p;

t_point get_next_point() {
	t_point point;
	int dx, dy;
	dx = 0;
	dy = 0;
	switch (p.dir) {
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

void blink_red() {
	// Set all positions to red
	for (int y = 0; y < SCREEN_H; y++) {
	    for (int x = 0; x < SCREEN_W; x++) {
	        mvaddch(y, x, ' ' | COLOR_PAIR(1));
	    }
	}
	refresh();
	usleep(200000);
}

void blink_blue() {
	// Set all positions to red
	for (int y = 0; y < SCREEN_H; y++) {
	    for (int x = 0; x < SCREEN_W; x++) {
	        mvaddch(y, x, ' ' | COLOR_PAIR(2));
	    }
	}
	refresh();
	usleep(200000);
}


void move_player() {
	t_point delta = get_next_point();
	t_point new_position;
	new_position = p.pos;
	new_position.x += delta.x*2;
	new_position.y += delta.y;

	if (new_position.x < MAP_W - SCREEN_W/2 && new_position.x > SCREEN_W/2) {
		p.pos.x = new_position.x;
	}
	if (new_position.y < MAP_H - SCREEN_H/2 && new_position.y > SCREEN_H/2) {
		p.pos.y = new_position.y;
	}

	if (map[p.pos.y][p.pos.x] == 'E'
			|| map[p.pos.y][p.pos.x - delta.x] == 'E') {
		p.health--;
		blink_red();
	}
}

// laser is hotter from right side
// it is because of solar wind blows from left to the right :)
void fire_laser(t_point abs_enemy_pos, t_point rel_enemy_pos) {
	// fire by laser: up and down
	for (int i = 1; i < LASER_RANGE; i++) {
		// not a wall, not out of screen
		if (map[abs_enemy_pos.y + i][abs_enemy_pos.x] != 'W' && rel_enemy_pos.y + i < SCREEN_H) {
			if ((p.pos.x == abs_enemy_pos.x
					&& p.pos.y == abs_enemy_pos.y + i)
					|| (p.pos.x-1 == abs_enemy_pos.x
					&& p.pos.y == abs_enemy_pos.y + i)) {
				p.health--;
				blink_red();
			}
			mvprintw(rel_enemy_pos.y+i, rel_enemy_pos.x, "+");
		}
		//not a wall, screen at the top, negaitive is ok?
		if (map[abs_enemy_pos.y - i][abs_enemy_pos.x] != 'W') {
			if ((p.pos.x == abs_enemy_pos.x
					&& p.pos.y == abs_enemy_pos.y - i)
					|| (p.pos.x - 1 == abs_enemy_pos.x
					&& p.pos.y == abs_enemy_pos.y - i)) {
				p.health--;
				blink_red();
			}
			mvprintw(rel_enemy_pos.y-i, rel_enemy_pos.x, "+");
		}
	}
}

void draw_screen() {
	// draw current screen
	// draw player at the center and his direction
	t_point scr_origin;
	scr_origin.x = p.pos.x - SCREEN_W/2;
	scr_origin.y = p.pos.y - SCREEN_H/2;

	if ( scr_origin.x < 0 ) {
		scr_origin.x = 0;
	}
	if (scr_origin.y < 0) {
		scr_origin.y = 0;
	}

	char c;
	for (int y = 0; y < SCREEN_H; y++) {
		for (int x = 0; x < SCREEN_W; x++) {
			// draw enemies, walls, and everything
			mvprintw(y, x, "%c", map[scr_origin.y + y][scr_origin.x + x]);

			c = map[scr_origin.y + y][scr_origin.x + x];
			// enemy type is Laser
			if (c == 'L') {
				t_point abs_enemy_pos;
				abs_enemy_pos.x = scr_origin.x + x;
				abs_enemy_pos.y = scr_origin.y + y;
				t_point rel_enemy_pos;
				rel_enemy_pos.x = x;
				rel_enemy_pos.y = y;
				int state = enemy_state[abs_enemy_pos.y][abs_enemy_pos.x];
				if (state == 0) {
					// decide randomly shoot or not shoot
					if (rand() % 5 == 1) {
						// ENEMY SHOOTED
						// start counting 1 second to keep laser on
						// start counting 3 seconds to make sure enemy is not shooting (recharging)
						enemy_state[abs_enemy_pos.y][abs_enemy_pos.x] = time_taken;
					} 
				// Laser is in switched on state, time is stored in seconds
				} else if (state > 0) {
					fire_laser(abs_enemy_pos, rel_enemy_pos);
				// Laser is in cool down/recharging state, time is stored in seconds (negative)
				} else if (state < 0) {
					//recharging, need to wait 1+ seconds
					if (time_taken - (-1 * state) > 1) {
						enemy_state[abs_enemy_pos.y][abs_enemy_pos.x] = 0;
					}
				}
			}
		}
	}

	// draw player
	mvprintw(SCREEN_H/2, SCREEN_W/2, "o");
	t_point delta = get_next_point();
	mvprintw(SCREEN_H/2+delta.y, SCREEN_W/2+delta.x, "%c", "^>v<"[p.dir]);
	
	// draw borders
	for (int y = 0; y <= SCREEN_H; y++) {
		for (int x = 0; x <= SCREEN_W; x++) {
			if (y == SCREEN_H || x == SCREEN_W) {
				mvprintw(y, x, "%c", '*');
			}
		}
	}
}

void slide(t_direction direction) {
	switch (direction) {
		case NORTH:
			if (p.pos.y - 1 > SCREEN_H/2) {
				p.pos.y--;
			}
			break;
		case EAST:
			if (p.pos.x + 1 < MAP_W - SCREEN_W/2) {
				p.pos.x++;
			}
			break;
		case SOUTH:
			if (p.pos.y + 1 < MAP_H - SCREEN_H/2) {
				p.pos.y++;
			}
			break;
		case WEST:
			if (p.pos.x - 1 > SCREEN_W/2) {
				p.pos.x--;
			}
			break;
	}
}

// controls //
// awds - to rotate
// hjkl - to slide
// space - to shoot
// q - to quit
// //
int	main() {
	init_ncurses();

	// Calculate the time taken in seconds
	start = clock();
	struct timeval start, end;
	gettimeofday(&start, NULL);

	p.health = 3;
	// type cast is ok?
	p.pos.x = MAP_W/2;
	p.pos.y = MAP_H/2;
	p.dir = EAST;
	p.frame = 0;

	// tests
	///
	// enemy count is 120
	for (int i = 0; i < 120; i++) {
		map[rand()%MAP_H][rand()%MAP_W] = 'L';
	}
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

	for (; p.health > 0;) {
		gettimeofday(&end, NULL);
		time_taken = end.tv_sec - start.tv_sec;
		p.frame++;
		int	ch = getch();
		clear();
		printw("hp=%d, ch=%c, pos [x:%d, y:%d], time lapsed [%ld]", p.health, ch, p.pos.x, p.pos.y, time_taken);
		int enemy_crossed_laser = 0;
		for (int y =0; y < MAP_H; y++) {
			for (int x =0; x < MAP_W; x++) {
				if (enemy_state[y][x] > 0) {
					if (time_taken - enemy_state[y][x] > 1) {
						enemy_state[y][x] = -time_taken;
					}
				}
			}
		}
		switch (ch) {
			case ' ':
				//fire by laser
				for (int i = 0; i < LASER_RANGE; i++) {
					t_point dir_pnt = get_next_point();
					mvprintw(SCREEN_H/2+dir_pnt.y*i, SCREEN_W/2+dir_pnt.x*i*2, "+");
					int y = p.pos.y + dir_pnt.y*i;
					int x1 = p.pos.x + dir_pnt.x*i;
					int x2 = p.pos.x + dir_pnt.x*i*2;
					if (map[y][x1] == 'E') {
						map[y][x1] = '8';
						enemy_crossed_laser = 1;
						break;
					}
					if (map[y][x2] == 'E') {
						map[y][x2] = '8';
						enemy_crossed_laser = 1;
						break;
					}
				}
				if (enemy_crossed_laser == 1) {
					blink_blue();
				}
				break;
			case 'a':
				p.dir = WEST;
				break;
			case 'd':
				p.dir = EAST;
				break;
			case 'w':
				p.dir = NORTH;
				break;
			case 's':
				p.dir = SOUTH;
				break;
			case 'i':
				slide(NORTH);
				break;
			case 'k':
				slide(SOUTH);
				break;
			case 'j':
				slide(WEST);
				break;
			case 'l':
				slide(EAST);
				break;
			case 'q':
				//exit game
				endwin();
				return 0;
		}
		move_player();
		draw_screen();
		mvprintw(0, 0, "hp=%d, ch=%c, pos [x:%d, y:%d], time lapsed [%ld]", p.health, ch, p.pos.x, p.pos.y, time_taken);
	}
	clear();
	mvprintw(SCREEN_H/2, SCREEN_W/2, "Game over, press q");
	timeout(100500);
	while(1) {
		char ch = getch();
		if (ch == 'q') {
			break;
		}
	}
	endwin();
	return 0;
}

/*
 * BUGS, TODO
 * when shooted do blink with red and some ascii art (some word)
 * blinking coursour on other non iterm terms: try other terminal
 * make enemies bigger: 5x5 characters or so
 * make enemy shoot at least one type of enemy
 * make player also bigger (so it will support multiplayer in future easier)?
 * make sure it compiles with all flags
 * show usage help before start and ask to press s
 */
