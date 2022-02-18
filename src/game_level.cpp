/*******************************************************************
** This code is part of Breakout.
**
** Breakout is free software: you can redistribute it and/or modify
** it under the terms of the CC BY 4.0 license as published by
** Creative Commons, either version 4 of the License, or (at your
** option) any later version.
******************************************************************/
#include "game_level.h"

#include <fstream>
#include <sstream>
#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <random>
#include <stdlib.h>
using namespace std;

// Initial velocity of the Ball
const glm::vec2 INITIAL_BALL_VELOCITY(50.0f, -150.0f);
// Radius of the ball object
const float BALL_RADIUS = 12.5f;

void GameLevel::Load(const char *file, unsigned int levelWidth, unsigned int levelHeight, unsigned int lvl)
{
    // clear old data
    this->Bricks.clear();
    this->Enemies.clear();
    this->Coins.clear();
    // load from file
    unsigned int tileCode;
    GameLevel level;
    std::string line;
    std::ifstream fstream(file);
    std::vector<std::vector<unsigned int>> tileData;
    if (fstream)
    {
        while (std::getline(fstream, line)) // read each line from level file
        {
            std::istringstream sstream(line);
            std::vector<unsigned int> row;
            while (sstream >> tileCode) // read each word separated by spaces
                row.push_back(tileCode);
            tileData.push_back(row);
        }
        if (tileData.size() > 0)
            this->init(tileData, levelWidth, levelHeight, lvl);
    }
}

void GameLevel::Draw(SpriteRenderer &renderer)
{
    for (GameObject &tile : this->Bricks)
        if (!tile.Destroyed)
            tile.Draw(renderer);

    for (BallObject &enemy : this->Enemies)
    {
        enemy.Draw(renderer);
    }
    for (GameObject &coin : this->Coins)
    {
        if (!coin.Destroyed)
            coin.Draw(renderer);
    }
}

bool GameLevel::IsCompleted()
{
    for (GameObject &tile : this->Bricks)
        if (!tile.IsSolid && !tile.Destroyed)
            return false;
    return true;
}

void GameLevel::init(std::vector<std::vector<unsigned int>> tileData, unsigned int levelWidth, unsigned int levelHeight, unsigned int lvl)
{
    // calculate dimensions
    unsigned int height = tileData.size();
    unsigned int width = tileData[0].size(); // note we can index vector at [0] since this function is only called if height > 0
    float unit_width = levelWidth / static_cast<float>(width), unit_height = levelHeight / height;

    int no_of_walls = lvl * 2 + 1;

    for (int x = 1; x < width - 1; x++)
    {
        for (int y = 1; y < height - 1; y++)
        {
            if (x + y < 34 && x+y >4)
            {
                int val = rand() % 100;
                if (val < no_of_walls)
                {
                    tileData[y][x] = 1;
                }
            }
        }
    }

    int no_of_coins = lvl * 2 - 1;
    for (int i = 0; i < no_of_coins; i++)
    {
        int x_val = rand() % 18 + 1;
        int y_val = rand() % 18 + 1;
        if (x_val + y_val < 4 || x_val + y_val > 34)
        {
            int x_val = rand() % 18 + 1;
            int y_val = rand() % 18 + 1;
        }
        tileData[y_val][x_val] = 3;

        glm::vec2 pos(unit_width * x_val, unit_height * y_val);
        glm::vec2 size(unit_width, unit_height);
        GameObject obj(pos, size, ResourceManager::GetTexture("coin"), glm::vec3(1.0f));
        obj.IsSolid = false;
        this->Coins.push_back(obj);
    }

    int no_of_enemies = lvl;
    for (int i = 0; i < no_of_enemies; i++)
    {
        int x_val = rand() % 18 + 1;
        int y_val = rand() % 18 + 1;
        if (x_val + y_val < 4 || x_val + y_val > 34)
        {
            int x_val = rand() % 18 + 1;
            int y_val = rand() % 18 + 1;
        }
        tileData[y_val][x_val] = 4;

        glm::vec2 pos(unit_width * x_val, unit_height * y_val);
        glm::vec2 size(unit_width, unit_height);
        BallObject e1(pos, BALL_RADIUS, INITIAL_BALL_VELOCITY, ResourceManager::GetTexture("enemy"));
        e1.Stuck = false;
        this->Enemies.push_back(e1);
    }

    // initialize level tiles based on tileData
    for (unsigned int y = 0; y < height; ++y)
    {
        for (unsigned int x = 0; x < width; ++x)
        {
            // check block type from level data (2D level array)
            if (tileData[y][x] == 1) // solid
            {
                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                GameObject obj(pos, size, ResourceManager::GetTexture("block_solid"), glm::vec3(0.8f, 0.8f, 0.7f));
                obj.IsSolid = true;
                obj.typ = 1;
                this->Bricks.push_back(obj);
            }
            else if (tileData[y][x] == 5) // non-solid; now determine its color based on level data
            {
                glm::vec3 color = glm::vec3(1.0f);
                if (tileData[y][x] == 5)
                    color = glm::vec3(1.0f, 0.5f, 0.0f);

                glm::vec2 pos(unit_width * x, unit_height * y);
                glm::vec2 size(unit_width, unit_height);
                GameObject obj(pos, size, ResourceManager::GetTexture("block"), color);
                obj.IsSolid = true;
                obj.typ = 5;
                this->Bricks.push_back(obj);
            }
        }
    }
}