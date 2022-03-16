/* Author: Lalit Shankar Chowdhury
 * Description: Contains symbolic constants and function declarations
 * Remark: Code is self explanatory
 */

// Include this file only once
#pragma once

#include <stdbool.h>
#include <stdlib.h>

#define SUCCESS 1
#define FAILURE 0

#define JUMP_SPRITE 2
#define CONTINUE_GAME 1
#define QUIT_GAME 0

int show_startup_screen(void);
int init_game(void);
int show_splash_screen(void);
int load_textures(void);
void init_rectangles(void);
int scroll_background(void);
int animate_sprites(void);
int get_keyboard_input(void);
int jump_player_space_ship(void);
void render_frame(void);
void delay();
int quit_game(void);
