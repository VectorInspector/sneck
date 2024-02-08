#define SDL_MAIN_HANDLED

#include <lodepng.h>
#include <iostream>
#include <algorithm>
#include <sdl2/sdl.h>
#include <string>
#include <random>
#include <unordered_map>

// Struct declarations.
struct PngFile;
struct AppData;
struct AppLoop;
struct StateData;
struct PlayFuncs;
struct Menu;
struct InputEvents;
struct SdlTexture;
struct SdlDrawFuncs;
struct SnakeMovement;
class AppState;
class AppPlayState;
class AppTitleState;

int main (int argc, char* argv []);

// Data and function declarations.
struct SdlDrawFuncs {
	SdlDrawFuncs (AppData& app_data);
	
	void Make ();
	void DrawTile (int x_tile, int y_tile, int x, int y);
	void DrawText (std::string text, int x, int y);
	void FontChara (char c);
	
	int font_x_tile;
	int font_y_tile;
	
	int text_x;
	int text_y;
	int cursor_x;
	int cursor_y;
	
	AppData& a;
};

struct SdlTexture {
	SdlTexture ();
	~SdlTexture ();
	bool LoadFromPng (SDL_Renderer* renderer, std::string path);
	
	SDL_Surface* surface;
	SDL_Texture* texture;
	int x_size;
	int y_size;
};

struct Menu {
	void OnTick (AppData& a, StateData& d);
	virtual void OnPress (AppData& a, StateData& d);
	virtual void OnLeftRight (AppData& a, StateData& d);
	
	int y_size;
	int y;
};

void Menu::OnTick (AppData& a, StateData& d) {
	
}

struct PngFile {
	PngFile (std::string path);
	~PngFile ();
	
	unsigned char* data;
	unsigned x_size;
	unsigned y_size;
};

struct InputEvents {
	void Tick (std::string key, bool cond);
	bool IsPressed (std::string key);
	bool IsReleased (std::string key);
	bool IsDown (std::string key);
	bool IsUp (std::string key);
	
	std::unordered_map <std::string, int> timers;
};

struct PlayFuncs {
	enum dirs {
		up = 0,
		right,
		down,
		left,
		none
	};
	
	struct DifficultyInfo {
		std::string name;
		int move_rate;
	};
	
	PlayFuncs (AppData& app_data, StateData& state_data);
	std::string DirName (int dir) const;
	DifficultyInfo Difficulty (int diff) const;
	int OppositeDir (int dir);
	bool DirsAreOrtho (int u, int v);
	void DirOffset (int x, int y, int dir);
	void RandomFreePos ();
	int Random (int u, int v);
	
	int new_dir;
	int new_x;
	int new_y;
	int x_offset;
	int y_offset;
	
	AppData& a;
	StateData& d;
};

class AppState {
public:
	AppState (AppData& app_data, StateData& state_data);
	virtual void OnFirstTick () {};
	virtual void OnTick () {};
	virtual void OnLastTick () {};
	virtual void OnDraw () {};
	
	AppData& a;
	StateData& d;
};

class AppPlayState : public AppState {
public:
	using AppState::AppState;
	virtual void OnFirstTick () override;
	virtual void OnTick () override;
	virtual void OnDraw () override;
};

class AppTitleState : public AppState {
public:
	using AppState::AppState;
	virtual void OnFirstTick () override;
	virtual void OnTick () override;
	virtual void OnDraw () override;
};

struct SnakeMovement {
	bool IsOutside (int x, int y);
	void Tick (AppData& a, StateData& d);
	
	int player_x;
	int player_y;
	int player_dir;
	int player_len;
	int food_x;
	int food_y;
	int score;
	int field_x_size;
	int field_y_size;
	int skip_shrink;
	static constexpr std::size_t max_field_x_size = 0xff;
	static constexpr std::size_t max_field_y_size = 0xff;
	int field [max_field_x_size] [max_field_y_size];
	int move_rate;
	int move_timer;
	bool player_controlled;
	bool is_invincible;
};

