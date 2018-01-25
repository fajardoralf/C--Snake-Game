/*
 * @file: GameManager.cpp
 * @author: Stig M. Halvorsen <halsti@nith.no>
 * @version: 1.0.0 <11.02.2013>
 *
 * @description: A singleton class to control all gameplay.
 */

#include "SDL/SDLBmp.h"
#include "GameManager.h"
#include "InputManager.h"
#include "Timer.h"
#include <vector>
#include <random>
#include <stdlib.h>

const int SQUARE_LENGTH = 20;
const int SCREEN_WIDTH = SQUARE_LENGTH * 30;
const int SCREEN_HEIGHT = SQUARE_LENGTH * 30;
const int SQUARE_PADDING = 2;

bool notGameOver = true;
bool showObstacle = false;
int snake_x = SCREEN_WIDTH/2;
int snake_y = SCREEN_HEIGHT/2;
int fruit_x;
int fruit_y;
int snake_body_length = 1;

// Score
unsigned int score = 0;

enum Direction { IDLE, LEFT, RIGHT, UP, DOWN };
Direction direction = IDLE;

struct Pos {
	int x;
	int y;
};

std::vector<Pos> obstacles;
bool obstacle_generated = false;
std::vector<Pos> snake_body;

Mix_Chunk * music = NULL;
Mix_Chunk * bgm = NULL;
/* Initializes SDL, creates the game window and fires off the timer. */
GameManager::GameManager()
{
	SDLManager::Instance().init();
	m_window = SDLManager::Instance().createWindow("Snake Game");
	Timer::Instance().init();
	
	// Initialize SDL_mixer
	if (Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 2048) < 0) {
		printf("SDL_mixer could not initialize! SDL_mixer Error: %s\n", Mix_GetError());
	}
}

/* Load sound */
bool loadSound()
{
	// Loading success flag
	bool success = true;

	music = Mix_LoadWAV("Assets/gfx/eatSoundfx.wav");
	if (music == NULL) {
		success = false;
	}
	return success;
}

bool loadBGM() {
	// Loading success flag
	bool success = true;

	bgm = Mix_LoadWAV("Assets/gfx/bgm.wav");
	if (bgm == NULL)
	{
		success = false;
	}
	return success;
}

void closeSound() {
	// Free the sound effects
	Mix_FreeChunk(music);
	Mix_FreeChunk(bgm);
	music = NULL;
	bgm = NULL;
	// Quit SDL subsystems
	Mix_Quit();
}

/* Check if snake collide with walls */
void GameManager::checkWallCollision()
{
	if (snake_x < 0 || snake_x >= SCREEN_WIDTH) 
	{
		notGameOver = false;
	}
	else if (snake_y < 0 || snake_y >= SCREEN_HEIGHT) {
		notGameOver = false;
	}
}

void GameManager::checkSelfCollision()
{
	if(snake_body.size() > 1)
	{
		for(int i = 0; i < snake_body.size() - 1; i++)
		{
			if(snake_x == snake_body[i].x && snake_y == snake_body[i].y)
			{
				notGameOver = false;
			}
		}
	}
}

void GameManager::generateFruit() 
{
	int tmp_x = SCREEN_WIDTH / SQUARE_LENGTH;
	int tmp_y = SCREEN_HEIGHT / SQUARE_LENGTH;

	bool check_overlapping = false;
	while(!check_overlapping)
	{
		fruit_x = (rand() % tmp_x) * SQUARE_LENGTH;
		fruit_y = (rand() % tmp_y) * SQUARE_LENGTH;

		bool collision_found = false;
		for(int i = 0; i < snake_body.size(); i++)
		{
			if(fruit_x == snake_body[i].x && fruit_y == snake_body[i].y)
			{
				collision_found = true;
			}
		}
		for (int i = 0; i < obstacles.size(); i++) 
		{
			if (fruit_x == obstacles[i].x && fruit_y == obstacles[i].y) {
				collision_found = true;
			}
		}
		if (!collision_found) {
			check_overlapping = true;
		}
	}
}

