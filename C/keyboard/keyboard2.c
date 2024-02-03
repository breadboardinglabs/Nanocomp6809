//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Nanocomp 6809 PS/2 Keyboard Controller Test Program #2 decode scan code to extended ASCII
// For non standard ASCII characters like Page Up/Page Down and arrow keys the 
// least significant Byte is 0 and the code is in the most signifcant byte
// Most standard ASCII characters have the ASCII code in LS byte and scan code in MS byte
// PgUp Scan code 0x49 (73 Dec) Extended ASCII 79 00
// a/A Scan code 0x1E Returns A=1E 41  a=1E 61
// cd /mnt/c/6809/code/keyboard
// cmoc --void-target --org=0000 --data=7200 --limit=7f80 -i --intdir=int --srec keyboard2.c nanocomp2.c
// --limit is required to warn if hitting the top of memory for stack
//
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

#include "cmoc.h"
#include "cmocextra.h"
#include "nanocomp.h"


int main(int argc, char **argv)
{
  // Enables debug output from printf() to serial port
  setConsoleOutHook(serialOutputRoutine);
  printf("Test Serial Debug \n\r");
  // Set Display to Character VGA with default VGA Palette
  CRTCInit(0);
  PaletteInit();

  if (keyboardInit()!=0){
    printf("Keyboard Init returned error \n");
  }
  byte kbdData; 
  byte kbdStatus;
  
  
	if (kbc_write_data(dev_cmd_echo)){ // send keyboard echo command
     printf("Error sending keyboard device echo command\n");
  }

  kbc_read_data(&kbdStatus, &kbdData); // check for echo in the buffer
  printf("Response from echo==%02X, expecting %02X\n",kbdData,dev_cmd_echo );

	if (kbc_write_data(0xED)){ // send keyboard LED command
     printf("Error sending keyboard LED command\n");
  }

  kbc_read_data(&kbdStatus, &kbdData); // check for ACK from disable in the buffer
  if (kbdData != dev_rsp_ack){
    printf("Error waiting for LED ACK code==%02X, expecting %02X\n",kbdData,dev_rsp_ack );
  } else {
    printf("LED ACK code OK\n");
  }

	if (kbc_write_data(0x07)){ // send keyboard LED data
     printf("Error sending LED data\n");
  }

  kbc_read_data(&kbdStatus, &kbdData); // check for ACK from disable in the buffer
  if (kbdData != dev_rsp_ack){
    printf("Error waiting for LED data ACK code==%02X, expecting %02X\n",kbdData,dev_rsp_ack );
  } else {
    printf("LED ACK data OK\n");
  }

  // Send scan code
  byte scanCode = 0x02;
/*
	if (kbc_write_data(0xF0)){ // send keyboard scan code command
     printf("Error sending keyboard scan code command\n");
  }

  kbc_read_data(&kbdStatus, &kbdData); // check for ACK from disable in the buffer
  if (kbdData != dev_rsp_ack){
    printf("Error waiting for scan code ACK code==%02X, expecting %02X\n",kbdData,dev_rsp_ack );
  } else {
    printf("scan code ACK code OK\n");
  }

	if (kbc_write_data(scanCode)){ // send keyboard scan code data
     printf("Error sending scan code data\n");
  }

  kbc_read_data(&kbdStatus, &kbdData); // check for ACK from disable in the buffer
  if (kbdData != dev_rsp_ack){
    printf("Error waiting for scan code data ACK code==%02X, expecting %02X\n",kbdData,dev_rsp_ack );
  } else {
    printf("Scan code data ACK OK\n");
  }
*/
  int wait=0;
  int count=1;
  while (count < 500){
    while (kbc_read_data(&kbdStatus, &kbdData)==1){
      wait++;  
    }		// wait and read the response, returns 0 with key, 1 with timeout
    printf("Keyboard Status %02X, Count=%d, Code %02X\n", kbdStatus, count, kbdData);
    count++;
    wait=0;
  }
  returnNC();  // Causes SWI for Nanocomp to return to monitor
  return 0;
}
