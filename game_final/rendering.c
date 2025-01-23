#include <stdint.h>    // Enable use of uintX_t
#include <pic32mx.h>   // Enable use of chipkit specific macros
#include "display.h"   // Enable communication with the display
#include "gamedata.h"  // Enable access to game data
#include "rendering.h" // Link with rendering header file

/**
 * 2D-array containing pixels of an animation
 * @author Olle Jernström
 */
uint8_t anim[96][32] = {};

/**
 * 2D-array containing nibbles for number rendering
 * @author Olle Jernström
 */
const uint8_t const numbers[10][9] = {
    {14, 10, 10, 10, 10, 10, 10, 10, 14},
    {4, 4, 4, 4, 4, 4, 4, 4, 4},
    {14, 8, 8, 8, 14, 2, 2, 2, 14},
    {14, 8, 8, 8, 14, 8, 8, 8, 14},
    {10, 10, 10, 10, 14, 8, 8, 8, 8},
    {14, 2, 2, 2, 14, 8, 8, 8, 14},
    {14, 2, 2, 2, 14, 10, 10, 10, 14},
    {14, 8, 8, 8, 8, 8, 8, 8, 8},
    {14, 10, 10, 10, 14, 10, 10, 10, 14},
    {14, 10, 10, 10, 14, 8, 8, 8, 14}};

/**
 * 2D-array containing bytes for letter rendering
 * @author Olle Jernström
 */
const uint8_t const letters[26][8] = {
    {0x0F, 0x09, 0x09, 0x0F, 0x09, 0x09, 0x09, 4},
    {0x07, 0x09, 0x09, 0x07, 0x09, 0x09, 0x07, 4},
    {0x0F, 0x09, 0x01, 0x01, 0x01, 0x09, 0x0F, 4},
    {0x07, 0x09, 0x09, 0x09, 0x09, 0x09, 0x07, 4},
    {0x0F, 0x01, 0x01, 0x07, 0x01, 0x01, 0x0F, 4},
    {0x0F, 0x01, 0x01, 0x07, 0x01, 0x01, 0x01, 4},
    {0x0F, 0x09, 0x01, 0x01, 0x0D, 0x09, 0x0F, 4},
    {0x09, 0x09, 0x09, 0x0F, 0x09, 0x09, 0x09, 4},
    {0x1C, 0x08, 0x08, 0x08, 0x08, 0x08, 0x1C, 3},
    {0x0E, 0x04, 0x04, 0x04, 0x04, 0x05, 0x07, 4},
    {0x09, 0x0D, 0x05, 0x03, 0x05, 0x0D, 0x09, 4},
    {0x01, 0x01, 0x01, 0x01, 0x01, 0x01, 0x0F, 4},
    {0x63, 0x63, 0x55, 0x55, 0x55, 0x49, 0x49, 7},
    {0x13, 0x13, 0x15, 0x15, 0x15, 0x19, 0x19, 5},
    {0x06, 0x09, 0x09, 0x09, 0x09, 0x09, 0x06, 4},
    {0x0F, 0x09, 0x09, 0x0F, 0x01, 0x01, 0x01, 4},
    {0x06, 0x09, 0x09, 0x09, 0x0D, 0x0D, 0x16, 5},
    {0x07, 0x09, 0x09, 0x07, 0x09, 0x09, 0x09, 4},
    {0x0F, 0x09, 0x01, 0x0F, 0x08, 0x09, 0x0F, 4},
    {0x1F, 0x04, 0x04, 0x04, 0x04, 0x04, 0x04, 5},
    {0x09, 0x09, 0x09, 0x09, 0x09, 0x09, 0x0F, 4},
    {0x11, 0x11, 0x11, 0x11, 0x0A, 0x0A, 0x04, 5},
    {0x41, 0x41, 0x41, 0x49, 0x2A, 0x2A, 0x14, 7},
    {0x11, 0x11, 0x0A, 0x04, 0x0A, 0x11, 0x11, 5},
    {0x11, 0x11, 0x11, 0x1F, 0x04, 0x04, 0x04, 5},
    {0x1F, 0x10, 0x08, 0x04, 0x02, 0x01, 0x1F, 5}};

