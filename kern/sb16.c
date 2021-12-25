#include <xv6/defs.h>
#include <xv6/x86.h>

/* From QEMU info qtree:

 dev: sb16, id ""
            audiodev = "pa"
            version = 1029 (0x405)
            iobase = 544 (0x220)
            irq = 5 (0x5)
            dma = 1 (0x1)
            dma16 = 5 (0x5)
            isa irq 5
*/

/* IRQ, base address and DMA channels */
#define SB_IRQ          5
#define SB_BASE_ADDR    0x220
#define SB_DMA_8        1
#define SB_DMA_16       5

/* IO ports for soundblaster */
#define DSP_RESET       0x6 + SB_BASE_ADDR
#define DSP_READ        0xA + SB_BASE_ADDR
#define DSP_WRITE       0xC + SB_BASE_ADDR
#define DSP_COMMAND     0xC + SB_BASE_ADDR
#define DSP_STATUS      0xC + SB_BASE_ADDR
#define DSP_DATA_AVL    0xE + SB_BASE_ADDR
#define DSP_DATA16_AVL  0xF + SB_BASE_ADDR
#define MIXER_REG       0x4 + SB_BASE_ADDR
#define MIXER_DATA      0x5 + SB_BASE_ADDR
#define OPL3_LEFT       0x0 + SB_BASE_ADDR
#define OPL3_RIGHT      0x2 + SB_BASE_ADDR
#define OPL3_BOTH       0x8 + SB_BASE_ADDR

#define SB_TIMEOUT              32000 // timeout count

/* DSP Commands */
#define DSP_INPUT_RATE          0x42  /* set input sample rate */
#define DSP_OUTPUT_RATE         0x41  /* set output sample rate */
#define DSP_CMD_SPKON           0xD1  /* set speaker on */
#define DSP_CMD_SPKOFF          0xD3  /* set speaker off */
#define DSP_CMD_DMA8HALT        0xD0  /* halt DMA 8-bit operation */  
#define DSP_CMD_DMA8CONT        0xD4  /* continue DMA 8-bit operation */
#define DSP_CMD_DMA16HALT       0xD5  /* halt DMA 16-bit operation */
#define DSP_CMD_DMA16CONT       0xD6  /* continue DMA 16-bit operation */
#define DSP_GET_VERSION         0xE1  /* get version number of DSP */
#define DSP_CMD_8BITAUTO_IN     0xCE  /* 8 bit auto-initialized input */
#define DSP_CMD_8BITAUTO_OUT    0xC6  /* 8 bit auto-initialized output */
#define DSP_CMD_16BITAUTO_IN    0xBE  /* 16 bit auto-initialized input */
#define DSP_CMD_16BITAUTO_OUT   0xB6  /* 16 bit auto-initialized output */
#define DSP_CMD_IRQREQ8         0xF2  /* Interrupt request 8 bit        */
#define DSP_CMD_IRQREQ16        0xF3  /* Interrupt request 16 bit        */

/* MIXER commands */
#define MIXER_RESET             0x00  /* Reset */
#define MIXER_DAC_LEVEL         0x04  /* Used for detection only */
#define MIXER_MASTER_LEFT       0x30  /* Master volume left */
#define MIXER_MASTER_RIGHT      0x31  /* Master volume right */
#define MIXER_DAC_LEFT          0x32  /* Dac level left */
#define MIXER_DAC_RIGHT         0x33  /* Dac level right */
#define MIXER_FM_LEFT           0x34  /* Fm level left */
#define MIXER_FM_RIGHT          0x35  /* Fm level right */
#define MIXER_CD_LEFT           0x36  /* Cd audio level left */
#define MIXER_CD_RIGHT          0x37  /* Cd audio level right */
#define MIXER_LINE_LEFT         0x38  /* Line in level left */
#define MIXER_LINE_RIGHT        0x39  /* Line in level right */
#define MIXER_MIC_LEVEL         0x3A  /* Microphone level */
#define MIXER_PC_LEVEL          0x3B  /* Pc speaker level */
#define MIXER_OUTPUT_CTRL       0x3C  /* Output control */
#define MIXER_IN_LEFT           0x3D  /* Input control left */
#define MIXER_IN_RIGHT          0x3E  /* Input control right */
#define MIXER_GAIN_IN_LEFT      0x3F  /* Input gain control left */
#define MIXER_GAIN_IN_RIGHT     0x40  /* Input gain control right */
#define MIXER_GAIN_OUT_LEFT     0x41  /* Output gain control left */
#define MIXER_GAIN_OUT_RIGHT    0x42  /* Output gain control rigth */
#define MIXER_AGC               0x43  /* Automatic gain control */
#define MIXER_TREBLE_LEFT       0x44  /* Treble left */
#define MIXER_TREBLE_RIGHT      0x45  /* Treble right */
#define MIXER_BASS_LEFT         0x46  /* Bass left */
#define MIXER_BASS_RIGHT        0x47  /* Bass right */
#define MIXER_SET_IRQ           0x80  /* Set irq number */
#define MIXER_SET_DMA           0x81  /* Set DMA channels */
#define MIXER_IRQ_STATUS        0x82  /* Irq status */