struct StateData {
	StateData (AppData& app_data);
	
	int tile_size;
	int timer;
	SnakeMovement snake_movement;
	PlayFuncs::DifficultyInfo difficulty;
	
	PlayFuncs play_funcs;
};

struct AppLoop {
	AppLoop (AppData& app_data);
	
	int MainLoop ();
	void OnTick ();
	void OnDraw ();
	
	AppData& a;
};

struct AppData {
	AppData ();
	~AppData ();
	
	void DefaultSettings ();
	
	AppState* current_state;

	InputEvents inputs;
	int window_x_size;
	int window_y_size;
	int canvas_x_size;
	int canvas_y_size;
	int exit_clicked;
	std::mt19937 random;
	
	int cmd_arg_count;
	char** cmd_args;
	
	SDL_Window*					window;
	SDL_Renderer*				renderer;
	SDL_Event					event;
	SDL_Texture*				target;
	SdlTexture					sprite;
	SdlDrawFuncs				draw_funcs;
	const unsigned char*		keyboard;
};

// Functions go here.
bool SnakeMovement::IsOutside (int x, int y) {
	return x < 0 || y < 0 || field_x_size <= x || field_y_size <= y;
}

void SnakeMovement::Tick (AppData& a, StateData& d) {
	move_timer = std::max(1, 1 + move_timer);
	
	// Change direction
	// player_controlled = false;
	
	if(player_controlled) {
		int new_dir = player_dir;
		
		std::unordered_map <std::string, PlayFuncs::dirs> input_to_dir = {
			{ "left", PlayFuncs::dirs::left },
			{ "right", PlayFuncs::dirs::right },
			{ "up", PlayFuncs::dirs::up },
			{ "down", PlayFuncs::dirs::down },
		};
		
		for(const auto& c: input_to_dir) {
			if(a.inputs.IsPressed(c.first)) {
				new_dir = c.second;
			}
		}
		
		if(d.play_funcs.DirsAreOrtho(new_dir, player_dir)) {
			player_dir = new_dir;
			move_timer = move_rate;
		}
	}
	
	// Movement on the title screen.
	else {
		
		// Just trace the circle
		if(0 == player_y && player_x < field_x_size - 1) {
			player_dir = PlayFuncs::dirs::right;
		}
		
		else if(field_x_size - 1 == player_x && player_y < field_y_size - 1) {
			player_dir = PlayFuncs::dirs::down;
		}
		
		else if(field_y_size - 1 == player_y && 0 < player_x) {
			player_dir = PlayFuncs::dirs::left;
		}
		
		else {
			player_dir = PlayFuncs::dirs::up;
		}
	}
	
	// Next movement tick.
	if(move_rate <= move_timer) {
		// Countdown fiel
		if(0 < skip_shrink) {
			skip_shrink--;
		}
		
		else {
			for(int x = 0; x < field_x_size; x++) {
				for(int y = 0; y < field_y_size; y++) {
					field [x] [y] = std::max(0, field [x] [y] - 1);
				}
			}
		}
		
		// Move player ahea
		move_timer = 0;
		d.play_funcs.DirOffset(player_x, player_y, player_dir);
		
		if(d.snake_movement.IsOutside(d.play_funcs.new_x, d.play_funcs.new_y)) {
			d.timer = -1;
		}
		
		else {
			player_x = d.play_funcs.new_x;
			player_y = d.play_funcs.new_y;
			
			field [player_x] [player_y] = player_len;
			
			if(food_x == player_x && food_y == player_y) {
				player_len++;
				skip_shrink++;
				score++;
				
				d.play_funcs.RandomFreePos();
				food_x = d.play_funcs.new_x;
				food_y = d.play_funcs.new_y;
			}
		}
	}
}

SdlDrawFuncs::SdlDrawFuncs (AppData& app_data) : a(app_data) {
}

void SdlDrawFuncs::Make () {
}

