/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game.h"
#include "resource_manager.h"
#include "sprite_renderer.h"
#include "game_object.h"
#include "ball_object.h"
#include <iostream>
#include <stdlib.h>
#include <unistd.h>
#include <cmath>
#include "text_renderer.h"
#include <sstream>
#include <ctime>

using namespace std;
clock_t time_done;

// Game-related State data
SpriteRenderer *Renderer;
GameObject *Player;
int lights = 1;
BallObject *Ball;
int prev_level = 0;
int level_num = 1;
TextRenderer *Text;

Game::Game(unsigned int width, unsigned int height)
    : State(GAME_MENU), Keys(), Width(width), Height(height)
{
}

Game::~Game()
{
    delete Renderer;
    delete Player;
    delete Ball;
    delete Text;
}

void Game::Init()
{
    srand(time(0));
    time_done = clock();

    // load shaders
    ResourceManager::LoadShader("../src/shaders/sprite.vs", "../src/shaders/sprite.frag", nullptr, "sprite");
    // configure shaders
    glm::mat4 projection = glm::ortho(0.0f, static_cast<float>(this->Width),
                                      static_cast<float>(this->Height), 0.0f, -1.0f, 1.0f);
    ResourceManager::GetShader("sprite").Use().SetInteger("image", 0);
    ResourceManager::GetShader("sprite").SetMatrix4("projection", projection);
    // set render-specific controls
    Shader myShader;
    myShader = ResourceManager::GetShader("sprite");
    Renderer = new SpriteRenderer(myShader);
    // Renderer = new SpriteRenderer(ResourceManager::GetShader("sprite"));
    //  load textures
    ResourceManager::LoadTexture("../src/textures/backgroundnew.jpg", false, "background");
    ResourceManager::LoadTexture("../src/textures/win.png", true, "win");
    ResourceManager::LoadTexture("../src/textures/gameover.png", true, "gameover");
    ResourceManager::LoadTexture("../src/textures/awesomeface.png", true, "face");
    ResourceManager::LoadTexture("../src/textures/block.png", false, "block");
    ResourceManager::LoadTexture("../src/textures/block_solid.png", false, "block_solid");
    ResourceManager::LoadTexture("../src/textures/doornew.png", true, "door");
    ResourceManager::LoadTexture("../src/textures/coinbright.png", true, "coin");
    ResourceManager::LoadTexture("../src/textures/enemy.png", true, "enemy");
    ResourceManager::LoadTexture("../src/textures/awesomeface2.png", true, "face2");
    ResourceManager::LoadTexture("../src/textures/marioleft.png", true, "mariol");
    ResourceManager::LoadTexture("../src/textures/marioright.png", true, "marior");

    Text = new TextRenderer(this->Width, this->Height);
    Text->Load("../src/fonts/ocratext.ttf", 24);
    // load levels
    GameLevel one;
    one.Load("../src/levels/one.lvl", this->Width, this->Height, 1);
    GameLevel two;
    two.Load("../src/levels/two.lvl", this->Width, this->Height, 2);
    GameLevel three;
    three.Load("../src/levels/three.lvl", this->Width, this->Height, 3);

    this->Levels.push_back(one);
    this->Levels.push_back(two);
    this->Levels.push_back(three);
    this->Level = 0;
    this->score = 0;
    // configure game objects
    glm::vec2 playerPos = glm::vec2(650.0f, 550.0f);
    Player = new GameObject(playerPos, PLAYER_SIZE, ResourceManager::GetTexture("door"));

    Ball = new BallObject(glm::vec2(50.0f, 35.0f), 15.0f, INITIAL_BALL_VELOCITY,
                          ResourceManager::GetTexture("marior"));
}

void Game::Update(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        this->DoCollisions();

        for (BallObject &enemy : this->Levels[this->Level].Enemies)
        {
            // cout << "here";
            enemy.Move(dt, this->Width);
        }
        if (lights == 1)
        {
            for (GameObject &coin : this->Levels[this->Level].Coins)
            {
                float valR = 0.65 + 0.35 * glm::sin(glfwGetTime() * 2);
                float valG = 0.65 + 0.35 * glm::sin(glfwGetTime() * 2);
                float valB = 0.65 + 0.35 * glm::sin(glfwGetTime() * 2);
                coin.Color = glm::vec3(valR, valG, valB);
            }

            if (level_num == 3)
            {
                float valR = 0.70 + 0.30 * glm::sin(glfwGetTime() * 2);
                float valG = 0.70 + 0.30 * glm::sin(glfwGetTime() * 2);
                float valB = 0;
                Player->Color = glm::vec3(valR, valG, valB);
            }
        }

        if (level_num == 2 && prev_level == 0)
        {
            this->ResetLevel();
            this->ResetPlayer();
            prev_level = 1;
            this->Level++;
        }
        if (level_num == 3 && prev_level == 1)
        {
            this->ResetLevel();
            this->ResetPlayer();
            Player->Color = glm::vec3(1.0f, 1.0f, 0.0f);
            prev_level = 2;
            this->Level++;
        }
        if (level_num == 4 && prev_level == 2)
        {
            cout << "GAME OVER";
        }

        if (lights == 0)
        {
            this->LightsOff();
        }
    }
}

