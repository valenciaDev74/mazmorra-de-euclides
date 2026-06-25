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
 * @brief Sistema de combate matematico basado en el algoritmo de Euclides.
 * @details Gestiona el ciclo completo de combate: generacion de divisiones,
 *          entrada numerica del jugador, validacion de respuestas, avance
 *          del algoritmo de Euclides, animacion del monstruo y renderizado.
 */
struct CombatSystem {
  bool active = false;
  bool won = false;

  // las coordenadas son para quitarlo despues
  int monsterTileX = 0;
  int monsterTileY = 0;

  int a = 0;
  int b = 0;
  int hp = 3;
  int maxHP = 3;
  int potions = 3;
  string input;
  int lastQuotient = 0;
  int lastRemainder = 0;
  bool showResult = false;
  float timer = 0;
  int feedback = 0;
  int monsterFrame = 0;
  float monsterAnimTimer = 0;
  int monsterAnimState = 0;

  /**
   * @brief Inicia un nuevo combate en las coordenadas dadas.
   * @details Genera una division aleatoria A/B, reinicia todas las
   *          variables de estado y marca el combate como activo.
   * @param tileX Coordenada X del monstruo en el mapa.
   * @param tileY Coordenada Y del monstruo en el mapa.
   */
  void Start(int tileX, int tileY) {
    a = 10 + rand() % 20;
    b = 2 + rand() % 9;
    input = "";
    lastQuotient = 0;
    lastRemainder = 0;
    showResult = false;
    timer = 0;
    // feedback es el 1: correcto o 2: incorrecto
    feedback = 0;
    won = false;
    monsterTileX = tileX;
    monsterTileY = tileY;
    monsterFrame = 0;
    monsterAnimTimer = 0;
    // 0: default, 1; miss, 2: hit
    monsterAnimState = 0;
    active = true;
  }

  /**
   * @brief Actualiza la logica del combate cada frame.
   * @details Procesa el temporizador entre rondas, la animacion del
   *          monstruo, la entrada numerica del jugador (0-9, BACKSPACE),
   *          el uso de pociones (P) y la validacion de la respuesta
   *          al presionar ENTER. Si la respuesta es correcta avanza el
   *          algoritmo de Euclides; si es incorrecta descuenta HP.
   *          Cuando active=false, el main loop debe revisar won para
   *          determinar si fue victoria o derrota.
   * @param dt Tiempo transcurrido desde el ultimo frame.
   * @param faa Efecto de sonido a reproducir en respuesta incorrecta.
   */
  void Update(float dt, Sound faa) {
    if (!active) return;

    // si hay timer de pausa
    if (timer > 0) {
      // decrementa
      timer -= dt;
      // si llega a 0
      if (timer <= 0) {
        // y gana
        if (won) {
          // active es false y retorna
          active = false;

          // si no solo quita los textos
        } else if (feedback == 1 || feedback == 2) {
          showResult = false;
          feedback = 0;
        }
      }
      return;
    }

    // si la animacion actual no es la default
    if (monsterAnimState != 0) {
      // aumenta el timer
      monsterAnimTimer += dt;
      // si llega a 2.5s
      if (monsterAnimTimer >= 2.5f) {
        // reinicia al default
        monsterFrame = 0;
        monsterAnimState = 0;
      }
    }

    for (int k = KEY_ZERO; k <= KEY_NINE; k++) {
      if (IsKeyPressed(k)) {
        if (input.length() < 4) {
          input += (char)('0' + (k - KEY_ZERO));
        }
      }
    }

    if (IsKeyPressed(KEY_BACKSPACE) && !input.empty()) {
      input.pop_back();
    }

    if (IsKeyPressed(KEY_P) && potions > 0 && hp < maxHP) {
      potions--;
      hp++;
    }

    // enviar respuesta
    if (IsKeyPressed(KEY_ENTER) && !input.empty()) {
      int playerAnswer = stoi(input);
      int correctQuotient = a / b;
      int remainder = a % b;

      lastQuotient = correctQuotient;
      lastRemainder = remainder;
      showResult = true;
      input = "";

      // si esta buena
      if (playerAnswer == correctQuotient) {
        // mostramos feedback
        feedback = 1;
        // actualizamos mounstruo
        monsterAnimState = 2;
        // cualquiera de los 3 frames
        monsterFrame = 4 + rand() % 3;
        // si el resto es cero
        if (remainder == 0) {
          // ya ganamos papi, cada quien pa su casa
          won = true;
        } else {
          // si no pues actualizamos los coeficientes
          a = b;
          b = remainder;
        }
        // ponemos el tiempo
        timer = 2.5f;
      } else {
        // JAJAJAJJA
        PlaySound(faa);
        hp--;
        feedback = 2;
        monsterAnimState = 1;
        monsterFrame = 1 + rand() % 3;
        timer = 2.5f;
      }
    }
  }

