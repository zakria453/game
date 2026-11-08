
#if __has_include("raylib.h")
#include "raylib.h"
#else
#include <algorithm>
#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cmath>
#include <string>

struct Vector2 {
    float x;
    float y;
};

struct Color {
    unsigned char r;
    unsigned char g;
    unsigned char b;
    unsigned char a;

    Color(unsigned char red = 0,
          unsigned char green = 0,
          unsigned char blue = 0,
          unsigned char alpha = 255)
        : r(red), g(green), b(blue), a(alpha) {}
};

static const int KEY_A = 65;
static const int KEY_D = 68;
static const int KEY_W = 87;
static const int KEY_S = 83;
static const int KEY_LEFT = 263;
static const int KEY_RIGHT = 262;
static const int KEY_UP = 265;
static const int KEY_DOWN = 264;
static const int KEY_SPACE = 32;
static const int KEY_ESCAPE = 256;
static const int KEY_ENTER = 257;

static const Color WHITE = Color(255, 255, 255, 255);
static const Color RAYWHITE = Color(245, 245, 245, 255);
static const Color LIGHTGRAY = Color(200, 200, 200, 255);
static const Color DARKGRAY = Color(80, 80, 80, 255);
static const Color SKYBLUE = Color(70, 220, 255, 255);
static const Color YELLOW = Color(255, 255, 0, 255);
static const Color RED = Color(255, 0, 0, 255);
static const Color GREEN = Color(0, 255, 0, 255);
static const Color ORANGE = Color(255, 140, 0, 255);
static const Color PURPLE = Color(180, 80, 255, 255);

inline int GetRandomValue(int minV, int maxV) {
    return minV + (std::rand() % (maxV - minV + 1));
}

inline float Clamp(float value, float minV, float maxV) {
    return std::max(minV, std::min(value, maxV));
}

inline bool IsKeyPressed(int key) { (void)key; return false; }
inline bool IsKeyDown(int key) { (void)key; return false; }
inline void InitWindow(int width, int height, const char *title) { (void)width; (void)height; (void)title; }
inline void SetTargetFPS(int fps) { (void)fps; }
inline bool WindowShouldClose() { return false; }
inline float GetFrameTime() { return 1.0f / 60.0f; }
inline void BeginDrawing() {}
inline void EndDrawing() {}
inline void ClearBackground(Color) {}
inline void DrawCircleV(Vector2 center, float radius, Color color) { (void)center; (void)radius; (void)color; }
inline void DrawLine(int x1, int y1, int x2, int y2, Color color) { (void)x1; (void)y1; (void)x2; (void)y2; (void)color; }
inline void DrawTriangle(Vector2 v1, Vector2 v2, Vector2 v3, Color color) { (void)v1; (void)v2; (void)v3; (void)color; }
inline void DrawRectangle(int x, int y, int width, int height, Color color) { (void)x; (void)y; (void)width; (void)height; (void)color; }
inline void DrawRectangleLines(int x, int y, int width, int height, Color color) { (void)x; (void)y; (void)width; (void)height; (void)color; }
inline void DrawCircle(int x, int y, float radius, Color color) { (void)x; (void)y; (void)radius; (void)color; }
inline void DrawText(const char *text, int x, int y, int fontSize, Color color) { (void)text; (void)x; (void)y; (void)fontSize; (void)color; }
inline void DrawText(const std::string &text, int x, int y, int fontSize, Color color) { (void)text; (void)x; (void)y; (void)fontSize; (void)color; }
inline char *TextFormat(const char *fmt, ...) {
    static char buffer[1024];
    va_list args;
    va_start(args, fmt);
    vsnprintf(buffer, sizeof(buffer), fmt, args);
    va_end(args);
    return buffer;
}
inline void CloseWindow() {}
inline bool CheckCollisionCircles(Vector2 centerA, float radiusA, Vector2 centerB, float radiusB) {
    float dx = centerB.x - centerA.x;
    float dy = centerB.y - centerA.y;
    return (dx * dx + dy * dy) <= (radiusA + radiusB) * (radiusA + radiusB);
}
#endif

#include <vector>
#include <cmath>
#include <algorithm>

using namespace std;

const int SCREEN_W = 1000;
const int SCREEN_H = 700;