void Game::ProcessInput(float dt)
{
    if (this->State == GAME_ACTIVE)
    {
        float velocity = PLAYER_VELOCITY * dt;

        if (this->Keys[GLFW_KEY_UP])
        {
            if (Ball->Position.y <= this->Height)
            {
                Ball->Position.y -= velocity;
            }
        }

        if (this->Keys[GLFW_KEY_LEFT])
        {
            if (Ball->Position.x >= 0.0f)
                Ball->Position.x -= velocity;
            Ball->Sprite = ResourceManager::GetTexture("mariol");
        }
        if (this->Keys[GLFW_KEY_RIGHT])
        {
            Ball->Position.x += velocity;
            Ball->Sprite = ResourceManager::GetTexture("marior");
        }
        if (this->Keys[GLFW_KEY_DOWN])
        {
            Ball->Position.y += velocity;
        }

        if (this->Keys[GLFW_KEY_SPACE])
            Ball->Stuck = false;

        if (this->Keys[GLFW_KEY_SPACE])
        {
            for (int i; i < this->Levels[this->Level].Enemies.size(); i++)
            {
                (this->Levels[this->Level].Enemies[i]).Stuck = false;
            }
        }
        if (this->Keys[GLFW_KEY_L])
        {
            clock_t calc_time;
            calc_time = clock() - time_done;

            if ((float)calc_time / CLOCKS_PER_SEC > float(0.70))
            {
                if (lights == 1)
                {
                    lights = 0;
                    time_done = clock();
                }
                else if (lights == 0)
                {
                    lights = 1;
                    time_done = clock();
                    this->LightsOn();
                }
            }
        }
    }
    if (this->State == GAME_MENU)
    {
        if (this->Keys[GLFW_KEY_ENTER])
        {
            this->State = GAME_ACTIVE;
        }
    }
}

void Game::Render()
{
    if (this->State == GAME_ACTIVE)
    {
        // draw background
        Texture2D myTexture;
        myTexture = ResourceManager::GetTexture("background");
        Renderer->DrawSprite(myTexture, glm::vec2(200, 200), glm::vec2(300, 400), 45.0f, glm::vec3(0.0f, 1.0f, 0.0f));
        // Renderer->DrawSprite(ResourceManager::GetTexture("background"), glm::vec2(0.0f, 0.0f), glm::vec2(this->Width, this->Height), 0.0f);

        // draw level
        this->Levels[this->Level].Draw(*Renderer);
        // draw player
        Player->Draw(*Renderer);
        Ball->Draw(*Renderer);
        std::stringstream ss;
        ss << this->score;
        Text->RenderText("Score:" + ss.str(), 5.0f, 5.0f, 1.0f);
        std::stringstream st;
        st << floor(glfwGetTime());
        Text->RenderText("Time:" + st.str(), 155.0f, 5.0f, 1.0f);
    }
    if (this->State == GAME_OVER)
    {
        Texture2D myTexture1;
        myTexture1 = ResourceManager::GetTexture("gameover");
        Renderer->DrawSprite(myTexture1, glm::vec2(0, 0), glm::vec2(790, 590), 0.0f, glm::vec3(1.0f));
    }
    if (this->State == GAME_WIN)
    {
        Texture2D myTexture2;
        myTexture2 = ResourceManager::GetTexture("win");
        Renderer->DrawSprite(myTexture2, glm::vec2(0, 0), glm::vec2(800, 600), 0.0f, glm::vec3(1.0f));
    }
     if (this->State == GAME_MENU)
    {
        Text->RenderText("Press ENTER to start", 250.0f, this->Height / 2.0f, 1.0f);
    }
}

void Game::ResetLevel()
{
    if (this->Level == 0)
        this->Levels[0].Load("levels/one.lvl", this->Width, this->Height, 1);
    else if (this->Level == 1)
        this->Levels[1].Load("levels/two.lvl", this->Width, this->Height, 2);
    else if (this->Level == 2)
        this->Levels[2].Load("levels/three.lvl", this->Width, this->Height, 3);
}