/**
 * 2D-array containing bytes for logo rendering
 * @author Olle Jernström
 */
const uint8_t const logo[7][4] = {
    {0xDF, 0xFB, 0x8E, 0xF3},
    {0x44, 0x20, 0x12, 0x91},
    {0x44, 0x20, 0x12, 0x11},
    {0xC4, 0x21, 0x0E, 0xF1},
    {0x44, 0x20, 0x12, 0x81},
    {0x44, 0x20, 0x12, 0x91},
    {0xC4, 0x23, 0x92, 0xF3}};

/**
 * 2D-array containing bytes for highscore rendering
 * @author Olle Jernström
 */
const uint8_t const hisc[7][4] = {
    {0x20, 0x1D, 0xB8, 0x07},
    {0x20, 0x09, 0xA8, 0x04},
    {0x20, 0x09, 0x88, 0x00},
    {0xE0, 0xC9, 0xBB, 0x00},
    {0x20, 0x09, 0xA0, 0x00},
    {0x20, 0x09, 0xA8, 0x04},
    {0x20, 0x1D, 0xB8, 0x07}};

/**
 * 2D-array containing bytes for "press to play" rendering
 * @author Olle Jernström
 */
const uint8_t const ptp[71][4] = {
    {0xF0, 0xCE, 0x7B, 0x0F},
    {0x90, 0x52, 0x48, 0x09},
    {0x90, 0x52, 0x08, 0x01},
    {0xF0, 0xCE, 0x79, 0x0F},
    {0x10, 0x52, 0x40, 0x08},
    {0x10, 0x52, 0x48, 0x09},
    {0x10, 0xD2, 0x7B, 0x0F},
    {0x00, 0x00, 0x00, 0x00},
    {0xE0, 0x7C, 0x53, 0x02},
    {0x20, 0x11, 0x53, 0x02},
    {0x20, 0x11, 0x55, 0x02},
    {0xE0, 0x10, 0xD5, 0x03},
    {0x20, 0x11, 0x15, 0x02},
    {0x20, 0x11, 0x19, 0x02},
    {0xE0, 0x10, 0x19, 0x02},
    {0x00, 0x00, 0x00, 0x00},
    {0x00, 0xF8, 0x1E, 0x00},
    {0x00, 0x20, 0x12, 0x00},
    {0x00, 0x20, 0x12, 0x00},
    {0x00, 0x20, 0x12, 0x00},
    {0x00, 0x20, 0x12, 0x00},
    {0x00, 0x20, 0x12, 0x00},
    {0x00, 0x20, 0x1E, 0x00},
    {0x00, 0x00, 0x00, 0x00},
    {0x78, 0xDF, 0x3B, 0x1F},
    {0x48, 0x44, 0x4A, 0x04},
    {0x08, 0x44, 0x4A, 0x04},
    {0x78, 0xC4, 0x3B, 0x04},
    {0x40, 0x44, 0x4A, 0x04},
    {0x48, 0x44, 0x4A, 0x04},
    {0x78, 0x44, 0x4A, 0x04},
    {0x00, 0x00, 0x00, 0x00},
    {0x00, 0x78, 0x07, 0x00},
    {0x00, 0x48, 0x09, 0x00},
    {0x00, 0x48, 0x09, 0x00},
    {0x00, 0x48, 0x07, 0x00},
    {0x00, 0x48, 0x09, 0x00},
    {0x00, 0x48, 0x09, 0x00},
    {0x00, 0x78, 0x09, 0x00},
    {0x00, 0x00, 0x00, 0x00},
    {0x70, 0xFE, 0xA6, 0x07},
    {0x90, 0x10, 0x26, 0x04},
    {0x90, 0x10, 0x2A, 0x04},
    {0x70, 0x10, 0xAA, 0x07},
    {0x90, 0x10, 0xAA, 0x00},
    {0x90, 0x10, 0xB2, 0x00},
    {0x70, 0x10, 0xB2, 0x07},
    {0x00, 0x00, 0x00, 0x00},
    {0x00, 0xDE, 0x3B, 0x00},
    {0x00, 0x42, 0x4A, 0x00},
    {0x00, 0x42, 0x4A, 0x00},
    {0x00, 0x4E, 0x3A, 0x00},
    {0x00, 0x42, 0x4A, 0x00},
    {0x00, 0x42, 0x4A, 0x00},
    {0x00, 0xC2, 0x4B, 0x00},
    {0x00, 0x00, 0x00, 0x00},
    {0x90, 0xEE, 0x25, 0x00},
    {0x90, 0x24, 0x25, 0x00},
    {0x90, 0x24, 0x24, 0x00},
    {0xF0, 0x24, 0xBD, 0x07},
    {0x90, 0x24, 0x25, 0x00},
    {0x90, 0x24, 0x25, 0x00},
    {0x90, 0xEE, 0x25, 0x00},
    {0x00, 0x00, 0x00, 0x00},
    {0xF0, 0xDE, 0x3B, 0x0F},
    {0x90, 0x52, 0x4A, 0x01},
    {0x10, 0x42, 0x4A, 0x01},
    {0xF0, 0x42, 0x3A, 0x07},
    {0x80, 0x42, 0x4A, 0x01},
    {0x90, 0x52, 0x4A, 0x01},
    {0xF0, 0xDE, 0x4B, 0x0F}};

