#include <iostream>
#include <vector>
#include <algorithm>
#include <fstream>
#include "raylib.h"

enum GameScreen { MENU, GAMEPLAY, GAMEOVER };
enum GameDifficulty { EASY, MEDIUM, HARD };

struct Particle {
    Vector2 position;
    Vector2 velocity;
    Color color;
    float lifeTime;
};

struct HighScore {
    char name[10];
    int score;
    GameDifficulty difficulty;
};

std::vector<HighScore> leaderboard;
char playerName[10] = "AAA";
bool showingLeaderboard = false;
int nameLetter = 0;
int currentScoreForLeaderboard = 0;
GameDifficulty currentDifficultyForLeaderboard = MEDIUM;

Music mainMenuMusic;
Music gamePlayMusic;
Music gameOverMusic;

void applyDifficulty(GameDifficulty difficulty, Vector2& targetVelocity, float& shrinkSpeed, float& gameTime, float& minRadius);
void LoadLeaderboard();
void SaveLeaderboard();
void AddToLeaderboard(int score, GameDifficulty diff);
void DrawLeaderboard();
void DrawNameInput();
void StopAllMusic();

int main() {
    const int screenWidth = 800;
    const int screenHeight = 600;
    InitWindow(screenWidth, screenHeight, "Click the target");

    Image cursorImage = LoadImage("resources/crosshair.png");
    ImageResize(&cursorImage, 256, 256);
    Texture2D cursorTexture = LoadTextureFromImage(cursorImage);
    UnloadImage(cursorImage);
    HideCursor();

    LoadLeaderboard();
    InitAudioDevice();
    SetMasterVolume(1.0f);
    SetTargetFPS(60);

    GameDifficulty currentDifficulty = MEDIUM;
    GameScreen currentScreen = MENU;

    mainMenuMusic = LoadMusicStream("resources/mainmenu.wav");
    gamePlayMusic = LoadMusicStream("resources/gameplay.wav");
    gameOverMusic = LoadMusicStream("resources/gameover.wav");

    Sound hitSound = LoadSound("resources/hit.wav");
    Sound powerUpSound = LoadSound("resources/powerup.wav");

    Vector2 circlePos = {400, 300};
    Vector2 targetVelocity = {2.0f, 1.5f};
    float radius = 50.0f;
    Color circleColor = RED;
    float hitTimer = 0.0f;
    bool hit = false;

    int score = 0;
    int combo = 0;
    float comboTimeLeft = 0.0f;
    float gameTime = 30.0f;
    bool gameOver = false;
    float shrinkSpeed = 1.0f;
    float minRadius = 15.0f;

    const int MAX_PARTICLES = 50;
    Particle particles[MAX_PARTICLES] = {0};

    Vector2 powerUpPos = {-100, -100};
    bool powerUpActive = false;

    Rectangle playButton = {screenWidth/2 - 100, 150, 200, 50};
    Rectangle leaderboardButton = {screenWidth/2 - 100, 220, 200, 50};
    Rectangle quitButton = {screenWidth/2 - 100, 290, 200, 50};
    Rectangle easyButton = {screenWidth/2 - 225, 450, 150, 40};
    Rectangle mediumButton = {screenWidth/2 - 75, 450, 150, 40};
    Rectangle hardButton = {screenWidth/2 + 75, 450, 150, 40};

    SetMusicVolume(mainMenuMusic, 0.5f);
    SetMusicVolume(gamePlayMusic, 0.5f);
    SetMusicVolume(gameOverMusic, 0.5f);

    while (!WindowShouldClose()) {
        Vector2 mousePos = GetMousePosition();

        bool playHover = CheckCollisionPointRec(mousePos, playButton);
        bool quitHover = CheckCollisionPointRec(mousePos, quitButton);
        bool easyHover = CheckCollisionPointRec(mousePos, easyButton);
        bool mediumHover = CheckCollisionPointRec(mousePos, mediumButton);
        bool hardHover = CheckCollisionPointRec(mousePos, hardButton);
        bool leaderboardHover = CheckCollisionPointRec(mousePos, leaderboardButton);

        UpdateMusicStream(mainMenuMusic);
        UpdateMusicStream(gamePlayMusic);
        UpdateMusicStream(gameOverMusic);

        switch (currentScreen) {
            case MENU:
                if (!IsMusicStreamPlaying(mainMenuMusic)) {
                    StopAllMusic();
                    PlayMusicStream(mainMenuMusic);
                }
                if (playHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    currentScreen = GAMEPLAY;
                    applyDifficulty(currentDifficulty, targetVelocity, shrinkSpeed, gameTime, minRadius);
                    score = 0;
                    combo = 0;
                    gameOver = false;
                    circlePos = {
                        (float)GetRandomValue(radius, screenWidth - radius),
                        (float)GetRandomValue(radius, screenHeight - radius)
                    };
                }
                if (quitHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    CloseWindow();
                }
                if (easyHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) currentDifficulty = EASY;
                if (mediumHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) currentDifficulty = MEDIUM;
                if (hardHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) currentDifficulty = HARD;
                if (leaderboardHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                    showingLeaderboard = true;
                }
                break;

            case GAMEPLAY:
                if (!gameOver) {
                    if (!IsMusicStreamPlaying(gamePlayMusic)) {
                        StopAllMusic();
                        PlayMusicStream(gamePlayMusic);
                    }
                    radius -= shrinkSpeed * GetFrameTime();
                    if (radius < minRadius) radius = minRadius;

                    if (!powerUpActive && GetRandomValue(0, 300) == 0) {
                        powerUpPos = {
                            (float)GetRandomValue(50, screenWidth-50),
                            (float)GetRandomValue(50, screenHeight-50)
                        };
                        powerUpActive = true;
                    }

                    circlePos.x += targetVelocity.x;
                    circlePos.y += targetVelocity.y;

                    if (circlePos.x <= radius || circlePos.x >= screenWidth - radius)
                        targetVelocity.x *= -1;
                    if (circlePos.y <= radius || circlePos.y >= screenHeight - radius)
                        targetVelocity.y *= -1;

                    if (comboTimeLeft > 0) {
                        comboTimeLeft -= GetFrameTime();
                    } else {
                        combo = 0;
                    }

                    gameTime -= GetFrameTime();
                    if (gameTime <= 0) {
                        gameOver = true;
                        gameTime = 0;
                    }

                    if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                        if (CheckCollisionPointCircle(mousePos, circlePos, radius)) {
                            for (int i = 0; i < MAX_PARTICLES; i++) {
                                particles[i] = {
                                    circlePos,
                                    {(float)GetRandomValue(-100, 100)/20.0f,
                                     (float)GetRandomValue(-100, 100)/20.0f},
                                    ORANGE,
                                    1.0f
                                };
                            }

                            radius = 50.0f;
                            circleColor = GREEN;
                            PlaySound(hitSound);
                            combo++;
                            comboTimeLeft = 2.0f;
                            score += (currentDifficulty == HARD ? 15 :
                                    (currentDifficulty == EASY ? 7 : 10)) * combo;
                            hit = true;
                            hitTimer = 0.0f;
                            circlePos = {
                                (float)GetRandomValue(radius, screenWidth - radius),
                                (float)GetRandomValue(radius, screenHeight - radius)
                            };
                            targetVelocity = {
                                (float)GetRandomValue(-5, 5),
                                (float)GetRandomValue(-5, 5)
                            };
                        }

                        if (powerUpActive && CheckCollisionPointCircle(mousePos, powerUpPos, 40)) {
                            score += 50;
                            PlaySound(powerUpSound);
                            powerUpActive = false;
                        }
                    }

                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].lifeTime > 0) {
                            particles[i].position.x += particles[i].velocity.x;
                            particles[i].position.y += particles[i].velocity.y;
                            particles[i].lifeTime -= GetFrameTime();
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
                    currentScoreForLeaderboard = score;
                    currentDifficultyForLeaderboard = currentDifficulty;
                    currentScreen = GAMEOVER;
                }
                break;

            case GAMEOVER:
                if (!IsMusicStreamPlaying(gameOverMusic)) {
                    StopAllMusic();
                    PlayMusicStream(gameOverMusic);
                }
                if (!showingLeaderboard) {
                    bool highScore = leaderboard.size() < 10 || score > leaderboard.back().score;

                    if (highScore) {
                        if (IsKeyPressed(KEY_LEFT)) nameLetter = (nameLetter - 1 + 3) % 3;
                        if (IsKeyPressed(KEY_RIGHT)) nameLetter = (nameLetter + 1) % 3;
                        if (IsKeyPressed(KEY_UP)) playerName[nameLetter] = (playerName[nameLetter] - 'A' + 1) % 26 + 'A';
                        if (IsKeyPressed(KEY_DOWN)) playerName[nameLetter] = (playerName[nameLetter] - 'A' - 1 + 26) % 26 + 'A';

                        if (IsKeyPressed(KEY_ENTER)) {
                            AddToLeaderboard(score, currentDifficulty);
                            showingLeaderboard = false;
                            currentScreen = MENU;
                        }
                    } else {
                        if (IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            currentScreen = MENU;
                        }

                        Rectangle viewLbButton = {screenWidth/2 - 100, screenHeight/2 + 100, 200, 40};
                        bool viewLbHover = CheckCollisionPointRec(GetMousePosition(), viewLbButton);

                        if (viewLbHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
                            showingLeaderboard = true;
                        }
                    }
                }
                break;
        }

        BeginDrawing();
        switch (currentDifficulty) {
            case EASY:
                ClearBackground((Color){230, 240, 255, 255});
            for (int y = 0; y < GetScreenHeight(); y += 40) {
                DrawLine(0, y, GetScreenWidth(), y, Fade(SKYBLUE, 0.1f));
            }
            break;

            case MEDIUM:
                ClearBackground((Color){240, 230, 200, 255});
            for (int x = 0; x < GetScreenWidth(); x += 40) {
                DrawLine(x, 0, x, GetScreenHeight(), Fade(GOLD, 0.1f));
            }
            break;

            case HARD:
                float pulse = 0.5f * sin(GetTime() * 2) + 0.5f;
            Color bgColor = {
                (unsigned char)(50 + pulse * 50),
                (unsigned char)(20 + pulse * 20),
                (unsigned char)(30 + pulse * 30),
                255
            };
            ClearBackground(bgColor);

            for (int i = 0; i < 20; i++) {
                float alpha = 0.05f + 0.1f * (i % 3);
                DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2,
                               i * 50 + fmod(GetTime() * 20, 50),
                               Fade((Color){200, 80, 100, 255}, alpha));
            }
            break;
        }

            const char* diffText = "";
            float time = GetTime();

            switch (currentScreen) {
                case MENU:
                    for (int i = 0; i < 10; i++) {
                        float offset = time * 0.5f + i * 0.3f;
                        float x = sin(offset) * 100 + GetScreenWidth()/2;
                        float y = cos(offset * 0.7f) * 50 + 100;
                        float radius = 30 + sin(offset * 1.3f) * 10;

                        Color inner = Fade((Color){255, (unsigned char)(100 + i*15), 100, 255}, 0.3f);
                        Color outer = Fade((Color){100, 100, (unsigned char)(255 - i*15), 255}, 0.1f);

                        DrawCircleGradient(x, y, radius, Color{(unsigned char)255, (unsigned char)(100 + i*15), (unsigned char)100, (unsigned char)255}, Color{(unsigned char)100, (unsigned char)100, (unsigned char)(255 - i*15), (unsigned char)255});
                    }

                    DrawText("CLICK THE TARGET", screenWidth/2 - MeasureText("CLICK THE TARGET", 40)/2, 80, 40, BLACK);

                    DrawRectangleRec(playButton, playHover ? SKYBLUE : LIGHTGRAY);
                    DrawRectangleLinesEx(playButton, 2, BLACK);
                    DrawText("PLAY", playButton.x + playButton.width/2 - MeasureText("PLAY", 30)/2, playButton.y + 10, 30, BLACK);

                    DrawRectangleRec(leaderboardButton, leaderboardHover ? PURPLE : LIGHTGRAY);
                    DrawRectangleLinesEx(leaderboardButton, 2, BLACK);
                    DrawText("LEADERBOARD",
                        leaderboardButton.x + leaderboardButton.width/2 - MeasureText("LEADERBOARD", 20)/2,
                        leaderboardButton.y + 15,
                        20, BLACK);
                    DrawRectangleRec(quitButton, quitHover ? SKYBLUE : LIGHTGRAY);
                    DrawRectangleLinesEx(quitButton, 2, BLACK);
                    DrawText("QUIT", quitButton.x + quitButton.width/2 - MeasureText("QUIT", 30)/2, quitButton.y + 10, 30, BLACK);

                    DrawText("SELECT DIFFICULTY:", screenWidth/2 - MeasureText("SELECT DIFFICULTY:", 20)/2, 420, 20, BLACK);

                    DrawRectangleRec(easyButton, currentDifficulty == EASY ? GREEN : (easyHover ? LIME : LIGHTGRAY));
                    DrawRectangleLinesEx(easyButton, 2, BLACK);
                    DrawText("EASY", easyButton.x + easyButton.width/2 - MeasureText("EASY", 20)/2, easyButton.y + 10, 20, BLACK);

                    DrawRectangleRec(mediumButton, currentDifficulty == MEDIUM ? GOLD : (mediumHover ? YELLOW : LIGHTGRAY));
                    DrawRectangleLinesEx(mediumButton, 2, BLACK);
                    DrawText("MEDIUM", mediumButton.x + mediumButton.width/2 - MeasureText("MEDIUM", 20)/2, mediumButton.y + 10, 20, BLACK);

                    DrawRectangleRec(hardButton, currentDifficulty == HARD ? RED : (hardHover ? PINK : LIGHTGRAY));
                    DrawRectangleLinesEx(hardButton, 2, BLACK);
                    DrawText("HARD", hardButton.x + hardButton.width/2 - MeasureText("HARD", 20)/2, hardButton.y + 10, 20, BLACK);

                    if (showingLeaderboard) {
                        DrawLeaderboard();
                    }
                    break;

                case GAMEPLAY:
                    DrawCircleV(circlePos, radius, circleColor);

                    for (int i = 0; i < MAX_PARTICLES; i++) {
                        if (particles[i].lifeTime > 0) {
                            DrawCircleV(particles[i].position, 3 * particles[i].lifeTime, Fade(particles[i].color, particles[i].lifeTime));
                        }
                    }

                    if (powerUpActive) {
                        float pulse = sin(GetTime() * 5) * 0.1f + 1.0f;
                        DrawCircleV(powerUpPos, 40 * pulse, GOLD);
                    }

                    switch (currentDifficulty) {
                        case EASY: diffText = "EASY"; break;
                        case MEDIUM: diffText = "MEDIUM"; break;
                        case HARD: diffText = "HARD"; break;
                    }
                    DrawText(TextFormat("Difficulty: %s", diffText), screenWidth - 200, 10, 20, PINK);
                    DrawText(TextFormat("Score: %d", score), 10, 10, 20, BLACK);
                    DrawText(TextFormat("Time: %.1f", gameTime), 10, 40, 20, BLACK);
                    DrawText(TextFormat("Combo: x%d", combo), 10, 70, 20, PURPLE);
                    break;

                case GAMEOVER:
                    if (!showingLeaderboard) {
                        bool highScore = leaderboard.size() < 10 || score > leaderboard.back().score;

                        if (highScore) {
                            DrawNameInput();
                        } else {
                            DrawRectangle(0, 0, screenWidth, screenHeight, Fade(BLACK, 0.5f));
                            DrawText("GAME OVER", screenWidth/2 - MeasureText("GAME OVER", 40)/2, screenHeight/2 - 50, 40, WHITE);
                            DrawText(TextFormat("FINAL SCORE: %d", score), screenWidth/2 - MeasureText("FINAL SCORE: 000", 20)/2, screenHeight/2, 20, WHITE);
                            DrawText("Click to return to menu", screenWidth/2 - MeasureText("Click to return to menu", 20)/2, screenHeight/2 + 50, 20, WHITE);

                            Rectangle viewLbButton = {screenWidth/2 - 100, screenHeight/2 + 100, 200, 40};
                            bool viewLbHover = CheckCollisionPointRec(GetMousePosition(), viewLbButton);

                            DrawRectangleRec(viewLbButton, viewLbHover ? PURPLE : LIGHTGRAY);
                            DrawRectangleLinesEx(viewLbButton, 2, BLACK);
                            DrawText("VIEW LEADERBOARD", viewLbButton.x + viewLbButton.width/2 - MeasureText("VIEW LEADERBOARD", 20)/2, viewLbButton.y + 10, 20, BLACK);
                            float pulseTime = GetTime() * 2.5f;
                            for (int i = 1; i <= 5; i++) {
                                float pulse = 0.5f + 0.5f * sin(pulseTime + i);
                                float radius = (100 + 50 * pulse) * i;
                                float alpha = 0.4f / i;

                                Color circleColor = RED;
                                if (currentDifficulty == EASY) circleColor = GREEN;
                                else if (currentDifficulty == MEDIUM) circleColor = GOLD;

                                DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2, radius,
                                              Fade(circleColor, alpha));

                                DrawCircleLines(GetScreenWidth()/2, GetScreenHeight()/2, radius - 5,
                                              Fade(WHITE, alpha * 0.5f));
                            }
                        }
                    } else {
                        DrawLeaderboard();
                    }
                break;
            }
            DrawTextureEx(cursorTexture, mousePos, 0.0f, 0.1f, WHITE);
        EndDrawing();
    }

    UnloadTexture(cursorTexture);
    ShowCursor();
    UnloadSound(hitSound);
    UnloadSound(powerUpSound);
    UnloadMusicStream(mainMenuMusic);
    UnloadMusicStream(gamePlayMusic);
    UnloadMusicStream(gameOverMusic);

    CloseAudioDevice();
    CloseWindow();
    return 0;
}

