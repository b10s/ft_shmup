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
	// text, background
	init_pair(1, COLOR_RED, COLOR_RED);
	init_pair(2, COLOR_BLUE, COLOR_BLUE);
}

char	map[MAP_H][MAP_W] = {};
int	enemies[MAP_H][MAP_W] = {};
long time_taken;

clock_t start;
t_player p;

// TODO: make better name
t_point get_next_step() {
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
	t_point delta = get_next_step();
	t_point new_position;
	new_position = p.cur_position;
	new_position.x += delta.x*2;
	new_position.y += delta.y;

	if (new_position.x < MAP_W - SCREEN_W/2 && new_position.x > SCREEN_W/2) {
		p.cur_position.x = new_position.x;
	}
	if (new_position.y < MAP_H - SCREEN_H/2 && new_position.y > SCREEN_H/2) {
		p.cur_position.y = new_position.y;
	}

	if (map[p.cur_position.y][p.cur_position.x] == 'E'
			|| map[p.cur_position.y][p.cur_position.x - delta.x] == 'E') {
		p.health--;
		blink_red();
	}
}

// a,d,w,s + ' ' 
// ijlk

void draw_enimy_laser(t_point enemy_pos, int x, int y) {
	int start_x = p.cur_position.x - SCREEN_W/2;
	int start_y = p.cur_position.y - SCREEN_H/2;
	// fire by laser
	for (int i = 1; i < LASER_RANGE; i++) {
		if (map[start_y + y + i][start_x + x] != 'W' && y + i < SCREEN_H) {
			if ((p.cur_position.x == enemy_pos.x
					&& p.cur_position.y == enemy_pos.y + i)
					|| (p.cur_position.x-1 == enemy_pos.x
					&& p.cur_position.y == enemy_pos.y + i)) {
				p.health--;
				blink_red();
			}
			mvprintw(y+i, x, "+");
		}
		if (map[start_y + y - i][start_x + x] != 'W') {
			if ((p.cur_position.x == enemy_pos.x
					&& p.cur_position.y == enemy_pos.y - i)
					|| (p.cur_position.x - 1 == enemy_pos.x
					&& p.cur_position.y == enemy_pos.y - i)) {
				p.health--;
				blink_red();
			}
			mvprintw(y-i, x, "+");
		}
	}
}

void draw_screen() {

	// TODO: slide - move with space: fire and move without changing the direction
	// draw current screen
	// draw player at the center and his direction
	
	int start_x = p.cur_position.x - SCREEN_W/2;
	int start_y = p.cur_position.y - SCREEN_H/2;
	if ( start_x < 0 ) {
		start_x = 0;
	}
	if (start_y < 0) {
		start_y = 0;
	}

	char c;
	for (int y = 0; y < SCREEN_H; y++) {
		for (int x = 0; x < SCREEN_W; x++) {
			// draw enemies, walls, and everything
			mvprintw(y, x, "%c", map[start_y + y][start_x + x]);
			c = map[start_y + y][start_x + x];
			// decide randomly shoot or not shoot
			// TODO: if eneymy type is S
			if (c == 'E') {
				t_point enemy_pos;
				enemy_pos.x = start_x + x;
				enemy_pos.y = start_y + y;
				// TODO: maybe we need real time clock and make decicion once a second
				// ENEMY SHOOTED
				// start counting 1 second to keep laser on
				// start counting 3 seconds to make sure enemy is not shooting (recharging)
				//if (player.frame % 100 < 25) {
				if (enemies[enemy_pos.y][enemy_pos.x] == 0) {
					if (rand() % 5 == 1) {
						enemies[enemy_pos.y][enemy_pos.x] = time_taken;
					} 
				} else if (enemies[enemy_pos.y][enemy_pos.x] > 0) {
					draw_enimy_laser(enemy_pos, x, y);
				} else if (enemies[enemy_pos.y][enemy_pos.x] < 0) {
					//recharging, need to wait 2-3 seconds
					if (time_taken - (-1 * enemies[enemy_pos.y][enemy_pos.x]) > 1) {
						enemies[enemy_pos.y][enemy_pos.x] = 0;
					}
				}
			}
		}
	}

	// draw player
	mvprintw(SCREEN_H/2, SCREEN_W/2, "o");
	t_point delta = get_next_step();
	mvprintw(SCREEN_H/2+delta.y, SCREEN_W/2+delta.x, "%c", "^>v<"[p.cur_direction]);
	
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
			if (p.cur_position.y - 1 > SCREEN_H/2) {
				p.cur_position.y--;
			}
			break;
		case EAST:
			if (p.cur_position.x + 1 < MAP_W - SCREEN_W/2) {
				p.cur_position.x++;
			}
			break;
		case SOUTH:
			if (p.cur_position.y + 1 < MAP_H - SCREEN_H/2) {
				p.cur_position.y++;
			}
			break;
		case WEST:
			if (p.cur_position.x - 1 > SCREEN_W/2) {
				p.cur_position.x--;
			}
			break;
	}
}


