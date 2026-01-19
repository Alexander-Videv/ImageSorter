#include "raylib.h"

#include <iostream>
#include <vector>
#include <algorithm>

#define GUI_WINDOW_FILE_DIALOG_IMPLEMENTATION
#include "../headers/gui_window_file_dialog.h"

#include "../headers/lumin.hpp"

#define RAYGUI_IMPLEMENTATION
#include "../headers/raygui.h"

#define SCREEN_WIDTH (1920)
#define SCREEN_HEIGHT (1080)

#define WINDOW_TITLE "Pixel Sorter"

#define TOOLBAR_HEIGHT (24.0f)
#define TOOLBAR_POSITION (0.0f)

struct LoadedImage
{
    Image original;
    Image mask;
    Image sorted;

    int width;
    int height;

    int img_x;
    int img_y;
};

struct Range
{
    int start_idx;
    int lenght;
};

std::vector<Range> makeMask(Image &mask, std::vector<Pixel> &pixels, float lumin_min, float lumin_max, bool inverted, bool vertical)
{
    float curr_lumin;
    bool in_range;

    Range curr_range{-1, -1};
    std::vector<Range> ranges;

    if (!vertical)
    {
        // Horizontal ranges
        for (size_t i = 0; i < pixels.size(); i++)
        {
            curr_lumin = getLuminescence(pixels.at(i));

            in_range = inverted ? !(curr_lumin >= lumin_min && curr_lumin <= lumin_max)
                                : curr_lumin >= lumin_min && curr_lumin <= lumin_max;

            if (in_range)
            {
                if (curr_range.start_idx == -1)
                    curr_range.start_idx = i;

                curr_range.lenght++;
                setWhite_4(i * 4, mask);
            }
            else
            {
                setBlack_4(i * 4, mask);

                if (curr_range.lenght > 5)
                    ranges.push_back(curr_range);

                curr_range = {-1, 0};
            }
        }
    }
    else
    {
        // Vertical ranges
        int index;
        for (size_t col = 0; col < mask.width; col++)
        {
            if (curr_range.start_idx != -1)
                ranges.push_back(curr_range);
            curr_range = {-1, 0};

            for (size_t row = 0; row < mask.height; row++)
            {
                index = row * mask.width + col;

                curr_lumin = getLuminescence(pixels.at(index));

                in_range = inverted ? !(curr_lumin >= lumin_min && curr_lumin <= lumin_max)
                                    : curr_lumin >= lumin_min && curr_lumin <= lumin_max;

                if (in_range)
                {
                    if (curr_range.start_idx == -1)
                        curr_range.start_idx = index;

                    curr_range.lenght++;
                    setWhite_4(index * 4, mask);
                }
                else
                {
                    setBlack_4(index * 4, mask);

                    if (curr_range.lenght > 5)
                        ranges.push_back(curr_range);

                    curr_range = {-1, 0};
                }
            }
        }
    }

    return ranges;
}

void sort(LoadedImage &img, std::vector<Pixel> &pixels, std::vector<Range> ranges, bool vertical)
{
    int index = 0;
    std::vector<Pixel> buff;
    for (const Range range : ranges)
    {
        if (!vertical)
        {
            std::sort(pixels.begin() + range.start_idx, pixels.begin() + range.start_idx + range.lenght, cmp);
        }
        else
        {
            for (size_t i = 0; i < range.lenght; i++)
            {
                index = range.start_idx + i * img.width;

                buff.push_back(pixels.at(index));
            }

            std::sort(buff.begin(), buff.end(), cmp);

            for (size_t i = 0; i < range.lenght; i++)
            {
                index = range.start_idx + i * img.width;

                pixels.at(index) = buff.at(i);
            }

            buff.clear();
        }
    }

    convertToRGBAPix(pixels, (char *)img.sorted.data);
}

void handleImage(LoadedImage &img, float lumin_min, float lumin_max, bool inverted, bool vertical)
{
    char *data = (char *)img.original.data;
    int size = img.width * img.height;

    std::vector<Pixel> pixels = convertToPixRGBA(data, size);

    img.mask = ImageCopy(img.original);
    img.sorted = ImageCopy(img.original);

    std::vector<Range> ranges = makeMask(img.mask, pixels, lumin_min, lumin_max, inverted, vertical);

    sort(img, pixels, ranges, vertical);
}

