#include "texture.h"
#include "libgpu.h"
#include "sys/types.h"
#include "display.h"

#define TEX_PRIMBUF_SIZE 4096
 static u_char tex_primbuf[2][TEX_PRIMBUF_SIZE];
 static u_char * tex_prim_next;
 static u_char * tex_prim_end;

void begin_image_draw() {
	tex_prim_next = tex_primbuf[db_idx];
	tex_prim_end = tex_prim_next + TEX_PRIMBUF_SIZE;
}
 
void load_texture(u_long * tim_buf, TIM_IMAGE * tim) {
	OpenTIM(tim_buf);
	ReadTIM(tim);

	LoadImage(tim->prect, (u_long *)tim->paddr);
	// Wait for this transfer to finish
	DrawSync(0);

	// If the CLUT is present with the texture,
	// Then load that to its area too
	if (tim->mode & 0x8) {
		LoadImage(tim->crect, tim->caddr);
		// Wait for this transfer to finish, too
		DrawSync(0);
	}
}

void init_image(Image * image, TIM_IMAGE * tim) {
    int mode = tim->mode & 3;          /* 0 = 4bpp, 1 = 8bpp, 2 = 16bpp */
    int per  = 4 >> mode;              /* texels per VRAM pixel: 4, 2, 1 */

    image->tpage_idx= getTPage(mode, 0, tim->prect->x, tim->prect->y);
    image->clut_idx  = (tim->mode & 8) ? getClut(tim->crect->x, tim->crect->y) : 0;
    image->u = (tim->prect->x & 63) * per;   /* offset from the page's left edge */
    image->v = tim->prect->y & 255;          /* offset from the page's top edge */
    image->w = tim->prect->w * per;
    image->h = tim->prect->h;	
}

void draw_image(Image * image, int x, int y, bool is_bg) {
	
	
	// You SHOULD check this against tex_prim_end though to not overflow
	SPRT * s = (SPRT *)tex_prim_next;
	tex_prim_next += sizeof(SPRT);
    DR_TPAGE * t = (DR_TPAGE *)tex_prim_next; tex_prim_next+= sizeof(DR_TPAGE);

    setSprt(s);
    setShadeTex(s, 1); // show the texture's own colours
    setXY0(s, x, y);
    setWH(s, image->w, image->h);
    setUV0(s, image->u, image->v);
    s->clut = image->clut_idx;
	// sprites have no tpage field, so select it first
	setDrawTPage(t, 0, 0, image->tpage_idx); 

    addPrim(&db[db_idx].ot[(OTSIZE - 1)* is_bg], s);                    
    addPrim(&db[db_idx].ot[(OTSIZE - 1)* is_bg], t);         
}
