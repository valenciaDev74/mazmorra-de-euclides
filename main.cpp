/**
 * @file main.cpp
 * @brief Mazmorra de Euclides - Juego matematico en 2D con raylib.
 *
 * @details Juego de mazmorras con movimiento en cuadricula, recoleccion de
 * objetos, combate matematico (algoritmo de Euclides) y shader CRT.
 */

#include <raylib.h>

#include <cstdlib>
#include <ctime>
#include <iostream>
#include <string>
#include <unordered_map>
#include <vector>

using namespace std;

/** @name Constantes del juego */
///@{
/** @brief Ancho de la ventana en pixels. */
const int WINDOW_WIDTH = 720;
/** @brief Alto de la ventana en pixels. */
const int WINDOW_HEIGHT = 720;
/** @brief Ancho del mapa en tiles. */
const int MAP_WIDTH = 9;
/** @brief Alto del mapa en tiles. */
const int MAP_HEIGHT = 9;
/** @brief Tamano de cada tile en pixels. */
const int TILE_SIZE = 16;
///@}

/** @brief Posibles estados del juego. */
enum class GameState {
  MENU,     /**< Menu principal. */
  PLAYING,  /**< Exploracion del mapa. */
  GAMEOVER, /**< Pantalla de derrota. */
  COMBAT,   /**< Pantalla de combate matematico. */
  WIN       /**< Pantalla de victoria. */
};

/** @brief Manejador de texturas con cacheo por identificador. */
struct TextureManager {
  unordered_map<string, Texture2D> textures; /**< Mapa id -> textura. */

  /**
   * @brief Carga una textura desde archivo si no esta en cache.
   * @param id Identificador unico para la textura.
   * @param filePath Ruta al archivo de imagen.
   */
  void Load(const string& id, const string& filePath) {
    if (textures.find(id) != textures.end()) return;

    Texture2D tex = LoadTexture(filePath.c_str());

    if (tex.id == 0) {
      cout << "ERROR: no se pudo cargar la textura: " << filePath << endl;
    } else {
      textures[id] = tex;
    }
  }

  /**
   * @brief Obtiene una textura cargada.
   * @param id Identificador de la textura.
   * @return Referencia a la Texture2D.
   */
  Texture2D Get(const string& id) { return textures[id]; }

  /** @brief Libera todas las texturas cargadas. */
  void UnloadAll() {
    for (auto& pair : textures) {
      UnloadTexture(pair.second);
    }
    textures.clear();
  }
};

/** @brief Manejador de sonidos y musica con cacheo por identificador. */
struct SoundManager {
  unordered_map<string, Sound> sounds; /**< Mapa id -> efecto de sonido. */
  unordered_map<string, Music> music;  /**< Mapa id -> stream de musica. */

  /**
   * @brief Carga un efecto de sonido desde archivo.
   * @param id Identificador unico.
   * @param filePath Ruta al archivo de audio.
   */
  void Load(const string& id, const string& filePath) {
    if (sounds.find(id) != sounds.end()) return;

    Sound snd = LoadSound(filePath.c_str());

    if (snd.frameCount == 0) {
      cout << "ERROR: no se pudo cargar el sonido: " << filePath << endl;
    } else {
      sounds[id] = snd;
    }
  }

  /**
   * @brief Obtiene un efecto de sonido cargado.
   * @param id Identificador del sonido.
   * @return Referencia al Sound.
   */
  Sound Get(const string& id) { return sounds[id]; }

  /**
   * @brief Carga un stream de musica desde archivo.
   * @param id Identificador unico.
   * @param filePath Ruta al archivo de musica.
   */
  void LoadMusic(const string& id, const string& filePath) {
    if (music.find(id) != music.end()) return;

    Music mus = LoadMusicStream(filePath.c_str());

    if (mus.frameCount == 0) {
      cout << "ERROR: no se pudo cargar la musica: " << filePath << endl;
    } else {
      music[id] = mus;
    }
  }

  /**
   * @brief Obtiene un stream de musica cargado.
   * @param id Identificador de la musica.
   * @return Referencia al Music.
   */
  Music GetMusic(const string& id) { return music[id]; }

