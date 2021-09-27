#pragma once

struct config_t {
    int power = 0;
    int carSpeed = 0;
    int carMode = 0;
    int enabled = 0;
};

extern config_t config;

void SaveConfig();
bool LoadConfig();