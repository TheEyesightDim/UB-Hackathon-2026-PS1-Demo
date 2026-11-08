#include "3d.h"
#include <stdlib.h>

/* =====================================================================
 * SHAPE GEOMETRY
 *
 * All vertex data sourced from three.js (MIT license):
 *   - Cube: standard 8-corner cube
 *   - Octahedron: three.js OctahedronGeometry
 *   - Icosahedron: three.js IcosahedronGeometry (t = golden ratio)
 *   - Pyramid: 4-tri sides + 1 quad base
 *   - Cone: 8-sided, 8 tri sides + 8 tri base fan
 *   - Torus: 6x8 segment grid, R=60, r=25
 * ===================================================================== */

/* ---- CUBE: 8 vertices, 6 quad faces ---- */

static SVECTOR cube_vertices[] = {
    {-98, -98, -98, 0}, { 98, -98, -98, 0},
    { 98,  98, -98, 0}, {-98,  98, -98, 0},
    {-98, -98,  98, 0}, { 98, -98,  98, 0},
    { 98,  98,  98, 0}, {-98,  98,  98, 0},
};

static int cube_indices[] = {
    0, 1, 2, 3,   // back  (z-)
    1, 5, 6, 2,   // right (x+)
    5, 4, 7, 6,   // front (z+)
    4, 0, 3, 7,   // left  (x-)
    4, 5, 1, 0,   // bottom(y-)
    6, 7, 3, 2,   // top   (y+)
};

/* ---- OCTAHEDRON: 6 vertices, 8 triangle faces (three.js) ---- */

static SVECTOR octa_vertices[] = {
    { 98,   0,   0, 0}, {-98,   0,   0, 0},
    {  0,  98,   0, 0}, {  0, -98,   0, 0},
    {  0,   0,  98, 0}, {  0,   0, -98, 0},
};

static int octa_indices[] = {
    0, 2, 4,   0, 4, 3,   0, 3, 5,   0, 5, 2,
    1, 2, 5,   1, 5, 3,   1, 3, 4,   1, 4, 2,
};

/* ---- ICOSAHEDRON: 12 vertices, 20 triangle faces (three.js) ----
 * t = golden ratio ≈ 1.618, scaled to fit ~98 unit radius
 * three.js uses t=(1+sqrt(5))/2 with unit vertices; we scale by ~35
 * so the circumscribed radius ≈ 98 (matches cube half-size)
 * vertex = (x, y, z) * 35 / |v| * 98 ... simplified: scale factor = 35
 * (three.js icosahedron has circumradius = sqrt(1+t^2) ≈ 1.902,
 *  so scale = 98/1.902 ≈ 51.5 → use 51)
 */

static SVECTOR icosa_vertices[] = {
    {-51,  82,   0, 0}, { 51,  82,   0, 0},
    {-51, -82,   0, 0}, { 51, -82,   0, 0},
    {  0, -51,  82, 0}, {  0,  51,  82, 0},
    {  0, -51, -82, 0}, {  0,  51, -82, 0},
    { 82,   0, -51, 0}, { 82,   0,  51, 0},
    {-82,   0, -51, 0}, {-82,   0,  51, 0},
};

static int icosa_indices[] = {
    0, 11, 5,   0,  5, 1,   0,  1, 7,   0,  7, 10,  0, 10, 11,
    1,  5, 9,   5, 11, 4,   11, 10, 2,  10,  7, 6,   7,  1, 8,
    3,  9, 4,   3,  4, 2,   3,  2, 6,   3,  6, 8,   3,  8, 9,
    4,  9, 5,   2,  4, 11,  6,  2, 10,  8,  6, 7,   9,  8, 1,
};

/* ---- PYRAMID: 5 vertices, 4 tri sides + 1 quad base ---- */

static SVECTOR pyramid_vertices[] = {
    {  0,   0,  98, 0},  // 0: apex
    {-98,  98, -98, 0},  // 1: base back-left
    { 98,  98, -98, 0},  // 2: base back-right
    { 98, -98, -98, 0},  // 3: base front-right
    {-98, -98, -98, 0},  // 4: base front-left
};

static int pyramid_indices[] = {
    0, 1, 2,   // back side (tri)
    0, 2, 3,   // right side (tri)
    0, 3, 4,   // front side (tri)
    0, 4, 1,   // left side (tri)
    1, 4, 3, 2, // base (quad)
};

/* ---- CONE (8-sided): 10 vertices, 8 tri sides + 8 tri base ---- */