  /**
   * @brief Dibuja la pantalla de combate.
   * @details Renderiza el fondo, titulo, monstruo animado con los
   *          numeros A y B, cociente/residuo (si showResult), feedback
   *          de acierto/error, barra de HP y pociones, espada decorativa,
   *          y el cuadro de entrada numerica con cursor parpadeante.
   * @param bigSword Textura de la espada grande (parte inferior).
   * @param bigMonster Textura del monstruo grande (spritesheet 48x48).
   */
  void Draw(Texture2D bigSword, Texture2D bigMonster) {
    DrawRectangle(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT, {34, 32, 52, 255});

    string title = "COMBATE";
    int titleW = MeasureText(title.c_str(), 28);
    DrawText(title.c_str(), (WINDOW_WIDTH - titleW) / 2, 20, 28,
             {200, 50, 50, 255});

    if (showResult) {
      string qText = "cociente: " + to_string(lastQuotient);
      int qW = MeasureText(qText.c_str(), 20);
      DrawText(qText.c_str(), (WINDOW_WIDTH - qW) / 2, 78, 20, YELLOW);
    }

    int monsterSize = 144;
    int monsterX = (WINDOW_WIDTH - monsterSize) / 2;
    int monsterY = 108;
    int srcX = (monsterFrame % 4) * 48;
    int srcY = (monsterFrame / 4) * 48;
    Rectangle monsterSrc = {(float)srcX, (float)srcY, 48, 48};
    Rectangle monsterDst = {(float)monsterX, (float)monsterY,
                            (float)monsterSize, (float)monsterSize};
    DrawTexturePro(bigMonster, monsterSrc, monsterDst, {0, 0}, 0, WHITE);

    string aText = to_string(a);
    int aW = MeasureText(aText.c_str(), 36);
    DrawText(aText.c_str(), monsterX - aW - 24, monsterY + 46, 36, WHITE);

    string bText = to_string(b);
    DrawText(bText.c_str(), monsterX + monsterSize + 24, monsterY + 46, 36,
             WHITE);

    if (showResult) {
      string rText = "residuo: " + to_string(lastRemainder);
      int rW = MeasureText(rText.c_str(), 20);
      DrawText(rText.c_str(), (WINDOW_WIDTH - rW) / 2,
               monsterY + monsterSize + 10, 20, YELLOW);
    }

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

    string hpText = "Vida: ";
    for (int i = 0; i < hp; i++) hpText += "X ";
    DrawText(hpText.c_str(), 30, 290, 22, RED);

    string potText = "Pociones: " + to_string(potions) + "  [P]";
    DrawText(potText.c_str(), 30, 320, 22, GREEN);

    Rectangle swordSrc = {0, 0, 144, 48};
    Rectangle swordDst = {0, 460, 720, 240};
    DrawTexturePro(bigSword, swordSrc, swordDst, {0, 0}, 0, WHITE);

    int inputBoxW = 250;
    int inputBoxH = 40;
    int inputBoxX = (WINDOW_WIDTH - inputBoxW) / 2;
    int inputBoxY = 560;

    string inputLabel = "Escribe el cociente:";
    int labelW = MeasureText(inputLabel.c_str(), 22);
    DrawText(inputLabel.c_str(), (WINDOW_WIDTH - labelW) / 2, inputBoxY - 24,
             22, {34, 32, 52, 255});

    DrawRectangle(inputBoxX, inputBoxY, inputBoxW, inputBoxH,
                  {34, 32, 52, 220});
    DrawRectangleLinesEx({(float)inputBoxX, (float)inputBoxY, (float)inputBoxW,
                          (float)inputBoxH},
                         2, {200, 200, 200, 255});

    string displayInput = "> " + input;
    int cursorBlink = (int)(GetTime() * 4) % 2;
    if (cursorBlink) displayInput += "_";
    DrawText(displayInput.c_str(), inputBoxX + 8, inputBoxY + 8, 20, WHITE);

    string instr = "ENTER para confirmar  |  BACKSPACE para borrar";
    int instrW = MeasureText(instr.c_str(), 16);
    DrawText(instr.c_str(), (WINDOW_WIDTH - instrW) / 2, inputBoxY + 50, 16,
             {34, 32, 52, 255});
  }
};

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
  texManager.Load("player", "assets/player.png");
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
  int espadaTileY = 8;

  int monstersDefeated = 0;

  string currentMessage = "";

  CombatSystem combat;

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

    // evita repetir la cancion si ya esta
    // con cada repeticion del ciclo se valida esta madre
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
          combat.hp = combat.maxHP;
          combat.potions = 3;
          monstersDefeated = 0;
          currentMessage = "";
          currentState = GameState::PLAYING;
        }
        break;

      case GameState::PLAYING: {
        player.Update(deltaTime, currentMap->baseLayer);

        // obtiene posicion del jugador en tiles
        int playerTileX = (int)(player.position.x / TILE_SIZE);
        int playerTileY = (int)(player.position.y / TILE_SIZE);

        if (currentMap->IsValidTile(playerTileX, playerTileY)) {
          // busca la posicion en el mapa
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
            combat.Start(playerTileX, playerTileY);
            currentState = GameState::COMBAT;
          }
        }
        break;
      }

      case GameState::COMBAT: {
        combat.Update(deltaTime, faa);
        if (!combat.active) {
          if (combat.won) {
            currentMap->baseLayer[combat.monsterTileY][combat.monsterTileX] =
                15;
            monstersDefeated++;
            currentState = GameState::PLAYING;
          } else {
            currentState = GameState::GAMEOVER;
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
      string sub = "~ un juego sobre el algoritmo de euclides ~";
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
      combat.Draw(bigSword, bigMonster);
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

      string cred1 = "juego creado por: Jesús Valencia";
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
