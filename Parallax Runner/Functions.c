/* Author: Lalit Shankar Chowdhury
 * Description: Contains detailed game logic
 * Remark: Safe versions of I/O functions are used wherever possible
 */

#include "Game.h"
#include <stdio.h>
#include <SDL.h>
#include <SDL_image.h>

#define MAX_PLAYERS 16

#define WINDOW_WIDTH 800
#define WINDOW_HEIGHT 450

#define ASCII_DIGIT_OFFSET 48

#define SPRITE_WIDTH 129
#define SPRITE_HEIGHT 37

static void load_user_records(void);
static int detect_collision(int i);
static void write_final_score();

static FILE* fptr;

struct Player
{
	char user_name[9];
	char pin[5];
	int score;
}record[MAX_PLAYERS];

static int player_count;
static int current_player_index;
static int current_player_score = 0;

static SDL_Window* window;

static SDL_Renderer* renderer;

static SDL_Texture* background_layer[3];

static SDL_Texture* space_ship[5][8];

static SDL_Rect part_1[3];
static SDL_Rect part_2[3];

static SDL_Rect space_ship_rect[5];

static int lives = 5;

bool jumping_allowed = true;

/*
 * Description: Prompt user to login, add new user or display scoreboard
 * Return: SUCCESS if everything works else FAILURE
 */
int show_startup_screen()
{
	if (fopen_s(&fptr, "Records.dat", "r+") != 0)
	{
		puts("Failed to open player data file");

		return FAILURE;
	}

	// This check is required according to the specification of fopen_s( ... )
	if (fptr == NULL)
	{
		puts("File pointer is NULL");

		return FAILURE;
	}

	load_user_records();

Prompt:

	// Show prompt
	puts("\nEnter:\n");
	puts(" 1 to login");
	puts(" 2 to create new user");
	puts(" 3 to display high score table\n");
	printf_s(">>> ");

	int choice = getchar();

	switch (choice)
	{
	case '1':
	{
		if (player_count >= 1)
		{
			char user_name[9] = "", pin[5] = "";
			bool player_present = false;

			// Remove trailing characters
			while (getchar() != '\n');

			printf_s("\nEnter username: ");
			scanf_s(
				"%s",
				user_name,
				_countof(user_name));

			// Remove trailing characters
			while (getchar() != '\n');

			printf_s("Enter pin: ");
			scanf_s(
				"%s",
				pin,
				_countof(pin));

			for (int i = 0; i < player_count; ++i)
			{
				// If username and pin match some record
				if (strcmp(user_name, record[i].user_name) == 0 && strcmp(pin, record[i].pin) == 0)
				{
					player_present = true;

					current_player_index = i;

					puts("\nLogin successful\n");

					break;
				}
			}

			if (!player_present)
			{
				puts("\nInvalid credentials");

				// Remove trailing characters
				while (getchar() != '\n');

				// Show initial prompt again
				goto Prompt;
			}
		}
		else
		{
			puts("\nNo player present");

			// Remove trailing characters
			while (getchar() != '\n');

			// Show initial prompt again
			goto Prompt;
		}

		break;
	}
	case '2':
	{
		if (player_count < MAX_PLAYERS)
		{
			char new_user_name[9] = "", new_user_pin[5] = "";

			// Remove trailing characters
			while (getchar() != '\n');

			printf_s("\nEnter username: ");
			scanf_s(
				"%s",
				new_user_name,
				_countof(new_user_name));

			// Remove trailing characters
			while (getchar() != '\n');

			printf_s("Enter pin: ");
			scanf_s(
				"%s",
				new_user_pin,
				_countof(new_user_pin));

			fprintf_s(
				fptr,
				"%s\n",
				new_user_name);

			fprintf_s(
				fptr,
				"%s\n",
				new_user_pin);

			fprintf_s(
				fptr,
				"%d\n",
				0);

			// Write to file immediately by flushing file stream
			fflush(fptr);

			puts("\nUser added");

			// Set file pointer to point to beginning of file so that records can be read again
			fseek(
				fptr,
				0,
				SEEK_SET
			);

			// Load user records again
			load_user_records();

			// Remove trailing characters
			while (getchar() != '\n');

			// Show initial prompt again
			goto Prompt;
		}
		else
		{
			puts("\nMaximum player limit reached");

			// Remove trailing characters
			while (getchar() != '\n');

			// Show initial prompt again
			goto Prompt;
		}
	}
	case '3':
	{
		if (player_count >= 1)
		{
			puts("\n+------------------+");
			puts("| Username | Score |");
			puts("+------------------+");

			for (int i = 0; i < player_count; ++i)
			{
				printf_s("| %8s | %5d |\n", record[i].user_name, record[i].score);
				puts("+------------------+");
			}

			// Remove trailing characters
			while (getchar() != '\n');

			// Show initial prompt again
			goto Prompt;
		}
		else
		{
			puts("\nNo player present");

			// Remove trailing characters
			while (getchar() != '\n');

			// Show initial prompt again
			goto Prompt;
		}

		break;
	}
	case '\n':
	{
		// Show initial prompt again
		goto Prompt;
	}
	default:
	{
		puts("\nInvalid choice");

		// Remove trailing characters
		while (getchar() != '\n');

		// Show initial prompt again
		goto Prompt;

		break;
	}
	}

	return SUCCESS;
}