int	main() {
	init_ncurses();

	// Calculate the time taken in seconds
	start = clock();
	struct timeval start, end;
	gettimeofday(&start, NULL);

	p.health = 3;
	// type cast is ok?
	p.cur_position.x = MAP_W/2;
	p.cur_position.y = MAP_H/2;
	p.cur_direction = EAST;
	p.frame = 0;

	// tests
	///
	// enemy count is 120
	for (int i = 0; i < 120; i++) {
		map[rand()%MAP_H][rand()%MAP_W] = 'E';
	}
	map[26][52] = 'E';
	map[36][52] = 'E';
	map[46][52] = 'E';

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
		printw("hp=%d, ch=%c, pos [x:%d, y:%d], time lapsed [%ld]", p.health, ch, p.cur_position.x, p.cur_position.y, time_taken);
		int enemy_crossed_laser = 0;
		for (int y =0; y < MAP_H; y++) {
			for (int x =0; x < MAP_W; x++) {
				if (enemies[y][x] > 0) {
					if (time_taken - enemies[y][x] > 1) {
						enemies[y][x] = -time_taken;
					}
				}
			}
		}
		switch (ch) {
			case ' ':
				//fire by laser
				for (int i = 0; i < LASER_RANGE; i++) {
					t_point dir_pnt = get_next_step();
					mvprintw(SCREEN_H/2+dir_pnt.y*i, SCREEN_W/2+dir_pnt.x*i*2, "+");
					int y = p.cur_position.y + dir_pnt.y*i;
					int x1 = p.cur_position.x + dir_pnt.x*i;
					int x2 = p.cur_position.x + dir_pnt.x*i*2;
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
				p.cur_direction = WEST;
				break;
			case 'd':
				p.cur_direction = EAST;
				break;
			case 'w':
				p.cur_direction = NORTH;
				break;
			case 's':
				p.cur_direction = SOUTH;
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
		mvprintw(0, 0, "hp=%d, ch=%c, pos [x:%d, y:%d], time lapsed [%ld]", p.health, ch, p.cur_position.x, p.cur_position.y, time_taken);
	}
	clear();
	mvprintw(SCREEN_H/2, SCREEN_W/2, "Game over, press");
	timeout(100500);
	getch();
	endwin();
	return 0;
}

/*
 * BUGS, TODO
 * when shooted do blink with red and some ascii art (some word)
 * blinking coursour on other non iterm terms: try other terminal
 * make enemies bigger: 5x5 characters or so
 * create second type of enemy (one type will make -1 health, another type will make -2 health, rare enemy - one type can shoot, another can not)
 * make enemy shoot at least one type of enemy
 * make player also bigger (so it will support multiplayer in future easier)?
 * put player as global (replace player with p, cur_pos with pos, cur_direction with dir)
 * make sure it compiles with all flags
 * fix but when next to laser
 */
