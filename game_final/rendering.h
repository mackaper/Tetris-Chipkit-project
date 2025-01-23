/**
 * Header file for rendering
 * @author Olle Jernström
 */
void render_start_screen(uint8_t b, uint8_t cv);
void render_highscores(uint32_t sl[5][5]);
void update_scores(const uint32_t current_score, const uint8_t high_score);
void render_scores_and_next_figure();
void render_playing_field();
void render_animation_down(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt);
void render_animation_right(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt);
void render_animation_left(uint8_t top, uint8_t bot, uint8_t lft, uint8_t rgt);
void render_name_selection_for_new_highscore(uint8_t sl[4], uint8_t lc);
