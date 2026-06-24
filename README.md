# Mazmorra de Euclides

![Imagen del juego](https://media.discordapp.net/attachments/628943954397888522/1519406231507308567/image.png?ex=6a3d70d2&is=6a3c1f52&hm=973d4f2038813dfedcf32fad71078be0effc5e2a2c5dcc5f79fcedc56c4d2bdf&=&format=webp&quality=lossless&width=631&height=630)

## Descripcion

Juego 2D tipo mazmorra donde el combate se resuelve con el **Algoritmo de Euclides** (tema visto en clase). El jugador explora un mapa, recoge una espada y debe derrotar monstruos resolviendo divisiones enteras para avanzar.

## Conceptos de Programacion II aplicados

- **Algoritmo de Euclides**: calculo del MCD mediante divisiones sucesivas durante el combate
- **Estructuras de datos**: vectores 2D para mapas, `unordered_map` para cache de texturas/sonidos, structs para entidades
- **Maquina de estados**: MENU -> PLAYING -> COMBAT -> GAMEOVER/WIN
- **Colisiones**: deteccion basada en tiles con bounding box
- **Animacion por spritesheets**: manejada con temporizadores y frames
- **Shader GLSL**: post-procesado CRT (distorsion barrel, aberracion cromatica, scanlines, bloom)

## Cosas hechas por mi mismo
- Todas las estructuras de datos
- Sistemas de carga de texturas, mapas, musica y sonido (del juego titulado: contra)
- Todos los sprites
- Organizacion del codigo, separada por responsibilidades (UPDATE, DRAW) pre y post graficos escalados
- Todas las pantallas
- Programacion de interacciones y sistemas
- Documentacion de funciones y estructuras de datos con Doxygen (incluida instalacion y setup)

## Cosas hechas por la IA
- Shaders
- parte de la funcion CanMoveTo() para colisiones

## Como jugar

| Tecla     | Accion                  |
|-----------|-------------------------|
| Flechas   | Moverse por el mapa     |
| A         | Recoger espada          |
| ENTER     | Confirmar en combate    |
| BACKSPACE | Borrar digito en combate|
| P         | Usar pocion curativa    |

## Combate

Al pisar un monstruo con la espada equipada, se genera una division `A / B`. El jugador debe ingresar el **cociente** correcto. Si acierta, los numeros se actualizan con el algoritmo de Euclides (`A = B, B = residuo`). Cuando el residuo es 0, el monstruo es derrotado.

## Compilar y ejecutar

```bash
make
./game
```

Requiere raylib y sus dependencias (X11, GL, pthread, etc.). raylib esta incluido en `lib/` e `include/`.

## Estructura del proyecto

```
.
├── assets/
│   ├── espada-grande.png
│   ├── espada.png
│   ├── mounstruo-grande.png
│   ├── pared-mazmorra.png
│   ├── player-2.png
│   ├── player.png
│   ├── shaders/
│   ├── sounds/
│   ├── Sprite-0001.jpg
│   └── Sprite-0001.png
├── docs/
│   └── html/
├── include/
│   └── raylib.h
├── lib/
│   └── libraylib.a
├── .vscode/
│   └── c_cpp_properties.json
├── .editorconfig
├── Doxyfile
├── LICENSE
├── main.cpp
├── Makefile
└── README.md
```