struct Bullet {
    Vector2 pos;
    float speed;
    bool enemy;
};

struct Enemy {
    Vector2 pos;
    float speed;
    float phase;
    int hp;
};

struct Star {
    Vector2 pos;
    float speed;
    float size;
};

struct PowerUp {
    Vector2 pos;
    float speed;
};

enum GameState {
    MENU,
    PLAYING,
    GAME_OVER
};

int main() {
    InitWindow(SCREEN_W, SCREEN_H, "SPACE SHOOTER - C++");
    SetTargetFPS(60);

    // Player
    Vector2 player = {
        SCREEN_W / 2.0f,
        SCREEN_H - 90.0f
    };

    const float playerRadius = 17.0f;
    const float playerSpeed = 330.0f;

    vector<Bullet> bullets;
    vector<Enemy> enemies;
    vector<Star> stars;
    vector<PowerUp> powerUps;

    // Background stars
    for (int i = 0; i < 110; i++) {
        stars.push_back({
            {(float)GetRandomValue(0, SCREEN_W),
             (float)GetRandomValue(0, SCREEN_H)},
            (float)GetRandomValue(25, 150),
            (float)GetRandomValue(1, 3)
        });
    }

    GameState state = MENU;

    int score = 0;
    int health = 100;
    int wave = 1;
    int kills = 0;

    float shootTimer = 0;
    float enemyTimer = 0;
    float powerTimer = 0;
    float damageFlash = 0;
    float elapsed = 0;

    auto resetGame = [&]() {
        player = {SCREEN_W / 2.0f, SCREEN_H - 90.0f};

        bullets.clear();
        enemies.clear();
        powerUps.clear();

        score = 0;
        health = 100;
        wave = 1;
        kills = 0;

        shootTimer = 0;
        enemyTimer = 0;
        powerTimer = 0;
        damageFlash = 0;
        elapsed = 0;

        state = PLAYING;
    };

    while (!WindowShouldClose()) {
        float dt = min(GetFrameTime(), 0.033f);

        // Animate stars even in the menu
        for (auto &s : stars) {
            s.pos.y += s.speed * dt;

            if (s.pos.y > SCREEN_H) {
                s.pos.y = 0;
                s.pos.x = (float)GetRandomValue(0, SCREEN_W);
            }
        }

        if (state == MENU) {
            if (IsKeyPressed(KEY_ENTER)) {
                resetGame();
            }
        }
        else if (state == PLAYING) {
            elapsed += dt;
            shootTimer -= dt;
            enemyTimer -= dt;
            powerTimer -= dt;
            damageFlash -= dt;

            wave = 1 + score / 500;

            // Player movement
            Vector2 move = {0, 0};

            if (IsKeyDown(KEY_A) || IsKeyDown(KEY_LEFT))
                move.x -= 1;

            if (IsKeyDown(KEY_D) || IsKeyDown(KEY_RIGHT))
                move.x += 1;

            if (IsKeyDown(KEY_W) || IsKeyDown(KEY_UP))
                move.y -= 1;

            if (IsKeyDown(KEY_S) || IsKeyDown(KEY_DOWN))
                move.y += 1;

            float length = sqrtf(
                move.x * move.x + move.y * move.y
            );

            if (length > 0) {
                move.x /= length;
                move.y /= length;

                player.x += move.x * playerSpeed * dt;
                player.y += move.y * playerSpeed * dt;
            }

            player.x = Clamp(player.x, 25, SCREEN_W - 25);
            player.y = Clamp(player.y, 40, SCREEN_H - 25);

            // Automatic shooting
            if (IsKeyDown(KEY_SPACE) && shootTimer <= 0) {
                bullets.push_back({
                    {player.x, player.y - 22},
                    650.0f,
                    false
                });

                // Double shot in later waves
                if (wave >= 3) {
                    bullets.push_back({
                        {player.x - 12, player.y - 12},
                        650.0f,
                        false
                    });

                    bullets.push_back({
                        {player.x + 12, player.y - 12},
                        650.0f,
                        false
                    });
                }

                shootTimer = 0.17f;
            }

            // Spawn enemies
            if (enemyTimer <= 0) {
                float speed = (float)GetRandomValue(85, 135)
                              + wave * 12.0f;

                enemies.push_back({
                    {(float)GetRandomValue(35, SCREEN_W - 35),
                     -30.0f},
                    speed,
                    (float)GetRandomValue(0, 628) / 100.0f,
                    wave >= 4 ? 2 : 1
                });

                enemyTimer = max(0.25f, 0.9f - wave * 0.055f);
            }

            // Spawn health power-ups
            if (powerTimer <= 0) {
                if (GetRandomValue(0, 100) < 30) {
                    powerUps.push_back({
                        {(float)GetRandomValue(30, SCREEN_W - 30),
                         -20.0f},
                        100.0f
                    });
                }

                powerTimer = 8.0f;
            }

            // Update bullets
            for (auto &b : bullets) {
                b.pos.y += (b.enemy ? 260.0f : -b.speed) * dt;
            }

            // Enemies move and fire
            for (auto &e : enemies) {
                e.pos.y += e.speed * dt;
                e.pos.x += sinf(elapsed * 2.0f + e.phase)
                           * 75.0f * dt;

                e.pos.x = Clamp(e.pos.x, 20, SCREEN_W - 20);

                // Enemy projectiles
                if (GetRandomValue(0, 1000) < 3 + wave) {
                    bullets.push_back({
                        {e.pos.x, e.pos.y + 15},
                        260.0f,
                        true
                    });
                }
            }

            // Update power-ups
            for (auto &p : powerUps) {
                p.pos.y += p.speed * dt;
            }

            // Player bullets hit enemies
            for (auto &b : bullets) {
                if (b.enemy) continue;
 
                for (auto &e : enemies) {
                    if (e.hp <= 0) continue;

                    if (CheckCollisionCircles(
                        b.pos, 5, e.pos, 18)) {
                        e.hp--;
                        b.pos.y = -100;

                        if (e.hp <= 0) {
                            score += 100;
                            kills++;
                        }
                        break;
                    }
                }
            }

            // Enemy collision with player
            for (auto &e : enemies) {
                if (e.hp <= 0) continue;

                if (CheckCollisionCircles(
                    player, playerRadius, e.pos, 18)) {
                    health -= 20;
                    damageFlash = 0.2f;
                    e.hp = 0;
                }

                if (e.pos.y > SCREEN_H + 20) {
                    e.hp = 0;
                    health -= 10;
                    damageFlash = 0.2f;
                }
            }

            // Enemy bullets hit player
            for (auto &b : bullets) {
                if (!b.enemy) continue;

                if (CheckCollisionCircles(
                    player, playerRadius, b.pos, 6)) {
                    health -= 10;
                    damageFlash = 0.2f;
                    b.pos.y = SCREEN_H + 100;
                }
            }

            // Collect health power-ups
            for (auto &p : powerUps) {
                if (CheckCollisionCircles(
                    player, playerRadius, p.pos, 13)) {
                    health = min(100, health + 25);
                    score += 25;
                    p.pos.y = SCREEN_H + 100;
                }
            }

            // Remove objects outside the screen
            bullets.erase(
                remove_if(bullets.begin(), bullets.end(),
                    [](const Bullet &b) {
                        return b.pos.y < -20 ||
                               b.pos.y > SCREEN_H + 20;
                    }),
                bullets.end()
            );

            enemies.erase(
                remove_if(enemies.begin(), enemies.end(),
                    [](const Enemy &e) {
                        return e.hp <= 0 ||
                               e.pos.y > SCREEN_H + 40;
                    }),
                enemies.end()
            );

            powerUps.erase(
                remove_if(powerUps.begin(), powerUps.end(),
                    [](const PowerUp &p) {
                        return p.pos.y > SCREEN_H + 30;
                    }),
                powerUps.end()
            );

            if (health <= 0) {
                health = 0;
                state = GAME_OVER;
            }
        }
        else if (state == GAME_OVER) {
            if (IsKeyPressed(KEY_ENTER)) {
                resetGame();
            }

            if (IsKeyPressed(KEY_ESCAPE)) {
                state = MENU;
            }
        }

        // ================= DRAWING =================

        BeginDrawing();
        ClearBackground(Color{5, 7, 22, 255});

        // Starfield
        for (const auto &s : stars) {
            DrawCircleV(s.pos, s.size,
                        Color{130, 175, 255, 180});
        }

        // Subtle space grid
        for (int x = 0; x < SCREEN_W; x += 100) {
            DrawLine(x, 0, x, SCREEN_H,
                     Color{20, 35, 65, 65});
        }

        if (state == MENU) {
            DrawText("SPACE SHOOTER",
                     200, 170, 62,
                     Color{80, 210, 255, 255});

            DrawText("ADVANCED C++ ARCADE",
                     350, 245, 22, RAYWHITE);

            DrawText("WASD / ARROWS  -  MOVE",
                     350, 340, 20, LIGHTGRAY);

            DrawText("SPACE  -  SHOOT",
                     380, 380, 20, LIGHTGRAY);

            DrawText("ENTER  -  START GAME",
                     345, 470, 24, YELLOW);
        }
        else {
            // Player spaceship
            Color shipColor = damageFlash > 0
                ? RED : Color{70, 220, 255, 255};

            DrawTriangle(
                {player.x, player.y - 24},
                {player.x - 19, player.y + 19},
                {player.x + 19, player.y + 19},
                shipColor
            );

            DrawTriangle(
                {player.x, player.y - 12},
                {player.x - 7, player.y + 8},
                {player.x + 7, player.y + 8},
                WHITE
            );

            // Engine flame
            if (state == PLAYING) {
                float flame = (float)GetRandomValue(8, 22);

                DrawTriangle(
                    {player.x - 7, player.y + 15},
                    {player.x + 7, player.y + 15},
                    {player.x, player.y + 15 + flame},
                    ORANGE
                );
            }

            // Bullets
            for (const auto &b : bullets) {
                if (b.enemy) {
                    DrawCircleV(b.pos, 6, RED);
                    DrawCircleV(b.pos, 3, ORANGE);
                } else {
                    DrawRectangle(
                        (int)b.pos.x - 2,
                        (int)b.pos.y - 8,
                        4, 16, SKYBLUE
                    );
                }
            }

            // Enemies
            for (const auto &e : enemies) {
                Color enemyColor = e.hp > 1 ? PURPLE : RED;

                DrawTriangle(
                    {e.pos.x, e.pos.y + 19},
                    {e.pos.x - 19, e.pos.y - 15},
                    {e.pos.x + 19, e.pos.y - 15},
                    enemyColor
                );

                DrawCircle(e.pos.x, e.pos.y, 5, ORANGE);
            }

            // Health power-ups
            for (const auto &p : powerUps) {
                DrawCircleV(p.pos, 13, GREEN);
                DrawText("+", (int)p.pos.x - 6,
                         (int)p.pos.y - 12, 24, WHITE);
            }

            // HUD
            DrawRectangle(0, 0, SCREEN_W, 58,
                          Color{5, 10, 30, 220});

            DrawText(TextFormat("SCORE: %06i", score),
                     20, 18, 22, RAYWHITE);

            DrawText(TextFormat("WAVE: %i", wave),
                     400, 18, 22, SKYBLUE);

            DrawText("HEALTH",
                     720, 18, 18, RAYWHITE);

            DrawRectangle(805, 19, 150, 18, DARKGRAY);

            DrawRectangle(805, 19,
                          (int)(150 * health / 100.0f),
                          18,
                          health > 30 ? GREEN : RED);

            DrawRectangleLines(805, 19, 150, 18, WHITE);

            if (state == GAME_OVER) {
                DrawRectangle(0, 0, SCREEN_W, SCREEN_H,
                              Color{0, 0, 0, 190});

                DrawText("GAME OVER",
                         320, 230, 58, RED);

                DrawText(TextFormat("FINAL SCORE: %i", score),
                         365, 320, 28, RAYWHITE);

                DrawText("ENTER - PLAY AGAIN",
                         355, 405, 23, YELLOW);

                DrawText("ESC - MAIN MENU",
                         380, 450, 21, LIGHTGRAY);
            }
        }

        DrawText("C++  |  RAYLIB",
                 15, SCREEN_H - 25, 14,
                 Color{120, 140, 180, 200});

        EndDrawing();
    }

    CloseWindow();
    return 0;
}
