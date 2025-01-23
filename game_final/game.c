#include <stdint.h>	   // Enable use of uintX_t
#include <pic32mx.h>   // Enable use of chipkit specific macros
#include "gamedata.h"  // Enable access to game data
#include "rendering.h" // Enable access to rendering functions
#include "game.h"	   // Link with game header file
#include "main.h"

#define T2IF (IFS(0) >> 8) & 1	  // value of timer 2 interrupt flag
#define RST_T2IF IFS(0) &= ~0x100 // reset timer 2 interrupt flag

#define BTN4 (PORTD >> 7) & 1		   // value of bit corresponding to button 4
#define BTN3 (PORTD >> 6) & 1		   // value of bit corresponding to button 3
#define BTN2 (PORTD >> 5) & 1		   // value of bit corresponding to button 2
#define BTN1 (PORTF >> 1) & 1		   // value of bit corresponding to button 1
#define BTNS BTN1 | BTN2 | BTN3 | BTN4 // wether a button is being pressed

#define SW4 (PORTD >> 11) & 1

uint8_t bf[3] = {0, 0, 0}; // flags for button 2, 3 & 4 (allows button presses to be "edge triggered")
uint8_t bf1 = 0;		   // flag for button 1 (allows button presses to be "edge triggered")
// list of highscores
uint32_t highscore_list[5][5] = {
	{0, 0, 0, 0, 0},
	{1, 1, 1, 1, 0},
	{2, 2, 2, 2, 0},
	{3, 3, 3, 3, 0},
	{4, 4, 4, 4, 0}};
int8_t highscore_to_beat; // index of the highscore the player currently attempts to beat

uint32_t current_score; // current score
uint32_t high_score;	// high score

uint8_t time_out_counter;		// timeout counter
uint8_t time_out_value;			// timeout value
uint8_t speed_increase_counter; // speed increase counter
uint8_t speed_increase_value;	// speed increase value

uint8_t pos_x, pos_y, offset, new_pos_x, new_pos_y;
int8_t move_x = 0;
uint8_t move_y = 0;
uint8_t rotation = 0;
uint8_t figure_type, next_figure_type;
int8_t completed_rows[4];

void select_shape(void);
static void update_current_figure(void);
void add_figure_to_screen_field(void);
void increase_score(uint8_t);
void game();

/**
 * Calls all the necessarry functions for rendering a frame
 * @author Olle Jernström
 */
static void render_frame()
{
	render_playing_field();
	update_scores(current_score, 0);
	update_scores(high_score, 1);
	render_scores_and_next_figure();
}

/**
 * A simple pseudo random number generator returning a number between 0-6
 * that does not allow two numbers to the same in a row
 * @author Olle Jernström
 */
static uint8_t random(uint8_t n)
{
	uint8_t rn;
	rn = (TMR2 + time_out_counter ^ current_score) % 7; // mainly uses the "random" value of TMR2
	if (rn == n)
		rn = (rn + 1) % 7;
	return rn;
}

/**
 * Sets up game variables and displays the start screen
 * @author Olle Jernström
 */
static void game_start(uint8_t start_with_highscore)
{
	uint8_t show_highscore_list = start_with_highscore; // show highscore list
	uint8_t b = 0;										// blink
	uint8_t c = 0;										// counter
	bf[2] = 1;											// set button 4 flag to 1

	display_init(); // initialize the display

	// set score and highscore
	highscore_to_beat = 4;
	while (!highscore_list[highscore_to_beat][4] && highscore_to_beat > 0)
		highscore_to_beat--;
	high_score = highscore_list[highscore_to_beat][4];
	current_score = 0;

	// set timing variables
	time_out_counter = 0;
	time_out_value = 10;
	speed_increase_counter = 0;
	speed_increase_value = 30;

	int blink_time = 8;

	while (1)
	{ // loop until player presses button 4
		if (T2IF)
		{
			// control blinking
			if (++c % blink_time == 0)
				b = b ? 0 : 1;

			// render_frame the correct screen
			if (show_highscore_list)
				render_highscores(highscore_list);
			else
				render_start_screen(b, c);

			// reset the counter if it becomes 8
			if (c == blink_time)
				c = 0;
			RST_T2IF;
		}
		// check if button 4 press should return to start screen or start game
		if (BTN4 && !bf[2])
		{
			bf[2] = 1;
			if (show_highscore_list)
				show_highscore_list = 0;
			else
				break;
		}
		// reset button 4 flag
		else if (!BTN4 && bf[2])
			bf[2] = 0;

		// check if highscore should be displayed instead of start screen
		if (BTN2 && !show_highscore_list)
		{
			show_highscore_list = 1;
		}
	}

	select_shape();			 // randomize a new next
	update_current_figure(); // set current as next
	select_shape();			 // randomize a new next
	add_figure_to_screen_field();
	render_frame(); // render_frame the play field
}

