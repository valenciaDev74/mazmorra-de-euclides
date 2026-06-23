// ============================================================
// MAZMORRA DE EUCLIDES - un jueguito en raylib
// ============================================================
// El jugador se mueve por un mapa cuadriculado con las flechas
// y dispara balas con la tecla A.
// ============================================================

#include <raylib.h>

#include <iostream>
#include <unordered_map>
#include <vector>

using namespace std;

// ----------------------------------------------------------
// Constantes del juego
// ----------------------------------------------------------
const int WINDOW_WIDTH = 720;
const int WINDOW_HEIGHT = 720;
const int MAP_WIDTH = 9;
const int MAP_HEIGHT = 9;
const int TILE_SIZE = 16;

// ----------------------------------------------------------
// Posibles estados del juego
// ----------------------------------------------------------
enum class GameState { MENU, PLAYING, GAMEOVER, MESSAGE, COMBAT };

// ----------------------------------------------------------
// Manejador de texturas
// Guarda texturas en un diccionario para usarlas donde sea
// ----------------------------------------------------------
struct TextureManager {
  unordered_map<string, Texture2D> textures;

  void Load(const string& id, const string& filePath) {
    if (textures.find(id) != textures.end()) return;

    Texture2D tex = LoadTexture(filePath.c_str());

    if (tex.id == 0) {
      cout << "ERROR: no se pudo cargar la textura: " << filePath << endl;
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

struct GameMap {
  string name;
  int width;
  int height;
  vector<vector<int>> baseLayer;

  bool IsValidTile(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height);
  }
};

GameMap Afuera = {
    "Afuera",
    9,
    9,
    {
        {4, 4, 4, 0, 0, 0, 4, 4, 4},
        {15, 15, 15, 4, 8, 4, 5, 15, 15},
        {1, 15, 5, 2, 15, 15, 15, 15, 15},
        {15, 15, 15, 15, 15, 15, 15, 15, 15},
        {15, 15, 15, 15, 15, 15, 1, 15, 15},
        {15, 15, 1, 15, 15, 15, 15, 5, 1},
        {15, 15, 15, 15, 15, 15, 15, 15, 15},
        {15, 15, 15, 15, 15, 1, 15, 15, 1},
        {15, 15, 15, 1, 15, 15, 15, 15, 15},
    },
};

GameMap Mazmorra = {
    "Mazmorra",
    9,
    9,
    {
        {0, 0, 4, 4, 4, 0, 0, 0, 0},
        {0, 0, 15, 15, 6, 0, 0, 0, 0},
        {0, 4, 15, 0, 0, 0, 0, 0, 0},
        {4, 2, 15, 4, 4, 4, 4, 4, 4},
        {15, 15, 5, 15, 15, 15, 7, 15, 8},
        {0, 0, 0, 0, 15, 0, 0, 0, 0},
        {0, 0, 0, 0, 5, 0, 0, 0, 0},
        {0, 0, 0, 0, 6, 0, 0, 0, 0},
        {0, 0, 0, 0, 0, 0, 0, 0, 0},
    },
};

// ----------------------------------------------------------
// Pregunta si un tipo de casilla es pared (no se puede pasar)
// ----------------------------------------------------------
bool IsWall(int tileType) {
  return tileType == 1 || tileType == 4 || tileType == 0;
}

// bool isSign(int tileType) { return tileType == 2 || tileType == 3; }

// ----------------------------------------------------------
// Revisa si el jugador se puede mover a una posicion
// sin chocar contra una pared
// ----------------------------------------------------------
bool CanMoveTo(Vector2 center, vector<vector<int>>& map) {
  int mapHeight = map.size();
  int mapWidth = map[0].size();

  int left = (int)(center.x - TILE_SIZE / 4) / TILE_SIZE;
  int right = (int)(center.x + TILE_SIZE / 4) / TILE_SIZE;
  int top = (int)(center.y - TILE_SIZE / 4) / TILE_SIZE;
  int bottom = (int)(center.y + TILE_SIZE / 4) / TILE_SIZE;

  left = max(0, left);
  right = min(mapWidth - 1, right);
  top = max(0, top);
  bottom = min(mapHeight - 1, bottom);

  for (int y = top; y <= bottom; y++) {
    for (int x = left; x <= right; x++) {
      if (IsWall(map[y][x])) return false;
    }
  }

  // que no se salga del mapa
  float halfSize = TILE_SIZE / 4;
  if (center.x - halfSize < 0) return false;
  if (center.x + halfSize > mapWidth * TILE_SIZE) return false;
  if (center.y - halfSize < 0) return false;
  if (center.y + halfSize > mapHeight * TILE_SIZE) return false;

  return true;
}

// ----------------------------------------------------------
// EL JUGADOR
// ----------------------------------------------------------
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

  // direcciones: 0=abajo, 1=izquierda, 2=derecha, 3=arriba

  void Update(float deltaTime, vector<vector<int>>& map) {
    bool moving = false;
    Vector2 nextPos = position;

    if (IsKeyDown(KEY_DOWN)) {
      nextPos.y += speed * deltaTime;
      direction = 0;
      moving = true;
    }

    if (IsKeyDown(KEY_UP)) {
      nextPos.y -= speed * deltaTime;
      direction = 3;
      moving = true;
    }

    if (IsKeyDown(KEY_LEFT)) {
      nextPos.x -= speed * deltaTime;
      direction = 2;
      moving = true;
    }
    if (IsKeyDown(KEY_RIGHT)) {
      nextPos.x += speed * deltaTime;
      direction = 1;
      moving = true;
    }

    if (moving) {
      // Intentar moverse en X, luego en Y (separado para no trabarse en
      // esquinas)
      Vector2 tryPos = {nextPos.x, position.y};
      if (CanMoveTo(tryPos, map)) position.x = nextPos.x;

      tryPos = {position.x, nextPos.y};
      if (CanMoveTo(tryPos, map)) position.y = nextPos.y;

      // Cambiar el frame de animacion cada cierto tiempo
      frameTimer += deltaTime;
      if (frameTimer >= ANIM_SPEED) {
        frameTimer = 0.0f;
        currentFrame = (currentFrame == 1) ? 2 : 1;
      }
    } else {
      // Quieto => frame 0 (sin animacion)
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

// ----------------------------------------------------------
// LAS BALAS que dispara el jugador
// ----------------------------------------------------------
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

// ----------------------------------------------------------
// Dibuja el mapa completo con la hoja de tiles
// ----------------------------------------------------------
void DrawMap(vector<vector<int>>& map, Texture2D tileset) {
  int tilesPerRow = tileset.width / TILE_SIZE;

  for (int y = 0; y < map.size(); y++) {
    for (int x = 0; x < map[y].size(); x++) {
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

// ----------------------------------------------------------
// Dibuja un cuadro de dialogo estilo pixel art
// ----------------------------------------------------------
void DrawDialogueBox(const string& message) {
  int boxX = 30;
  int boxY = WINDOW_HEIGHT - 110;
  int boxW = WINDOW_WIDTH - 60;
  int boxH = 90;

  // Fondo del cuadro
  DrawRectangle(boxX, boxY, boxW, boxH, {34, 32, 52, 220});

  // Borde exterior
  DrawRectangleLinesEx({(float)boxX, (float)boxY, (float)boxW, (float)boxH}, 3,
                       {70, 65, 90, 255});

  // Brillo en la esquina superior izquierda (efecto pixel art)
  DrawRectangle(boxX + 4, boxY + 4, boxW - 40, 1, {90, 85, 110, 255});

  // Texto centrado, dividido por saltos de linea
  int fontSize = 20;
  int padding = 4;
  int drawY = boxY + (boxH / 2) - fontSize;
  string line = "";
  for (char c : message) {
    if (c == '\n') {
      int lineW = MeasureText(line.c_str(), fontSize);
      DrawText(line.c_str(), (WINDOW_WIDTH - lineW) / 2, drawY, fontSize,
               WHITE);
      drawY += fontSize + 6;
      line = "";
    } else {
      line += c;
    }
  }
  int lineW = MeasureText(line.c_str(), fontSize);
  DrawText(line.c_str(), (WINDOW_WIDTH - lineW) / 2, drawY, fontSize, WHITE);
}

// ----------------------------------------------------------
// PROGRAMA PRINCIPAL
// ----------------------------------------------------------
int main() {
  // --- INICIALIZACION ---
  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "mazmorra de euclides");
  SetTargetFPS(60);

  TextureManager texManager;
  texManager.Load("tileset", "assets/pared-mazmorra.png");
  texManager.Load("player", "assets/player.png");
  texManager.Load("sword", "assets/espada.png");
  texManager.Load("big-sword", "assets/espada-grande.png");

  Texture2D tileset = texManager.Get("tileset");
  SetTextureFilter(tileset, TEXTURE_FILTER_POINT);
  GameMap* currentMap = &Afuera;

  // El "lienzo" donde dibujamos el juego
  // (para luego estirarlo al tamano de la ventana)
  int virtualWidth = currentMap->width * TILE_SIZE;
  int virtualHeight = currentMap->height * TILE_SIZE;

  RenderTexture2D canvas = LoadRenderTexture(virtualWidth, virtualHeight);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

  Rectangle canvasSourceRec = {0.0f, 0.0f, (float)virtualWidth,
                               (float)-virtualHeight};
  Rectangle destRec = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
  Vector2 origin = {0.0f, 0.0f};

  // --- VARIABLES DEL JUEGO ---
  GameState currentState = GameState::MENU;

  // El jugador empieza en la posicion (72, 72), velocidad 50, frame 0
  Player player = {{72, 72}, 50.0f, texManager.Get("player"), 0, 0.0f, 0};
  Texture2D sword = texManager.Get("sword");

  bool tieneEspada = false;
  int espadaTileX = 7;
  int espadaTileY = 1;

  int potions = 3;
  int mostersDefeated = 0;

  vector<Bullet> bullets;  // lista de balas activas

  string currentMessage = "";
  // --- BUCLE PRINCIPAL DEL JUEGO ---
  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    // ============ ACTUALIZAR ============
    switch (currentState) {
      case GameState::MENU:
        if (IsKeyPressed(KEY_ENTER)) {
          currentState = GameState::PLAYING;
        }
        break;

      case GameState::PLAYING: {
        player.Update(deltaTime, currentMap->baseLayer);

        // Obtener la posicion del jugador en el mapa
        // osea en que tile esta
        int playerTileX = (int)(player.position.x / TILE_SIZE);
        int playerTileY = (int)(player.position.y / TILE_SIZE);

        if (currentMap->IsValidTile(playerTileX, playerTileY)) {
          int tileID = currentMap->baseLayer[playerTileY][playerTileX];
          string newMessage = "";

          // Cartel
          if (tileID == 2) {
            if (currentMap == &Afuera) {
              newMessage = "Bienvenido a la mazmorra de euclides";
            } else if (currentMap == &Mazmorra) {
              newMessage =
                  "estos monstruos protegen la puerta de salida \n para "
                  "poder "
                  "salir debes derrotarlos matematicamente";
            }
          }

          // Espada: muestra mensaje y recoge al presionar A
          if (!tieneEspada && playerTileX == espadaTileX &&
              playerTileY == espadaTileY) {
            newMessage = "Presiona A para recoger la espada";
            if (IsKeyPressed(KEY_A)) {
              tieneEspada = true;
            }
          }

          currentMessage = newMessage;

          if (tileID == 8) {
            if (!tieneEspada) {
              currentMessage = "te la van a meter si entras sin espada";
            } else {
              currentMap = &Mazmorra;
              player.position.x = TILE_SIZE / 2.0f;
              player.position.y = 4 * TILE_SIZE + TILE_SIZE / 2.0f;
            };
          };

          if (tileID == 6 || tileID == 7) {
            currentState = GameState::COMBAT;
          };

          // Disparar una bala con la tecla A (solo si no esta recogiendo la
          // espada)
          if (IsKeyPressed(KEY_A)) {
            bool sobreEspada = (!tieneEspada && playerTileX == espadaTileX &&
                                playerTileY == espadaTileY);
            if (!sobreEspada) {
              bullets.push_back(
                  {{player.position.x, player.position.y}, 500.0f, true});
            }
          }

          // Mover todas las balas una por una
          for (int i = 0; i < bullets.size(); i++) {
            bullets[i].Update(deltaTime);
          }

          // Guardar solo las balas que siguen activas
          {
            vector<Bullet> activeBullets;
            for (int i = 0; i < bullets.size(); i++) {
              if (bullets[i].active) {
                activeBullets.push_back(bullets[i]);
              }
            }
            bullets = activeBullets;
          }
          break;
        }

        case GameState::GAMEOVER:
          bullets.clear();
          player.position = {72, 72};
          break;
      }

        // ============ DIBUJAR ============
        BeginTextureMode(canvas);
        ClearBackground({34, 32, 52, 255});

        switch (currentState) {
          case GameState::MENU:
            break;

          case GameState::PLAYING:
            DrawMap(currentMap->baseLayer, tileset);

            // dibujar espada
            if (!tieneEspada) {
              Vector2 pos = {(float)(espadaTileX * TILE_SIZE),
                             (float)(espadaTileY * TILE_SIZE)};
              DrawTextureV(sword, pos, WHITE);
            }

            player.Draw();

            // Dibujar todas las balas
            for (int i = 0; i < bullets.size(); i++) {
              bullets[i].Draw();
            }

            break;

          case GameState::COMBAT:

            break;
          case GameState::GAMEOVER:
            DrawText("Game Over", 16, 16, 20, LIGHTGRAY);
            break;

          default:
            break;
        }

        EndTextureMode();

        BeginDrawing();
        DrawTexturePro(canvas.texture, canvasSourceRec, destRec, origin, 0.0f,
                       WHITE);
        // casos especiales que no quiero que me toque el texturemode
        if (currentState == GameState::MENU) {
          string text = "Presiona ENTER para empezar";
          int textWidth = MeasureText("Presiona ENTER para empezar", 20);
          DrawText(text.c_str(), (WINDOW_WIDTH - textWidth) / 2.0f,
                   WINDOW_HEIGHT / 2.0f, 20, LIGHTGRAY);
        }
        if (currentState == GameState::PLAYING && !currentMessage.empty()) {
          DrawDialogueBox(currentMessage);
        }
        EndDrawing();
    }

    // --- LIMPIEZA ---
    texManager.UnloadAll();
    CloseWindow();
    return 0;
  }
