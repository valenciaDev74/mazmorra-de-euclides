#include <raylib.h>

#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

const int WINDOW_WIDTH = 720;
const int WINDOW_HEIGHT = 720;
const int MAP_WIDTH = 9;
const int MAP_HEIGHT = 9;
const int TILE_SIZE = 16;

enum class GameState { MENU, PLAYING, GAMEOVER };

struct TextureManager {
  unordered_map<string, Texture2D> textures;

  void Load(const string& id, const string& filePath) {
    if (textures.find(id) != textures.end()) return;

    Texture2D tex = LoadTexture(filePath.c_str());

    if (tex.id == 0) {
      cout << "no se pudo cargar la textura desde: " << filePath << endl;
    } else {
      textures[id] = tex;
    }
  }

  Texture2D Get(const string& id) { return textures[id]; }

  void UnloadAll() {
    for (auto& pair : textures) {
      UnloadTexture(pair.second);
    }
    textures.clear();
  }
};

bool IsWall(int tileType) { return tileType == 1 || tileType == 4; };

bool CanMoveTo(Vector2 center, int map[MAP_HEIGHT][MAP_WIDTH]) {
  int left = (int)(center.x - TILE_SIZE / 2) / TILE_SIZE;
  int right = (int)(center.x + TILE_SIZE / 2) / TILE_SIZE;
  int top = (int)(center.y - TILE_SIZE / 2) / TILE_SIZE;
  int bottom = (int)(center.y + TILE_SIZE / 2) / TILE_SIZE;

  left = max(0, left);
  right = min(MAP_WIDTH - 1, right);
  top = max(0, top);
  bottom = min(MAP_HEIGHT - 1, bottom);

  for (int y = top; y <= bottom; y++) {
    for (int x = left; x <= right; x++) {
      if (IsWall(map[y][x])) return false;
    }
  }

  return true;
};

struct Player {
  Vector2 position;
  float speed;
  Texture2D texture;
  int currentFrame;
  float frameTimer;
  int direction;

  static constexpr int FRAME_WIDTH = 16;
  static constexpr int FRAME_HEIGHT = 16;
  static constexpr int FRAME_COUNT = 3;
  static constexpr float ANIM_SPEED = 0.15f;

  // direction order: 0=down, 1=left, 2=right, 3=up

  void Update(float deltaTime, int map[MAP_HEIGHT][MAP_WIDTH]) {
    bool moving = false;
    Vector2 nextPos = position;

    if (IsKeyDown(KEY_UP)) {
      nextPos.y -= speed * deltaTime;
      direction = 3;
      moving = true;
    }
    if (IsKeyDown(KEY_DOWN)) {
      nextPos.y += speed * deltaTime;
      direction = 0;
      moving = true;
    }
    if (IsKeyDown(KEY_LEFT)) {
      nextPos.x -= speed * deltaTime;
      direction = 1;
      moving = true;
    }
    if (IsKeyDown(KEY_RIGHT)) {
      nextPos.x += speed * deltaTime;
      direction = 2;
      moving = true;
    }

    if (moving) {
      Vector2 tryPos = {nextPos.x, position.y};
      if (CanMoveTo(tryPos, map)) position.x = nextPos.x;

      tryPos = {position.x, nextPos.y};
      if (CanMoveTo(tryPos, map)) position.y = nextPos.y;

      frameTimer += deltaTime;
      if (frameTimer >= ANIM_SPEED) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame == 1) ? 2 : 1;
      }
    } else {
      currentFrame = 0;
      frameTimer = 0.0f;
    }
  }

  void Draw() {
    Rectangle src = {(float)(currentFrame * FRAME_WIDTH),
                     (float)(direction * FRAME_HEIGHT), (float)FRAME_WIDTH,
                     (float)FRAME_HEIGHT};
    Vector2 drawPos = {position.x - FRAME_WIDTH / 2.0f,
                       position.y - FRAME_HEIGHT / 2.0f};
    DrawTextureRec(texture, src, drawPos, WHITE);
  }
};

struct Bullet {
  Vector2 position;
  float speed;
  bool active;

  void Update(float deltaTime) {
    position.y += deltaTime * speed;
    if (position.y < 0) active = false;
  }

  void Draw() {
    if (active) {
      DrawCircleV(position, 5, YELLOW);
    }
  }
};