  /** @brief Libera todos los sonidos y streams de musica cargados. */
  void UnloadAll() {
    for (auto& pair : sounds) {
      UnloadSound(pair.second);
    }
    sounds.clear();
    for (auto& pair : music) {
      UnloadMusicStream(pair.second);
    }
    music.clear();
  }
};

/** @brief Representa un mapa del juego con capa de tiles. */
struct GameMap {
  string name;                   /**< Nombre del mapa. */
  int width;                     /**< Ancho en tiles. */
  int height;                    /**< Alto en tiles. */
  vector<vector<int>> baseLayer; /**< Matriz de tiles (y, x). */

  /**
   * @brief Verifica si una coordenada de tile esta dentro del mapa.
   * @param x Coordenada X en tiles.
   * @param y Coordenada Y en tiles.
   * @return true si la posicion es valida.
   */
  bool IsValidTile(int x, int y) const {
    return (x >= 0 && x < width && y >= 0 && y < height);
  }
};

/** @brief Mapa exterior inicial del juego. */
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

/** @brief Mapa de la mazmorra interior con monstruos. */
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

/**
 * @brief Determina si un tipo de tile es una pared (no transitable).
 * @param tileType Tipo de tile a evaluar.
 * @return true si es pared (tipo 0, 1 o 4).
 */
bool IsWall(int tileType) {
  return tileType == 1 || tileType == 4 || tileType == 0;
}

/**
 * @brief Verifica colision rectangular contra el mapa.
 * @details Comprueba si las cuatro esquinas del bounding box del jugador
 *          colisionan con paredes o se salen de los limites del mapa.
 * @param center Posicion central del jugador.
 * @param map Matriz de tiles del mapa actual.
 * @return true si el movimiento es valido (sin colision).
 */
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

  float halfSize = TILE_SIZE / 4;
  if (center.x - halfSize < 0) return false;
  if (center.x + halfSize > mapWidth * TILE_SIZE) return false;
  if (center.y - halfSize < 0) return false;
  if (center.y + halfSize > mapHeight * TILE_SIZE) return false;

  return true;
}

/** @brief Entidad del jugador con animacion y movimiento. */
struct Player {
  Vector2 position;  /**< Posicion actual en pixels. */
  float speed;       /**< Velocidad de movimiento (px/s). */
  Texture2D texture; /**< Spritesheet del jugador. */
  int currentFrame;  /**< Frame actual de animacion (0-2). */
  float frameTimer;  /**< Temporizador para cambio de frame. */
  int direction; /**< Direccion: 0=abajo, 1=derecha, 2=izquierda, 3=arriba. */

  static constexpr int FRAME_WIDTH = 16;
  static constexpr int FRAME_HEIGHT = 16;
  static constexpr int FRAME_COUNT = 3;
  static constexpr float ANIM_SPEED = 0.3f;

  /**
   * @brief Actualiza posicion y animacion del jugador.
   * @param deltaTime Tiempo transcurrido desde el ultimo frame.
   * @param map Matriz de tiles del mapa actual.
   */
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
      Vector2 tryPos = {nextPos.x, position.y};
      if (CanMoveTo(tryPos, map)) position.x = nextPos.x;

      tryPos = {position.x, nextPos.y};
      if (CanMoveTo(tryPos, map)) position.y = nextPos.y;

      frameTimer += deltaTime;
      if (frameTimer >= ANIM_SPEED) {
        frameTimer = 0.0f;

        // recorre los frames
        currentFrame = (currentFrame + 1) % 3;
      }
    } else {
      currentFrame = 0;
      frameTimer = 0.0f;
    }
  }

  /**
   * @brief Dibuja el frame actual del jugador en pantalla.
   */
  void Draw() {
    Rectangle src = {(float)(currentFrame * FRAME_WIDTH),
                     (float)(direction * FRAME_HEIGHT), (float)FRAME_WIDTH,
                     (float)FRAME_HEIGHT};
    Vector2 drawPos = {position.x - FRAME_WIDTH / 2.0f,
                       position.y - FRAME_HEIGHT / 2.0f};
    DrawTextureRec(texture, src, drawPos, WHITE);
  }
};