/**
 * Sets up the display with neccessary commands before rendering can occur
 * NOTE: Borrowed from labs!
 * @author F Lundevall & Axel Isaksson
 */
static void setup_screen(uint8_t c, uint8_t s)
{
    PORTFCLR = 0x10;
    spi_send_recv(0x22);
    spi_send_recv(c);
    spi_send_recv(s & 0xF);
    spi_send_recv(0x10 | ((s >> 4) & 0xF));
    PORTFSET = 0x10;
}

/**
 * Renders the start screen
 * @author Olle Jernström
 */
void render_start_screen(uint8_t b, uint8_t cv)
{
    uint8_t c, r; // iteration variables
    for (c = 0; c < 4; c++)
    {                       // render_frame in 4 columns
        setup_screen(c, 0); // setup display for data

        for (r = 0; r < 23; r++) // start with 23 rows of nothing
            spi_send_recv(0);

        if (b) // whether the "press to play" text should be rendered or not (used for blinking)
            for (r = 0; r < 71; r++)
                spi_send_recv(0);
        else
            for (r = 71; r-- > 0;)
                spi_send_recv(ptp[r][c]);

        for (r = 0; r < 23; r++) // spacing for logo
            spi_send_recv(0);

        spi_send_recv(0xFF); // bottom line for logo
        spi_send_recv(0);
        for (r = 7; r-- > 0;) // logo rendering
            spi_send_recv(logo[r][c]);
        spi_send_recv(0);
        spi_send_recv(0xFF); // top line for logo
    }
}

/**
 * Updates the scores 2D array with a given score
 * Updates score part if high_score = 0, else highscore
 * @author Olle Jernström
 */
void update_scores(const uint32_t current_score, const uint8_t high_score)
{
    uint32_t tmp_sc = current_score; // temporary score
    uint8_t d, i, n;                 // iteration variables
    uint8_t s = high_score ? 0 : 10; // s keeps track of where the new score should be put

    for (d = 8; d-- > 2;)
    {                           // loop through every digit
        n = tmp_sc % 10;        // n becomes last digit
        for (i = 0; i < 9; i++) // update correct digit in scores
            scores[s + i][d] = numbers[n][i];
        tmp_sc /= 10; // divide away last digit
    }
}

/**
 * Renders the highscore, score and next figure
 * @author Olle Jernström
 */
