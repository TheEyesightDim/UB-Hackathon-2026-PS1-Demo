#ifndef _3D_H
#define _3D_H

#include <libgte.h>
#include <libgpu.h>

/* Max faces per object (torus 6x8 = 48 quads) */
#define MAX_FACES 48

/* Default screen depth: z == SCREEN_Z gives 1:1 pixel mapping */
#define SCREEN_Z 512

/* Shape identifiers */
typedef enum {
    SHAPE_CUBE = 0,
    SHAPE_OCTAHEDRON,
    SHAPE_ICOSAHEDRON,
    SHAPE_PYRAMID,
    SHAPE_CONE,
    SHAPE_TORUS,
    SHAPE_DIAMOND,
    SHAPE_COUNT
} ShapeType;

/* Single 8-8-8 color */
typedef struct {
    unsigned char r, g, b;
} Color;

/* Per-face colors (indexed by face number, wraps at num_colors) */
typedef struct {
    Color colors[MAX_FACES];
    int   num_colors;
} ColorPalette;

/* A face can be a triangle or quad */
typedef union {
    POLY_F3 tri;
    POLY_F4 quad;
} FacePoly;

/* Static shape definition (vertices + indices + per-face type) */
typedef struct {
    SVECTOR *vertices;
    int     *indices;
    unsigned char *face_types; // 3=tri, 4=quad, one per face
    int            num_faces;
} Shape;

/* A drawable 3D object (shape + transform + per-face palette) */
typedef struct {
    ShapeType shape;
    SVECTOR   rotation;     /* fixed-point angles (1/256 degree) */
    VECTOR    translation;  /* world position */
    ColorPalette palette;   /* per-face colors (see palette.h) */
    VECTOR    scale;        /* fixed-point scale (0x10000 = 1.0) */
    FacePoly  polys[MAX_FACES];
} Object3D;

/*
 * Initialize the 3D system.
 * Sets up GTE geometry: origin offset, screen depth.
 * Call once at startup.
 */
void init_3d(int screen_w, int screen_h);

/*
 * Initialize a 3D object with a given shape.
 * Sets rotation to zero, translation to (x,y,SCREEN_Z), and a default
 * white palette. Use set_3d_palette to change colors.
 */
void init_3d_object(Object3D *obj, ShapeType shape,
                    int x, int y);

/*
 * Set the per-face color palette on an object.
 * Pass a pointer to a ColorPalette (see palette.h for named palettes).
 */
void set_3d_palette(Object3D *obj, const ColorPalette *palette);

/*
 * Draw a 3D object into the ordering table.
 * Builds the rotation+translation matrix, loads it into the GTE,
 * projects all faces, and inserts polys at their depth.
 */
void draw_3d_object(Object3D *obj, u_long *ot, int otsize);

/*
 * Set uniform scale on an object (fixed-point, 16.16).
 * 0x10000 (65536) = 1.0x, 0x08000 (32768) = 0.5x (default), 0x20000 (131072) = 2.0x.
 */
void set_3d_scale(Object3D *obj, int scale);

/*
 * Move an object to a screen position (z stays at SCREEN_Z).
 */
void move_3d_object(Object3D *obj, int x, int y);

/*
 * Add a rotation delta to an object (fixed-point, 1/256 degree).
 */
void rotate_3d_object(Object3D *obj, int dx, int dy, int dz);


/*
 * Get the static shape definition for a shape type.
 */
const Shape *get_shape(ShapeType type);

#endif /* _3D_H */
