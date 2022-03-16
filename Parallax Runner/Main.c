/* Author: Lalit Shankar Chowdhury
 * Description: Contains simplified version of game logic
 */

#include "Game.h"

int main()
{
	if (show_startup_screen() == FAILURE)
	{
		return EXIT_FAILURE;
	}

	if (init_game() == FAILURE)
	{
		return EXIT_FAILURE;
	}

	if (show_splash_screen() == FAILURE)
	{
		return EXIT_FAILURE;
	}

	init_rectangles();

	if (load_textures() == FAILURE)
	{
		return EXIT_FAILURE;
	}

	int keyboard_input;

	while (true)
	{
		if (scroll_background() == FAILURE)
		{
			return EXIT_FAILURE;
		}

		if (animate_sprites() == FAILURE)
		{
			return EXIT_FAILURE;
		}

		keyboard_input = get_keyboard_input();

		if (keyboard_input == QUIT_GAME)
		{
			break;
		}
		if (keyboard_input == JUMP_SPRITE)
		{
			// Background scrolling, animating sprites and rendering is handled by this function
			if (jump_player_space_ship() == FAILURE)
			{
				return EXIT_SUCCESS;
			}
		}
		if (keyboard_input == CONTINUE_GAME)
		{
			render_frame();

			continue;
		}

		delay();
	}

	quit_game();

	return EXIT_SUCCESS;
}