void applyDifficulty(GameDifficulty difficulty, Vector2& targetVelocity,
                    float& shrinkSpeed, float& gameTime, float& minRadius) {
    switch (difficulty) {
        case EASY:
            targetVelocity = {1.0f, 0.8f};
            shrinkSpeed = 0.1f;
            gameTime = 45.0f;
            minRadius = 25.0f;
            break;
        case MEDIUM:
            targetVelocity = {2.0f, 1.5f};
            shrinkSpeed = 0.3f;
            gameTime = 30.0f;
            minRadius = 15.0f;
            break;
        case HARD:
            targetVelocity = {3.5f, 2.8f};
            shrinkSpeed = 0.5f;
            gameTime = 20.0f;
            minRadius = 10.0f;
            break;
    }
}

void LoadLeaderboard() {
    leaderboard.clear();
    std::ifstream file("leaderboard.dat", std::ios::binary);
    if (file) {
        HighScore entry;
        while (file.read((char*)&entry, sizeof(HighScore))) {
            leaderboard.push_back(entry);
        }
    }
    std::sort(leaderboard.begin(), leaderboard.end(),
        [](const HighScore& a, const HighScore& b) {
            return a.score > b.score;
        });
}

void SaveLeaderboard() {
    std::ofstream file("leaderboard.dat", std::ios::binary);
    for (const auto& entry : leaderboard) {
        file.write((char*)&entry, sizeof(HighScore));
    }
}