void Game::ResetPlayer()
{
    // reset player/ball stats
    Player->Size = PLAYER_SIZE;
    Player->Position = glm::vec2(650.0f, 550.0f);
    Ball->Reset(glm::vec2(50.0f, 35.0f), INITIAL_BALL_VELOCITY);
}

float CheckDist(BallObject &one, GameObject &two);

void Game::LightsOff()
{
    float range = 300;
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {

        float distance = CheckDist(*Ball, box);
        if (distance > range)
        {
            distance = range;
        }
        if (box.typ == 1)
        {
            float rVal = 0.8 * (1 - distance / range);
            float gVal = 0.8 * (1 - distance / range);
            float bVal = 0.7 * (1 - distance / range);
            box.Color = glm::vec3(rVal, gVal, bVal);
        }
        if (box.typ == 5)
        {
            float rVal = 1.0 * (1 - distance / range);
            float gVal = 0.5 * (1 - distance / range);
            float bVal = 0.0 * (1 - distance / range);
            box.Color = glm::vec3(rVal, gVal, bVal);
        }
    }
    for (GameObject &enemy : this->Levels[this->Level].Enemies)
    {
        float distance = CheckDist(*Ball, enemy);
        if (distance > range)
        {
            distance = range;
        }
        float rVal = 1.0 * (1 - distance / range);
        float gVal = 1.0 * (1 - distance / range);
        float bVal = 1.0 * (1 - distance / range);
        enemy.Color = glm::vec3(rVal, gVal, bVal);
    }
    for (GameObject &coin : this->Levels[this->Level].Coins)
    {
        float distance = CheckDist(*Ball, coin);
        if (distance > range)
        {
            distance = range;
        }
        float rVal = 1.0 * (1 - distance / range);
        float gVal = 1.0 * (1 - distance / range);
        float bVal = 1.0 * (1 - distance / range);
        coin.Color = glm::vec3(rVal, gVal, bVal);
    }
    float dist = CheckDist(*Ball, *Player);
    if (dist > range)
    {
        dist = range;
    }
    float rVal = 1.0 * (1 - dist / range);
    float gVal = 1.0 * (1 - dist / range);
    float bVal;
    if (level_num == 3)
    {
        bVal = 0.0 * (1 - dist / range);
    }
    else
    {
        bVal = 1.0 * (1 - dist / range);
    }

    Player->Color = glm::vec3(rVal, gVal, bVal);
}

void Game::LightsOn()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        if (box.typ == 1)
        {
            box.Color = glm::vec3(0.8f, 0.8f, 0.7f);
        }
        if (box.typ == 5)
        {
            box.Color = glm::vec3(1.0f, 0.5f, 0.0f);
        }
    }
    for (GameObject &enemy : this->Levels[this->Level].Enemies)
    {
        enemy.Color = glm::vec3(1.0f);
    }
    for (GameObject &coin : this->Levels[this->Level].Coins)
    {
        coin.Color = glm::vec3(1.0f);
    }
    if (level_num == 3)
    {
        Player->Color = glm::vec3(1.0f, 1.0f, 0.0f);
    }
    else
    {
        Player->Color = glm::vec3(1.0f, 1.0f, 1.0f);
    }
}

float CheckDist(BallObject &one, GameObject &two)
{
    glm::vec2 center(one.Position + one.Radius);
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    glm::vec2 difference = center - aabb_center;
    float dist = sqrt(pow(difference.x, 2) + pow(difference.y, 2));
    return dist;
}

// collision detection
bool CheckCollision(GameObject &one, GameObject &two);
Collision CheckCollision(BallObject &one, GameObject &two);
Direction VectorDirection(glm::vec2 closest);