void SdlDrawFuncs::DrawTile (int x_tile, int y_tile, int x, int y) {
	auto tile_size = 16;
	y_tile += 2;
	
	SDL_Rect src { x_tile * tile_size, y_tile * tile_size, tile_size, tile_size };
	SDL_Rect dest { x * tile_size, y * tile_size, tile_size, tile_size };
	SDL_RenderCopy(a.renderer, a.sprite.texture, &src, &dest);
}

void SdlDrawFuncs::DrawText (std::string text, int x, int y) {
	auto font_tile_size = 7;
	
	SDL_Rect src { 0, 0, font_tile_size, font_tile_size };
	SDL_Rect dest { x, y, font_tile_size, font_tile_size };
	auto& texture = a.sprite;
	
	for(auto c: text) {
		FontChara(c);
		src.x = font_tile_size * font_x_tile;
		src.y = font_tile_size * font_y_tile;
		SDL_RenderCopy(a.renderer, texture.texture, &src, &dest);
		dest.x += font_tile_size;
	}
}

void SdlDrawFuncs::FontChara (char c) {
	if('A' <= c && c <= 'Z') {
		font_x_tile = c - 'A';
		font_y_tile = 0;
	}
	
	else if('a' <= c && c <= 'z') {
		font_x_tile = c - 'a';
		font_y_tile = 1;
	}
	
	else if('0' <= c && c <= '9') {
		font_x_tile = c - '0';
		font_y_tile = 2;
	}
	
	else {
		font_y_tile = 2;
		switch(c) {
		case ' ':
			font_x_tile = 26;
			font_y_tile = 0;
			break;
		case '-': font_x_tile = 10; break;
		case '+': font_x_tile = 11; break;
		case '.': font_x_tile = 12; break;
		case ',': font_x_tile = 13; break;
		case '/': font_x_tile = 14; break;
		case '\\': font_x_tile = 15; break;
		case '(': font_x_tile = 16; break;
		case ')': font_x_tile = 17; break;
		case '[': font_x_tile = 18; break;
		case ']': font_x_tile = 19; break;
		case '<': font_x_tile = 20; break;
		case '>': font_x_tile = 21; break;
		case '*': font_x_tile = 22; break;
		case '_': font_x_tile = 23; break;
		case '!': font_x_tile = 24; break;
		case '?': font_x_tile = 25; break;
		case '\'': font_x_tile = 26; break;
		case '&': font_x_tile = 27; break;
		case '%': font_x_tile = 28; break;
		case '{': font_x_tile = 29; break;
		case '}': font_x_tile = 30; break;
		case ':': font_x_tile = 31; break;
		}
	}
}

PngFile::PngFile (std::string path) {
	data = nullptr;
	x_size = 0;
	y_size = 0;
	
	lodepng_decode32_file(&data, &x_size, &y_size, path.c_str());
}

PngFile::~PngFile () {
	if(data) {
		delete [] data;
	}
}

StateData::StateData (AppData& app_data) : play_funcs(app_data, *this) {
	timer = 0;
}

AppState::AppState (AppData& app_data, StateData& state_data) : a(app_data), d(state_data) {
}

void InputEvents::Tick (std::string key, bool cond) {
	if(cond) {
		if(timers [key] <= 0) {
			timers [key] = 1;
		}
		
		else {
			timers [key]++;
		}
	}
	
	else {
		if(0 < timers [key]) {
			timers [key] = 0;
		}
		
		else {
			timers [key]--;
		}
	}
}

bool InputEvents::IsPressed (std::string key) {
	return 1 == timers [key];
}

bool InputEvents::IsReleased (std::string key) {
	return 0 == timers [key];
}

bool InputEvents::IsDown (std::string key)  {
	return 0 < timers [key];
}

bool InputEvents::IsUp (std::string key) {
	return timers [key] <= 0;
}

PlayFuncs::PlayFuncs (AppData& app_data, StateData& state_data) : a(app_data), d(state_data) {
}

