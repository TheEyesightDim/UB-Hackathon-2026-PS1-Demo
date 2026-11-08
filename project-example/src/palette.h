#ifndef _PALETTE_H
#define _PALETTE_H

#include "3d.h"

/*
 * Named color palettes. Colors cycle by face index, wrapping at num_colors.
 *
 * Face order for SHAPE_DIAMOND:
 *   0-3: upper faces (back, right, front, left)
 *   4-7: lower faces (back, right, front, left)
 */

/* Diamond: white top, icy blue lower */
static const ColorPalette palette_diamond = {
    {
        {255,255,255}, {210,235,255}, {235,248,255}, {190,225,255},
        {160,205,245}, {185,222,252}, {150,198,240}, {175,215,250},
    },
    8
};

/* Emerald: green gem */
static const ColorPalette palette_emerald = {
    {
        {120,255,180}, { 60,220,140}, { 90,240,160}, { 40,200,120},
        { 20,160, 90}, { 30,180,105}, { 10,140, 75}, { 25,170, 95},
    },
    8
};

/* Ruby: red gem */
static const ColorPalette palette_ruby = {
    {
        {255,120,140}, {230, 60, 90}, {245, 90,115}, {210, 40, 70},
        {180, 20, 50}, {195, 30, 60}, {160, 10, 40}, {185, 25, 55},
    },
    8
};

/* Sapphire: blue gem */
static const ColorPalette palette_sapphire = {
    {
        {130,170,255}, { 70,110,240}, {100,140,250}, { 50, 90,220},
        { 30, 60,180}, { 40, 75,200}, { 20, 45,150}, { 35, 65,170},
    },
    8
};

/* Gold: metallic yellow */
static const ColorPalette palette_gold = {
    {
        {255,230,120}, {230,190, 80}, {245,215,100}, {215,170, 60},
        {190,140, 40}, {205,155, 50}, {170,120, 30}, {195,145, 45},
    },
    8
};

/* Neon: high-contrast arcade */
static const ColorPalette palette_neon = {
    {
        {  0,255,255}, {255,  0,255}, {255,255,  0}, {  0,255,  0},
        {255,  0,  0}, {  0,128,255}, {255,128,  0}, {128,  0,255},
    },
    8
};

/* Ice: pale cyan/white */
static const ColorPalette palette_ice = {
    {
        {240,255,255}, {210,240,250}, {225,248,255}, {200,235,248},
        {180,225,245}, {195,232,250}, {170,218,240}, {185,228,246},
    },
    8
};

/* Steel: gray metal */
static const ColorPalette palette_steel = {
    {
        {200,205,215}, {160,165,175}, {180,185,195}, {140,145,155},
        {120,125,135}, {135,140,150}, {105,110,120}, {125,130,140},
    },
    8
};

#endif /* _PALETTE_H */