void Game::DoCollisions()
{
    for (GameObject &box : this->Levels[this->Level].Bricks)
    {
        Collision collision = CheckCollision(*Ball, box);
        if (std::get<0>(collision)) // if collision is true
        {

            // collision resolution
            Direction dir = std::get<1>(collision);
            glm::vec2 diff_vector = std::get<2>(collision);
            if (dir == LEFT || dir == RIGHT) // horizontal collision
            {
                Ball->Velocity.x = -Ball->Velocity.x; // reverse horizontal velocity
                // relocate
                float penetration = Ball->Radius - std::abs(diff_vector.x);
                if (dir == LEFT)
                    Ball->Position.x += penetration; // move ball to right
                else
                    Ball->Position.x -= penetration; // move ball to left;
            }
            else // vertical collision
            {
                Ball->Velocity.y = -Ball->Velocity.y; // reverse vertical velocity
                // relocate
                float penetration = Ball->Radius - std::abs(diff_vector.y);
                if (dir == UP)
                    Ball->Position.y -= penetration; // move ball bback up
                else
                    Ball->Position.y += penetration; // move ball back down
            }
        }
        for (BallObject &enemy : this->Levels[this->Level].Enemies)
        {
            Collision collision1 = CheckCollision(enemy, box);
            if (std::get<0>(collision1)) // if collision1 is true
            {

                // collision1 resolution
                Direction dir = std::get<1>(collision1);
                glm::vec2 diff_vector = std::get<2>(collision1);
                if (dir == LEFT || dir == RIGHT) // horizontal collision1
                {
                    enemy.Velocity.x = -enemy.Velocity.x; // reverse horizontal velocity
                    // relocate
                    float penetration = enemy.Radius - std::abs(diff_vector.x);
                    if (dir == LEFT)
                        enemy.Position.x += penetration; // move ball to right
                    else
                        enemy.Position.x -= penetration; // move ball to left;
                }
                else // vertical collision1
                {
                    enemy.Velocity.y = -enemy.Velocity.y; // reverse vertical velocity
                    // relocate
                    float penetration = enemy.Radius - std::abs(diff_vector.y);
                    if (dir == UP)
                        enemy.Position.y -= penetration; // move ball bback up
                    else
                        enemy.Position.y += penetration; // move ball back down
                }
            }
        }
    }
    ////edit
    for (BallObject &enemy : this->Levels[this->Level].Enemies)
    {
        Collision collision1 = CheckCollision(*Ball, enemy);
        if (std::get<0>(collision1)) // if collision1 is true
        {
            cout << "GAME OVER!!" << endl;
            this->State = GAME_OVER;
        }
    }
    ///

    for (GameObject &box : this->Levels[this->Level].Coins)
    {
        if (!box.Destroyed)
        {
            Collision collision = CheckCollision(*Ball, box);
            if (std::get<0>(collision)) // if collision is true
            {
                // destroy block if not solid
                if (!box.IsSolid)
                    box.Destroyed = true;
                // collision resolution
                cout << "got coin" << endl;
                if (lights == 0)
                {
                    this->score = this->score + 20;
                }
                else
                {
                    this->score = this->score + 10;
                }
            }
        }
    }

    ////

    // check collisions for player pad (unless stuck)
    Collision result = CheckCollision(*Ball, *Player);
    if (std::get<0>(result))
    {
        if (level_num < 3)
        {
            level_num++;
        }
        else
        {
            this->State = GAME_WIN;
            cout << "You Won!!" << endl;
        }
    }
}

Collision CheckCollision(BallObject &one, GameObject &two) // AABB - Circle collision
{
    // get center point circle first
    glm::vec2 center(one.Position + one.Radius);
    // calculate AABB info (center, half-extents)
    glm::vec2 aabb_half_extents(two.Size.x / 2.0f, two.Size.y / 2.0f);
    glm::vec2 aabb_center(two.Position.x + aabb_half_extents.x, two.Position.y + aabb_half_extents.y);
    // get difference vector between both centers
    glm::vec2 difference = center - aabb_center;
    glm::vec2 clamped = glm::clamp(difference, -aabb_half_extents, aabb_half_extents);
    // now that we know the the clamped values, add this to AABB_center and we get the value of box closest to circle
    glm::vec2 closest = aabb_center + clamped;
    // now retrieve vector between center circle and closest point AABB and check if length < radius
    difference = closest - center;

    if (glm::length(difference) < one.Radius) // not <= since in that case a collision also occurs when object one exactly touches object two, which they are at the end of each collision resolution stage.
        return std::make_tuple(true, VectorDirection(difference), difference);
    else
        return std::make_tuple(false, UP, glm::vec2(0.0f, 0.0f));
}

// calculates which direction a vector is facing (N,E,S or W)
Direction VectorDirection(glm::vec2 target)
{
    glm::vec2 compass[] = {
        glm::vec2(0.0f, 1.0f),  // up
        glm::vec2(1.0f, 0.0f),  // right
        glm::vec2(0.0f, -1.0f), // down
        glm::vec2(-1.0f, 0.0f)  // left
    };
    float max = 0.0f;
    unsigned int best_match = -1;
    for (unsigned int i = 0; i < 4; i++)
    {
        float dot_product = glm::dot(glm::normalize(target), compass[i]);
        if (dot_product > max)
        {
            max = dot_product;
            best_match = i;
        }
    }
    return (Direction)best_match;
}