void GameManager::draw()
{
	struct Pos new_pos;
	new_pos.x = snake_x;
	new_pos.y = snake_y;
	snake_body.push_back(new_pos);
	int size = snake_body.size();

	// Draw snake
	for (int i = 0; i < size; i++) {
		SDL_Rect snakeRect = { snake_body[i].x, snake_body[i].y, SQUARE_LENGTH - SQUARE_PADDING, SQUARE_LENGTH - SQUARE_PADDING };
		SDL_SetRenderDrawColor(SDLManager::Instance().getRenderer(*(SDLManager::Instance().getMainWindow())), 0, 0xb5, 0x0c, 0xFF);
		SDL_RenderFillRect(SDLManager::Instance().getRenderer(*(SDLManager::Instance().getMainWindow())), &snakeRect);
	}

	if (snake_body.size() == 5) {
		while (!obstacle_generated) {
			struct Pos newPos;
			newPos.x = (rand() % 30) * 20;
			newPos.y = (rand() % 30) * 20;
			obstacles.push_back(newPos);
			obstacle_generated = true;
		}
	}

	if (snake_body.size() == 10) {
		while (obstacle_generated) {
			for (int i = 0; i < 3; i++) {
				struct Pos newPos;
				newPos.x = (rand() % 30) * 20;
				newPos.y = (rand() % 30) * 20;
				obstacles.push_back(newPos);
			}
			obstacle_generated = false;
		}
	}

	if (snake_body.size() == 15) {
		while (!obstacle_generated) {
			for (int i = 0; i < 5; i++) {
				struct Pos newPos;
				newPos.x = (rand() % 30) * 20;
				newPos.y = (rand() % 30) * 20;
				obstacles.push_back(newPos);
			}
			obstacle_generated = true;
		}
	}

	if(snake_body.size() >= 5)
		showObstacle = true;

	if (size >= snake_body_length) {
		snake_body.erase(snake_body.begin());
	}

	switch (direction)
	{
	case UP:
		snake_y -= 20;
		break;
	case DOWN:
		snake_y += 20;
		break;
	case LEFT:
		snake_x -= 20;
		break;
	case RIGHT:
		snake_x += 20;
		break;
	default:
		break;
	}

	checkSelfCollision();

	if(snake_x == fruit_x && snake_y == fruit_y)
	{
		Mix_PlayChannel(-1, music, 0);
		snake_body_length++;
		score++;
		generateFruit();
	}

	checkWallCollision();
}

/* Kicks off/is the the gameloop */
void GameManager::play()
{
	srand(time(NULL));
	// Load bitmaps
	SDLBmp backround("Assets/gfx/background.bmp");
	SDLBmp treasure("Assets/gfx/treasure.bmp");
	SDLBmp obstacle("Assets/gfx/bomb.bmp");

	obstacle.x = -1.f;
	obstacle.y = -1.f;

	// Calculate render frames per second (second / frames) (60)
	float render_fps = 1.f / 10.f;
	m_lastRender = render_fps; // set it to render immediately

	generateFruit();

	if (!loadSound()) {
		printf("Failed to load sound effect!\n");
	}
	if (!loadBGM()) {
		printf("Failed to load BGM!\n");
	}
	else {
		if (Mix_PlayingMusic() == 0) {
			Mix_PlayChannel(1, bgm, -1);
		}
	}

	// Gameloop
	while (notGameOver)
	{
		
		if (snake_body.size() >= 4 && snake_body.size() < 9) {
			render_fps = 1.f / 15.f;
		}
		else if (snake_body.size() >= 9) {
			render_fps = 1.f / 20.f;

		}

		for (int i = 0; i < obstacles.size(); i++) {
			if (snake_x == obstacles[i].x && snake_y == obstacles[i].y)
				notGameOver = false;
		}

		treasure.x = fruit_x;
		treasure.y = fruit_y;
		// Update input and deltatime
		InputManager::Instance().Update();
		Timer::Instance().update();

		// Calculate displacement based on deltatime
		float displacement = 150.F * Timer::Instance().deltaTime();

		/* Input Management */

		// Left key
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_LEFT) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_LEFT))
		{
			if(direction != RIGHT)
				direction = LEFT;
		}
		
		// Right key
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_RIGHT) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_RIGHT))
		{
			if (direction != LEFT)
				direction = RIGHT;
		}

		// Key up
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_UP) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_UP))
		{
			if(direction != DOWN)
			direction = UP;
		}

		// Key down
		if (InputManager::Instance().KeyDown(SDL_SCANCODE_DOWN) ||
			InputManager::Instance().KeyStillDown(SDL_SCANCODE_DOWN))
		{
			if(direction != UP)
			direction = DOWN;
		}

		// Exit on [Esc], or window close (user X-ed out the window)
		if (InputManager::Instance().hasExit() || InputManager::Instance().KeyDown(SDL_SCANCODE_ESCAPE))
		{
			notGameOver = false;
		}

		// Update time since last render
		m_lastRender += Timer::Instance().deltaTime();

		// Check if it's time to render
		if (m_lastRender >= render_fps)
		{
			// Add bitmaps to renderer
			backround.draw();
			treasure.draw();
			draw();
			if (showObstacle) {
				if (obstacles.size() > 0) {
					for (int i = 0; i < obstacles.size(); i++) {
						obstacle.x = obstacles[i].x;
						obstacle.y = obstacles[i].y;
						obstacle.draw();
					}
				}
			}

			// Render window
			SDLManager::Instance().renderWindow(m_window);
			m_lastRender = 0.f;
		}

		// Sleep to prevent CPU exthaustion (1ms == 1000 frames per second)
		SDL_Delay(1);
	}
	closeSound();
}