std::string PlayFuncs::DirName (int dir) const {
	std::vector <std::string> dir_names = {
		"up",
		"right",
		"down",
		"left",
		"none"
	};
	
	return dir_names [std::clamp(dir, 0, (int)dir_names.size() - 1)];
}

PlayFuncs::DifficultyInfo PlayFuncs::Difficulty (int diff) const {
	std::vector <PlayFuncs::DifficultyInfo> difficulty = {
		{ "Slow", 14 },
		{ "Mid", 10 },
		{ "Fast", 7 },
		{ "Light", 5 },
		{ "Dunno", 4 },
	};
	
	return difficulty [std::clamp(diff, 0, (int)difficulty.size() - 1)];
}

bool PlayFuncs::DirsAreOrtho (int u, int v) {
	int opposite_v = OppositeDir(v);
	return u != v && u != opposite_v;
}

int PlayFuncs::OppositeDir (int dir) {
	int opposite_dirs [] = { 2, 3, 0, 1, 4 };
	new_dir = opposite_dirs [std::clamp(dir, 0, 4)];
	return new_dir;
}

void PlayFuncs::DirOffset (int x, int y, int dir) {
	int x_offsets [] = { 0, 1, 0, -1, 0 };
	int y_offsets [] = { -1, 0, 1, 0, 0 };
	
	x_offset = x_offsets [dir];
	y_offset = y_offsets [dir];
	new_x = x_offset + x;
	new_y = y_offset + y;
}

void PlayFuncs::RandomFreePos () {
	int x = 0;
	int y = 0;
	
	do {
		x = Random(0, d.snake_movement.field_x_size);
		y = Random(0, d.snake_movement.field_y_size);
	} while(
	(x == d.snake_movement.food_x && y == d.snake_movement.food_y) ||
	(0 < d.snake_movement.field [x] [y]));
	
	new_x = x;
	new_y = y;
}

int PlayFuncs::Random (int u, int v) {
	return a.random() % (v - u) + u;
}

SdlTexture::SdlTexture () {
	texture		= nullptr;
	surface		= nullptr;
	x_size		= 0;
	y_size		= 0;
}

SdlTexture::~SdlTexture () {
	if(texture) {
		SDL_DestroyTexture(texture);
	}
	
	if(surface) {
		SDL_FreeSurface(surface);
	}
}

bool SdlTexture::LoadFromPng (SDL_Renderer* renderer, std::string path) {
	PngFile png(path);
	surface = SDL_CreateRGBSurfaceFrom(
		png.data,
		png.x_size,
		png.y_size,
		32,
		4 * png.x_size,
		0x000000ff,
		0x0000ff00,
		0x00ff0000,
		0xff000000);
	
	texture = SDL_CreateTextureFromSurface(renderer, surface);
	x_size = png.x_size;
	y_size = png.y_size;
	return texture != nullptr;
}

void AppTitleState::OnFirstTick () {
}

void AppTitleState::OnTick () {
}

void AppTitleState::OnDraw () {
}

void AppPlayState::OnFirstTick () {
	
	d.difficulty = d.play_funcs.Difficulty(3);
	
	auto& s = d.snake_movement;
	d.tile_size = 16;
	s.field_x_size = std::max(1, a.canvas_x_size / d.tile_size);
	s.field_y_size = std::max(1, a.canvas_y_size / d.tile_size - 1);
	s.player_x = 1;
	s.player_y = s.field_y_size / 2;
	s.food_x = s.field_x_size - 1;
	s.food_y = s.player_y;
	s.player_dir = PlayFuncs::dirs::right;
	s.move_rate = d.difficulty.move_rate;
	s.player_len = 4;
	s.move_timer = 0;
	s.skip_shrink = 0;
	s.score = 0;
	s.player_controlled = true;
	
	// Clear the grid.
	for(int x = 0; x < s.field_x_size; x++) {
		for(int y = 0; y < s.field_y_size; y++) {
			s.field [x] [y] = 0;
		}
	}
}

void AppPlayState::OnTick () {
	d.snake_movement.Tick(a, d);
}