/**
 * Initializes the game
 * @author Olle Jernström
 */
void game_init(void)
{
	TRISFSET = 1;	 // configure PORTF bit 1 to be input (button 1)
	TRISDSET = 0xe0; // configure PORTD bits 7-5 to be input (buttons 2-4)

	TMR2 = 0;		   // make sure timer starts at 0
	PR2 = 31250;	   // 80MHz / 10 (desired frequency) / 256 (prescale)
	T2CONSET = 0x70;   // Set prescale to 256
	T2CONSET = 0x8000; // enable timer 2

	game_start(0); // set game to start state
}

/**
 * Updates the current figure to be what next figure is
 * @author Olle Jernström
 */
static void update_current_figure(void)
{
	uint8_t i, j;
	for (i = 0; i < 2; i++)
		for (j = 0; j < 4; j++)
			current[i][j] = next[i][j];

	for (i = 2; i < 4; i++)
		for (j = 0; j < 4; j++)
			current[i][j] = 0;
	// update movement vars and what num current figure is
	pos_x = new_pos_x;
	pos_y = new_pos_y;
	rotation = 0;
	figure_type = next_figure_type;
}

/**
 * Randomizes the next figure
 * it also gives the new x and y values for the next figure
 * @author Olle Jernström
 */
void select_shape(void)
{
	next_figure_type = random(figure_type);
	move_y = 0;
	move_x = 0;
	offset = 2;

	if (next_figure_type == 0) // I
	{
		new_pos_y = 1;
		new_pos_x = 4;
		next[0][0] = 1;
		next[0][1] = 1;
		next[0][2] = 1;
		next[0][3] = 1;
		next[1][0] = 0;
		next[1][1] = 0;
		next[1][2] = 0;
		next[1][3] = 0;
	}
	else if (next_figure_type == 1) // J
	{
		new_pos_y = 2;
		new_pos_x = 3;
		next[0][0] = 1;
		next[0][1] = 0;
		next[0][2] = 0;
		next[0][3] = 0;
		next[1][0] = 1;
		next[1][1] = 1;
		next[1][2] = 1;
		next[1][3] = 0;
	}
	else if (next_figure_type == 2) // L
	{
		new_pos_y = 2;
		new_pos_x = 3;
		next[0][0] = 0;
		next[0][1] = 0;
		next[0][2] = 1;
		next[0][3] = 0;
		next[1][0] = 1;
		next[1][1] = 1;
		next[1][2] = 1;
		next[1][3] = 0;
	}
	else if (next_figure_type == 3) // O
	{
		new_pos_y = 2;
		new_pos_x = 2;
		next[0][0] = 1;
		next[0][1] = 1;
		next[0][2] = 0;
		next[0][3] = 0;
		next[1][0] = 1;
		next[1][1] = 1;
		next[1][2] = 0;
		next[1][3] = 0;
	}
	else if (next_figure_type == 4) // S
	{
		new_pos_y = 2;
		new_pos_x = 3;
		next[0][0] = 0;
		next[0][1] = 1;
		next[0][2] = 1;
		next[0][3] = 0;
		next[1][0] = 1;
		next[1][1] = 1;
		next[1][2] = 0;
		next[1][3] = 0;
	}
	else if (next_figure_type == 5) // T
	{
		new_pos_y = 2;
		new_pos_x = 3;
		next[0][0] = 1;
		next[0][1] = 1;
		next[0][2] = 1;
		next[0][3] = 0;
		next[1][0] = 0;
		next[1][1] = 1;
		next[1][2] = 0;
		next[1][3] = 0;
	}
	else if (next_figure_type == 6) // Z
	{
		new_pos_y = 2;
		new_pos_x = 3;
		next[0][0] = 1;
		next[0][1] = 1;
		next[0][2] = 0;
		next[0][3] = 0;
		next[1][0] = 0;
		next[1][1] = 1;
		next[1][2] = 1;
		next[1][3] = 0;
	}
}

