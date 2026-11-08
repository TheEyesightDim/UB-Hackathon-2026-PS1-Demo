#include "texture.h"
#include "libgpu.h"
#include "sys/types.h"

void load_texture(u_long * tim_buf, TIM_IMAGE * tparams) {
	OpenTIM(tim_buf);
	ReadTIM(tparams);

	LoadImage(tparams->prect, (u_long *)tparams->paddr);
	// Wait for this transfer to finish
	DrawSync(0);

	// If the CLUT is present with the texture,
	// Then load that to its area too
	if (tparams->mode & 0x8) {
		LoadImage(tparams->crect, tparams->caddr);
		// Wait for this transfer to finish, too
		DrawSync(0);
	}
}
