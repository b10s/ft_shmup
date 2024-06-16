#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <unistd.h>
#include <curses.h>
#include <time.h>
#include <string.h>

#include "./ft_shmup.h"


void game_over() {
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
}

__attribute__((destructor))
void	destructor(void) {
	char	s[42];
	snprintf(s, sizeof(s), "leaks -q %d", getpid());
	system(s);
}

void	init_ncurses() {
	initscr();
	noecho();
	curs_set(0);
	cbreak();
	timeout(42);
	start_color();
  // Define color pairs
	// color of text, color of background
  init_pair(1, COLOR_RED, COLOR_BLACK);
  init_pair(2, COLOR_GREEN, COLOR_BLACK);
  init_pair(3, COLOR_BLUE, COLOR_BLACK);
  init_pair(4, COLOR_RED, COLOR_RED);
  init_pair(5, COLOR_BLUE, COLOR_BLUE);
  init_pair(6, COLOR_BLACK, COLOR_BLACK);
}

char	map[MAP_H][MAP_W] = {};
int	enemy_state[MAP_H][MAP_W] = {};
long time_taken;

// E - Enemy
// L - Laser
char *enemy_list = "EL";

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

void bounce_player_out_of_laser() {
	switch (p.dir) {
		case NORTH:
			p.dir = SOUTH;
			break;
		case EAST:
			p.dir = WEST;
			break;
		case SOUTH:
			p.dir = NORTH;
			break;
		case WEST:
			p.dir = EAST;
			break;
	}
	t_point delta = get_next_point();
	p.pos.x += 3*delta.x;
	p.pos.y += 3*delta.y;
}

void draw_big_text(WINDOW *win, const char *text[], int start_y, int start_x, int color_pair) {
    for (int i = 0; text[i] != NULL; i++) {
        mvwprintw(win, start_y + i, start_x, "%s", text[i]);
        wattron(win, COLOR_PAIR(color_pair));
        mvwprintw(win, start_y + i, start_x, "%s", text[i]);
        wattroff(win, COLOR_PAIR(color_pair));
    }
}

void make_all_black() {
	// Set all positions to red
	for (int y = 0; y < SCREEN_H; y++) {
	    for (int x = 0; x < SCREEN_W; x++) {
	        mvaddch(y, x, ' ' | COLOR_PAIR(6));
	    }
	}
	refresh();
}

void blink_red() {
	// Set all positions to red
	for (int y = 0; y < SCREEN_H; y++) {
	    for (int x = 0; x < SCREEN_W; x++) {
	        mvaddch(y, x, ' ' | COLOR_PAIR(4));
	    }
	}
	refresh();
	usleep(100000);

	make_all_black();
  // Define big text (this is a simple example, you can create more complex designs)
	// https://patorjk.com/software/taag/#p=display&h=2&v=0&f=Basic&t=o%20o%20p%20s
  const char *big_text[] = {
"",
" .d88b.       .d88b.      d8888b.     .d8888. ",
".8P  Y8.     .8P  Y8.     88  `8D     88'  YP ",
"88    88     88    88     88oodD'     `8bo.   ",
"88    88     88    88     88~~~         `Y8b. ",
"`8b  d8'     `8b  d8'     88          db   8D ",
" `Y88P'       `Y88P'      88          `8888Y' ",
"",
      NULL
  };
  // Draw the big text with color
  draw_big_text(stdscr, big_text, SCREEN_H/2, SCREEN_W/2, 1);  // Using color pair 1 (red)

	if (p.health == 0) {
		game_over();
		return;
	}
	mvprintw(10, 10, "You have [%d] health points.", p.health);
	mvprintw(12, 10, "Press C to continue or Q to panic.");
	refresh();
	while(1) {
		char ch = getch();
		if (ch == 'c' || ch == 'C') {
			break;
		}
		if (ch == 'q' || ch == 'Q') {
			endwin();
			exit(1);
		}
	}
}

