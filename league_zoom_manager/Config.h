#pragma once

class Config
{
public:
    Config(float height, float fov) :
        camera_height(height),
        camera_fov(fov)
    {

    }

public:
    float camera_height{};
    float camera_fov{};

private:
    const float standard_max_zoom{ 2250.f };
    const float default_extended_zoom{ 3200.f };
    const float zoom_step{ 50.f };

public:
    void handle_input();
};