void AppPlayState::OnDraw () {
	
	auto& s = d.snake_movement;
	
	// Border
	{
		SDL_Rect rect = { 1, 1, d.tile_size * s.field_x_size - 2, d.tile_size * s.field_y_size - 2 };
		SDL_SetRenderDrawColor(a.renderer, 0xcd, 0xdf, 0x6c, 0xff);
		SDL_RenderDrawRect(a.renderer, &rect);
	}
	
	// Snake and food.
	SDL_SetRenderDrawColor(a.renderer, 0xff, 0xff, 0xff, 0xff);
	
	for(int x = 0; x < s.field_x_size; x++) {
		for(int y = 0; y < s.field_y_size; y++) {
			auto v = s.field [x] [y];
			
			if(0 < v) {
				if(v == s.player_len) {
					a.draw_funcs.DrawTile(5 + s.player_dir, 0, x, y);
				}
				
				else {
					a.draw_funcs.DrawTile(1 + 4 * v / s.player_len, 0, x, y);
				}
				// SDL_SetRenderDrawColor(a.renderer, 0xff, 0xff, 0xff, 0xff);
				// SDL_Rect rect = { x * d.tile_size, y * d.tile_size, d.tile_size, d.tile_size };
				// SDL_RenderFillRect(a.renderer, &rect);
			}
		}
	}
	
	{
		a.draw_funcs.DrawTile(0, 0, s.food_x, s.food_y);
		// SDL_Rect rect = { s.food_x * d.tile_size, s.food_y * d.tile_size, d.tile_size, d.tile_size };
		// SDL_SetRenderDrawColor(a.renderer, 0xff, 0x80, 0x30, 0xff);
		// SDL_RenderFillRect(a.renderer, &rect);
	}
	
	// Low bar for info display.
	{
		SDL_Rect rect = { 0, a.canvas_y_size - 8, a.canvas_x_size, 8 };
		SDL_SetRenderDrawColor(a.renderer, 0, 0, 0, 0xff);
		SDL_RenderFillRect(a.renderer, &rect);
	}
	
	// Score
	{
		auto bottom_y = a.canvas_y_size - 8;
		a.draw_funcs.DrawText(d.difficulty.name, 0, bottom_y);
		a.draw_funcs.DrawText(std::string("Score-") + std::to_string(d.snake_movement.score), 7 * 6, bottom_y);
		
		// Enough for 99:59
		int max_timer = 99 * 60 + 59;
		
		int timer = std::min(d.timer, max_timer);
		int seconds = (timer / 60) % 60;
		int minutes = timer / (60 * 60);
		
		// Output
		std::string time_string = "00-00";
		time_string [0] = (minutes / 10) + '0';
		time_string [1] = (minutes % 10) + '0';
		time_string [3] = (seconds / 10) + '0';
		time_string [4] = (seconds % 10) + '0';
		a.draw_funcs.DrawText(time_string, 9 * 14, bottom_y);
	}
}

AppLoop::AppLoop (AppData& app_data) : a(app_data) {}