void AddToLeaderboard(int score, GameDifficulty diff) {
    if (leaderboard.size() < 10 || score > leaderboard.back().score) {
        HighScore newEntry;
        strncpy(newEntry.name, playerName, 9);
        newEntry.name[9] = '\0';
        newEntry.score = score;
        newEntry.difficulty = diff;

        leaderboard.push_back(newEntry);
        std::sort(leaderboard.begin(), leaderboard.end(),
            [](const HighScore& a, const HighScore& b) {
                return a.score > b.score;
            });

        if (leaderboard.size() > 10) {
            leaderboard.pop_back();
        }
        SaveLeaderboard();
    }
}

void DrawLeaderboard() {
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));

    Rectangle leaderboardRect = {
        (float)GetScreenWidth()/2 - 300,
        (float)GetScreenHeight()/2 - 250,
        600,
        400
    };
    DrawRectangleRec(leaderboardRect, LIGHTGRAY);
    DrawRectangleLinesEx(leaderboardRect, 3, DARKGRAY);

    DrawText("LEADERBOARD",
        leaderboardRect.x + leaderboardRect.width/2 - MeasureText("LEADERBOARD", 40)/2,
        leaderboardRect.y + 20,
        40, DARKBLUE);

    float columnY = leaderboardRect.y + 80;
    DrawText("RANK", leaderboardRect.x + 50, columnY, 25, DARKBLUE);
    DrawText("NAME", leaderboardRect.x + 150, columnY, 25, DARKBLUE);
    DrawText("SCORE", leaderboardRect.x + 300, columnY, 25, DARKBLUE);
    DrawText("DIFFICULTY", leaderboardRect.x + 450, columnY, 25, DARKBLUE);

    for (int i = 0; i < (int)leaderboard.size() && i < 10; i++) {
        const auto& entry = leaderboard[i];
        float entryY = columnY + 40 + (i * 30);
        bool isCurrent = (strcmp(entry.name, playerName) == 0 && entry.score == currentScoreForLeaderboard);

        Color rowColor = i % 2 == 0 ? RAYWHITE : LIGHTGRAY;
        if (isCurrent) rowColor = Color{200, 255, 200, 255};

        DrawRectangle(leaderboardRect.x + 20, entryY - 5, leaderboardRect.width - 40, 30, rowColor);

        DrawText(TextFormat("%d.", i+1), leaderboardRect.x + 50, entryY, 20, BLACK);

        const char* nameToDisplay = entry.name;
        int nameWidth = MeasureText(nameToDisplay, 20);
        if (nameWidth > 120) {
            char shortenedName[10];
            strncpy(shortenedName, entry.name, 6);
            shortenedName[6] = '\0';
            strcat(shortenedName, "...");
            nameToDisplay = shortenedName;
        }
        DrawText(nameToDisplay, leaderboardRect.x + 150, entryY, 20, BLACK);

        DrawText(entry.name, leaderboardRect.x + 150, entryY, 20, BLACK);

        DrawText(TextFormat("%d", entry.score), leaderboardRect.x + 300, entryY, 20, BLACK);

        const char* diffText = "";
        Color diffColor = BLACK;
        switch(entry.difficulty) {
            case EASY: diffText = "EASY"; diffColor = GREEN; break;
            case MEDIUM: diffText = "MEDIUM"; diffColor = ORANGE; break;
            case HARD: diffText = "HARD"; diffColor = RED; break;
        }
        DrawText(diffText, leaderboardRect.x + 450, entryY, 20, diffColor);
    }

    Rectangle closeButton = {
        leaderboardRect.x + leaderboardRect.width - 120,
        leaderboardRect.y + leaderboardRect.height - 50,
        100,
        30
    };
    bool closeHover = CheckCollisionPointRec(GetMousePosition(), closeButton);

    DrawRectangleRec(closeButton, closeHover ? RED : MAROON);
    DrawRectangleLinesEx(closeButton, 2, BLACK);
    DrawText("CLOSE",
        closeButton.x + closeButton.width/2 - MeasureText("CLOSE", 20)/2,
        closeButton.y + 5,
        20, WHITE);

    if (closeHover && IsMouseButtonPressed(MOUSE_LEFT_BUTTON)) {
        showingLeaderboard = false;
    }
}