/*
 * Description: Initialize critical game components
 * Return: SUCCESS if everything works else FAILURE
 */
int init_game()
{
	// Try to initialize SDL
	if (SDL_Init(SDL_INIT_EVERYTHING) < 0)
	{
		printf_s("%s\n", SDL_GetError());

		return FAILURE;
	}

	window = SDL_CreateWindow(
		"Parallax Runner",
		SDL_WINDOWPOS_CENTERED,
		SDL_WINDOWPOS_CENTERED,
		WINDOW_WIDTH,
		WINDOW_HEIGHT,
		SDL_WINDOW_SHOWN);

	// Raise game window and set input focus
	SDL_RaiseWindow(window);

	renderer = SDL_CreateRenderer(
		window,
		-1,
		SDL_RENDERER_ACCELERATED
		| SDL_RENDERER_PRESENTVSYNC
	);

	// Try to initialize SDL image
	if (IMG_Init(IMG_INIT_PNG) != IMG_INIT_PNG)
	{
		printf_s("%s\n", IMG_GetError());

		return FAILURE;
	}

	return SUCCESS;
}

/*
 * Description: Show splash screen
 * Return: SUCCESS if everything works else FAILURE
 */
int show_splash_screen(void)
{
	SDL_Surface* splash_screen_surface = IMG_Load("../Assets/Splash.png");

	if (splash_screen_surface == NULL)
	{
		printf_s("%s\n", IMG_GetError());

		return FAILURE;
	}

	SDL_Texture* splash_screen_texture = SDL_CreateTextureFromSurface(
		renderer,
		splash_screen_surface);

	if (splash_screen_texture == NULL)
	{
		printf_s("%s\n", IMG_GetError());

		return FAILURE;
	}

	// Copy splash screen texture to rendering target
	if (SDL_RenderCopy(renderer, splash_screen_texture, NULL, NULL) == -1)
	{
		printf_s("%s\n", IMG_GetError());

		return FAILURE;
	}

	// Update the screen
	SDL_RenderPresent(renderer);

	SDL_FreeSurface(splash_screen_surface);

	SDL_Delay(3000);

	SDL_DestroyTexture(splash_screen_texture);

	// Clear rendering target so that scrolling background can be drawn
	SDL_RenderClear(renderer);

	return SUCCESS;
}

/*
 * Description: Load textures for gameplay
 * Return: SUCCESS if everything works else FAILURE
 */