static SVECTOR cone_vertices[] = {
    {  0,   0,  98, 0},  // 0: apex
    { 69,   0, -98, 0},  // 1: base (0°)
    { 49,  49, -98, 0},  // 2: base (45°)
    {  0,  69, -98, 0},  // 3: base (90°)
    {-49,  49, -98, 0},  // 4: base (135°)
    {-69,   0, -98, 0},  // 5: base (180°)
    {-49, -49, -98, 0},  // 6: base (225°)
    {  0, -69, -98, 0},  // 7: base (270°)
    { 49, -49, -98, 0},  // 8: base (315°)
    {  0,   0, -98, 0},  // 9: base center
};

static int cone_indices[] = {
    // 8 side triangles (apex + two adjacent base verts)
    0, 1, 2,   0, 2, 3,   0, 3, 4,   0, 4, 5,
    0, 5, 6,   0, 6, 7,   0, 7, 8,   0, 8, 1,
    // 8 base triangles (fan from center vertex 9)
    9, 1, 2,   9, 2, 3,   9, 3, 4,   9, 4, 5,
    9, 5, 6,   9, 6, 7,   9, 7, 8,   9, 8, 1,
};

/* ---- TORUS: 63 vertices, 48 quad faces (R=60, r=25, 6x8) ---- */

static SVECTOR torus_vertices[] = {
    {  85,   0,   0, 0}, {  60,  60,   0, 0}, {   0,  85,   0, 0}, { -60,  60,   0, 0},
    { -85,   0,   0, 0}, { -60, -60,   0, 0}, {   0, -85,   0, 0}, {  60, -60,   0, 0},
    {  85,   0,   0, 0}, {  72,   0,  22, 0}, {  51,  51,  22, 0}, {   0,  72,  22, 0},
    { -51,  51,  22, 0}, { -72,   0,  22, 0}, { -51, -51,  22, 0}, {   0, -72,  22, 0},
    {  51, -51,  22, 0}, {  72,   0,  22, 0}, {  48,   0,  22, 0}, {  34,  34,  22, 0},
    {   0,  48,  22, 0}, { -34,  34,  22, 0}, { -48,   0,  22, 0}, { -34, -34,  22, 0},
    {   0, -48,  22, 0}, {  34, -34,  22, 0}, {  48,   0,  22, 0}, {  35,   0,   0, 0},
    {  25,  25,   0, 0}, {   0,  35,   0, 0}, { -25,  25,   0, 0}, { -35,   0,   0, 0},
    { -25, -25,   0, 0}, {   0, -35,   0, 0}, {  25, -25,   0, 0}, {  35,   0,   0, 0},
    {  47,   0, -22, 0}, {  34,  34, -22, 0}, {   0,  47, -22, 0}, { -34,  34, -22, 0},
    { -47,   0, -22, 0}, { -34, -34, -22, 0}, {   0, -47, -22, 0}, {  34, -34, -22, 0},
    {  47,   0, -22, 0}, {  72,   0, -22, 0}, {  51,  51, -22, 0}, {   0,  72, -22, 0},
    { -51,  51, -22, 0}, { -72,   0, -22, 0}, { -51, -51, -22, 0}, {   0, -72, -22, 0},
    {  51, -51, -22, 0}, {  72,   0, -22, 0}, {  85,   0,   0, 0}, {  60,  60,   0, 0},
    {   0,  85,   0, 0}, { -60,  60,   0, 0}, { -85,   0,   0, 0}, { -60, -60,   0, 0},
    {   0, -85,   0, 0}, {  60, -60,   0, 0}, {  85,   0,   0, 0},
};

static int torus_indices[] = {
     9,  0, 10,  1,  10,  1, 11,  2,  11,  2, 12,  3,  12,  3, 13,  4,
    13,  4, 14,  5,  14,  5, 15,  6,  15,  6, 16,  7,  16,  7, 17,  8,
    18,  9, 19, 10,  19, 10, 20, 11,  20, 11, 21, 12,  21, 12, 22, 13,
    22, 13, 23, 14,  23, 14, 24, 15,  24, 15, 25, 16,  25, 16, 26, 17,
    27, 18, 28, 19,  28, 19, 29, 20,  29, 20, 30, 21,  30, 21, 31, 22,
    31, 22, 32, 23,  32, 23, 33, 24,  33, 24, 34, 25,  34, 25, 35, 26,
    36, 27, 37, 28,  37, 28, 38, 29,  38, 29, 39, 30,  39, 30, 40, 31,
    40, 31, 41, 32,  41, 32, 42, 33,  42, 33, 43, 34,  43, 34, 44, 35,
    45, 36, 46, 37,  46, 37, 47, 38,  47, 38, 48, 39,  48, 39, 49, 40,
    49, 40, 50, 41,  50, 41, 51, 42,  51, 42, 52, 43,  52, 43, 53, 44,
    54, 45, 55, 46,  55, 46, 56, 47,  56, 47, 57, 48,  57, 48, 58, 49,
    58, 49, 59, 50,  59, 50, 60, 51,  60, 51, 61, 52,  61, 52, 62, 53,
};

