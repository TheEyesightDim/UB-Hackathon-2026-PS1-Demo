#include "loader.h"

u_long load_buffer[(1 << 16)/sizeof(u_long)];

u_long load_file(char * filename) {
	// CdSetDebug(2);
	CdlFILE file;
	int cd_read_result;
	u_char cd_control_result[8] = {0};

	CdSearchFile(&file, filename);
	CdControl(CdlSetloc, (u_char *)&file.pos, cd_control_result);
	CdRead(len_to_sectors(file.size),
	       load_buffer,
	       CdlModeSpeed);

	// Block until read finishes
	return !CdReadSync(0, 0) ? file.size : 0;
}

