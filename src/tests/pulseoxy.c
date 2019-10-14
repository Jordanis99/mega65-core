
/*
  Simple "colour in the screen in your colour" game as
  demo of C65.

*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

#define POKE(a,v) *((uint8_t *)a) = (uint8_t)v
#define PEEK(a) ((uint8_t)(*((uint8_t *)a)))

unsigned short i;

struct dmagic_dmalist {

  unsigned char option_0b;
  unsigned char option_80;
  unsigned char source_mb;
  unsigned char option_81;
  unsigned char dest_mb;
  unsigned char end_of_options;

  // F018B format DMA request
  unsigned char command;
  unsigned int count;
  unsigned int source_addr;
  unsigned char source_bank;
  unsigned int dest_addr;
  unsigned char dest_bank;
  unsigned char sub_cmd;  // F018B subcmd
  unsigned int modulo;

};

struct dmagic_dmalist dmalist;
unsigned char dma_byte;

void do_dma(void) {

  // Now run DMA job (to and from anywhere, and list is in low 1MB)
  POKE(0xd702U,0);
  POKE(0xD704U,0x00);
  POKE(0xd701U,(((unsigned int)&dmalist) >> 8));
  POKE(0xd705U,((unsigned int)&dmalist)&0xff); // triggers enhanced DMA

}

void lpoke(long address, unsigned char value) {

  dmalist.option_0b=0x0b;
  dmalist.option_80=0x80;
  dmalist.source_mb=0;
  dmalist.option_81=0x81;
  dmalist.dest_mb=address>>20;
  dmalist.end_of_options=0x00;

  dma_byte = value;
  dmalist.command = 0x00; // copy
  dmalist.sub_cmd = 0;
  dmalist.modulo = 0;
  dmalist.count = 1;
  dmalist.source_addr = (unsigned int)&dma_byte;
  dmalist.source_bank = 0;
  dmalist.dest_addr = address&0xffff;
  dmalist.dest_bank = (address >> 16)&0xf;

  do_dma(); 
  return;

}

unsigned char lpeek(long address) {

  dmalist.option_0b=0x0b;
  dmalist.option_80=0x80;
  dmalist.source_mb=address>>20;
  dmalist.option_81=0x81;
  dmalist.dest_mb=0;
  dmalist.end_of_options=0x00;

  dmalist.command = 0x00; // copy
  dmalist.count = 1;
  dmalist.source_addr = address&0xffff;
  dmalist.source_bank = (address >> 16)&0x0f;
  dmalist.dest_addr = (unsigned int)&dma_byte;
  dmalist.source_bank = 0;
  dmalist.dest_addr = address&0xffff;
  dmalist.dest_bank = (address >> 16)&0x0f;
  // Make list work on either old or new DMAgic
  dmalist.sub_cmd = 0;
  dmalist.modulo = 0;
  
  do_dma();
  return dma_byte;

}

void lcopy(long source_address, long destination_address, unsigned int count) {

  dmalist.option_0b=0x0b;
  dmalist.option_80=0x80;
  dmalist.source_mb=source_address>>20;
  dmalist.option_81=0x81;
  dmalist.dest_mb=destination_address>>20;
  dmalist.end_of_options=0x00;

  dmalist.command = 0x00; // copy
  dmalist.count = count;
  dmalist.sub_cmd = 0;
  dmalist.modulo = 0;
  dmalist.source_addr = source_address&0xffff;
  dmalist.source_bank = (source_address >> 16)&0x0f;
  //  if (source_address >= 0xd000 && source_address < 0xe000)
  //    dmalist.source_bank|=0x80;  
  dmalist.dest_addr = destination_address&0xffff;
  dmalist.dest_bank = (destination_address >> 16)&0x0f;
  //  if (destination_address>=0xd000 && destination_address<0xe000)
  //    dmalist.dest_bank|=0x80;

  do_dma();
  return;

}

void lfill(long destination_address, unsigned char value, unsigned int count) {

  dmalist.option_0b=0x0b;
  dmalist.option_80=0x80;
  dmalist.source_mb=0;
  dmalist.option_81=0x81;
  dmalist.dest_mb=destination_address>>20;
  dmalist.end_of_options=0x00;

  dmalist.command = 0x03; // fill
  dmalist.sub_cmd = 0;
  dmalist.count = count;
  dmalist.source_addr = value;
  dmalist.dest_addr = destination_address&0xffff;
  dmalist.dest_bank = (destination_address >> 16)&0x0f;
  if (destination_address >= 0xd000 && destination_address < 0xe000)
    dmalist.dest_bank |= 0x80;

  do_dma();
  return;

}

unsigned char offset = 0;
unsigned char v;
unsigned char v1;

unsigned char frame[5];

unsigned char fnum = 0;
unsigned char flast = 99;
unsigned char spo2;
unsigned short prh;
unsigned short pr;

unsigned long j;

unsigned char needed[25][5] = {{ 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*2*/       { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*3*/       { 0x01, 0x80, 0x80, /*SPO2*/0x61, 0xC8 },
              /*4*/       { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*5*/       { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*6*/       { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*7*/       { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*8*/       { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*9*/       { 0x01, 0x80, 0x80, /*SPO2*/0x61, 0xC8 },
              /*10*/      { 0x01, 0x80, 0x80, /*SPO2/PR*/0x64, 0xC8 },
              /*11*/      { 0x01, 0x80, 0x80, /*SPO2*/0x61, 0xC8 },
              /*12*/      { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*13*/      { 0x01, 0x80, 0x80, 0x61, 0xC8 },
              /*14*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*15*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*16*/      { 0x01, 0x80, 0x80, /*SPO2*/0x61, 0xC8 },
              /*17*/      { 0x01, 0x80, 0x80, /*SPO2*/0x61, 0xC8 },
              /*18*/      { 0x01, 0x80, 0x80, 0x60, 0xC8 },
              /*19*/      { 0x01, 0x80, 0x80, 0x60, 0xC8 },
              /*20*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*21*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*22*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*23*/      { 0x01, 0x80, 0x80, /*PR*/0x65, 0xC8 },
              /*24*/      { 0x01, 0x80, 0x80, 0x64, 0xC8 },
              /*25*/      { 0x01, 0x80, 0x80, 0x64, 0xC8 }};

  unsigned short *screen = 0xA000U;
  int scr;
  
  unsigned int x,x1;
  unsigned int y;

  unsigned int z;

  unsigned char c;
  unsigned long a;
  int n = 0;

void plot_pixel() {
  
  // Integer halving the number with bitwise shift
  x1 = x>>1;
  
  // Pixel memory address location
  a = 0x40000L + 
  // Bitwise AND with 7 to keep resulting value below 4 bits
  (x1&7) +
  // Multiply y with unsigned int 8 because of character size
  (y*8U) +
  // Bitwise shift by 3 (long)
  (x1>>3L) *
  // Screen dimension multiplication
  (50*64L);

  /* Assign 2 bytes of memory at address value (of a) before re-assigning to 0. This seems redundant.
  v = lpeek(a); */ 
  
  // Assign v to 0
  v = 0;
  
  // Replacement for modulus (runs statement if odd)
  if (!(x&1)) { 
    
    // 
    v &= 0xf0;
    v |= c; 
    
  } 
  // Else runs if even
  else { 
    
    v &= 0xf;
    v |= (c<<4);
  
  }

  // Initially has v instead of 0x01
  if (y == 198) {
  
    lpoke(a,0x11);
  
  }
  else if (y == 297) {

    lpoke(a,0x22);

  }
  else {

    lpoke(a,0x33);

  }

  //lpoke(0x40322L,0x03);

}

void main(void) {

  // Fast CPU
  POKE(0,65);

  // Enable access to serial port and other devices
  POKE(53295L,0x47);
  POKE(53295L,0x53);

  // High res in X and Y directions
  POKE(0xD031U,0x88);

  // Set serial port speed to 9600
  POKE(0xd0e6U,0x46);
  POKE(0xd0e7U,0x10);

  // Accessing the VIC registers
  POKE(0xd02f,0x47);
  POKE(0xd02f,0x53);

  // Setting 16 bit character mode
  // also enable full colour chars for chars >$FF
  POKE(0xd054,0x05);

  // Move screen to $A000
  POKE(0xD060,0x00);
  POKE(0xD061,0xA0);

  // Logical lines are 80 bytes long
  POKE(0xD058,90);
  POKE(0xD05E,45);

  // Make screen background black
  POKE(0xD020,0);
  POKE(0xD021,0);

  // Clear colour RAM and set correct bits for showing full colour chars
  lfill(0xff80000L,0x08,45*50*2);

  // Initialise the screen RAM
  n = 0x1000;
  lfill((unsigned long)screen,0,30*50*2);
  for (x=0; x<30; x++) {
    
    for(y=0; y<50; y++) {
      
      screen[x + y * 45U] = n;
      n++;
      n=n&0x1fff;
    
    }

  }

  for (x=30; x<45; x++) {
    
    for(y=0; y<50; y++) {
      screen[x + y * 45U] = 0;
      lpoke(0xff80000L+x*2+y*90U+0,0);
      lpoke(0xff80000L+x*2+y*90U+1,1);
    }
  }

  // Clear pixel memory
  lfill(0x40000L,0,0xFFFF);
  lfill(0x50000L,0,0xFFFF);

  x = 0; 
  y = 99;
  z = 45;
  n = 0;
  
  while (1) {

    x = x + 1;
    if (x > 477) {
    
      x = 0;
          
    }
    
    y = y + 99;
    if (y > 395) {
      
      y = 99;
    
    }
    
    n++;
    c = n>>8;
    plot_pixel();

  }

  while(1) {


    if (x == 24 && y == 5) {

      x = 0;
      y = 0;

    }

    //v=PEEK(0xd0e0U);
    v = needed[x][y];

    if (v) {

      frame[0] = frame[1];
      frame[1] = frame[2];
      frame[2] = frame[3];
      frame[3] = frame[4];
      frame[4] = v;

      if (frame[0] == 0x01) {
	      if (frame[2]&0x80)
	        if (!(frame[3]&0x80)) {
	      
            flast = fnum;
	          if (frame[1]&1) {
		
              fnum = 0;
	          
            }
            
            else fnum++;

	          switch(fnum) {
	      
              case 0:
                prh = frame[3];
                break;
	        
              case 1:
		            if (flast == 0) {
		          
                  pr = (frame[3]&0x7f); //+((prh&3)<<7);
		        
                }
		            break;
	        
              case 2:
		            if (flast == 1) {
              
                  //printf("pr = %d\n",pr);
		        
                }
                spo2 = frame[3];
		            break;
	            case 3:
		            if (flast == 2)
		            //printf("spo2 = %d\n",spo2);
		            break;
	          }

	        }

      }

    }

    if (y % 5 == 0) {

      x++;
      y = 0;

    }

    y++;
    
  }
  
}