int load_textures()
{
	SDL_Surface* surface_layer[3];

	// Load background textures

	surface_layer[0] = IMG_Load("../Assets/Background/1.png");
	surface_layer[1] = IMG_Load("../Assets/Background/2.png");
	surface_layer[2] = IMG_Load("../Assets/Background/3.png");

	for (int i = 0; i < 3; ++i)
	{
		if (surface_layer[i] == NULL)
		{
			printf_s("%s\n", SDL_GetError());

			return FAILURE;
		}

		background_layer[i] = SDL_CreateTextureFromSurface(
			renderer,
			surface_layer[i]);

		if (background_layer[i] == NULL)
		{
			printf_s("%s\n", SDL_GetError());

			return FAILURE;
		}

		SDL_FreeSurface(surface_layer[i]);
	}

	SDL_Surface* space_ship_surface[5][8];

	// Load spaceship textures

	char file_path[28] = "../Assets/Spaceship/ / .png";

	// file_path = "../Assets/Spaceship/[i]/[j].png"

	for (int i = 0; i < 5; ++i)
	{
		for (int j = 0; j < 8; ++j)
		{
			// Load spaceship surfaces

			// Place folder number as (char) i + 1
			file_path[20] = (char)(i + 1 + ASCII_DIGIT_OFFSET);

			// Place file number as (char) j + 1
			file_path[22] = (char)(j + 1 + ASCII_DIGIT_OFFSET);

			// Load space ship surfaces from image files
			space_ship_surface[i][j] = IMG_Load(file_path);

			if (space_ship_surface[i][j] == NULL)
			{
				printf_s("%s\n", IMG_GetError());

				return FAILURE;
			}

			// Create sprite textures from surfaces
			space_ship[i][j] = SDL_CreateTextureFromSurface(renderer, space_ship_surface[i][j]);

			if (space_ship[i][j] == NULL)
			{
				printf_s("%s\n", IMG_GetError());

				return FAILURE;
			}

			// Free space ship surfaces
			SDL_FreeSurface(space_ship_surface[i][j]);
		}
	}

	return SUCCESS;
}

/*
 * Description: Set initial attributes of part_1, part_2, and sprite_rect
 * Return: SUCCESS if everything works else FAILURE
 * Comment: Background textures are scaled larger than their actual size for better "pixelated / retro" effect
 */
void init_rectangles()
{
	for (int i = 0; i < 3; ++i)
	{
		part_1[i].x = 0, part_2[i].x = WINDOW_WIDTH;
		part_1[i].y = 0, part_2[i].y = 0;

		part_1[i].w = WINDOW_WIDTH, part_2[i].w = WINDOW_WIDTH;
		part_1[i].h = WINDOW_HEIGHT, part_2[i].h = WINDOW_HEIGHT;
	}

	for (int i = 0; i < 5; ++i)
	{
		space_ship_rect[i].h = SPRITE_HEIGHT;
		space_ship_rect[i].w = SPRITE_WIDTH;
	}

	// Position of main space ship

	// Center horizontally
	space_ship_rect[0].x = (WINDOW_WIDTH / 2) - (SPRITE_WIDTH / 2);
	// Set height of main space ship
	space_ship_rect[0].y = 200;

	// Initial position of the rest space ships
	space_ship_rect[1].x = 550;
	space_ship_rect[1].y = 275;

	space_ship_rect[2].x = 650;
	space_ship_rect[2].y = 86;

	space_ship_rect[3].x = 0;
	space_ship_rect[3].y = 200;

	space_ship_rect[4].x = 120;
	space_ship_rect[4].y = 10;
}

/*
 * Description: Get keyboard input
 * Return: EXIT_GAME if Esc is pressed or JUMP_SPRITE if spacebar is pressed
 * Remark: If no key is pressed, CONTINUE_GAME is returned
 */
int get_keyboard_input()
{
	SDL_Event event;

	SDL_PollEvent(&event);

	switch (event.type)
	{
		// If some key is pressed
	case SDL_KEYDOWN:
	{
		switch (event.key.keysym.sym)
		{
		case SDLK_SPACE:
		{
			return JUMP_SPRITE;
		}
		case SDLK_ESCAPE:
		{
			// Write final score
			write_final_score();

			return QUIT_GAME;
		}
		}
		break;
	}
	}

	return CONTINUE_GAME;
}