/**
 * The method adds our figure to the display (the field)
 * @author Olle Jernström
 */
void add_figure_to_screen_field(void)
{
	uint8_t i, j;
	for (i = 0; i < pos_y; i++)
	{
		for (j = 0; j < pos_x; j++)
		{
			if (current[i][j] == 1)
				field[i + move_y][j + offset + move_x] = 1;
		}
	}
}

/**
 * The method removes our figure from the display (the field)
 * @author Olle Jernström
 */
void remove_figure_from_screen_field(void)
{
	uint8_t i, j;
	for (i = 0; i < pos_y; i++)
	{
		for (j = 0; j < pos_x; j++)
		{
			if (current[i][j] == 1)
				field[i + move_y][j + offset + move_x] = 0;
		}
	}
}

/**
 * Check path down. (if it is possible to move down in the y direction)
 * @author Olle Jernström
 */
uint8_t check_if_move_possible_down(void)
{
	uint8_t i, j;
	if (pos_y + move_y > 23)
		return 0;

	for (i = 0; i < pos_y; i++)
	{
		for (j = 0; j < pos_x; j++)
		{
			if (field[pos_y + move_y - i][j + offset + move_x] == 1 && current[pos_y - i - 1][j] == 1)
			{
				return 0;
			}
		}
	}
	return 1;
}

/**
 * Check path right. (if it is possible to move right in the x direction)
 * @author Olle Jernström
 */
uint8_t check_if_move_possible_right()
{
	uint8_t i, j;
	if (pos_x + offset + move_x >= 8)
		return 0;
	for (i = 0; i < pos_y; i++)
	{
		for (j = 0; j < pos_x; j++)
		{
			if (field[pos_y - 1 + move_y - i][pos_x + offset + move_x] == 1 && current[pos_y - 1 - i][pos_x - 1] == 1)
				return 0;
		}
	}
	return 1;
}

/**
 * Check path left. (if it is possible to move left in the x direction)
 * @author Olle Jernström
 */
uint8_t check_if_move_possible_left()
{
	uint8_t i, j;
	if (offset + move_x <= 0)
		return 0;
	for (i = 0; i < pos_y; i++)
	{
		for (j = 0; j < pos_x; j++)
		{
			if (field[pos_y - 1 + move_y - i][offset + move_x - 1] == 1 && current[pos_y - 1 - i][0] == 1)
				return 0;
		}
	}
	return 1;
}

/**
 * Check if you can slam
 * @author Olle Jernström
 */
uint8_t check_if_slam_possible()
{
	return 1;
}

