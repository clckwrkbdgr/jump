/**
 * version 0.9.0
 * Ball jumps all over the screen, bouncing out of borders, until got tired.
 * Then it rest while power is recharging, then jump again in random direction.
 * Ball is represented by digit, that indicates current power charge.
 */

#include <ncurses.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>
#include <limits.h>

#define GRAVITY (9.81274) // g - the free fall acceleration constant.
#define QUIT_KEY ('q')
#define RND_ANGLE ((M_PI * 0.5) * (double)rand() / (double)RAND_MAX / 2 + (M_PI * 0.25)); // 45*..135*
#define DPOWER (0.2) // Power loss rate.
#define RECHARGE_RATE (2.0) // Power recharging rate.
#define DTIME (0.1) // Time increment. Ajusting it will cause system speed changing.

#define SPRITE_COUNT (10)
char sprite[SPRITE_COUNT + 1] = {'0', '1', '2', '3' ,'4', '5', '6', '7', '8' ,'9', '.'}; // Last one is for the trails and ignition.

#define BALL_COUNT 255

struct Ball {
	bool recharging;
	double power, maxPower;
	double x, y, vx, vy, angle;
};

void ball_init(struct Ball *ball, int maxX, int maxY) {
	ball->recharging = false;
	ball->power = 0.0;
	ball->maxPower = sqrt(2.0 * GRAVITY * (double)maxY); // Max power the ball should be thrown with to not overfly the ceiling.

	ball->x = 0.0;
	ball->y = 0.0; // Current position.
	ball->vx = 0.0;
	ball->vy = 0.0; // Starting velocity.
	ball->angle = 0.0; // Initial angle (used by reflect-jumping)

	ball->x = (double)(rand() % maxX);
	ball->y = (double)(rand() % maxY);
}

void ball_process(struct Ball *ball, int maxX, int maxY) {
	if(!ball->recharging) { // Moving.
		// Physics.
		ball->vy += -GRAVITY * DTIME;
		ball->x += ball->vx * DTIME;
		ball->y += ball->vy * DTIME;

		// Ball lose its power.
		if(ball->power > DPOWER)
			ball->power -= DPOWER;

		// Bouncing from the walls.
		if(ball->x > (double)maxX)
			ball->vx = - ball->vx;
		if(ball->x < 0.0)
			ball->vx = - ball->vx;

		// When tired, stop.
		if(ball->y < 0.0) {
			// If it has power yet, then jump again (reflect from floor).
			if(ball->power <= DPOWER) {
				ball->recharging = true;
				ball->y = 0.0;
				ball->vy = 0.0;
				ball->vx = 0.0;
			} else {
				ball->power -= DPOWER;

				// Calculate angle sign for current system state.
				if(cos(ball->angle) * ball->vx < 0) // Hor. direction of initial and current states are different.
					ball->angle = M_PI - ball->angle;

				ball->vx = ball->power * cos(ball->angle);
				ball->vy = ball->power * sin(ball->angle);
			}
		}

	} else { // Resting and recharging.
		ball->power += RECHARGE_RATE; // Collecting power.

		// When ready, jump.
		if(ball->power >= ball->maxPower) {
			ball->power = ball->maxPower;
			ball->recharging = false;

			// Pick random direction and jump with power.
			ball->angle = RND_ANGLE;
			ball->vx = ball->power * cos(ball->angle);
			ball->vy = ball->power * sin(ball->angle);
		}
	}
}

char getSprite(struct Ball *ball) {
	// Appearance: 0..9
	int index = (int)((double)SPRITE_COUNT * ball->power / ball->maxPower);
	return (index <= SPRITE_COUNT) ? sprite[index] : sprite[SPRITE_COUNT];
}

int main() {
	// Application init.
	initscr();
	cbreak();
	noecho();
	halfdelay(1);
	srand(time(NULL));

	int maxX, maxY;
	getmaxyx(stdscr, maxY, maxX);

	// Objects init.
	struct Ball ball[BALL_COUNT];
	int i;
	for(i = 0; i < BALL_COUNT; ++i) {
		ball_init(&ball[i], maxX, maxY);
	}

	// Loop.
	while(getch() != QUIT_KEY) {
		// Drawing.
		erase();
		for(i = 0; i < BALL_COUNT; ++i)
			mvaddch(maxY - 1 - (int)ball[i].y, (int)ball[i].x, getSprite(&ball[i]));
		mvaddch(maxY - 1, maxX - 1, ' ');
		refresh();

		// System.
		for(i = 0; i < BALL_COUNT; ++i)
			ball_process(&ball[i], maxX, maxY);
	}

	endwin();
	return 0;
}