//XXX static unsigned int DspFragmentSize;

int sb16_inb(int port) {
  return inb(port);
}

void sb16_outb(int port, int value) {
  outb(port, value);
}

int drv_reset(void) {
  int i;
  
  cprintf("drv_reset()\n");
  
  sb16_outb(DSP_RESET, 1);
  for (i = 0; i < 1000; i++); // wait a while
  sb16_outb(DSP_RESET, 0);
  
  for (i = 0; i < 1000 && !(sb16_inb(DSP_DATA_AVL) & 0x80); i++);
  
  if (sb16_inb(DSP_READ) != 0xAA) return -1; /* No SoundBlaster */
  
  return 0;
}

int dsp_command(int value) {
  int i;
  
  for (i = 0; i < SB_TIMEOUT; i++) {
    if((sb16_inb(DSP_STATUS) & 0x80) == 0) {
      sb16_outb(DSP_COMMAND, value);
      return 0;
    }
  }
  
  cprintf("sb16: SoundBlaster: DSP Command(%x) timeout.\n", value);
  return -1;
}

int mixer_set(int reg, int data) {
  int i;
  
  sb16_outb(MIXER_REG, reg);
  for(i = 0; i < 100; i++);
  sb16_outb(MIXER_DATA, data);
  
  return 0;
}

int mixer_get(int reg) {
  int i;
  
  sb16_outb(MIXER_REG, reg);
  for(i = 0; i < 100; i++);
  return sb16_inb(MIXER_DATA) & 0xff;
}

int drv_init_hw(void) {
  int i;
  int DspVersion[2];
  cprintf("drv_init_hw()\n");
  
  if (drv_reset () != 0) {
    cprintf("sb16: No SoundBlaster card detected.\n");
    return -1;
  }
  
  DspVersion[0] = DspVersion[1] = 0;
  dsp_command(DSP_GET_VERSION);   /* Get DSP version bytes */
  
  for(i = 1000; i; i--) {
    if(sb16_inb(DSP_DATA_AVL) & 0x80) {
      if(DspVersion[0] == 0) {
        DspVersion[0] = sb16_inb(DSP_READ);
      } else {
        DspVersion[1] = sb16_inb(DSP_READ);
        break;
      }
    }
  }

  if(DspVersion[0] < 4) {
    cprintf("sb16: No SoundBlaster 16 compatible card detected.\n");
    return -1;
  }
  
  cprintf("sb16: SoundBlaster DSP version %d.%d detected.\n", DspVersion[0], DspVersion[1]);

  /* set SB to use our IRQ and DMA channels */
  mixer_set(MIXER_SET_IRQ, (1 << (SB_IRQ / 2 - 1)));
  mixer_set(MIXER_SET_DMA, (1 << SB_DMA_8 | 1 << SB_DMA_16));
  
  //XXX DspFragmentSize = sub_dev[AUDIO].DmaSize / sub_dev[AUDIO].NrOfDmaFragments;
    
  return 0;
}
