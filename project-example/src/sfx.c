#include "sfx.h"
#include <libspu.h>
#include "loader.h"
#include <stdio.h>

/*
 * SPU RAM layout for SFX samples.
 *
 * Samples are allocated with SpuMalloc (guaranteed 64-byte aligned in
 * SPU RAM). The whole VAG file is DMA'd in one transfer: the 48-byte
 * header lands in SPU RAM harmlessly (the SPU only reads from the
 * voice's waveform start address), so source, destination, and size
 * are all 64-byte aligned — required by the SPU DMA, which truncates
 * on unaligned transfers.
 *
 * The first 4 KB of SPU RAM is reserved for SPU capture buffers.
 */
#define SPU_MALLOC_RECSIZ 8
#define SFX_SPU_MALLOC_MAX SFX_COUNT
#define VAG_HEADER_SIZE   48  /* psxavenc standard VAG header; ADPCM data starts here */

/*
 * Pitch formula (libover47.pdf): pitch = (sample_rate << 12) / 44100
 *   44100 Hz -> 0x1000
 *   22050 Hz -> 0x0800
 *   11025 Hz -> 0x0400
 */
#define SFX_PITCH_NORMAL  0x1000

/* Random pitch spread applied by play_sfx: ±10% of the sample's default
 * pitch (0x1000 ± 0x191), so repeated hits don't sound machine-gunned. */
#define SFX_PITCH_JITTER  0x191

/* Tiny LCG (no stdlib on the PSX); returns 0..0xFFFF. */
static u_long sfx_rng = 0x2545F491UL;
static u_short sfx_rand(void) {
    sfx_rng = sfx_rng * 1103515245UL + 12345UL;
    return (u_short)(sfx_rng >> 16);
}

typedef struct {
    u_long  spu_addr;   /* SpuMalloc'd base in SPU RAM (0 = not loaded) */
    u_long  wdsa;       /* waveform data start = spu_addr + VAG_HEADER_SIZE */
    int     voice;      /* SPU voice number (2-23; 0-1 are CD-DA) */
    u_short pitch;      /* default pitch for this sample */
    u_short volume;     /* L/R volume 0..0x3FFF (0x3FFF = full) */
    char    cd_name[32];/* CD file name */
} SFX_Sample;

static SFX_Sample sfx_samples[SFX_COUNT] = {
    { 0, 0, 2, SFX_PITCH_NORMAL, 0x3FFF, "\\DATA\\BOOM.VAG;1" },
    { 0, 0, 3, SFX_PITCH_NORMAL, 0x3FFF, "\\DATA\\HURT.VAG;1" },
    { 0, 0, 4, SFX_PITCH_NORMAL, 0x3FFF, "\\DATA\\DING.VAG;1" },
    { 0, 0, 5, SFX_PITCH_NORMAL, 0x0FFF, "\\DATA\\SHOOT.VAG;1" }, /* 25% vol */
};

/* 64-byte aligned DMA source buffer (load_buffer is only 4-byte aligned) */
static u_long sfx_dma_buffer[(1 << 16) / sizeof(u_long)] __attribute__((aligned(64)));

static char spu_malloc_rec[SPU_MALLOC_RECSIZ * (SFX_SPU_MALLOC_MAX + 1)];

/* byte-wise copy (no string.h in freestanding build) */
static void copy_bytes(u_long *dst, const u_long *src, u_long n) {
    while (n >= sizeof(u_long)) {
        *dst++ = *src++;
        n -= sizeof(u_long);
    }
}

void init_sfx(void) {
    if (SpuInitMalloc(SFX_SPU_MALLOC_MAX, spu_malloc_rec) <= 0) {
        printf("sfx: SpuInitMalloc failed\n");
        return;
    }

    for (int i = 0; i < SFX_COUNT; i++) {
        SFX_Sample *s = &sfx_samples[i];
        u_long fsize = load_file(s->cd_name);
        if (fsize < VAG_HEADER_SIZE) {
            printf("sfx: failed to load %s\n", s->cd_name);
            continue;
        }
        /* 64-byte aligned allocation in SPU RAM */
        s->spu_addr = SpuMalloc(fsize);
        if (!s->spu_addr) {
            printf("sfx: SpuMalloc failed for %s (%lu bytes)\n", s->cd_name, fsize);
            continue;
        }
        s->wdsa = s->spu_addr + VAG_HEADER_SIZE;
        /* copy to a 64-byte aligned source, then DMA the whole file */
        copy_bytes(sfx_dma_buffer, load_buffer, fsize);
        SpuSetTransferStartAddr(s->spu_addr);
        SpuWrite((unsigned char *)sfx_dma_buffer, fsize);
        SpuIsTransferCompleted(SPU_TRANSFER_WAIT);
        printf("sfx: loaded %s (%lu bytes) at SPU 0x%lx, wdsa 0x%lx\n",
               s->cd_name, fsize, s->spu_addr, s->wdsa);
    }
}

/*
 * Configure a voice for a one-shot ADPCM sample:
 * immediate attack, full sustain, gentle release.
 * Matches the working config from vagsfx.c (Schnappy).
 */
static void setup_voice(SFX_Sample *s, u_short pitch) {
    SpuVoiceAttr va;
    va.voice       = SPU_KEYCH(s->voice);
    va.mask        = SPU_VOICE_VOLL | SPU_VOICE_VOLR
                   | SPU_VOICE_PITCH | SPU_VOICE_WDSA
                   | SPU_VOICE_ADSR_AMODE | SPU_VOICE_ADSR_SMODE
                   | SPU_VOICE_ADSR_RMODE
                   | SPU_VOICE_ADSR_AR | SPU_VOICE_ADSR_DR
                   | SPU_VOICE_ADSR_SR | SPU_VOICE_ADSR_RR
                   | SPU_VOICE_ADSR_SL;
    va.volume.left  = s->volume;
    va.volume.right = s->volume;
    va.pitch        = pitch;
    va.addr         = s->wdsa;
    va.a_mode       = SPU_VOICE_LINEARIncN;
    va.s_mode       = SPU_VOICE_LINEARIncN;
    va.r_mode       = SPU_VOICE_LINEARDecN;
    va.ar           = 0x0;
    va.dr           = 0x0;
    va.sr           = 0x0;
    va.rr           = 0x0;
    va.sl           = 0xF;
    SpuSetVoiceAttr(&va);
}

void play_sfx_pitch(SFX_ID id, u_short pitch) {
    if (id >= SFX_COUNT) return;
    SFX_Sample *s = &sfx_samples[id];
    if (!s->spu_addr) return;

    /* restart if already playing */
    SpuSetKey(SPU_OFF, SPU_KEYCH(s->voice));
    setup_voice(s, pitch);
    SpuSetKey(SPU_ON, SPU_KEYCH(s->voice));
}

void play_sfx(SFX_ID id) {
    if (id >= SFX_COUNT) return;
    SFX_Sample *s = &sfx_samples[id];
    u_short p = s->pitch;
    p += (u_short)(sfx_rand() % (2 * SFX_PITCH_JITTER + 1)) - SFX_PITCH_JITTER;
    play_sfx_pitch(id, p);
}

void stop_sfx(SFX_ID id) {
    if (id >= SFX_COUNT) return;
    SpuSetKey(SPU_OFF, SPU_KEYCH(sfx_samples[id].voice));
}

void stop_all_sfx(void) {
    for (int i = 0; i < SFX_COUNT; i++)
        SpuSetKey(SPU_OFF, SPU_KEYCH(sfx_samples[i].voice));
}
