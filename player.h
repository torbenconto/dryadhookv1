#pragma once

#include <Windows.h>

class Player {
public:
    static DWORD base;

    static float posX;
    static float posY;
    static float velX;
    static float velY;
    static int width;
    static int height;
    static int health;
    static int maxHealth;

    static void Read(DWORD baseAddr) {
        base = baseAddr;
        posX = *(float*)(base + 0x28);
        posY = *(float*)(base + 0x2C);
        velX = *(float*)(base + 0x30);
        velY = *(float*)(base + 0x34);
        width = *(int*)(base + 0x18);
        height = *(int*)(base + 0x1C);
        health = *(int*)(base + 0x404);
        maxHealth = *(int*)(base + 0x3FC);
    }

    static void Apply() {
        if (!base) return;

        *(float*)(base + 0x28) = posX;
        *(float*)(base + 0x2C) = posY;
        *(float*)(base + 0x30) = velX;
        *(float*)(base + 0x34) = velY;
        *(int*)(base + 0x404) = health;
        *(int*)(base + 0x3FC) = maxHealth;
    }
};

DWORD Player::base = 0;
float Player::posX = 0.0f;
float Player::posY = 0.0f;
float Player::velX = 0.0f;
float Player::velY = 0.0f;
int Player::width = 0;
int Player::height = 0;
int Player::health = 0;
int Player::maxHealth = 0;