void blink_blue() {
	// Set all positions to red
	for (int y = 0; y < SCREEN_H; y++) {
	    for (int x = 0; x < SCREEN_W; x++) {
	        mvaddch(y, x, ' ' | COLOR_PAIR(5));
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


	// since we do two steps by X, we need to check both
	char step1 = map[p.pos.y][p.pos.x - delta.x];
	char step2 = map[p.pos.y][p.pos.x];
	if ( (step1 != 0 && strchr(enemy_list, step1) != NULL)
			|| (step2 != 0 && strchr(enemy_list, step2) != NULL) ) {
		p.health--;
		blink_red();
		bounce_player_out_of_laser();
	}
}

t_point find_laser_center(t_point laser) {
	t_point res;

	int y = laser.y;
	while(map[y][laser.x] == 'L') {
		y--;
	}
	y++;
	int x = laser.x;
	while(map[y][x] == 'L') {
		x--;
	}
	x++;

	res.x = x + 1;
	res.y = y + 1;
	return res;
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
				bounce_player_out_of_laser();
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
				bounce_player_out_of_laser();
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

				t_point laser_center = find_laser_center(abs_enemy_pos);
				t_point laser_upper_edge = laser_center;
				t_point laser_bottom_edge = laser_center;
				laser_upper_edge.y--;
				laser_bottom_edge.y++;

				int state = enemy_state[laser_center.y][laser_center.x];

				if (state == 0) {
					// decide randomly shoot or not shoot
					if (rand() % 5 == 1) {
						// ENEMY SHOOTED
						// start counting 1 second to keep laser on
						// start counting 3 seconds to make sure enemy is not shooting (recharging)
						enemy_state[laser_center.y][laser_center.x] = time_taken;
					} 
				// Laser is in switched on state, time is stored in seconds
				} else if (state > 0) {
					fire_laser(laser_center, rel_enemy_pos);
				// Laser is in cool down/recharging state, time is stored in seconds (negative)
				} else if (state < 0) {
					//recharging, need to wait 1+ seconds
					if (time_taken - (-1 * state) > 1) {
						enemy_state[laser_center.y][laser_center.x] = 0;
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

void kill_enemy(int en_x, int en_y) {
	char en = map[en_y][en_x];
	for (int y = en_y - 2; y <= en_y+2; y++) {
		for (int x = en_x - 2; x <= en_x+2; x++) {
			if (map[y][x] == en) {
				map[y][x] = '8';
			}
		}
	}
}

void put_big_enemy(int x, int y, char e) {
	map[y][x] = e;
	map[y][x-1] = e;
	map[y][x+1] = e;
	map[y-1][x] = e;
	map[y-1][x-1] = e;
	map[y-1][x+1] = e;
	map[y+1][x] = e;
	map[y+1][x-1] = e;
	map[y+1][x+1] = e;
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

	p.health = 33;
	// type cast is ok?
	p.pos.x = MAP_W/2;
	p.pos.y = MAP_H/2;
	p.dir = EAST;
	p.frame = 0;

	int x, y;
	for (int i = 0; i < LASER_CNT; i++) {
		y = rand() % MAP_H;
		x = rand() % MAP_W;

		// big enemy has born, ughrr
		if (x > 5 && y > 5 && x < MAP_W - 5 && y < MAP_H - 5 ) {
			put_big_enemy(x, y, 'L');
		} else {
			//small enemy
			map[y][x] = 'L';
		}

	}
	for (int i = 0; i < ENEMY_CNT; i++) {
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
		int enemy_killed = 0;
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
					t_point delta = get_next_point();
					mvprintw(SCREEN_H/2+delta.y*i, SCREEN_W/2+delta.x*i*2, "+");

					// since we do two steps by X, we need to check both
					char step1 = map[p.pos.y + delta.y * i][p.pos.x + delta.x*i];
					char step2 = map[p.pos.y + delta.y * i][p.pos.x + delta.x*i*2];
					if (step1 != 0 && strchr(enemy_list, step1) != NULL) {
						kill_enemy(p.pos.x + delta.x*i, p.pos.y + delta.y*i);
						//map[p.pos.y + delta.y*i][p.pos.x + delta.x*i] = '8';
						enemy_killed = 1;
						break;
					}
					if (step2 != 0 && strchr(enemy_list, step2) != NULL) {
						kill_enemy(p.pos.x +delta.x*i*2, p.pos.y + delta.y*i);
						//map[p.pos.y + delta.y*i][p.pos.x +delta.x*i*2] = '8';
						enemy_killed = 1;
						break;
					}
				}
				if (enemy_killed == 1) {
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
	game_over();
	return 0;
}

/*
 * TODO
 * when shooted do blink with red and some ascii art (some word)
 * blinking coursour on other non iterm terms: try other terminal
 * make enemies bigger: 5x5 characters or so
 * make enemy shoot at least one type of enemy
 * make player also bigger (so it will support multiplayer in future easier)?
 * make sure it compiles with all flags
 * show usage help before start and ask to press s
 * make sure when we slide we also can hit the enemy
 * make big laser to shoot only one laser from middle
 * when kill laser kill adjustent
 */