/* ---- DIAMOND (brilliant-cut gem): 6 vertices, 8 tri faces ----
 * Two square pyramids joined at a girdle.
 * Girdle radius 70 → classic gem proportion (height 196, width 140).
 */

static SVECTOR diamond_vertices[] = {
    {  0, -98,   0, 0},  // 0: top point
    {  0,  98,   0, 0},  // 1: bottom point
    {-70,   0, -70, 0},  // 2: girdle back-left
    { 70,   0, -70, 0},  // 3: girdle back-right
    { 70,   0,  70, 0},  // 4: girdle front-right
    {-70,   0,  70, 0},  // 5: girdle front-left
};

static int diamond_indices[] = {
    0, 2, 3,   // upper back
    0, 3, 4,   // upper right
    0, 4, 5,   // upper front
    0, 5, 2,   // upper left
    1, 3, 2,   // lower back
    1, 4, 3,   // lower right
    1, 5, 4,   // lower front
    1, 2, 5,   // lower left
};

/* =====================================================================
 * SHAPE TABLE
 * ===================================================================== */

static unsigned char cube_ft[]     = {4,4,4,4,4,4};
static unsigned char octa_ft[]     = {3,3,3,3,3,3,3,3};
static unsigned char icosa_ft[]    = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};
static unsigned char pyramid_ft[]  = {3,3,3,3,4};
static unsigned char cone_ft[]     = {3,3,3,3,3,3,3,3,3,3,3,3,3,3,3,3};
static unsigned char torus_ft[]    = {4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                      4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,4,
                                      4,4,4,4,4,4,4,4,4,4,4,4};
static unsigned char diamond_ft[]  = {3,3,3,3,3,3,3,3};

static Shape shapes[SHAPE_COUNT] = {
    [SHAPE_CUBE]        = { cube_vertices,    cube_indices,    cube_ft,    6  },
    [SHAPE_OCTAHEDRON]  = { octa_vertices,    octa_indices,    octa_ft,    8  },
    [SHAPE_ICOSAHEDRON] = { icosa_vertices,   icosa_indices,   icosa_ft,  20  },
    [SHAPE_PYRAMID]     = { pyramid_vertices, pyramid_indices, pyramid_ft, 5  },
    [SHAPE_CONE]        = { cone_vertices,    cone_indices,    cone_ft,   16  },
    [SHAPE_TORUS]       = { torus_vertices,   torus_indices,   torus_ft,  48  },
    [SHAPE_DIAMOND]     = { diamond_vertices, diamond_indices, diamond_ft, 8  },
};

/* =====================================================================
 * API
 * ===================================================================== */

void init_3d(int screen_w, int screen_h) {
    InitGeom();
    SetGeomOffset(0, 0);
    SetGeomScreen(SCREEN_Z);
}

const Shape *get_shape(ShapeType type) {
    if (type >= SHAPE_COUNT) return &shapes[SHAPE_CUBE];
    return &shapes[type];
}

void init_3d_object(Object3D *obj, ShapeType shape,
                    int x, int y) {
    int i;
    obj->shape = shape;
    obj->rotation.vx = 0;
    obj->rotation.vy = 0;
    obj->rotation.vz = 0;
    obj->rotation.pad = 0;
    obj->translation.vx = x;
    obj->translation.vy = y;
    obj->translation.vz = SCREEN_Z;
    obj->scale.vx = 0x08000;  // 50% default
    obj->scale.vy = 0x08000;
    obj->scale.vz = 0x08000;
    obj->scale.pad = 0;
    // Default: flat white palette (wraps at 1)
    for (i = 0; i < MAX_FACES; i++) {
        obj->palette.colors[i].r = 255;
        obj->palette.colors[i].g = 255;
        obj->palette.colors[i].b = 255;
    }
    obj->palette.num_colors = 1;
    obj->flash_frames = 0;
    obj->flash_hue = 0;
    obj->flash_mode = FLASH_RAINBOW;
}

void set_3d_palette(Object3D *obj, const ColorPalette *palette) {
    int i;
    for (i = 0; i < MAX_FACES; i++) {
        obj->palette.colors[i] = palette->colors[i];
    }
    obj->palette.num_colors = palette->num_colors;
}

/* =====================================================================
 * COLOR FLASH
 * ===================================================================== */

/* Hue (0-255) → full-brightness RGB, six-sector rainbow. */
static void hsv_to_rgb(int hue, Color *out) {
    int h = hue & 255;
    int sector = h / 43;        /* 0..5 */
    int t = (h % 43) * 6;       /* 0..252 */
    int a = 255, b = 255 - t, c = t;
    switch (sector) {
        case 0:  out->r = a; out->g = c; out->b = 0;  break;
        case 1:  out->r = b; out->g = a; out->b = 0;  break;
        case 2:  out->r = 0; out->g = a; out->b = c;  break;
        case 3:  out->r = 0; out->g = b; out->b = a;  break;
        case 4:  out->r = c; out->g = 0; out->b = a;  break;
        default: out->r = a; out->g = 0; out->b = b;  break;
    }
}