void DrawMap(int map[MAP_HEIGHT][MAP_WIDTH], Texture2D tileset) {
  int tilesPerRow = tileset.width / TILE_SIZE;

  for (int y = 0; y < MAP_HEIGHT; y++) {
    for (int x = 0; x < MAP_WIDTH; x++) {
      int tileType = map[y][x];

      Rectangle tileSrc = {
          (float)((tileType % tilesPerRow) * TILE_SIZE),
          (float)((tileType / tilesPerRow) * TILE_SIZE),
          (float)TILE_SIZE,
          (float)TILE_SIZE,
      };
      Vector2 screenPos = {(float)(x * TILE_SIZE), (float)(y * TILE_SIZE)};

      DrawTextureRec(tileset, tileSrc, screenPos, WHITE);
    }
  }
}

int main() {
  cout << "Hello World!" << endl;

  // Initialization
  //--------------------------------------------------------------------------------------
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "mazmorra de euclides");
  SetTargetFPS(60);

  TextureManager texManager;
  texManager.Load("tileset", "assets/pared-mazmorra.png");
  texManager.Load("player", "assets/player.png");

  Texture2D tileset = texManager.Get("tileset");
  SetTextureFilter(tileset, TEXTURE_FILTER_POINT);

  int virtualWidth = MAP_WIDTH * TILE_SIZE;
  int virtualHeight = MAP_HEIGHT * TILE_SIZE;

  RenderTexture2D canvas = LoadRenderTexture(virtualWidth, virtualHeight);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

  Rectangle canvasSourceRec = {0.0f, 0.0f, (float)virtualWidth,
                               (float)-virtualHeight};
  Rectangle destRec = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
  Vector2 origin = {0.0f, 0.0f};

  // clang-format off
  int map[MAP_HEIGHT][MAP_WIDTH] = {{4, 4, 4, 0, 15, 0, 4, 4, 4},
                                    {15, 15, 15, 4, 15, 4, 15, 15, 15},
                                    {1, 15, 15, 15, 15, 15, 15, 15, 15},
                                    {15, 15, 15, 15, 15, 15, 15, 15, 15},
                                    {15, 15, 15, 15, 15, 15, 1, 15, 15},
                                    {15, 15, 1, 15, 15, 15, 15, 15, 1},
                                    {15, 15, 15, 15, 15, 15, 15, 15, 15},
                                    {15, 15, 15, 15, 15, 1, 15, 15, 1},
                                    {15, 15, 15, 1, 15, 15, 15, 15, 15}};
  // clang-format on
  //--------------------------------------------------------------------------------------

  GameState currentState = GameState::MENU;
  Player player = {{72, 72}, 50.0f, texManager.Get("player"), 0, 0.0f, 0};
  vector<Bullet> bullets;

  // Main game loop
  //--------------------------------------------------------------------------------------
  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    // Update
    //--------------------------------------------------------------------------------------
    switch (currentState) {
      case GameState::MENU:
        if (IsKeyPressed(KEY_ENTER)) {
          currentState = GameState::PLAYING;
        }
        break;

      case GameState::PLAYING:
        player.Update(deltaTime, map);

        if (IsKeyPressed(KEY_A)) {
          bullets.push_back(
              {{player.position.x, player.position.y}, 500.0f, true});
        }

        for (auto& b : bullets) b.Update(deltaTime);

        erase_if(bullets, [](const Bullet& b) { return !b.active; });
        break;

      case GameState::GAMEOVER:
        break;
    }

    // Draw
    //--------------------------------------------------------------------------------------
    BeginTextureMode(canvas);
    ClearBackground({34, 32, 52, 255});

    switch (currentState) {
      case GameState::MENU:
        DrawText("Press ENTER to Start", 190, 200, 20, LIGHTGRAY);
        break;

      case GameState::PLAYING:
        DrawMap(map, tileset);
        for (auto& b : bullets) b.Draw();
        player.Draw();
        break;

      default:
        break;
    }

    EndTextureMode();

    BeginDrawing();
    DrawTexturePro(canvas.texture, canvasSourceRec, destRec, origin, 0.0f,
                   WHITE);
    EndDrawing();
  }

  // Cleanup
  //--------------------------------------------------------------------------------------
  texManager.UnloadAll();
  CloseWindow();
  return 0;
}