// Metoden är till för att rotera vår figur 90 grader
// såsom figuren ska göra enligt tetris regler
void rotate_figure()
{
	rotation++;
	if (figure_type == 0)
	{
		rotation %= 2;
		if (rotation == 0)
		{
			pos_y = 1;
			pos_x = 4;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 1;
			current[0][3] = 1;
			current[1][0] = 0;
			current[1][1] = 0;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 4;
			pos_x = 1;
			current[0][0] = 1;
			current[0][1] = 0;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 0;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 1;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
	else if (figure_type == 1)
	{
		rotation %= 4;
		if (rotation == 0)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 1;
			current[0][1] = 0;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 1;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 3)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 0;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 0;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 1;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 0;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 2)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 1;
			current[0][3] = 0;
			current[1][0] = 0;
			current[1][1] = 0;
			current[1][2] = 1;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
	else if (figure_type == 2)
	{
		rotation %= 4;
		if (rotation == 0)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 0;
			current[0][1] = 0;
			current[0][2] = 1;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 1;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 3)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 0;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 1;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 2)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 1;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 0;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 1;
			current[0][1] = 0;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 0;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 1;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
	else if (figure_type == 3)
	{
		pos_y = 2;
		pos_x = 2;
		current[0][0] = 1;
		current[0][1] = 1;
		current[0][2] = 0;
		current[0][3] = 0;
		current[1][0] = 1;
		current[1][1] = 1;
		current[1][2] = 0;
		current[1][3] = 0;
		current[2][0] = 0;
		current[2][1] = 0;
		current[2][2] = 0;
		current[2][3] = 0;
		current[3][0] = 0;
		current[3][1] = 0;
		current[3][2] = 0;
		current[3][3] = 0;
	}
	else if (figure_type == 4)
	{
		rotation %= 2;
		if (rotation == 0)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 0;
			current[0][1] = 1;
			current[0][2] = 1;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 1;
			current[0][1] = 0;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 1;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
	else if (figure_type == 5)
	{
		rotation %= 4;
		if (rotation == 0)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 1;
			current[0][3] = 0;
			current[1][0] = 0;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 0;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 1;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 2)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 0;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 1;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 3)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 1;
			current[0][1] = 0;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
	else if (figure_type == 6)
	{
		rotation %= 2;
		if (rotation == 0)
		{
			pos_y = 2;
			pos_x = 3;
			current[0][0] = 1;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 0;
			current[1][1] = 1;
			current[1][2] = 1;
			current[1][3] = 0;
			current[2][0] = 0;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
		else if (rotation == 1)
		{
			pos_y = 3;
			pos_x = 2;
			current[0][0] = 0;
			current[0][1] = 1;
			current[0][2] = 0;
			current[0][3] = 0;
			current[1][0] = 1;
			current[1][1] = 1;
			current[1][2] = 0;
			current[1][3] = 0;
			current[2][0] = 1;
			current[2][1] = 0;
			current[2][2] = 0;
			current[2][3] = 0;
			current[3][0] = 0;
			current[3][1] = 0;
			current[3][2] = 0;
			current[3][3] = 0;
		}
	}
}

/**
 * Checks if the current figure can be rotated
 * @author Olle Jernström
 */
uint8_t check_rotate()
{
	uint8_t m, tx, ty;
	if (figure_type == 0)
	{
		m = (rotation + 1) % 2;
		if (m == 0)
		{
			tx = pos_x;
			ty = pos_y;
			pos_x = 4;
			pos_y = 1;

			if ((-pos_y + move_y + 1 >= 0) && (pos_y + move_y + 1 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
		else if (m == 1)
		{
			tx = pos_x;
			ty = pos_y;
			pos_x = 1;
			pos_y = 4;

			if ((-pos_y + move_y + 1 >= 0) && (pos_y + move_y + 1 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
	}
	if (figure_type == 1 || figure_type == 2 || figure_type == 5)
	{
		m = (rotation + 1) % 4;
		if (m == 0 || m == 2)
		{
			tx = pos_x;
			ty = pos_y;
			pos_y = 2;
			pos_x = 3;

			if ((-pos_y + move_y + 2 >= 0) && (pos_y + move_y + 2 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
		else if (m == 1 || m == 3)
		{
			tx = pos_x;
			ty = pos_y;
			pos_y = 3;
			pos_x = 2;

			if ((-pos_y + move_y + 2 >= 0) && (pos_y + move_y + 2 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
	}
	if (figure_type == 3)
		return 1;

	if (figure_type == 4 || figure_type == 6)
	{
		m = (rotation + 1) % 2;
		if (m == 0)
		{
			tx = pos_x;
			ty = pos_y;
			pos_x = 3;
			pos_y = 2;

			if ((-pos_y + move_y + 2 >= 0) && (pos_y + move_y + 2 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
		else if (m == 1)
		{
			tx = pos_x;
			ty = pos_y;
			pos_x = 2;
			pos_y = 3;

			if ((-pos_y + move_y + 2 >= 0) && (pos_y + move_y + 2 <= 24))
			{
				if ((offset + move_x >= 0) && (pos_x + offset + move_x <= 8))
				{
					pos_x = tx;
					pos_y = ty;
					return 1;
				}
			}
			pos_x = tx;
			pos_y = ty;
			return 0;
		}
	}
}

/**
 * Checks if there are any full rows.
 * If thats the case they are stored so that they can be removed
 * by remove_checked_completed_rows
 * @author Olle Jernström
 */
void check_if_completed_rows_exist()
{
	uint8_t i, j;
	uint8_t check = 0;
	uint8_t index = 0;
	for (i = 0; i < 4; i++)
		completed_rows[i] = -1;

	for (i = 0; i < 24; i++)
	{
		for (j = 0; j < 8; j++)
		{
			if (field[i][j] == 1)
				check = 1;
			else
				check = 0;

			if (check == 0)
				break;
		}
		if (check == 1)
			completed_rows[index++] = i;
	}
}

/**
 * Checks if there are any full rows.
 * If thats the case they are stored so that they can be removed
 * by remove_checked_completed_rows
 * @author Olle Jernström
 */
void remove_checked_completed_rows()
{
	uint8_t k, j, h;
	uint8_t i = 0;

	while (completed_rows[i] != -1 && i < 4)
	{
		render_animation_down(0, completed_rows[i], 0, 8);
		for (k = completed_rows[i]; k > 0; k--)
		{
			for (j = 0; j < 8; j++)
			{
				field[k][j] = field[k - 1][j];
			}
		}

		for (h = 0; h < 8; h++)
		{
			field[0][h] = 0;
		}
		i++;
		increase_score(10);
	}
}

/**
 * Increases the score with a value
 * @param value The value to increase the score with
 * @author Olle Jernström
 */
void increase_score(uint8_t value)
{
	current_score += value;
}

/**
 * Updates the highscore to the current score
 * @author Olle Jernström
 */
void update_highscore_to_current_score()
{
	high_score = current_score;
}

/**
 * Updates the highscore list with the current score
 * Use three letter together with highscore to save it
 * in our list of highscores.
 * Also makes sure that the lowest highscore is removed
 * and everything under the new highscore is "moved down" one step
 * @author Olle Jernström
 */
void new_highscore()
{
	uint8_t i, j;
	uint8_t ltr[4] = {0, 0, 0, 0};
	int8_t ltr_ctr = 0;
	bf[2] = 1;

	while (1)
	{
		if (BTN2 && !bf[0])
		{
			bf[0] = 1;
			ltr[ltr_ctr] = ltr[ltr_ctr] == 0 ? 25 : ltr[ltr_ctr] - 1;
		}
		else if (!BTN2 && bf[0])
			bf[0] = 0;

		if (BTN3 && !bf[1])
		{
			bf[1] = 1;
			ltr[ltr_ctr] = ltr[ltr_ctr] == 25 ? 0 : ltr[ltr_ctr] + 1;
		}
		else if (!BTN3 && bf[1])
			bf[1] = 0;

		if (BTN4 && !bf[2])
		{
			bf[2] = 1;
			ltr_ctr = ltr_ctr == 3 ? 0 : ltr_ctr + 1;
		}
		else if (!BTN4 && bf[2])
			bf[2] = 0;

		// if button 1 is pressed decrease the current letter
		if (BTN1)
			break;

		render_name_selection_for_new_highscore(ltr, ltr_ctr);
	}

	for (i = 5; i-- < 0;)
	{
		if (highscore_list[highscore_to_beat][4] < current_score)
			highscore_to_beat--;
		else
			break;
	}

	for (i = 4; i > highscore_to_beat + 1; i--)
	{
		for (j = 0; j < 5; j++)
		{
			highscore_list[i][j] = highscore_list[i - 1][j];
		}
	}

	for (i = 0; i < 4; i++)
		highscore_list[highscore_to_beat + 1][i] = ltr[i];

	highscore_list[highscore_to_beat + 1][4] = current_score;

	highscore_to_beat = 4;
}

// ska kolla om figuren hamnar utanför spelplanen och därmed ska spelet avslutas
uint8_t check_game_over()
{
	uint8_t i, j;
	for (i = 0; i < pos_y; i++)
		for (j = offset; j < (pos_x + offset); j++)
			if (field[i][j] == 1 && current[i][j - offset] == 1)
				return 1;
	return 0;
}

/**
 * Game is over
 * @author Olle Jernström
 */
static void game_over()
{
	uint8_t i, j; // iteration variables
	uint8_t new_highscore_added = 0;

	render_frame(); // render_frame playing field
	while (!BTN4)
		; // halt til button 4 is pressed

	if (highscore_to_beat < 4)
	{ // check if a new highscore should be added

		new_highscore();
		new_highscore_added = 1;
	}

	// reset field
	for (i = 0; i < 24; i++)
		for (j = 0; j < 8; j++)
			field[i][j] = 0;

	game_start(new_highscore_added); // go to highscore screen
}

/**
 * Gets called non-stop from the main function and one call corresponds to one frame
 * @author Marcus Bardvall & Olle Jernström
 */
void game(void)
{
	// Input specific logic is handled below
	if (BTNS)
	{
		remove_figure_from_screen_field();

		// Speed the figure down while the button is pressed
		if (BTN1)
		{
			if (check_if_move_possible_down())
			{
				add_figure_to_screen_field();
				render_animation_down(move_y, pos_y + move_y, move_x + offset, pos_x + move_x + offset);
				remove_figure_from_screen_field();
				move_y++;
			}
		}

		// Rotate the figure if possible
		if (BTN2 && !bf[0])
		{
			bf[0] = 1;
			if (check_if_move_possible_down() && check_rotate())
				rotate_figure();
		}

		// Move the figure left if possible
		if (BTN3 && !bf[1])
		{
			bf[1] = 1;
			if (check_if_move_possible_left())
			{
				add_figure_to_screen_field();
				render_animation_left(move_y, pos_y + move_y, move_x + offset, pos_x + move_x + offset);
				remove_figure_from_screen_field();
				move_x--;
			}
		}

		// Move the figure right if possible
		if (BTN4 && !bf[2])
		{
			bf[2] = 1;
			if (check_if_move_possible_right())
			{
				add_figure_to_screen_field();
				render_animation_right(move_y, pos_y + move_y, move_x + offset, pos_x + move_x + offset);
				remove_figure_from_screen_field();
				move_x++;
			}
		}

		add_figure_to_screen_field();
		render_playing_field();
	}

	if (SW4)
	{
		// reset field
		uint8_t i, j;
		for (i = 0; i < 24; i++)
			for (j = 0; j < 8; j++)
				field[i][j] = 0;
		game_start(0);
	}

	// reset button flags when a button is no longer pressed
	if (!BTN2 && bf[0])
		bf[0] = 0;
	if (!BTN3 && bf[1])
		bf[1] = 0;
	if (!BTN4 && bf[2])
		bf[2] = 0;
	if (!BTN1 && bf1)
		bf1 = 0;

	if (T2IF) // only move a block when timer 2 has called for an interrupt AND time out counter equals time out value
	{

		if (++time_out_counter == time_out_value)
		{
			remove_figure_from_screen_field();
			// move block down if possible
			if (check_if_move_possible_down())
			{
				add_figure_to_screen_field();
				render_animation_down(move_y, pos_y + move_y, move_x + offset, pos_x + move_x + offset);
				remove_figure_from_screen_field();
				move_y++;
				add_figure_to_screen_field();
			}
			else
			{
				add_figure_to_screen_field();

				increase_score(5);

				// remove all completed rows
				check_if_completed_rows_exist();
				remove_checked_completed_rows();

				// update highscore...
				if (current_score > high_score)
				{
					// ... to current score if all highscores have been beaten
					if (highscore_to_beat < 0)
					{
						update_highscore_to_current_score();
					}
					// ... next highscore to beat
					else
					{
						highscore_to_beat--;
						if (highscore_to_beat >= 0)
							high_score = highscore_list[highscore_to_beat][4];
						else
							update_highscore_to_current_score();
					}
				}

				update_current_figure(); // current becomes next

				// check if game is over
				if (check_game_over())
				{
					game_over();
					RST_T2IF;
					return;
				}

				select_shape(); // update next with new shape
			}
			render_frame();

			// increase game speed if speed increase counter >= speed increase value
			if (++speed_increase_counter >= speed_increase_value)
			{
				time_out_value = time_out_value == 1 ? 1 : time_out_value - 1;
				speed_increase_counter = 0;
			}
			time_out_counter = 0; // reset time out counter
		}
		RST_T2IF;
	}
}
