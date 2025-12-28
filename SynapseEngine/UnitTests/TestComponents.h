#pragma once

struct Position {
    float x, y;
    bool operator==(const Position& other) const { return x == other.x && y == other.y; }
};

struct Velocity {
    float dx, dy;
};

struct Health {
    int hp;
    int maxHp;
};

struct TagEnemy {};

struct Vec3 {
    float x, y, z;
    bool operator==(const Vec3& o) const { return x == o.x && y == o.y && z == o.z; }
};

struct Matrix4 {
    float data[16];
};