int main(void)
{
    // Window configuration flags

    SetConfigFlags(FLAG_WINDOW_RESIZABLE);
    // SetConfigFlags(FLAG_FULLSCREEN_MODE);

    InitWindow(SCREEN_WIDTH, SCREEN_HEIGHT, WINDOW_TITLE);

    // SetTargetFPS(60);

    // State Init

    int screenW = SCREEN_WIDTH;
    int screenH = SCREEN_HEIGHT;

    GuiSetStyle(STATUSBAR, BASE_COLOR_NORMAL, 0x7cb677FF);

    // Image State

    bool loaded = false;

    float lumin_floor = 0.20;
    float lumin_ceil = 0.80;
    bool invert_range = false;
    bool vertical = false;

    LoadedImage img;
    Texture2D img_tex;

    // Toolbar State

    bool draw_sliders = false;
    int btn_count = 6;

    // Dropdown State

    bool dropdown_edit_mode = false;
    const char *dropdown_options = "Horizontal;Vertical";
    int selected = 0;

    // File Dialog State

    GuiWindowFileDialogState fileDialogState = InitGuiWindowFileDialog(GetWorkingDirectory());

    char fileNameToLoad[512] = {0};

    // Bounds definitions
    float padding = 10.0f;
    float btn_width = std::min(200.0f, float(screenW / btn_count) / 2);

    Rectangle toolbar_bounds{0, 0, float(screenW), TOOLBAR_HEIGHT};

    Rectangle slider_bounds{padding, TOOLBAR_HEIGHT + padding, 300.0, 200.0};

    Rectangle dropdown_bounds{slider_bounds.x + 80.0f + 90.0f, slider_bounds.y + 160.0f, 90.0f, 20.0};

    // Button Bounds

    Rectangle sort_button_bounds = {padding * 1 + btn_width * 0, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
    Rectangle reset_button_bounds = {padding * 2 + btn_width * 1, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
    Rectangle mask_button_bounds = {padding * 3 + btn_width * 2, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
    Rectangle recalculate_button_bounds = {padding * 4 + btn_width * 3, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
    Rectangle sliders_button_bounds = {padding * 5 + btn_width * 4, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
    Rectangle open_image_button_bounds = {padding * 6 + btn_width * 5, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};

    while (!WindowShouldClose())
    {
        // File Dialog

        if (fileDialogState.SelectFilePressed)
        {
            // Load image file (if supported extension)
            if (IsFileExtension(fileDialogState.fileNameText, ".png"))
            {

                loaded = false;

                strcpy(fileNameToLoad, TextFormat("%s" PATH_SEPERATOR "%s", fileDialogState.dirPathText, fileDialogState.fileNameText));

                // Load Image

                img.original = LoadImage(fileNameToLoad);
                ImageResize(&img.original, 1600, 900);

                img.height = img.original.height;
                img.width = img.original.width;

                img.img_x = screenW / 2 - img.width / 2;
                img.img_y = screenH / 2 - img.height / 2;

                handleImage(img, lumin_floor, lumin_ceil, invert_range, vertical);
                img_tex = LoadTextureFromImage(img.original);

                loaded = true;
            }

            fileDialogState.SelectFilePressed = false;
        }

        // Resizing

        if (IsWindowResized())
        {
            screenW = GetScreenWidth();
            screenH = GetScreenHeight();
            toolbar_bounds.width = screenW;

            btn_width = std::min(200.0f, float(screenW / btn_count) / 2);

            sort_button_bounds = {padding * 1 + btn_width * 0, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
            reset_button_bounds = {padding * 2 + btn_width * 1, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
            mask_button_bounds = {padding * 3 + btn_width * 2, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
            recalculate_button_bounds = {padding * 4 + btn_width * 3, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
            sliders_button_bounds = {padding * 5 + btn_width * 4, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};
            open_image_button_bounds = {padding * 6 + btn_width * 5, TOOLBAR_POSITION, btn_width, TOOLBAR_HEIGHT};

            // Resize Image

            img.img_x = screenW / 2 - img.width / 2;
            img.img_y = screenH / 2 - img.height / 2;
        }

        // Drawing

        BeginDrawing();
        ClearBackground(GetColor(GuiGetStyle(DEFAULT, BACKGROUND_COLOR)));

        // Toolbar drawing

        GuiStatusBar(toolbar_bounds, "Toolbar");

        if (GuiButton(sort_button_bounds, "Sort"))
        {
            UpdateTexture(img_tex, img.sorted.data);
        }

        if (GuiButton(reset_button_bounds, "Reset"))
        {
            UpdateTexture(img_tex, img.original.data);
        }

        if (GuiButton(mask_button_bounds, "Show Mask"))
        {
            UpdateTexture(img_tex, img.mask.data);
        }

        if (GuiButton(recalculate_button_bounds, "Recalculate"))
        {
            if (loaded)
            {
                UnloadImage(img.mask);
                UnloadImage(img.sorted);
            }

            loaded = false;

            handleImage(img, lumin_floor, lumin_ceil, invert_range, vertical);

            loaded = true;
        }

        GuiToggle(sliders_button_bounds, "Config", &draw_sliders);

        // Image draw

        DrawTexture(img_tex, img.img_x, img.img_y, WHITE);

        // File opener

        if (fileDialogState.windowActive)
            GuiLock();

        if (GuiButton(open_image_button_bounds, "Open File"))
        {
            fileDialogState.windowActive = true;
        }

        GuiUnlock();

        GuiWindowFileDialog(&fileDialogState);

        // Slider menu

        if (draw_sliders)
        {
            GuiPanel(slider_bounds, "Slider Panel");

            GuiSlider((Rectangle){slider_bounds.x + 80.0f, slider_bounds.y + 40.0f, 170, 20},
                      "Luminocity\nFloor", TextFormat("%2.2f", lumin_floor), &lumin_floor, 0.0f, 1.0f);

            GuiSlider((Rectangle){slider_bounds.x + 80.0f, slider_bounds.y + 100.0f, 170, 20},
                      "Luminocity\nCeiling", TextFormat("%2.2f", lumin_ceil), &lumin_ceil, 0.0f, 1.0f);

            GuiToggle((Rectangle){slider_bounds.x + 70.0f, slider_bounds.y + 160.0f, 90.0f, 20}, "Invert Range", &invert_range);

            if (dropdown_edit_mode)
                GuiLock();

            if (GuiDropdownBox(dropdown_bounds, dropdown_options, &selected, dropdown_edit_mode))
                dropdown_edit_mode = !dropdown_edit_mode;

            vertical = selected;

            GuiUnlock();
        }

        EndDrawing();
    }

    CloseWindow();

    return 0;
}