void flash_3d_object(Object3D *obj, FlashMode mode, int frames) {
    int i;
    if (obj->flash_frames <= 0) {
        /* Save the current palette so it can be restored on expiry. */
        for (i = 0; i < MAX_FACES; i++)
            obj->flash_saved.colors[i] = obj->palette.colors[i];
        obj->flash_saved.num_colors = obj->palette.num_colors;
    }
    obj->flash_frames = frames;
    obj->flash_hue = 0;
    obj->flash_mode = mode;
}

void update_3d_flash(Object3D *obj) {
    int i;
    Color c;
    if (obj->flash_frames <= 0) return;

    obj->flash_frames--;
    if (obj->flash_mode == FLASH_WHITE) {
        c.r = 255; c.g = 255; c.b = 255;
    } else {
        hsv_to_rgb(obj->flash_hue, &c);
        obj->flash_hue = (obj->flash_hue + 8) & 255;
    }
    for (i = 0; i < MAX_FACES; i++)
        obj->palette.colors[i] = c;
    obj->palette.num_colors = 1;

    if (obj->flash_frames <= 0) {
        for (i = 0; i < MAX_FACES; i++)
            obj->palette.colors[i] = obj->flash_saved.colors[i];
        obj->palette.num_colors = obj->flash_saved.num_colors;
    }
}

void draw_3d_object(Object3D *obj, u_long *ot, int otsize) {
    const Shape *s = get_shape(obj->shape);
    MATRIX transform;
    long p, otz, flg;
    int nclip;
    int i;
    int ci;   // palette color index (wraps)
    int idx = 0; // running index offset into s->indices

    // Build transform: scale (object space) → rotate → translate
    RotMatrix(&obj->rotation, &transform);
    ScaleMatrix(&transform, &obj->scale);
    TransMatrix(&transform, &obj->translation);
    // Load into GTE hardware (critical!)
    SetRotMatrix(&transform);
    SetTransMatrix(&transform);

    // Project each face
    for (i = 0; i < s->num_faces; i++) {
        ci = i % obj->palette.num_colors;
        if (s->face_types[i] == 4) {
            // Quad face
            nclip = RotAverageNclip4(
                &s->vertices[s->indices[idx + 0]],
                &s->vertices[s->indices[idx + 1]],
                &s->vertices[s->indices[idx + 2]],
                &s->vertices[s->indices[idx + 3]],
                (long *)&obj->polys[i].quad.x0,
                (long *)&obj->polys[i].quad.x1,
                (long *)&obj->polys[i].quad.x3,
                (long *)&obj->polys[i].quad.x2,
                &p, &otz, &flg);
            idx += 4;

            if (nclip <= 0) continue;
            if (otz <= 0 || otz >= otsize) continue;

            SetPolyF4(&obj->polys[i].quad);
            setRGB0(&obj->polys[i].quad,
                    obj->palette.colors[ci].r,
                    obj->palette.colors[ci].g,
                    obj->palette.colors[ci].b);
            AddPrim(&ot[otz], &obj->polys[i].quad);

        } else {
            // Triangle face
            nclip = RotAverageNclip3(
                &s->vertices[s->indices[idx + 0]],
                &s->vertices[s->indices[idx + 1]],
                &s->vertices[s->indices[idx + 2]],
                (long *)&obj->polys[i].tri.x0,
                (long *)&obj->polys[i].tri.x1,
                (long *)&obj->polys[i].tri.x2,
                &p, &otz, &flg);
            idx += 3;

            if (nclip <= 0) continue;
            if (otz <= 0 || otz >= otsize) continue;

            SetPolyF3(&obj->polys[i].tri);
            setRGB0(&obj->polys[i].tri,
                    obj->palette.colors[ci].r,
                    obj->palette.colors[ci].g,
                    obj->palette.colors[ci].b);
            AddPrim(&ot[otz], &obj->polys[i].tri);
        }
    }
}

void set_3d_scale(Object3D *obj, int scale) {
    long s = scale;
    obj->scale.vx = s;
    obj->scale.vy = s;
    obj->scale.vz = s;
}

void move_3d_object(Object3D *obj, int x, int y) {
    obj->translation.vx = x;
    obj->translation.vy = y;
}

void rotate_3d_object(Object3D *obj, int dx, int dy, int dz) {
    obj->rotation.vx += dx;
    obj->rotation.vy += dy;
    obj->rotation.vz += dz;
}
