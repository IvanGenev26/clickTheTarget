#include <iostream>
#include "raylib.h"

enum GameScreen {MENU, GAMEPLAY, GAMEOVER};

int main() {
    InitWindow(800, 450, "Click the target");
    SetTargetFPS(60);

    GameScreen currentScreen = MENU;
    float screenWidth = GetScreenWidth();
    float screenHeight = GetScreenHeight();

    Vector2 circlePos = {400, 300};
    float radius = 50.0f;
    Color circleColor = RED;
    float hitTimer = 0.0f;
    bool hit = false;
    int score = 0;

    float gameTime = 30.0f;
    bool gameOver = false;

    Rectangle playButton = {300, 200, 200, 50};
    Rectangle quitButton = {300, 300, 200, 50};
    bool playHover = false , quitHover = false;

    while (!WindowShouldClose()) {
        Vector2 mousePos = {(float)GetMouseX(), (float)GetMouseY()};

        switch (currentScreen) {
            case MENU:
                playHover = CheckCollisionPointRec(mousePos, playButton);
                quitHover = CheckCollisionPointRec(mousePos, quitButton);

                if (playHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    currentScreen = GAMEPLAY;
                    score = 0;
                    gameTime = 30.0f;
                    gameOver = false;
                    circlePos = {(float)GetRandomValue(radius, screenWidth - radius), (float)GetRandomValue(radius, screenHeight - radius)};
                }
                if (quitHover && IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    CloseWindow();
                }
                break;
            case GAMEPLAY:
                if (!gameOver) {
                    gameTime -= GetFrameTime();
                    if (gameTime <= 0) {
                        gameOver = true;
                        gameTime = 0;
                    }
                    if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                        if (CheckCollisionPointCircle(mousePos, circlePos, radius)) {
                            circleColor = GREEN;
                            score += 10;
                            hit = true;
                            hitTimer = 0.0f;
                            circlePos.x = (float)GetRandomValue(radius, screenWidth - radius);
                            circlePos.y = (float)GetRandomValue(radius, screenHeight - radius);
                        }
                    }

                    if (hit) {
                        hitTimer += GetFrameTime();
                        if (hitTimer >= 0.1f) {
                            circleColor = RED;
                            hit = false;
                        }
                    }
                } else {
                    currentScreen = GAMEOVER;
                }
                break;
            case GAMEOVER:
                if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT)) {
                    currentScreen = MENU;
                }
            break;
        }

        if (IsKeyPressed(KEY_ESCAPE)) {
            break;
        }

        BeginDrawing();
            ClearBackground(RAYWHITE);
            switch (currentScreen) {
                case MENU:
                    DrawText("CLICK THE TARGET", 250, 100, 40, BLACK);
                    DrawRectangleRec(playButton, playHover ? SKYBLUE : LIGHTGRAY);
                    DrawText("PLAY", playButton.x + playButton.width/2 - MeasureText("PLAY", 30)/2,playButton.y + 15, 30, BLACK);

                    DrawRectangleRec(quitButton, quitHover ? SKYBLUE : LIGHTGRAY);
                    DrawText("QUIT",quitButton.x + quitButton.width/2 - MeasureText("QUIT", 30)/2,quitButton.y + 15, 30, BLACK);
                    break;
                case GAMEPLAY:
                    DrawCircleV(circlePos, radius, circleColor);
                    DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
                    DrawText(TextFormat("Time: %.1f", gameTime), 10, 40, 20, BLACK);
                    break;
                case GAMEOVER:
                    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.5f));
                    DrawText("GAME OVER",  GetScreenWidth()/2 - MeasureText("GAME OVER", 40)/2, GetScreenHeight()/2 - 50, 40, WHITE);
                    DrawText(TextFormat("Final Score: %d", score),GetScreenWidth()/2 - MeasureText("Final Score: 000", 20)/2,GetScreenHeight()/2, 20, WHITE);
                    DrawText("Click to return to menu",GetScreenWidth()/2 - MeasureText("Click to return to menu", 20)/2, GetScreenHeight()/2 + 50, 20, WHITE);
                break;
            }
        EndDrawing();
    }
    CloseWindow();
    return 0;
}