/**
 * @brief Estructura de bala (codigo residual, no utilizado).
 * @details Codigo sobrante de cuando se estaba aprendiendo raylib.
 *          Se mantiene comentado como referencia.
 */
// struct Bullet {
//   Vector2 position;
//   float speed;
//   bool active;

//   void Update(float deltaTime) {
//     position.y += deltaTime * speed;
//     if (position.y < 0) active = false;
//   }

//   void Draw() {
//     if (active) {
//       DrawCircleV(position, 5, YELLOW);
//     }
//   }
// };

/**
 * @brief Dibuja el mapa completo recorriendo su matriz de tiles.
 * @param map Matriz de tiles del mapa.
 * @param tileset Textura del tileset a usar.
 */
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

/**
 * @brief Dibuja un cuadro de dialogo en la parte inferior de la pantalla.
 * @details Renderiza un recuadro semitransparente con el texto centrado,
 *          soporta saltos de linea con '\\n'.
 * @param message Mensaje a mostrar.
 */
void DrawDialogueBox(const string& message) {
  int boxX = 30;
  int boxY = WINDOW_HEIGHT - 110;
  int boxW = WINDOW_WIDTH - 60;
  int boxH = 90;

  DrawRectangle(boxX, boxY, boxW, boxH, {34, 32, 52, 220});

  DrawRectangleLinesEx({(float)boxX, (float)boxY, (float)boxW, (float)boxH}, 3,
                       {70, 65, 90, 255});

  DrawRectangle(boxX + 4, boxY + 4, boxW - 40, 1, {90, 85, 110, 255});

  int fontSize = 20;
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

/**
 * @brief Dibuja la pantalla de combate matematico.
 * @details Muestra el monstruo animado, los numeros A y B, el cociente y
 *          residuo, la barra de vida/pociones y el cuadro de entrada.
 * @param a Dividendo actual.
 * @param b Divisor actual.
 * @param quotient Cociente calculado (A/B).
 * @param remainder Residuo de la division.
 * @param showResult true para mostrar resultado de la ronda.
 * @param input Texto ingresado por el jugador.
 * @param hp Puntos de vida actuales del jugador.
 * @param pots Pociones restantes.
 * @param feedback Estado del feedback: 0=nada, 1=correcto, 2=incorrecto.
 * @param bigSword Textura de la espada grande.
 * @param bigMonster Textura del monstruo grande.
 * @param monsterFrame Frame actual de animacion del monstruo.
 */
void DrawCombatScreen(int a, int b, int quotient, int remainder,
                      bool showResult, const string& input, int hp, int pots,
                      int feedback, Texture2D bigSword, Texture2D bigMonster,
                      int monsterFrame) {
  // Fondo oscuro
  DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {34, 32, 52, 255});

  // Titulo
  string title = "COMBATE";
  int titleW = MeasureText(title.c_str(), 28);
  DrawText(title.c_str(), (WINDOW_WIDTH - titleW) / 2, 20, 28,
           {200, 50, 50, 255});

  // Cociente (aparece arriba del monstruo)
  if (showResult) {
    string qText = "cociente: " + to_string(quotient);
    int qW = MeasureText(qText.c_str(), 20);
    DrawText(qText.c_str(), (WINDOW_WIDTH - qW) / 2, 78, 20, YELLOW);
  }

  // Monstruo animado
  int monsterSize = 144;
  int monsterX = (WINDOW_WIDTH - monsterSize) / 2;
  int monsterY = 108;
  int srcX = (monsterFrame % 4) * 48;
  int srcY = (monsterFrame / 4) * 48;
  Rectangle monsterSrc = {(float)srcX, (float)srcY, 48, 48};
  Rectangle monsterDst = {(float)monsterX, (float)monsterY, (float)monsterSize,
                          (float)monsterSize};
  DrawTexturePro(bigMonster, monsterSrc, monsterDst, {0, 0}, 0, WHITE);

  // Numero A a la izquierda
  string aText = to_string(a);
  int aW = MeasureText(aText.c_str(), 36);
  DrawText(aText.c_str(), monsterX - aW - 24, monsterY + 46, 36, WHITE);

  // Numero B a la derecha
  string bText = to_string(b);
  DrawText(bText.c_str(), monsterX + monsterSize + 24, monsterY + 46, 36,
           WHITE);

  // Residuo (aparece debajo del monstruo)
  if (showResult) {
    string rText = "residuo: " + to_string(remainder);
    int rW = MeasureText(rText.c_str(), 20);
    DrawText(rText.c_str(), (WINDOW_WIDTH - rW) / 2,
             monsterY + monsterSize + 10, 20, YELLOW);
  }

  // Feedback
  if (feedback == 1) {
    string fText = "Correcto!";
    int fW = MeasureText(fText.c_str(), 24);
    DrawText(fText.c_str(), (WINDOW_WIDTH - fW) / 2,
             monsterY + monsterSize + 40, 24, GREEN);
  } else if (feedback == 2) {
    string fText = "Incorrecto!";
    int fW = MeasureText(fText.c_str(), 24);
    DrawText(fText.c_str(), (WINDOW_WIDTH - fW) / 2,
             monsterY + monsterSize + 40, 24, RED);
  }

  // HP y pociones
  string hpText = "Vida: ";
  for (int i = 0; i < hp; i++) hpText += "X ";
  DrawText(hpText.c_str(), 30, 290, 22, RED);

  string potText = "Pociones: " + to_string(pots) + "  [P]";
  DrawText(potText.c_str(), 30, 320, 22, GREEN);

  // Espada grande
  Rectangle swordSrc = {0, 0, 144, 48};
  Rectangle swordDst = {0, 460, 720, 240};
  DrawTexturePro(bigSword, swordSrc, swordDst, {0, 0}, 0, WHITE);

  // Input box
  int inputBoxW = 250;
  int inputBoxH = 40;
  int inputBoxX = (WINDOW_WIDTH - inputBoxW) / 2;
  int inputBoxY = 560;

  string inputLabel = "Escribe el cociente:";
  int labelW = MeasureText(inputLabel.c_str(), 22);
  DrawText(inputLabel.c_str(), (WINDOW_WIDTH - labelW) / 2, inputBoxY - 24, 22,
           {34, 32, 52, 255});

  DrawRectangle(inputBoxX, inputBoxY, inputBoxW, inputBoxH, {34, 32, 52, 220});
  DrawRectangleLinesEx(
      {(float)inputBoxX, (float)inputBoxY, (float)inputBoxW, (float)inputBoxH},
      2, {200, 200, 200, 255});

  string displayInput = "> " + input;
  // el tiempo en segundos float (4) que se trunca a int divido % 2, siempre
  // dara par o impar asi que se puede usar para alternar
  int cursorBlink = (int)(GetTime() * 4) % 2;
  if (cursorBlink) displayInput += "_";
  DrawText(displayInput.c_str(), inputBoxX + 8, inputBoxY + 8, 20, WHITE);

  // Instruccion
  string instr = "ENTER para confirmar  |  BACKSPACE para borrar";
  int instrW = MeasureText(instr.c_str(), 16);
  DrawText(instr.c_str(), (WINDOW_WIDTH - instrW) / 2, inputBoxY + 50, 16,
           {34, 32, 52, 255});
}