void DrawNameInput() {
    // Dark background
    DrawRectangle(0, 0, GetScreenWidth(), GetScreenHeight(), Fade(BLACK, 0.7f));

    DrawText("NEW HIGH SCORE!", GetScreenWidth()/2 - MeasureText("NEW HIGH SCORE!", 40)/2, 150, 40, GOLD);
    DrawText(TextFormat("YOUR SCORE: %d", currentScoreForLeaderboard),
           GetScreenWidth()/2 - MeasureText(TextFormat("YOUR SCORE: %d", currentScoreForLeaderboard), 30)/2,
           200, 30, WHITE);

    DrawText("ENTER YOUR INITIALS:", GetScreenWidth()/2 - MeasureText("ENTER YOUR INITIALS:", 30)/2, 250, 30, WHITE);

    for (int i = 0; i < 3; i++) {
        Rectangle box = {(float)GetScreenWidth()/2 - 50 + i*40, 300, 40, 50};
        bool active = (nameLetter == i);
        DrawRectangleRec(box, active ? YELLOW : LIGHTGRAY);
        DrawRectangleLinesEx(box, 2, active ? GOLD : GRAY);
        DrawText(TextFormat("%c", playerName[i]), box.x + 15, box.y + 15, 30, BLACK);
    }

    DrawText("Use ARROW KEYS to change letters",
            GetScreenWidth()/2 - MeasureText("Use ARROW KEYS to change letters", 20)/2,
            370, 20, LIGHTGRAY);
    DrawText("Press ENTER when done",
            GetScreenWidth()/2 - MeasureText("Press ENTER when done", 20)/2,
            400, 20, LIGHTGRAY);
}

void StopAllMusic() {
    StopMusicStream(mainMenuMusic);
    StopMusicStream(gamePlayMusic);
    StopMusicStream(gameOverMusic);
}