/*
 * Description: Scrolls background in parallax manner
 * Return: SUCCESS if everything works else FAILURE
 */
int scroll_background()
{
	for (int i = 0; i < 3; ++i)
	{
		// Check second part reaches x = 0
		if (part_2[i].x == 0)
		{
			// Reset rectangle positions
			part_1[i].x = 0;
			part_2[i].x = WINDOW_WIDTH;
		}

		if (SDL_RenderCopy(renderer, background_layer[i], NULL, &part_1[i]) == -1)
		{
			printf_s("%s\n", SDL_GetError());

			return FAILURE;
		}

		if (SDL_RenderCopy(renderer, background_layer[i], NULL, &part_2[i]) == -1)
		{
			printf_s("%s\n", SDL_GetError());

			return FAILURE;
		}
	}

	// Set scrolling speed in pixels
	part_1[0].x -= 1, part_2[0].x -= 1;
	part_1[1].x -= 2, part_2[1].x -= 2;
	part_1[2].x -= 4, part_2[2].x -= 4;

	return SUCCESS;
}

/*
 * Description: Animate all sprite
 * Return: SUCCESS if everything works else FAILURE
 */
int animate_sprites()
{
	// Change position of all sprites other than main
	for (int i = 1; i < 5; ++i)
	{
		// If right edge is reached
		if (space_ship_rect[i].x > 800)
		{
			// Position space ship on the left edge
			space_ship_rect[i].x = -1 * SPRITE_WIDTH;
		}
		else
		{
			// Advance NPC space ship (speed varies based on index)
			space_ship_rect[i].x += i;

			// If main space ship collides with some NPC space ship
			if (detect_collision(i) == SUCCESS)
			{
				// If last life is left
				if (lives == 1)
				{
					write_final_score();

					return FAILURE;
				}

				// Reset game texture positions
				init_rectangles();

				// Since position is reset jumping cannot be done
				// jumping_allowed = false;

				// Print lives left
				printf("Lives left: %d\n", --lives);
			}
			else
			{
				// If no collision occurs, then juping is allowed
				// jumping_allowed = true;
			}
		}
	}

	// Static variable for storing sprite animation index
	static int j = 0;

	// Repeat for every sprite
	for (int i = 0; i < 5; ++i)
	{
		if (j <= 7)
		{
			SDL_RenderCopy(renderer, space_ship[i][j], NULL, &space_ship_rect[i]);
		}
		else
		{
			j = 0;

			SDL_RenderCopy(renderer, space_ship[i][j], NULL, &space_ship_rect[i]);
		}
	}

	++j;

	return SUCCESS;
}

/*
 * Description: Jump main space ship
 * Return: SUCCESS if everything works else FAILURE
 */
int jump_player_space_ship()
{
	if (jumping_allowed)
	{
		for (int i = 50; i >= 0; --i)
		{
			// Raise main space ship at a decreasing rate
			space_ship_rect[0].y -= (i / 10 + 1);

			// Character should start jumping in the next frame
			if (i < 50)
			{
				if (scroll_background() == FAILURE)
				{
					return FAILURE;
				}

				if (animate_sprites() == FAILURE)
				{
					return FAILURE;
				}
			}

			render_frame();
		}
		for (int i = 0; i <= 50; ++i)
		{
			// Lower main space ship at an increasing rate
			space_ship_rect[0].y += (i / 10 + 1);

			if (scroll_background() == FAILURE)
			{
				return FAILURE;
			}

			if (animate_sprites() == FAILURE)
			{
				return FAILURE;
			}

			render_frame();
		}
	}

	return SUCCESS;
}

/*
 * Description: Render frame and clear rendering target.
 * Return: Nothing
 * Remark: Function inlined due to frequently being used
 */
inline void render_frame()
{
	// Update screen to show rendered frame
	SDL_RenderPresent(renderer);

	// Clear rendering target so that next frame can be rendered
	SDL_RenderClear(renderer);
}

/*
 * Description: Wait for some time before next frame is rendered. Saves CPU cycles.
 * Return: Nothing
 */