/**
 * @brief Punto de entrada del juego.
 * @details Inicializa ventana, audio, texturas y sonidos. Ejecuta el bucle
 *          principal con maquina de estados (MENU, PLAYING, COMBAT, GAMEOVER,
 *          WIN). Aplica post-procesado CRT. Limpia recursos al finalizar.
 * @return 0 al cerrar correctamente.
 */
int main() {
  srand(time(0));

  InitWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "mazmorra de euclides");
  InitAudioDevice();
  SetTargetFPS(60);

  // cargar sonidos y musica
  SoundManager sndManager;
  sndManager.LoadMusic("mounstruo", "assets/sounds/mounstro.ogg");
  sndManager.LoadMusic("mazmorra", "assets/sounds/mazmorra.ogg");
  sndManager.LoadMusic("menu", "assets/sounds/menu.ogg");
  sndManager.Load("faa", "assets/sounds/faa.ogg");

  Music musicMenu = sndManager.GetMusic("menu");
  Music musicMazmorra = sndManager.GetMusic("mazmorra");
  Music musicMounstruo = sndManager.GetMusic("mounstruo");
  Music* currentMusic = nullptr;

  musicMenu.looping = false;

  Sound faa = sndManager.Get("faa");

  // cargar texturas
  TextureManager texManager;
  texManager.Load("tileset", "assets/pared-mazmorra.png");
  texManager.Load("player", "assets/player-2.png");
  texManager.Load("sword", "assets/espada.png");
  texManager.Load("big-sword", "assets/espada-grande.png");
  texManager.Load("big-monster", "assets/mounstruo-grande.png");

  Texture2D tileset = texManager.Get("tileset");
  SetTextureFilter(tileset, TEXTURE_FILTER_POINT);
  Texture2D bigSword = texManager.Get("big-sword");
  SetTextureFilter(bigSword, TEXTURE_FILTER_POINT);
  GameMap* currentMap = &Afuera;

  Texture2D bigMonster = texManager.Get("big-monster");
  SetTextureFilter(bigMonster, TEXTURE_FILTER_POINT);

  int virtualWidth = currentMap->width * TILE_SIZE;
  int virtualHeight = currentMap->height * TILE_SIZE;

  RenderTexture2D canvas = LoadRenderTexture(virtualWidth, virtualHeight);
  SetTextureFilter(canvas.texture, TEXTURE_FILTER_POINT);

  Rectangle canvasSourceRec = {0.0f, 0.0f, (float)virtualWidth,
                               (float)-virtualHeight};
  Rectangle destRec = {0.0f, 0.0f, (float)WINDOW_WIDTH, (float)WINDOW_HEIGHT};
  Vector2 origin = {0.0f, 0.0f};

  // CRT shader
  Shader crtShader = LoadShader(0, "assets/shaders/crt.fs");
  RenderTexture2D screenRT = LoadRenderTexture(WINDOW_WIDTH, WINDOW_HEIGHT);
  SetTextureFilter(screenRT.texture, TEXTURE_FILTER_BILINEAR);
  Rectangle screenRTSrc = {0.0f, 0.0f, (float)WINDOW_WIDTH,
                           (float)-WINDOW_HEIGHT};
  Rectangle screenRTDst = {0.0f, 0.0f, (float)WINDOW_WIDTH,
                           (float)WINDOW_HEIGHT};

  // --- VARIABLES DEL JUEGO ---
  GameState currentState = GameState::MENU;

  Texture2D playerTex = texManager.Get("player");
  SetTextureFilter(playerTex, TEXTURE_FILTER_POINT);
  Player player = {{72, 72}, 50.0f, playerTex, 0, 0.0f, 0};
  Texture2D sword = texManager.Get("sword");

  bool tieneEspada = false;
  int espadaTileX = 7;
  int espadaTileY = 1;

  int potions = 3;
  int monstersDefeated = 0;

  string currentMessage = "";

  // --- VARIABLES DE COMBATE ---
  int combatA = 0;
  int combatB = 0;
  int playerHP = 3;
  int playerMaxHP = 3;
  string combatInput = "";
  int lastQuotient = 0;
  int lastRemainder = 0;
  bool combatShowResult = false;
  float combatTimer = 0;
  int combatFeedback = 0;  // 0=nada, 1=correcto, 2=incorrecto
  bool combatWon = false;
  int combatMonsterTileX = 0;
  int combatMonsterTileY = 0;
  int monsterFrame = 0;
  float monsterAnimTimer = 0.0f;
  int monsterAnimState = 0;  // 0=idle, 1=ataque, 2=danho

  // --- BUCLE PRINCIPAL ---
  while (!WindowShouldClose()) {
    float deltaTime = GetFrameTime();

    // Musica de fondo
    if (currentMusic) {
      UpdateMusicStream(*currentMusic);
    }

    Music* targetMusic = nullptr;
    if (currentState == GameState::MENU || currentState == GameState::WIN ||
        currentState == GameState::GAMEOVER) {
      targetMusic = &musicMenu;
    } else if (currentState == GameState::PLAYING) {
      targetMusic = &musicMazmorra;
    } else if (currentState == GameState::COMBAT) {
      targetMusic = &musicMounstruo;
    }

    if (targetMusic != currentMusic) {
      if (currentMusic) StopMusicStream(*currentMusic);
      currentMusic = targetMusic;
      if (currentMusic) PlayMusicStream(*currentMusic);
    }

    // ============ ACTUALIZAR ============
    switch (currentState) {
      case GameState::MENU:
        if (IsKeyPressed(KEY_ENTER)) {
          currentMap = &Afuera;
          player.position = {72, 72};
          tieneEspada = false;
          playerHP = playerMaxHP;
          potions = 3;
          monstersDefeated = 0;
          currentMessage = "";
          currentState = GameState::PLAYING;
        }
        break;

      case GameState::PLAYING: {
        player.Update(deltaTime, currentMap->baseLayer);

        int playerTileX = (int)(player.position.x / TILE_SIZE);
        int playerTileY = (int)(player.position.y / TILE_SIZE);

        if (currentMap->IsValidTile(playerTileX, playerTileY)) {
          int tileID = currentMap->baseLayer[playerTileY][playerTileX];
          string newMessage = "";

          // Cartel
          if (tileID == 2) {
            if (currentMap == &Afuera) {
              newMessage =
                  "Bienvenido a la mazmorra de euclides \n"
                  "el tamaño no importa, sino como lo usas";
            } else if (currentMap == &Mazmorra) {
              newMessage =
                  "estos monstruos protegen la puerta de salida \n "
                  "para poder salir debes derrotarlos euclidianamente";
            }
          }

          // Espada
          if (!tieneEspada && playerTileX == espadaTileX &&
              playerTileY == espadaTileY) {
            newMessage =
                "Esta es la legendaria espada de la division\n"
                "con ella podras dividir a cualquier enemigo\n"
                "Presiona A para recoger la espada";
            if (IsKeyPressed(KEY_A)) {
              tieneEspada = true;
            }
          }

          currentMessage = newMessage;

          // Transicion entre mapas
          if (tileID == 8) {
            if (currentMap == &Afuera) {
              if (!tieneEspada) {
                currentMessage = "si entras sin espada te haran una piedra!!!";
              } else {
                currentMap = &Mazmorra;
                player.position.x = TILE_SIZE / 2.0f;
                player.position.y = 4 * TILE_SIZE + TILE_SIZE / 2.0f;
              }
            } else if (currentMap == &Mazmorra) {
              if (monstersDefeated != 3) {
                currentMessage =
                    "debes derrotar a todos los monstruos \n"
                    "no seas como hernanadez con flojera ";

              } else {
                currentState = GameState::WIN;
                player.position.x = TILE_SIZE / 2.0f;
                player.position.y = TILE_SIZE / 2.0f;
              }
            }
          }

          // Iniciar combate
          if ((tileID == 6 || tileID == 7) && tieneEspada) {
            combatA = 10 + rand() % 20;
            combatB = 2 + rand() % 9;
            combatInput = "";
            lastQuotient = 0;
            lastRemainder = 0;
            combatShowResult = false;
            combatTimer = 0;
            combatFeedback = 0;
            combatWon = false;
            combatMonsterTileX = playerTileX;
            combatMonsterTileY = playerTileY;
            monsterFrame = 0;
            monsterAnimTimer = 0;
            monsterAnimState = 0;
            currentState = GameState::COMBAT;
          }
        }
        break;
      }

      case GameState::COMBAT: {
        // Si estamos mostrando resultado, esperar el timer
        if (combatTimer > 0) {
          combatTimer -= deltaTime;
          if (combatTimer <= 0) {
            if (combatWon) {
              // Volver al mapa

              // quitar al enemigo de ahi, reemplaza con el tile 15
              currentMap->baseLayer[combatMonsterTileY][combatMonsterTileX] =
                  15;
              monstersDefeated++;
              currentState = GameState::PLAYING;
            } else if (combatFeedback == 1) {
              // Siguiente ronda (ya se actualizaron A y B)
              combatShowResult = false;
              combatFeedback = 0;
            } else {
              // Feedback incorrecto: misma ronda
              combatShowResult = false;
              combatFeedback = 0;
            }
          }
          break;
        }

        // Actualizar animacion del monstruo
        if (monsterAnimState != 0) {
          monsterAnimTimer += deltaTime;
          if (monsterAnimTimer >= 2.5f) {
            monsterFrame = 0;
            monsterAnimState = 0;
          }
        }

        // Input numerico
        for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
          if (IsKeyPressed(k)) {
            // esta vaina usa hexagesimal
            if (combatInput.length() < 4) {
              combatInput += (char)('0' + (k - KEY_ZERO));
            }
          }
        }

        if (IsKeyPressed(KEY_BACKSPACE) && !combatInput.empty()) {
          combatInput.pop_back();
        }

        // Usar pocion
        if (IsKeyPressed(KEY_P) && potions > 0 && playerHP < playerMaxHP) {
          potions--;
          playerHP++;
        }

        // Confirmar respuesta
        if (IsKeyPressed(KEY_ENTER) && !combatInput.empty()) {
          int playerAnswer = stoi(combatInput);
          int correctQuotient = combatA / combatB;
          int remainder = combatA % combatB;

          lastQuotient = correctQuotient;
          lastRemainder = remainder;
          combatShowResult = true;
          combatInput = "";

          if (playerAnswer == correctQuotient) {
            combatFeedback = 1;
            monsterAnimState = 2;
            monsterAnimTimer = 0;
            monsterFrame = 4 + rand() % 3;
            if (remainder == 0) {
              combatWon = true;
              combatTimer = 2.5f;
            } else {
              combatA = combatB;
              combatB = remainder;
              combatTimer = 2.5f;
            }
          } else {
            PlaySound(faa);
            playerHP--;
            combatFeedback = 2;
            monsterAnimState = 1;
            monsterAnimTimer = 0;
            monsterFrame = 1 + rand() % 3;
            if (playerHP <= 0) {
              currentState = GameState::GAMEOVER;
            } else {
              combatTimer = 2.5f;
            }
          }
        }
        break;
      }

      case GameState::GAMEOVER:
        if (IsKeyPressed(KEY_ENTER)) {
          currentState = GameState::MENU;
        }
        break;

      case GameState::WIN:
        if (IsKeyPressed(KEY_ENTER)) {
          currentState = GameState::MENU;
        }
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

        if (!tieneEspada) {
          Vector2 pos = {(float)(espadaTileX * TILE_SIZE),
                         (float)(espadaTileY * TILE_SIZE)};
          DrawTextureV(sword, pos, WHITE);
        }

        // esto es para que sostenga la espada
        if (tieneEspada) {
          DrawTextureV(sword,
                       {player.position.x - sword.width,
                        player.position.y - sword.height + 6},
                       WHITE);
        }

        player.Draw();
        break;

      case GameState::COMBAT:
        break;

      case GameState::GAMEOVER:
        break;

      case GameState::WIN:
        break;

      default:
        break;
    }

    EndTextureMode();

    // Componer canvas + UI en screenRT
    BeginTextureMode(screenRT);
    ClearBackground({34, 32, 52, 255});

    DrawTexturePro(canvas.texture, canvasSourceRec, screenRTDst, origin, 0.0f,
                   WHITE);

    if (currentState == GameState::MENU) {
      // Fondo
      DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {20, 18, 35, 255});

      // Titulo
      string title = "MAZMORRA DE EUCLIDES";
      int titleW = MeasureText(title.c_str(), 40);
      DrawText(title.c_str(), (WINDOW_WIDTH - titleW) / 2, 100, 40,
               {220, 180, 60, 255});

      // Subtitulo
      string sub = "~ un juego matematico ~";
      int subW = MeasureText(sub.c_str(), 18);
      DrawText(sub.c_str(), (WINDOW_WIDTH - subW) / 2, 150, 18,
               {180, 180, 180, 255});

      // Monstruo decorativo
      int monSize = 144;
      Rectangle monSrc = {0, 0, 48, 48};
      Rectangle monDst = {(float)((WINDOW_WIDTH - monSize) / 2), 200,
                          (float)monSize, (float)monSize};
      DrawTexturePro(bigMonster, monSrc, monDst, {0, 0}, 0, WHITE);

      // Espada decorativa
      Rectangle swordSrc = {0, 0, 144, 48};
      Rectangle swordDst = {(float)((WINDOW_WIDTH - 288) / 2), 380, 288, 96};
      DrawTexturePro(bigSword, swordSrc, swordDst, {0, 0}, 0, WHITE);

      // Texto de inicio con parpadeo
      int blink = (int)(GetTime() * 2) % 2;
      if (blink) {
        string start = "Presiona ENTER para empezar";
        int sw = MeasureText(start.c_str(), 20);
        DrawText(start.c_str(), (WINDOW_WIDTH - sw) / 2, 520, 20, LIGHTGRAY);
      }

      // Creditos
      string cred = "~ juego creado por: Jesús Valencia ~";
      int cw = MeasureText(cred.c_str(), 14);
      DrawText(cred.c_str(), (WINDOW_WIDTH - cw) / 2, 680, 14,
               {120, 120, 120, 255});
    }

    if (currentState == GameState::PLAYING && !currentMessage.empty()) {
      DrawDialogueBox(currentMessage);
    }

    if (currentState == GameState::COMBAT) {
      DrawCombatScreen(combatA, combatB, lastQuotient, lastRemainder,
                       combatShowResult, combatInput, playerHP, potions,
                       combatFeedback, bigSword, bigMonster, monsterFrame);
    }

    if (currentState == GameState::GAMEOVER) {
      string text = "GAME OVER";
      int tw = MeasureText(text.c_str(), 48);
      DrawText(text.c_str(), (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 30,
               48, RED);

      string text2 = "Presiona ENTER para volver al menu";
      int tw2 = MeasureText(text2.c_str(), 20);
      DrawText(text2.c_str(), (WINDOW_WIDTH - tw2) / 2, WINDOW_HEIGHT / 2 + 30,
               20, LIGHTGRAY);
    }

    if (currentState == GameState::WIN) {
      string text = "GANASTE!";
      int tw = MeasureText(text.c_str(), 48);
      DrawText(text.c_str(), (WINDOW_WIDTH - tw) / 2, WINDOW_HEIGHT / 2 - 30,
               48, GREEN);

      string text2 = "Presiona ENTER para volver al menu";
      int tw2 = MeasureText(text2.c_str(), 20);
      DrawText(text2.c_str(), (WINDOW_WIDTH - tw2) / 2, WINDOW_HEIGHT / 2 + 30,
               20, LIGHTGRAY);

      string cred1 = "juego creado por: xxx";
      int cw1 = MeasureText(cred1.c_str(), 16);
      DrawText(cred1.c_str(), (WINDOW_WIDTH - cw1) / 2, WINDOW_HEIGHT / 2 + 70,
               16, {180, 180, 180, 255});

      string cred2 = "agradecimientos especiales a Taylor";
      int cw2 = MeasureText(cred2.c_str(), 16);
      DrawText(cred2.c_str(), (WINDOW_WIDTH - cw2) / 2, WINDOW_HEIGHT / 2 + 95,
               16, {180, 180, 180, 255});
    }

    EndTextureMode();

    // Dibujar screenRT en la pantalla real con CRT
    BeginDrawing();
    ClearBackground({34, 32, 52, 255});
    BeginShaderMode(crtShader);
    DrawTexturePro(screenRT.texture, screenRTSrc, screenRTDst, origin, 0.0f,
                   WHITE);
    EndShaderMode();
    EndDrawing();
  }

  // --- LIMPIEZA ---
  UnloadShader(crtShader);
  UnloadRenderTexture(screenRT);
  texManager.UnloadAll();
  CloseWindow();
  return 0;
}
