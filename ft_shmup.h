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

typedef struct {
	int health;
	t_point pos;
	t_direction dir;
	int frame;
} t_player;

#define MAP_W 2000
#define MAP_H 100
#define SCREEN_H 50
#define SCREEN_W 100
#define LASER_RANGE 10

#define ENEMY_CNT 125
#define LASER_CNT 125