void delay()
{
	// Wait for 5 milliseconds
	SDL_Delay(5);
}

/*
 * Description: Quit game by deallocating game components
 * Return: SUCCESS if everything works else FAILURE
 */
int quit_game(void)
{
	IMG_Quit();

	SDL_DestroyRenderer(renderer);

	SDL_DestroyWindow(window);

	fclose(fptr);

	SDL_Quit();

	return SUCCESS;
}

/*
 * Description: Read player data and store in record[MAX_PLAYERS]
 * Return: Nothing
 * Remark: Records are read till EOF or MAX_PLAYERS is reached
 */
static void load_user_records()
{
	player_count = 0;

	int ret_value_1 = 0, ret_value_2 = 0, ret_value_3 = 0;

	for (int i = 0; i < MAX_PLAYERS; ++i)
	{
		ret_value_1 = fscanf_s(
			fptr,
			"%s",
			record[i].user_name,
			_countof(record[i].user_name));

		ret_value_2 = fscanf_s(
			fptr,
			"%s",
			record[i].pin,
			_countof(record[i].pin));

		ret_value_3 = fscanf_s(
			fptr,
			"%d",
			&record[i].score);

		if (ret_value_1 == EOF || ret_value_2 == EOF || ret_value_3 == EOF)
		{
			break;
		}

		++player_count;
	}
}

/*
 * Description: Detect whether player space ship collided with NPC space ship or not
 * Return: SUCCESS if collision detected else FAILURE
 * Remark: i is the index of NPC space ship
 */
static int detect_collision(int i)
{
	// Store the edges of main space ship
	int top_edge = space_ship_rect[0].y;
	int bottom_edge = space_ship_rect[0].y + WINDOW_HEIGHT;
	int left_edge = space_ship_rect[0].x;
	int right_edge = space_ship_rect[0].x + WINDOW_WIDTH;

	// If top edge of space is equal to or between vertical edges of NPC space ship
	if (top_edge >= space_ship_rect[i].y && top_edge <= space_ship_rect[i].y + SPRITE_HEIGHT)
	{
		// If left and/or right edge of main space ship is equal to or between horizontal edges of NPC

		if (left_edge >= space_ship_rect[i].x && left_edge <= space_ship_rect[i].x + SPRITE_WIDTH)
		{
			return SUCCESS;
		}

		if (right_edge >= space_ship_rect[i].x && right_edge <= space_ship_rect[i].x + SPRITE_WIDTH)
		{
			return SUCCESS;
		}
	}

	// If bottom edge of space is equal to or between vertical edges of NPC space ship
	if (bottom_edge >= space_ship_rect[i].y && bottom_edge <= space_ship_rect[i].y + SPRITE_HEIGHT)
	{
		// If left and/or right edge of main space ship is equal to or between horizontal edges of NPC

		if (left_edge >= space_ship_rect[i].x && left_edge <= space_ship_rect[i].x + SPRITE_WIDTH)
		{
			return SUCCESS;
		}

		if (right_edge >= space_ship_rect[i].x && right_edge <= space_ship_rect[i].x + SPRITE_WIDTH)
		{
			return SUCCESS;
		}
	}

	// Increase score if no collision is detected
	++current_player_score;

	return FAILURE;
}

/*
 * Description: Write final player score
 * Return: Nothing
 */
static void write_final_score()
{
	printf_s("Your current score is: %d.\n", current_player_score);

	if (current_player_score > record[current_player_index].score)
	{
		// Store the offset of one player record times current player index
		long offset = current_player_index * sizeof(record[0]);

		fseek(
			fptr,
			offset,
			SEEK_SET);

		// Write current player record again

		fprintf_s(
			fptr,
			"%s\n",
			record[current_player_index].user_name);

		fprintf_s(
			fptr,
			"%s\n",
			record[current_player_index].pin);

		fprintf_s(
			fptr,
			"%d\n",
			current_player_score);

		// Write to file immediately
		fflush(fptr);
	}
}