// SDL App loop.
int AppLoop::MainLoop () {
	SDL_Init(SDL_INIT_EVERYTHING);
	
	a.window = SDL_CreateWindow(
		"Snake Game",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		a.window_x_size, a.window_y_size,
		SDL_WINDOW_RESIZABLE);
	a.renderer = SDL_CreateRenderer(a.window, -1, 0);
	a.keyboard = SDL_GetKeyboardState(nullptr);
	
	StateData state_data(a);
	state_data.timer = 0;
	
	AppPlayState play_state(a, state_data);
	AppTitleState title_state(a, state_data);
	a.current_state = &play_state;
	
	long long previous_tick = 0;
	long long ticks_per_frame = 1000.0 / 60;
	
	auto canvas_x_size = a.canvas_x_size;
	auto canvas_y_size = a.canvas_y_size;
	a.target = SDL_CreateTexture(a.renderer, SDL_PIXELFORMAT_RGB888, SDL_TEXTUREACCESS_TARGET, canvas_x_size, canvas_y_size);
	
	a.sprite.LoadFromPng(a.renderer, "sprites.png");
	
	while(a.exit_clicked < 1) {
		
		// Limit frames per second.
		if(SDL_GetTicks() < previous_tick + ticks_per_frame) {
			continue;
		}
		
		// Let SDL handle window, keys, OS specific events.
		while(SDL_PollEvent(&a.event)) {
			switch(a.event.type) {
			case SDL_QUIT:
				a.exit_clicked++;
				break;
			}
		}
		
		// Get window sizes and calculate canvas offsets.
		SDL_GetWindowSize(a.window, &a.window_x_size, &a.window_y_size);
		
		auto canvas_scale = std::max(1, std::min(a.window_x_size / canvas_x_size, a.window_y_size / canvas_y_size));
		auto canvas_scaled_x = canvas_scale * canvas_x_size;
		auto canvas_scaled_y = canvas_scale * canvas_y_size;
		auto canvas_x = 0.5 * (a.window_x_size - canvas_scaled_x);
		auto canvas_y = 0.5 * (a.window_y_size - canvas_scaled_y);
		
		// Count all inputs.
		a.inputs.Tick("up", a.keyboard [SDL_SCANCODE_UP] || a.keyboard [SDL_SCANCODE_W]);
		a.inputs.Tick("down", a.keyboard [SDL_SCANCODE_DOWN] || a.keyboard [SDL_SCANCODE_S]);
		a.inputs.Tick("left", a.keyboard [SDL_SCANCODE_LEFT] || a.keyboard [SDL_SCANCODE_A]);
		a.inputs.Tick("right", a.keyboard [SDL_SCANCODE_RIGHT] || a.keyboard [SDL_SCANCODE_D]);
		a.inputs.Tick("confirm", a.keyboard [SDL_SCANCODE_Y] || a.keyboard [SDL_SCANCODE_Z]);
		a.inputs.Tick("cancel", a.keyboard [SDL_SCANCODE_X]);
		
		// Game state handling.
		OnTick();
		
		// Renderer handling.
		SDL_SetRenderDrawColor(a.renderer, 0x1a, 0x36, 0x2e, 0xff);
		SDL_RenderClear(a.renderer);
		
		SDL_SetRenderTarget(a.renderer, a.target);
		SDL_SetRenderDrawColor(a.renderer, 0x0a, 0x26, 0x1e, 0xff);
		SDL_RenderClear(a.renderer);
		
		// Draw the game state to the smaller canvas. It will be scaled up.
		OnDraw();
		
		SDL_Rect canvas_rect = {
			(int)canvas_x, (int)canvas_y,
			canvas_scaled_x, canvas_scaled_y
		};
		
		SDL_SetRenderTarget(a.renderer, nullptr);
		SDL_RenderCopy(a.renderer, a.target, nullptr, &canvas_rect);
		SDL_RenderPresent(a.renderer);
		previous_tick = SDL_GetTicks();
	}
	
	SDL_DestroyTexture(a.target);
	SDL_Quit();
	return 0;
}

void AppLoop::OnTick () {
	if(0 == a.current_state->d.timer) {
		a.current_state->OnFirstTick();
	}
	
	a.current_state->OnTick();
	a.current_state->d.timer++;
}

void AppLoop::OnDraw () {
	a.current_state->OnDraw();
}

AppData::AppData () : draw_funcs(*this) {
	DefaultSettings();
}

AppData::~AppData () {
}

void AppData::DefaultSettings () {
	window_x_size = 800;
	window_y_size = 600;
	canvas_x_size = 160;
	canvas_y_size = 144;
	exit_clicked = 0;
}

int main (int argc, char* argv []) {
	AppData app_data;
	AppLoop app_loop(app_data);
	
	app_data.cmd_arg_count = argc;
	app_data.cmd_args = argv;
	return app_loop.MainLoop();
}