void render_scores_and_next_figure(void)
{
    uint8_t c, rn, rs, h, cb, nb, i; // function variables
    for (c = 0; c < 4; c++)
    {                        // render_frame in 4 columns
        setup_screen(c, 96); // setup display for data

        // renders next figure
        spi_send_recv(0xFF);
        if (c == 0)
            for (i = 0; i < 10; i++)
                spi_send_recv(1);
        else if (c == 3)
            for (i = 0; i < 10; i++)
                spi_send_recv(0x80);
        else
        {
            spi_send_recv(0);
            for (rn = 2; rn-- > 0;)
            {
                cb = next[rn][c * c - c];
                nb = next[rn][c * c - c + 1];
                for (h = 0; h < 4; h++)
                    spi_send_recv((cb * 0xF) | (nb * 0xF0));
            }
            spi_send_recv(0);
        }
        spi_send_recv(0xFF);
        spi_send_recv(0);

        // renders score and highscore
        for (rs = 19; rs-- > 0;)
            spi_send_recv(scores[rs][2 * c] | ((scores[rs][2 * c + 1] << 4) & 0xF0));
    }
}

/**
 * Converts the playing field data to pixel by pixel data for animation
 * @author Olle Jernström
 */
static void animation_setup_pixel_by_pixel()
{
    uint8_t r, c;
    for (r = 0; r < 96; r++)
        for (c = 0; c < 32; c++)
            anim[r][c] = field[r / 4][c / 4];
}

/**
 * Renders the animation based on the animation 2D-array
 * @author Olle Jernström
 */
static void render_animation()
{
    uint8_t c, r, i, data; // function variables

    for (c = 0; c < 4; c++)
    {                       // render_frame in 4 columns
        setup_screen(c, 0); // setup display for data

        for (r = 96; r-- > 0;)
        {                           // current row
            data = 0;               // make sure data is 0
            for (i = 0; i < 8; i++) // send pixels in the correct order
                data |= (anim[r][c * 8 + i] << i);
            spi_send_recv(data);
        }
    }
}

/**
 * Controls which animation should be played
 * @author Olle Jernström
 */
static void render_animation_control(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt, uint8_t a)
{
    uint8_t r, c;          // iteration variables
    uint8_t anim_ctrl = 0; // controls which frame of the animation should be played
    animation_setup_pixel_by_pixel();

    while (anim_ctrl++ < 4)
    { // 4 frames
        render_animation();

        if (a == 0)
        { // down animation
            for (c = lft * 4; c < rgt * 4; c++)
            {
                for (r = bot * 4 + anim_ctrl - 1; r-- > top * 4 + anim_ctrl;)
                    anim[r + 1][c] = anim[r][c];      // shift the animated block down
                anim[top * 4 + anim_ctrl - 1][c] = 0; // set top row to be 0
            }
        }
        else if (a == 1)
        { // right animation
            for (r = top * 4; r < bot * 4; r++)
            {
                for (c = rgt * 4 + anim_ctrl - 1; c-- > lft * 4 + anim_ctrl;)
                {
                    anim[r][c + 1] = anim[r][c];          // shift the animated block to the right
                    anim[r][lft * 4 + anim_ctrl - 1] = 0; // set left row to be 0
                }
            }
        }
        else if (a == 2)
        { // left animation
            for (r = top * 4; r < bot * 4; r++)
            {
                for (c = lft * 4 - anim_ctrl + 1; c < rgt * 4 - 1 + anim_ctrl - 1; c++)
                {
                    anim[r][c - 1] = anim[r][c];              // shift the animated block to the left
                    anim[r][rgt * 4 - 1 + anim_ctrl - 1] = 0; // set right row to be 0
                }
            }
        }
        sleep(50000); // delay between frames
    }
}

/**
 * Sends the correct control signal to render_animation_control to render_frame the down animation
 * @author Olle Jernström
 */
void render_animation_down(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt)
{
    render_animation_control(top, bot, lft, rgt, 0);
}

/**
 * Sends the correct control signal to render_animation_control to render_frame the right animation
 * @author Olle Jernström
 */
void render_animation_right(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt)
{
    render_animation_control(top, bot, lft, rgt, 1);
}

/**
 * Sends the correct control signal to render_animation_control to render_frame the left animation
 * @author Olle Jernström
 */
void render_animation_left(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt)
{
    render_animation_control(top, bot, lft, rgt, 2);
}

void render_animation_slam()
{
}

/**
 * Renders the playing field
 * @author Olle Jernström
 */
void render_playing_field(void)
{
    uint8_t c, r, h, cb, nb; // function definitions
    for (c = 0; c < 4; c++)
    {                       // render_frame in 4 columns
        setup_screen(c, 0); // setup display for data

        for (r = 24; r-- > 0;)
        {                             // current row
            cb = field[r][2 * c];     // current block
            nb = field[r][2 * c + 1]; // next block (right)
            for (h = 0; h < 4; h++)   // block height
                spi_send_recv((cb * 0xF) | (nb * 0xF0));
        }
    }
}

/**
 * Renders the name selection menu for new highscore
 * @author Olle Jernström
 */
void render_name_selection_for_new_highscore(uint8_t sl[4], uint8_t lc)
{
    uint8_t c, r, rs; // iteration variables

    for (c = 0; c < 4; c++)
    {                            // render_frame in 4 columns
        setup_screen(c, 0);      // setup display for data
        for (r = 0; r < 59; r++) // spacing
            spi_send_recv(0);

        if (c == lc)
        { // renders correct line under selected letter
            if (letters[sl[c]][7] == 3)
                spi_send_recv(0x1C);
            else if (letters[sl[c]][7] == 4)
                spi_send_recv(0x0F);
            else if (letters[sl[c]][7] == 5)
                spi_send_recv(0x1F);
            else if (letters[sl[c]][7] == 7)
                spi_send_recv(0x7F);
        }
        else
            spi_send_recv(0);
        spi_send_recv(0);

        for (r = 7; r-- > 0;) // render_frame selected letter
            spi_send_recv(letters[sl[c]][r]);

        for (r = 0; r < 10; r++) // spacing
            spi_send_recv(0);

        for (rs = 19; rs-- > 0;)
            spi_send_recv(scores[rs][2 * c] | ((scores[rs][2 * c + 1] << 4) & 0xF0));

        for (r = 0; r < 60 - 19 - 10; r++) // spacing
            spi_send_recv(0);
    }
}

/**
 * Renders the highscore list
 * @author Olle Jernström
 */
void render_highscores(uint32_t sl[5][5])
{
    uint8_t c, r, s, sr; // function variables
    for (c = 0; c < 4; c++)
    {                       // render_frame in 4 columns
        setup_screen(c, 0); // setup display for data

        for (s = 5; s-- > 0;)
        {                               // which score to render_frame
            update_scores(sl[s][4], 0); // update the score in the scores 2D-array to use its numbers
            for (sr = 19; sr-- > 9;)
            {               // row in scores
                if (c != 3) // render_frame score for the first 3 columns
                    spi_send_recv(scores[sr][2 * (c + 1)] | ((scores[sr][2 * (c + 1) + 1] << 4) & 0xF0));
                else
                    spi_send_recv(0);
            }
            spi_send_recv(0);

            for (r = 7; r-- > 0;) // render_frame the name of current score holder
                spi_send_recv(letters[sl[s][c]][r]);

            for (r = 0; r < 5; r++)
                spi_send_recv(0);
        }

        // render_frame highscore text
        spi_send_recv(0);
        spi_send_recv(0);
        spi_send_recv(0xFF);
        spi_send_recv(0);
        for (r = 7; r-- > 0;)
            spi_send_recv(hisc[r][c]);
        spi_send_recv(0);
        spi_send_recv(0xFF);
    }
}
