/* 6809dasm.c - a 6809 opcode disassembler */
/* Version 1.7 10-OCT-2022 */

/* Copyright © 1995 Sean Riddle */

/* thanks to Franklin Bowen for bug fixes, ideas */

/* Freely distributable on any medium given all copyrights are retained */
/* by the author and no charge greater than $7.00 is made for obtaining */
/* this software */

/* Please send all bug reports, update ideas and data files to: */
/* sriddle@ionet.net */

/* latest version at: */
/* <a href="http://www.ionet.net/~sriddle">Please don't hurl on my URL!</a> */

/* the data files must be kept compatible across systems! */

/* usage: 6809disasm <file> [<data file>] */
/* output to stdout, so use redirection */

/* The optional data file contains 7 types of lines: */
/* o Remarks - these are lines beginning with a semi-colon (;) */
/*   they are completely ignored. */
/* o 1 ORG line - gives the origin of the code; this is the starting */
/*   address to be used for the disassembly. */
/* o COMMENT lines - used to add comments to the end of lines of the */
/*   disassembly. */
/* o COMMENTLINE lines - provide full-line comments to be included before */
/*   a given address in the disassembly. */
/* o DATA lines - mark sections as data.  These sections will not be */
/*   disassembled, but dumped as hex data instead. */
/* o ASCII lines - mark sections as text.  These sections will not be */
/*   disassembled, but printed as text instead. */
/* o WTEXT lines - interprets section as text encoded as in Joust, */
/*   Bubbles, Sinistar (0x0=0,...,0xa=space,0xb=A,...,0x24=Z,...,0x32=:*/
/* See sample data files (*.dat) for examples. */

/* current limitations: */
/* o number of LABEL, DATA/ASCII, COMMENT and COMMENTLINE lines determined */
/*   at compile-time - see MAXLABEL, MAXDATA, MAXCOMMENT and MAXCOMMLINE */
/* o all DATA/ASCII lines in data file must be sorted in ascending */
/*   address order */
/* o ditto for COMMENT and COMMENTLINE lines */
/* o if a DATA/ASCII area is preceded by what 6809dasm thinks is code */
/*   that continues into the DATA/ASCII area, the data will not be marked */
/*   as such, and an error will be printed.  If this is the case, mark the */
/*   line before the data as data also. */

/* to do: */
/* o sort comment, ascii, data lines */
/* o look at JMP and JSR addresses- set to code unless overridden */
/*   in data file */
/* o perhaps a 'scan' that creates a first-guess .dat file? */
/*   generate dummy labels, mark code, find ASCII, etc. */

/* compiled on Amiga SAS/C 6.51 and Sun 10 Unix cc */




/* changes by Chris */
/* (PC, sadly not Amiga anymore) */
/* june 1996 */
/* - SWI2 numopcodes were wrong */
/* - comand line parameters differ from original */
/* - help (small) included */
/* - compatability switch included for compatability to motorala's public */
/*   domain crossassembler AS9 */
/*   in order to be able to reassamble any given binary, some additional */
/*   testing is done (mostly postbyte specific in indirect address mode) */
/*   offsets of a size which might fit into a smaller size offset instruction */
/*   (there are 5bit, 8bit and 16bit offsets) may cause trouble, if exact */
/*   recompilation must be asured, therefor some of these statements are */
/*   transfered to DATA status */
/*   (anybody know why something like : OPCODE <[$xxxx,R] */
/*    can't be assembled with AS9?) */
/* - if non compatability switch is specified output should be as original */

/* Dave Henry Changes for use with ASM6809 https://www.6809.org.uk/asm6809/ */
/* When using -C use $ rather than 0x for Hex */
/* Replace .db with FCB */
/* Correct smartquotes for ' */
/* Remove CODE line output */
/* Correct direct addressing need 00-0F and 90-9F */
/* Remove 	strip 6809dasm and coff2exe 6809dasm lines from MAKEFILE */
/* OPCODE.H Corrected cpx to cmpx */



#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include "defs.h"

#define DATA_MAX_LINE_DEFAULT 16
#define ASCII_MAX_LINE_DEFAULT 70
#define DATA_MAX_LINE_COMPATIBLE 10
#define ASCII_MAX_LINE_COMPATIBLE 60
#define DATA_PER_LINE_DEFAULT 16
#define ASCII_PER_LINE_DEFAULT 32
#define DATA_PER_LINE_COMPATIBLE 10
#define ASCII_PER_LINE_COMPATIBLE 50

int as9_comp = NO;

#define SHOW_IT 2

/* array sizes for labels, DATA/ASCII definitions and COMMENT/COMMENTLINE */
#define MAXLABEL 999
#define MAXDATA 999                           /* both DATA and ASCII lines */
#define MAXCOMMENT 999
#define MAXCOMMLINE 999

/* output listing spacing */
#define TABOPNAME 22
#define TABOPERAND 31
#define TABLINENUM 48
#define TABCOMM 55

#define OPNAMETAB (TABOPNAME-1)
#define OPERANDTAB (TABOPERAND-1)
#define COMMTAB (TABCOMM-1)
#define LINENUMTAB (TABLINENUM-1)

#include "opcode.h"

char force_len[5];

long file_act_read;
int count;                             /* current program counter for disasm */

/* getbyte() - get a byte from a file, and increment the byte counter */
int getbyte(FILE *fp)
{
 int c;
 file_act_read++;
 count++;
 c=getc(fp);
 return(c);
}

/* pregetbyte() - get the next byte from a file, and store it back 'lookahead' */
int pregetbyte(FILE *fp)
{
 int c;
 c=getc(fp);
 ungetc(c,fp);
 return(c);
}

#ifdef NOCONST                                /* TFB had to undefine const */
 #define const
#endif
const char *regs[5]={"x","y","u","s","pc"};
const char *teregs[16]={"d","x","y","u","s","pc","inv","inv","a","b","cc",
   "dp","inv","inv","inv","inv"};
const int teregssize[16]={16,16,16,16,16,16,0,0,8,8,8,8,0,0,0,0};

BOOL PC=FALSE;                      /* to see if a PUL instr is pulling PC */

#define LABELSIZE 40
/* label structure */
struct lastruct
{
 unsigned short lab;     /* label address */
 char label[LABELSIZE];        /* label text */
} *labarray=NULL;
int numlab=0;                                  /* number of labels defined */

#ifndef AMIGA
/* hmmm, these aren't ANSI */
/* stricmp() - compare two strings, case insensitive */
#ifndef __BORLANDC__

int stricmp(const char *s1, const char *s2)
{
 for(;toupper(*s1)==toupper(*s2);++s1,++s2)
  if(*s1=='\0')
   return(0);
 return((toupper(*(unsigned char *)s1)<toupper(*(unsigned char *)s2))?-1:1);
}

/* strnicmp() - compare two strings, case insensitive, length limited */
int strnicmp(const char *s1, const char *s2, size_t n)
{
 for(;0<n;++s1,++s2,--n)
  if (toupper(*s1)!=toupper(*s2))
   return((toupper(*(unsigned char *)s1)<toupper(*(unsigned char *)s2))?-1:1);
  else if(*s1=='\0')
   return(0);
 return(0);
}
#endif
#endif

char labtemp[30];                   /* global return for checklabs() - tsk */
char *fulladdr="$%04X";            /* DH Change from 0x to $ */
char *shortaddr="$%02X";

/* checklabs() - check the defined labels from data file */
/* substitute label for address if found */
char *checklabs(int address,BOOL lab2,BOOL printdollar,BOOL dosmall)
{
 int i;
 char *printaddr;
// address&=0xffff;
 labtemp[0]='\0';
 if (((address>=128)||(address<-128)) || (dosmall))
 {
  for (i=0;i<numlab;i++)
   if ((address&0xffff)==labarray[i].lab)
   {
    sprintf(labtemp,"%s",labarray[i].label);
    if(lab2)
     strcat(labtemp,":\n");
    i=numlab;
   }
//  if ((address<256)||(address>0xffff-255))
  if ((address<128)&&(address>-129))
  {
//   address&=0xff;
   printaddr=shortaddr;
  }
  else
  {
   printaddr=fulladdr;
  }
  if(!strlen(labtemp)&&!lab2)
  {
   if(!printdollar)
    printaddr++;
   if (address>=0)
   {
    sprintf(labtemp,printaddr,address);
   }
   else
   {
    if ((address<128)&&(address>-129))
    {
     sprintf(labtemp,"-$%02X",-address);
    }
    else
    {
     sprintf(labtemp,"-$%04X",-address);
    }
   }
  }
 }
 return(labtemp);
}

#define B1(b) (  (b>=1)  ? (1) : 0 )
#define B2(b) (  (b>=10)  ? (2+B1(b-10)) : B1(b) )
#define B3(b) (  (b>=100)  ? (4+B2(b-100)) : B2(b) )
#define B4(b) (  (b>=1000)  ? (8+B3(b-1000)) : B3(b) )
#define B5(b) (  (b>=10000)  ? (16+B4(b-10000)) : B4(b) )
#define B6(b) (  (b>=100000)  ? (32+B5(b-100000)) : B5(b) )
#define B7(b) (  (b>=1000000)  ? (64+B6(b-1000000)) : B6(b) )
#define B(b)  (  (b>=10000000) ? (128+B7(b-10000000)) : B7(b) )

/* bittest() - test on a single bit, 1-8 */
int bittest(unsigned char c, int bitnumber)
{
 while (--bitnumber)
 {
  c=c>>1;
 }
 return ((c&1)==1);
}

/* fitbinarypattern() - test on a bit pattern, 0 1 ? */
int fitbinarypattern(unsigned char c,unsigned char *pattern)
{
 if (strlen(pattern)==0)
  return TRUE;
 if (pattern[0]=='?')
  return fitbinarypattern(c,pattern+1);
 if (pattern[0]=='0')
  if  ( bittest(c,strlen(pattern))==FALSE )
   return fitbinarypattern(c,pattern+1);
  else
   return FALSE;
 if (pattern[0]=='1')
  if  ( bittest(c,strlen(pattern))==TRUE )
   return fitbinarypattern(c,pattern+1);
  else
   return FALSE;
 return FALSE; /* not supposed to ever get here */
}

/* printbinary() - 8 bit ... */
int printbinary(unsigned char c)
{
 int i=8;
 while (i)
 {
  if (bittest(c,i))
   printf("1");
  else
   printf("0");
  i--;
 }
}

/* checkindpostbyte() - see if postbyte of IND is OK */
int checkindpostbyte(unsigned char c,UBYTE numoperands,UBYTE *operandarray, FILE *fp)
{
/* I'm aware, that this checking could have been done more efficiently.
   but this way it reflects exactly the technical manual I got */

 if (fitbinarypattern(c,"0???????")==TRUE) /* EA = ,R + 5 bit offset */
 {
  if (fitbinarypattern(c,"0??00000")==TRUE)
  {
// no action needed, since assembler assumes 0 offset only with syntax like "lda ,x"
// the output we generate is "lda 0x00,x", which is correctly translated to 5 bit
//   force_len[0]='_';
//   force_len[1]='5';
//   force_len[2]=(char)0;
   return TRUE;
//   printf("; 5bit nil-offset will be translated to 0-offset, changing to data\n");
//   return SHOW_IT;
  }
  return TRUE;
 }
 if (fitbinarypattern(c,"1??00000")==TRUE) /* ,R+ */
  return TRUE;
 if (fitbinarypattern(c,"1???0001")==TRUE) /* ,R++ */
  return TRUE;
 if (fitbinarypattern(c,"1??00010")==TRUE) /* ,R- */
  return TRUE;
 if (fitbinarypattern(c,"1???0011")==TRUE) /* ,R-- */
  return TRUE;
 if (fitbinarypattern(c,"1???0100")==TRUE) /* EA = ,R + 0 offset */
  return TRUE;
 if (fitbinarypattern(c,"1???0101")==TRUE) /* EA = ,R + ACCB offset */
  return TRUE;
 if (fitbinarypattern(c,"1???0110")==TRUE) /* EA = ,R + ACCA offset */
  return TRUE;
 if (fitbinarypattern(c,"1???1000")==TRUE) /* EA = ,R + 8bit offset */
 {
  if ((pregetbyte(fp)<15)||(pregetbyte(fp)>240))
  {
   if (fitbinarypattern(c,"???0????")==TRUE)
   {
    printf("; small 8bit ind offset will be falsely assembled, forcing 8bit\n");
    force_len[0]='_';
    force_len[1]='8';
    force_len[2]=(char)0;
    return TRUE;
// printf("; small 8bit ind offset will be falsely assembled, changing to data\n");
// return SHOW_IT;
// return FALSE;
   }
  }
  return TRUE;
 }
 if (fitbinarypattern(c,"1???1001")==TRUE) /* EA = ,R + 16bit offset */
 {
  if (pregetbyte(fp)==0)
  {
   if (fitbinarypattern(c,"???1????")==TRUE)
   {
    printf("; small 16bit ind offset will be falsely assembled, forcing 16bit\n");
    force_len[0]='_';
    force_len[1]='1';
    force_len[2]='6';
    force_len[3]=(char)0;
    return TRUE;
//  printf("; small 16bit ind offset will be falsely assembled, changing to data\n");
//  return FALSE;
   }
  }
  return TRUE;
 }
 if (fitbinarypattern(c,"1???1011")==TRUE) /* EA = ,R + D offset */
  return TRUE;
 if (fitbinarypattern(c,"1???1100")==TRUE) /* EA = ,PC + 8bit offset */
 {
  if (fitbinarypattern(c,"?00?????")!=TRUE)
  {
   printf("; assembler expects don't care bits to be zeroed, changing to data\n");
   return FALSE;
  }
  return TRUE;
 }
 if (fitbinarypattern(c,"100?1101")==TRUE) /* EA = ,PC + 16bit offset */
 {
  if (fitbinarypattern(c,"?00?????")!=TRUE)
  {
   printf("; assembler expects don't care bits to be zeroed, changing to data\n");
   return FALSE;
  }
  return TRUE;
 }
 if (fitbinarypattern(c,"1???1111")==TRUE) /* EA = ,[,adress] */
 {
  if (fitbinarypattern(c,"10011111")!=TRUE)
  {
   printf("; invalid extended indirect adressing, changing to data\n");
   return FALSE;
  }
  return TRUE;
 }
 printf("; illegal postbyte encountered, changing to data\n");
 return FALSE;
}

/* printoperands() - print operands for the given opcode */
void printoperands(int opcode,UBYTE numoperands,UBYTE *operandarray,
     UBYTE mode,FILE *fp,char *opname,char *str,int page)
{
 int i,rel,pb,offset=0,reg,pb2;
 BOOL comma;
 char out2[80];
 int sp;
 BOOL printdollar;      /* print a leading $ before address? */
 printdollar=FALSE;
 PC=FALSE;
 force_len[0]=(char)0;
 if((opcode!=0x1f)&&(opcode!=0x1e))
 {
  switch(mode)
  {     /* print before operands */
   case IMM:
   {
    strcat(str,"#");
    if (as9_comp == YES)
     printdollar=TRUE;
    /* fixme? */
   }
   case DIR:
    break;
   case EXT:
    printdollar=TRUE;
    break;
   default:
    break;
  }
 }
 switch(mode)
 {
  case REL:          /* 8-bit relative */
   rel=operandarray[0];
   sprintf(out2,checklabs((short)(count+((rel<128) ? rel : rel-256)), FALSE,TRUE,TRUE));
   strcat(str,out2);
   break;
  case LREL:           /* 16-bit long relative */
   rel=(operandarray[0]<<8)+operandarray[1];
   sprintf(out2,checklabs(count+((rel<32768) ? rel : rel-65536),FALSE,TRUE,TRUE));
   strcat(str,out2);
   break;
  case IND:         /* indirect- many flavors */
   pb=operandarray[0];
   if (as9_comp == YES)
   {
    int s_helper=checkindpostbyte(pb,numoperands,operandarray,fp);
    if (s_helper!=TRUE)
    {
     for(sp=strlen(str);sp<OPNAMETAB;sp++)
      strcat(str," ");
     sprintf(out2,"FCB     ");
     strcat(str, out2);
     if (page)
     {
      sprintf(out2, " $%02X,", (UBYTE) page+15);
      strcat(str, out2);
     }
     sprintf(out2, " $%02X", (UBYTE) opcode);
     strcat(str, out2);
     for (i=0;i<numoperands;i++)
     {
      sprintf(out2, ", $%02X" , (UBYTE) operandarray[i]);
      strcat(str, out2);
     }
     if (s_helper==SHOW_IT)
     {
      printf("%s",str);
      sprintf(str, "\n; Instruction was:   ");
     }
     else
      break;
    }
   }
   reg=(pb>>5)&0x3;
   pb2=pb&0x8f;
   if ((pb2==0x88)||(pb2==0x8c))
   {   /* 8-bit offset */
    offset=getbyte(fp);
    sprintf(out2,"%02X ",offset);
    if (as9_comp == NO)
    {
     /* hex of operands */
     strcat(str, out2);
    }
    if ((offset==EOF)&&(as9_comp == YES))
    {
     {
      for(sp=strlen(str);sp<OPNAMETAB;sp++)
       strcat(str," ");
      sprintf(out2,"FCB     ");
      strcat(str, out2);
      if (page)
      {
       sprintf(out2, " $%02X,", (UBYTE) page+15);
       strcat(str, out2);
      }
      sprintf(out2, " $%02X", (UBYTE) opcode);
      strcat(str, out2);
      for (i=0;i<numoperands;i++)
      {
       sprintf(out2, ", $%02X" , (UBYTE) operandarray[i]);
       strcat(str, out2);
      }
      sprintf(out2, ";    File end, can't continue opcode");
      strcat(str, out2);
     }
     break;
    }
    if (offset>127)         /* convert to signed */
     offset=offset-256;
    if (pb==0x8c) /* should perhaps be: ((pb&0x8c)==0x8c) */
     reg=4;
    for(sp=strlen(str);sp<OPNAMETAB;sp++)
     strcat(str," ");
///////////////////
    sprintf(out2,"%s%s",opname,force_len);
    strcat(str,out2);
    for(sp=strlen(str);sp<OPERANDTAB;sp++)
     strcat(str," ");
/* only when NOT indirect
    if (   ((offset<16)&&(offset>0))
        || ((offset>-16)&&(offset<0))
        || (offset==0))
    {
     strcat(str,"<"); // supposed to force to lower 8 bit...
    }
*/
    if((pb&0x90)==0x90)
     strcat(str,"[");
    if(pb==0x8c)
     sprintf(out2,"%s,%s",checklabs(offset,FALSE,TRUE,TRUE),regs[reg]);
    else if(offset>=0)
     sprintf(out2,"$%02X,%s",offset,regs[reg]);
    else
     sprintf(out2,"-$%02X,%s",-offset,regs[reg]);
    strcat(str,out2);
    if(pb==0x8c)
    {
     sprintf(out2," ; ($%04X)",offset+count);
     strcat(str,out2);
    }
   }
   else if((pb2==0x89)||(pb2==0x8d)||(pb2==0x8f))
   { /* IND 16-bit */
    int helper;
    helper=getbyte(fp);
    offset=(helper<<8);
    if ((helper==EOF)&&(as9_comp == YES))
    {
     {
      for(sp=strlen(str);sp<OPNAMETAB;sp++)
       strcat(str," ");
      sprintf(out2,"FCB     ");
      strcat(str, out2);
      if (page)
      {
       sprintf(out2, " $%02X,", (UBYTE) page+15);
       strcat(str, out2);
      }
      sprintf(out2, " $%02X", (UBYTE) opcode);
      strcat(str, out2);
      for (i=0;i<numoperands;i++)
      {
       sprintf(out2, ", $%02X" , (UBYTE) operandarray[i]);
       strcat(str, out2);
      }
      sprintf(out2, ";    File end, can't continue opcode");
      strcat(str, out2);
     }
     break;
    }
    sprintf(out2,"%02X ",offset>>8);
    if (as9_comp == NO)
    {
     /* hex of operands */
     strcat(str, out2);
    }
    helper=getbyte(fp);
    if ((helper==EOF)&&(as9_comp == YES))
    {
     {
      for(sp=strlen(str);sp<OPNAMETAB;sp++)
       strcat(str," ");
      sprintf(out2,"FCB     ");
      strcat(str, out2);
      if (page)
      {
       sprintf(out2, " $%02X,", (UBYTE) page+15);
       strcat(str, out2);
      }
      sprintf(out2, " $%02X", (UBYTE) opcode);
      strcat(str, out2);
      for (i=0;i<numoperands;i++)
      {
       sprintf(out2, ", $%02X" , (UBYTE) operandarray[i]);
       strcat(str, out2);
      }
      sprintf(out2,", $%02X ",offset>>8);
      strcat(str, out2);
      sprintf(out2, "    File end, can't continue opcode");
      strcat(str, out2);
     }
     break;
    }
    offset+=helper;
    sprintf(out2,"%02X ",offset&0xff);
    if (as9_comp == NO)
    {
     /* hex of operands */
     strcat(str, out2);
    }
    if ((pb!=0x8f)&&(offset>32767))
     offset=offset-65536;
    offset&=0xffff;
    if (pb==0x8d)
     reg=4;
    for(sp=strlen(str);sp<OPNAMETAB;sp++)
     strcat(str," ");
///////////////////
    sprintf(out2,"%s%s",opname,force_len);
    strcat(str,out2);
    for(sp=strlen(str);sp<OPERANDTAB;sp++)
     strcat(str," ");
/*
    if (   ((offset<128)&&(offset>0))
        || ((offset>-128)&&(offset<0))
        || (offset==0))
    {
     strcat(str,">");
    }
*/
/*    ...*/
    if ((pb&0x90)==0x90)
     strcat(str,"[");
    if(pb==0x9f)
     sprintf(out2,"%s",checklabs(offset,FALSE,TRUE,TRUE));
    else if(pb==0x8d)
     sprintf(out2,"%s,%s",checklabs(offset,FALSE,TRUE,TRUE),regs[reg]);
    else if (offset>=0)
     sprintf(out2,"$%04X,%s",offset,regs[reg]);
    else
     sprintf(out2,"-$%04X,%s",offset,regs[reg]);
    strcat(str,out2);
    if(pb==0x8d)
    {
     sprintf(out2," ; ($%04X)",offset+count);
     strcat(str,out2);
    }
   }
   else if(pb&0x80)
   {
    for(sp=strlen(str);sp<OPNAMETAB;sp++)
     strcat(str," ");
///////////////////////
    sprintf(out2,"%s%s",opname,force_len);
    strcat(str,out2);
    for(sp=strlen(str);sp<OPERANDTAB;sp++)
     strcat(str," ");
    if((pb&0x90)==0x90)
     strcat(str,"[");
    if((pb&0x8f)==0x80)
    {
     sprintf(out2,",%s+",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x81)
    {
     sprintf(out2,",%s++",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x82)
    {
     sprintf(out2,",-%s",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x83)
    {
     sprintf(out2,",--%s",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x84)
    {
     sprintf(out2,",%s",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x85)
    {
     sprintf(out2,"b,%s",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x86)
    {
     sprintf(out2,"a,%s",regs[reg]);
     strcat(str,out2);
    }
    else if((pb&0x8f)==0x8b)
    {
     sprintf(out2,"d,%s",regs[reg]);
     strcat(str,out2);
    }
   }
   else
   {       /* IND 5-bit offset */
    offset=pb&0x1f;
    if (offset>15)
     offset=offset-32;
    for (sp=strlen(str);sp<OPNAMETAB;sp++)
     strcat(str," ");
/////////////////////
    sprintf(out2,"%s%s",opname,force_len);
    strcat(str,out2);
    for(sp=strlen(str);sp<OPERANDTAB;sp++)
     strcat(str," ");
//    sprintf(out2,"%s,%s",checklabs(offset,FALSE,TRUE,TRUE),regs[reg]);
    if (as9_comp == YES)
    {
     if (offset<0)
      sprintf(out2,"-$%02X,%s",-offset,regs[reg]);
//      sprintf(out2,"-0d%i,%s",-offset,regs[reg]);
     else
      sprintf(out2,"$%02X,%s",offset,regs[reg]);
//      sprintf(out2,"0d%i,%s",offset,regs[reg]);
    }
    else
     sprintf(out2,"%s,%s",checklabs(offset,FALSE,TRUE,TRUE),regs[reg]);
    strcat(str,out2);
//...   ...
   }
   if((pb&0x90)==0x90)
    strcat(str,"]");
   break;
  default:
   if((opcode==0x1f)||(opcode==0x1e))
   {         /* TFR/EXG */
    sprintf(out2,"%s,%s",teregs[(operandarray[0]>>4)&0xf], teregs[operandarray[0]&0xf]);
    strcat(str,out2);
   }
   else if((opcode==0x34)||(opcode==0x36))
   {    /* PUSH */
    comma=FALSE;
    if(operandarray[0]&0x80)
    {
     strcat(str,"pc");
     comma=TRUE;
     PC=TRUE;
    }
    if(operandarray[0]&0x40)
    {
     if(comma)
      strcat(str,",");
     if((opcode==0x34)||(opcode==0x35))
      strcat(str,"u");
     else
      strcat(str,"s");
     comma=TRUE;
    }
    if(operandarray[0]&0x20)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"y");
      comma=TRUE;
    }
    if(operandarray[0]&0x10)
    {
     if (comma)
      strcat(str,",");
     strcat(str,"x");
     comma=TRUE;
    }
    if(operandarray[0]&0x8)
    {
     if (comma)
      strcat(str,",");
     strcat(str,"dp");
     comma=TRUE;
    }
    if(operandarray[0]&0x4)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"b");
     comma=TRUE;
    }
    if(operandarray[0]&0x2)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"a");
     comma=TRUE;
    }
    if(operandarray[0]&0x1)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"cc");
    }
   }
   else if((opcode==0x35)||(opcode==0x37))
   {    /* PULL */
    comma=FALSE;
    if(operandarray[0]&0x1)
    {
     strcat(str,"cc");
     comma=TRUE;
    }
    if(operandarray[0]&0x2)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"a");
     comma=TRUE;
    }
    if(operandarray[0]&0x4)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"b");
     comma=TRUE;
    }
    if(operandarray[0]&0x8)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"dp");
     comma=TRUE;
    }
    if(operandarray[0]&0x10)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"x");
     comma=TRUE;
    }
    if(operandarray[0]&0x20)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"y");
     comma=TRUE;
    }
    if(operandarray[0]&0x40)
    {
     if(comma)
      strcat(str,",");
     if((opcode==0x34)||(opcode==0x35))
      strcat(str,"u");
     else
      strcat(str,"s");
     comma=TRUE;
    }
    if(operandarray[0]&0x80)
    {
     if(comma)
      strcat(str,",");
     strcat(str,"pc");
     strcat(str," ;(pul? pc=rts)");
     PC=TRUE;
    }
   }
   else
   {
    if(numoperands==2)
    {
     if ((mode==EXT)&&(as9_comp == YES))
     {
/* DH Extended check for >
      new gone */
      if ((operandarray[0]<<8)+operandarray[1]<256)
       strcat(str,">");
/**/
     }
     strcat(str,checklabs((operandarray[0]<<8)+operandarray[1], FALSE,TRUE,TRUE));
    /* fixme? */
    }
    else if((numoperands==1)&&(mode!=IMM))
    {
/* 
     if ((mode==DIR)&&(as9_comp == YES))
     {
      if ((operandarray[0]<16)||(operandarray[0]>256-16))
       strcat(str,"<");
     }
*/
     if ((mode==DIR)&&(as9_comp == YES))
     {
    /* DH Correct direct addressing need 00-0F and 90-9F <*/
     if ((operandarray[0]<16)|| ((operandarray[0]>16)||(operandarray[0]<160)))
       strcat(str,"<"); /**/
     }
     strcat(str,checklabs(operandarray[0],FALSE,TRUE,TRUE));
    }
    else
    {
     if (printdollar)
     {
      strcat(str,"$");
     }
     for(i=0;i<numoperands;i++)
     {
      sprintf(out2,"%02X",operandarray[i]);
      strcat(str,out2);
     }
    }
   }
  break;
 }
}

#define DATA 0                                  /* type of data to display */
#define ASCII 1
#define WTEXT 2

/* DATA/ASCII structure definition */
struct dastruct
{
 unsigned short start;        /* beginning address of DATA/ASCII */
 unsigned short end;           /* ending address */
 unsigned short per_line;      /* values to print on a line */
 short type;            /* DATA or ASCII ? */
} *dataarray=NULL;

#define COMMENTSIZE 80
/* COMMENT/COMMENTLINE structure definition */
struct castruct
{
 unsigned short comline;    /* comment line address */
 char comment[COMMENTSIZE];     /* comment text */
} *commarray=NULL,*commlinearray=NULL;

char out[512],out2[80];                             /* temp string buffers */

void readdatafile(char *filename, int *org,int *numlab,int *numdata,int *numcomm,int *numcommline)
{
 char line2[256];
 char *line;
 FILE *fp;
 int dataline,data,data2,per_line,i,k;
 if(fp=fopen(filename,"r"))
 {       /* read data file */
  while(fgets(line2,255,fp))
  {
   while(strlen(line2)&&!isprint(line2[strlen(line2)-1])) /* strip cr */
    line2[strlen(line2)-1]='\0';
   for(i=0;i<strlen(line2);i++)
    if((line2[i]==';')||isalpha(line2[i]))
    {
     line= &(line2[i]);
     i=strlen(line2);
    }
   if(!strnicmp(line,"ORG ",4))
   {
    if(*org==-1)
     sscanf(&line[4],"%X",org);
    else
     printf("More than one ORG line\n");
   }
   else if(!strnicmp(line,"LABEL ",6))
   {
    if(*numlab<MAXLABEL)
    {
     sscanf(&line[6],"%X",&dataline);
     labarray[*numlab].lab=dataline;
     k=6;
     while(line[k]==' ')
      k++;
     while(line[k]!=' ')
      k++;
     while(line[k]==' ')
      k++;
     strncpy(labarray[*numlab].label,&line[k],LABELSIZE);
     labarray[*numlab].label[LABELSIZE-1]=0;  /* just in case */
     for(i=0;i<*numlab;i++)
      if(!strcmp(labarray[i].label,labarray[*numlab].label))
      {
       printf("duplicate label: %s\n",labarray[i].label);
       break;
      }
     if(i>=*numlab)
      (*numlab)++;
    }
    else
     printf("Too many labels\n");
   }
   else if(!strnicmp(line,"DATA ",5))
   {
    if (*numdata<MAXDATA)
    {
     data2=0;
     sscanf(&line[5],"%X-%X",&data,&data2);
     k=5;
     while(line[k]==' ')
      k++;
     while(line[k]!=' ')
      k++;
     while(line[k]==' ')
      k++;
     if (k < i)
      sscanf(&(line[k]),"%d",&per_line);
     else
      per_line = 0;
     if(data2<data)
      data2=data;
     dataarray[*numdata].type=DATA;
     dataarray[*numdata].start=data;
     dataarray[*numdata].end=data2;
     dataarray[*numdata].per_line=per_line;
     if(((*numdata)&&(dataarray[*numdata-1].start<dataarray[*numdata].start))||!*numdata)
      (*numdata)++;
     else
     /* DH correct smartquote */
      printf("'%s'\nDATA out of order\n\n",line);
    }
    else
     printf("Too many DATA/ASCII lines\n");
   }
   else if(!strnicmp(line,"ASCII ",6))
   {
    if(*numdata<MAXDATA)
    {
     data2=0;
     per_line=0;
     sscanf(&line[6],"%X-%X %d",&data,&data2,&per_line);
     if(data2<data)
      data2=data;
     dataarray[*numdata].type=ASCII;
     dataarray[*numdata].start=data;
     dataarray[*numdata].end=data2;
     dataarray[*numdata].per_line=per_line;
     if(((*numdata)&&(dataarray[*numdata-1].start<dataarray[*numdata].start))||!*numdata)
      (*numdata)++;
     else
      if (*numdata)
       /* DH correct smartquote */
       printf("'%s'\nASCII out of order\n\n",line);
    }
    else
     printf("Too many DATA/ASCII lines\n");
   }
   else if(!strnicmp(line,"WTEXT ",6))
   {
    if(*numdata<MAXDATA)
    {
     data2=0;
     per_line=0;
     sscanf(&line[6],"%X-%X %d",&data,&data2,&per_line);
     if(data2<data)
      data2=data;
     dataarray[*numdata].type=WTEXT;
     dataarray[*numdata].start=data;
     dataarray[*numdata].end=data2;
     dataarray[*numdata].per_line=per_line;
     if(((*numdata)&&(dataarray[*numdata-1].start<dataarray[*numdata].start))||!*numdata)
      (*numdata)++;
     else if(*numdata)
      printf("'%s'\nWTEXT out of order\n\n",line);
    }
    else
     printf("Too many DATA/ASCII lines\n");
   }
   else if(!strnicmp(line,"COMMENT ",8))
   {
    if(*numcomm<MAXCOMMENT)
    {
     sscanf(&line[8],"%X",&dataline);
     commarray[*numcomm].comline=dataline;
     k=8;
     while(line[k]==' ')
      k++;
     while(line[k]!=' ')
      k++;
     while(line[k]==' ')
      k++;
     strncpy(commarray[*numcomm].comment,&line[k],COMMENTSIZE);
     commarray[*numcomm].comment[COMMENTSIZE-1]=0; /* in case */
     if(((*numcomm)&&(commarray[(*numcomm)-1].comline<=commarray[*numcomm].comline))||!*numcomm)
      (*numcomm)++;
     else
      printf("'%s'\nCOMMENT out of order\n\n",line);
    }
    else
     printf("Too many COMMENT lines\n");
   }
   else if(!strnicmp(line,"COMMENTLINE ",12))
   {
    if(*numcommline<MAXCOMMLINE)
    {
     sscanf(&line[12],"%X",&dataline);
     commlinearray[*numcommline].comline=dataline;
     k=12;
     while(line[k]==' ')
      k++;
     while(line[k]!=' ')
      k++;
     while(line[k]==' ')
      k++;
     strncpy(commlinearray[*numcommline].comment,&line[k],COMMENTSIZE);
     commlinearray[*numcommline].comment[COMMENTSIZE-1]=0;
     if(((*numcommline)&&(commlinearray[(*numcommline)-1].comline<=commlinearray[*numcommline].comline))||!*numcommline)
      (*numcommline)++;
     else
      printf("'%s'\nCOMMENTLINE out of order\n\n",line);
    }
    else
     printf("Too many COMMENTLINE lines\n");
   }
   else if(line[0]==';')                    /* remark in data file */
    ;
   else if(strlen(line))
    printf("'%s'\nError in file '%s'\n\n",line,filename);
  }
  fclose(fp);
 }
 else
  fprintf(stderr,"Can't open file '%s'\n",filename);
}

void docomment(int line,int *curcomm,int numcomm,int numoperands)
{
 int  sp;
 BOOL first_comment = TRUE;
 if (!numcomm)
  return;
 if (*curcomm<numcomm)
 {     /* see if we've passed a comment */
  while(((line-1)>commarray[*curcomm].comline+numoperands)&&(*curcomm<numcomm))
  {
   printf("Error: missed a comment at %X, line=%X\n", commarray[*curcomm].comline,line);
   (*curcomm)++;
  }
  while ((((line-1)+numoperands)>=commarray[*curcomm].comline)||
 ((line==commarray[*curcomm].comline)&&(numoperands==1))||
 (((line-1)==commarray[*curcomm].comline)&&(numoperands==0)))
  {
   if (*curcomm>=numcomm)
    break;
   if(first_comment!=TRUE)
   {
    printf("%s\n",out);
    out[0]='\0';
   }
   for(sp=strlen(out);sp<COMMTAB;sp++)
    strcat(out," ");
   strcat(out,";");
   strcat(out,commarray[*curcomm].comment);
   (*curcomm)++;
   first_comment=FALSE;
  }
 }
}

void docommline(int line,int *curcommline,int numcommline)
{
 BOOL didone=FALSE;
 if(numcommline>0)
 {
  if(*curcommline<numcommline)
  { /* see if we've passed a comment line*/
   while(((line-1)>commlinearray[*curcommline].comline)&&(*curcommline<numcommline))
   {
    printf("Error: missed a comment line at %X, line=%X\n", commlinearray[*curcommline].comline,line);
    (*curcommline)++;
   }
   while(((line-1)==commlinearray[*curcommline].comline)&&(*curcommline<numcommline))
   {
    if (!didone)
     printf("\n");
    printf(";   %s\n",commlinearray[*curcommline].comment);
    (*curcommline)++;
    didone=TRUE;
   }
  }
 }
}

BOOL newl=FALSE;
char wchars[15]={"<=-?!()',./&\":"};

char wtext(int data)
{
 if(data>127)
 {
  data-=128;
  newl=TRUE;
 }
 else
  newl=FALSE;
 if(data<10)
  data+=48;
 else if(data==10)
  data=32;
 else if(data<37)
  data+=54;
 else if(data<51)
   data=(int)wchars[data-37];
 else
  data=39;
 return((char)data);
}

BOOL diddata=FALSE;

int dumpdata(int opcode, int *curdata, int numdata, FILE *fp, int *curcomm, int numcomm, int *curcommline, int numcommline)
{
 int pnum, tnum;
 int numoperands;
 int k;
 int numchars = 0;
 int sp;
 numoperands = 2+dataarray[*curdata].end - count;
 pnum = dataarray[*curdata].per_line;                                          /* print up to pnum bytes data */
 if ((dataarray[*curdata].type == DATA) || ((dataarray[*curdata].type == WTEXT)&&(as9_comp == YES)) )
 {
  if (as9_comp == YES)
  {
   for (sp = 0; sp < OPNAMETAB-1; sp++)
   {
    sprintf(out2," ");
    strcat(out, out2);
   }
   sprintf(out2,"FCB ");
   strcat(out, out2);
   sprintf(out2, " $%02X", (UBYTE) opcode);
  }
  else
   sprintf(out2, "%02X ", (UBYTE) opcode);
  strcat(out, out2);
  if (as9_comp == NO)
  {
   if ((pnum < 1) || (pnum > DATA_MAX_LINE_DEFAULT))
   {
    pnum = DATA_PER_LINE_DEFAULT;           /* no more than 24 bytes hex data */
   }
  }
  else
  {
   if ((pnum < 1) || (pnum > DATA_MAX_LINE_COMPATIBLE))
   {
    pnum = DATA_PER_LINE_COMPATIBLE;              /* no more than 24 bytes hex data */
   }
  }
 }
 else
 {
  if (as9_comp == YES)
  {
   for (sp = 0; sp < OPNAMETAB-1; sp++)
   {
    sprintf(out2," ");
    strcat(out, out2);
   }
   sprintf(out2,".ascii \""); /* delimiter ist '"' */
   strcat(out, out2);
   out2[0] = opcode;
   out2[1] = '\0';
   strcat(out, out2);
  }
  else
  {
   if (dataarray[*curdata].type == ASCII)
    out2[0] = opcode;
   else
    out2[0] = wtext(opcode);
   out2[1] = '\0';
   strcat(out, out2);
  }
  if (as9_comp == NO)
  {
   if ((pnum < 1) || (pnum > ASCII_MAX_LINE_DEFAULT))
   {
    pnum = ASCII_PER_LINE_DEFAULT;            /* no more than 24 bytes hex data */
   }
  }
  else
  {
   if ((pnum < 1) || (pnum > ASCII_MAX_LINE_COMPATIBLE))
   {
    pnum = ASCII_PER_LINE_COMPATIBLE;               /* no more than 24 bytes hex data */
   }
  }
 }
 newl = FALSE;
 tnum = (pnum > numoperands) ? numoperands - 2 : pnum - 1;
 for (k = 0; k < numoperands - 1; k++)
 {                                                                             /* print the data */
  if ((++ numchars >= pnum) || newl)
  {
   if ((tnum) || (pnum == 1))
   {
    docomment(count - tnum, curcomm, numcomm, tnum);
    docommline(count - tnum, curcommline, numcommline);
   }
   if ((as9_comp == NO)|| ((dataarray[*curdata].type == DATA) || ((dataarray[*curdata].type == WTEXT)&&(as9_comp == YES)) ))
    printf("%s\n", out);
   else
    printf("%s\"\n", out);
   if (as9_comp == NO)
    sprintf(out, "%04X: ", count);
   else
    sprintf(out, " ", count);
   if (as9_comp == YES)
   {
    for (sp = 0; sp < OPNAMETAB-1; sp++)
    {
     sprintf(out2," ");
     strcat(out, out2);
    }
    if ((dataarray[*curdata].type == DATA) || ((dataarray[*curdata].type == WTEXT)&&(as9_comp == YES)) )
     sprintf(out2,"FCB ");
    else
     sprintf(out2,".ascii \""); /* delimiter ist '"' */
    strcat(out, out2);
   }
   numchars = 0;
   newl = FALSE;
  }
  if ((dataarray[*curdata].type == DATA) || ((dataarray[*curdata].type == WTEXT)&&(as9_comp == YES)) )
  {
   if (as9_comp == YES)
   {
    if (numchars==0)
     sprintf(out2, " $%02X", getbyte(fp));                                        /* hex */
    else
     sprintf(out2, ", $%02X", getbyte(fp));                                        /* hex */
   }
   else
    sprintf(out2, "%02X ", getbyte(fp));                                        /* hex */
  }
  else
  {
   if (as9_comp == YES)
   {
    out2[0] = getbyte(fp);             /* ASCII */
    out2[1] = '\0';
   }
   else
   {
    if (dataarray[*curdata].type == ASCII)
     out2[0] = getbyte(fp);       /* ASCII */
    else
     out2[0] = wtext(getbyte(fp));      /* text */
    out2[1] = '\0';
   }
  }
  strcat(out, out2);
 }
 if ((dataarray[*curdata].type == ASCII) && (as9_comp == YES))
 {
  out2[0] = '\"';
  out2[1] = '\0';
  strcat(out, out2);
 }
 if (*curdata < numdata)
  (*curdata)++;
 diddata = TRUE;
 return (numoperands - 2);
}

void freearrays(void)
{
 if (labarray)
  free(labarray);
 if (dataarray)
   free(dataarray);
 if (commarray)
  free(commarray);
 if (commlinearray)
  free(commlinearray);
}

BOOL mallocarrays(void)
{
 BOOL gotit=FALSE;
 if(labarray=(struct lastruct *) malloc(sizeof(struct lastruct)*MAXLABEL))
  if (dataarray=(struct dastruct *) malloc(sizeof(struct dastruct)*MAXDATA))
   if(commarray=(struct castruct *) malloc(sizeof(struct castruct)*MAXCOMMENT))
    if(commlinearray=(struct castruct *) malloc(sizeof(struct castruct)*MAXCOMMLINE))
     gotit=TRUE;
 if(!gotit)
  freearrays();
 return(gotit);
}

char *fullequaddr1="%s";
char *shortequaddr1="%s";
char *fullequaddr2="0x%04X\n";
char *shortequaddr2="0x%02X\n";


long filesize(FILE *stream)
{
 long curpos, length;
 curpos = ftell(stream);
 fseek(stream, 0L, SEEK_END);
 length = ftell(stream);
 fseek(stream, curpos, SEEK_SET);
 return length;
}

void show_inctruction(void)
{
 printf("-h   Help\n");
 printf("-H   Help\n");
 printf("-?   Help\n");
 printf("-ifile_in input filename\n");
 printf("-dfile_dat data filename (optional)\n");
 printf("-c asxxxx compatability (default not compatible)\n");
 printf("-l print line numbers as labels (compatabilty modus)\n");
 printf("-r print line numbers as remarks (compatabilty modus)\n");
 printf("\"list\" show all instructions\n");
}

void main(int argc,char *argv[])
{
 int i,j,k;
 int opcode,page;
 UBYTE operand[4];
 FILE *fp;
 int org=~0;
 opcodeinfo *op;
 int numoperands;
 int sp;
 int numcomm=0,numdata=0,numcommline=0;
 int curcomm=0,curdata=0,curcommline=0;
 int labellinenumber=NO;
 int commentlinenumber=NO;
 long file_len=0;
 long file_act_read=0;
 char *printaddr1;
 char *printaddr2;
 char in_file[128];
 char dat_file[128];
 file_act_read=0;
 in_file[0] = 0;
 dat_file[0] = 0;
 if(argc>1)
 {
  int arg_counter = 1;
  while (argv[arg_counter])
  {
   char *pointer = argv[arg_counter];
   if ((pointer[0] == '-') || (pointer[0] == '/') || (pointer[0] == ':') || ( pointer[0] == '\\'))
   {
    if ((pointer[1] == 'h') || (pointer[1] == 'H') || (pointer[1] == '?'))
    {
     show_inctruction();
     exit(0);
    }
    if ((pointer[1] == 'i') || (pointer[1] == 'I'))
    {
     strcpy(in_file, pointer + 2);
    }
    if ((pointer[1] == 'd') || (pointer[1] == 'D'))
    {
     strcpy(dat_file, pointer + 2);
    }
    if ((pointer[1] == 'c') || (pointer[1] == 'C'))
    {
     as9_comp = YES;
    }
    if ((pointer[1] == 'l') || (pointer[1] == 'L'))
    {
     labellinenumber=YES;
    }
    if ((pointer[1] == 'r') || (pointer[1] == 'R'))
    {
     commentlinenumber=YES;
    }
   }
   if(!stricmp(argv[arg_counter],"list"))
   {    /* show all instructions */
    for(i=0;i<numops[0];i++)
     printf("opcode %02X, operands %d, name %6s, mode %s, cycles %d\n",
     pg1opcodes[i].opcode,pg1opcodes[i].numoperands,
     pg1opcodes[i].name,
     modenames[pg1opcodes[i].mode],pg1opcodes[i].numcycles);
    for(i=0;i<numops[1];i++)
     printf("opcode 10 %02X, operands %d, name %6s, mode %s, cycles %d\n",
     pg2opcodes[i].opcode,pg2opcodes[i].numoperands,
     pg2opcodes[i].name,
     modenames[pg2opcodes[i].mode],pg2opcodes[i].numcycles);
    for(i=0;i<numops[2];i++)
     printf("opcode 11 %02X, operands %d, name %6s, mode %s, cycles %d\n",
     pg3opcodes[i].opcode,pg3opcodes[i].numoperands,
     pg3opcodes[i].name,
     modenames[pg3opcodes[i].mode],pg3opcodes[i].numcycles);
    exit(0);
   }
   arg_counter++;
  }
 }
 else /* test for number of args */
 {
  show_inctruction();
  exit (0);
 }
 if (!mallocarrays())
 {
  printf("Can't get memory for arrays\n");
  exit(2);
 }
 if (dat_file[0] != 0)
  readdatafile(dat_file, &org, &numlab, &numdata, &numcomm, &numcommline);
 if (org > - 1)              /* int PC to ORG val or 0 */
  count = org;
 else
  count = 0;
 for(k=0;k<numlab;k++)
 {         /* print labels first */
  if (labarray[k].lab<256)
  {
   printaddr1=shortequaddr1;
   printaddr2=shortequaddr2;
  }
  else
  {
   printaddr1=fullequaddr1;
   printaddr2=fullequaddr2;
  }
  printf(printaddr1,labarray[k].label);
  for (i=0;i<30-strlen(labarray[k].label);i++)
   printf(" ");
  printf("=  ");
  printf(printaddr2,labarray[k].lab);
 }
 if (numlab)
  printf("\n");
 if (as9_comp == YES)
 {
  for (sp = 0; sp < OPNAMETAB; sp++)
   printf(" ");
  /* printf("\n.area CODE (ABS)\n",count); */
  printf("ORG $%04X\n",count);
 }
 if (fp = fopen(in_file, "rb"))
 {                  /* open binary file */
  /*
  file_len = filesize(fp);
  */
  while((opcode=getbyte(fp))!=EOF)
  {
   long this_line_adress=count-1;
   op=NULL;
   docommline(count,&curcommline,numcommline);
   printf(checklabs(count-1,TRUE,FALSE,FALSE)); /* if add lab, print */
   if (as9_comp == NO)
    sprintf(out, "%04X: ", count - 1);
   else
   {
    if (labellinenumber==YES)
     sprintf(out, "_%X: ", count - 1);
    else
     sprintf(out, " ", count - 1);
   }
   if(numdata)
   {
    while(((count-1)>dataarray[curdata].end)&&(curdata<numdata))
    {
     printf("Error: missed a data line, start=%X, end=%X\n",dataarray[curdata].start,dataarray[curdata].end);
     curdata++;
    }
   }
   if (numdata&& ((count-1)>=dataarray[curdata].start)&&  /* data? */
      ((count-1)<=dataarray[curdata].end))
   {
    numoperands=dumpdata(opcode,&curdata,numdata,fp,&curcomm,numcomm,&curcommline,numcommline);
    i=numops[0]+1;        /* skip decoding as an opcode */
   }
   else
   {   /* not data - search for opcode */
    sprintf(out2,"%02X ",(UBYTE)opcode);
    if (as9_comp == NO)
    {
     /* hex of opcode */
     strcat(out,out2);
    }
    for(i=0;(i<numops[0])&&(pg1opcodes[i].opcode!=opcode);i++)
     ;
    if(i<numops[0])
    {    /* opcode found */
     if(pg1opcodes[i].mode>=PG2)
     {     /* page switch */
      int helper=getbyte(fp);
      if ((helper==EOF)&&(as9_comp == YES))
      {
       for(sp=strlen(out);sp<OPNAMETAB;sp++)
       strcat(out," ");
       sprintf(out2,"FCB   ");
       strcat(out, out2);
       sprintf(out2, " 0x%02X    File end, can't continue opcode", (UBYTE) opcode);
       strcat(out, out2);
       printf("%s\n",out);
       break;
      }
      opcode=helper;
      sprintf(out2,"%02X ",(UBYTE)opcode);
      if (as9_comp == NO)
      {
       /* hex of other page opcode */
       strcat(out,out2);
      }
      page=pg1opcodes[i].mode-PG2+1;       /* get page # */
      for(k=0;(k<numops[page])&&(opcode!=pgpointers[page][k].opcode);k++)
       ;
      if(k!=numops[page])
      {   /* opcode found */
       op=(opcodeinfo *) &(pgpointers[page][k]);
       numoperands=pgpointers[page][k].numoperands;
       for(j=0;j<numoperands-1;j++)
       {
        int helper;
        sprintf(out2,"%02X ",(helper=getbyte(fp)));
        if (as9_comp == NO)
        {
         /* hex of operands */
         strcat(out,out2);
        }
        operand[j]=helper;
        if ((helper==EOF)&&(as9_comp == YES))
        {
         {
          for(sp=strlen(out);sp<OPNAMETAB;sp++)
          strcat(out," ");
          sprintf(out2,"FCB     ");
          strcat(out, out2);
          sprintf(out2, " 0x%02X", (UBYTE) page+15);
          strcat(out, out2);
          sprintf(out2, ", 0x%02X", (UBYTE) opcode);
          strcat(out, out2);
          for (i=0;i<j;i++)
          {
           sprintf(out2, ", 0x%02X" , (UBYTE) operand[i]);
           strcat(out, out2);
          }
          sprintf(out2, ";    File end, can't continue opcode");
          strcat(out, out2);
         }
         printf("%s\n",out);
         break;
        }
       }
       if (j!=numoperands-1)
       break;
       if(pgpointers[page][k].mode!=IND)
       {
        for(sp=strlen(out);sp<OPNAMETAB;sp++)
         strcat(out," ");
        sprintf(out2,"%s",pgpointers[page][k].name);
        strcat(out,out2);
        for(sp=strlen(out);sp<OPERANDTAB;sp++)
         strcat(out," ");
       }
       printoperands(opcode,numoperands-1,operand,pgpointers[page][k].mode,fp,pgpointers[page][k].name,out,page);
      }
      else
      {        /* not found in alternate page */
       for(sp=strlen(out);sp<OPNAMETAB;sp++)
        strcat(out," ");
       if (as9_comp == NO)
       {
        strcat(out, "Illegal Opcode, changing to data");
       }
       else
       {
        sprintf(out2,"FCB     ");
        strcat(out, out2);
        if (page==1)
         strcat(out, "$10");
        else
         strcat(out, "$11");
        printf("; Illegal Opcode on page %i: 0x%02X, changing to data\n", page+1,(UBYTE) opcode);
        sprintf(out2, ", 0x%02X ", (UBYTE) opcode);
        strcat(out, out2);
       }
      }
     }
     else
     {          /* page 1 opcode */
      op=(opcodeinfo *) &(pg1opcodes[i]);
      numoperands=pg1opcodes[i].numoperands;
      for(j=0;j<numoperands;j++)
      {
       int helper;
       sprintf(out2,"%02X ",(helper=getbyte(fp)));
       if (as9_comp == NO)
       {
        strcat(out,out2);
       }
       operand[j]=helper;
       if ((helper==EOF)&&(as9_comp == YES))
       {
        {
         for(sp=strlen(out);sp<OPNAMETAB;sp++)
         strcat(out," ");
         sprintf(out2,"FCB     ");
         strcat(out, out2);
         sprintf(out2, " 0x%02X", (UBYTE) opcode);
         strcat(out, out2);
         for (i=0;i<j;i++)
         {
          sprintf(out2, ", $%02X" , (UBYTE) operand[i]);
          strcat(out, out2);
         }
         sprintf(out2, ";    File end, can't continue opcode");
         strcat(out, out2);
        }
        printf("%s\n",out);
        break;
       }
      }
      if (j!=numoperands)
       break;
      if (  (  ( (opcode==0x1f) || (opcode==0x1e) ) && (as9_comp == YES) )
         &&
           (  (   (!(stricmp(teregs[(operand[0]>>4)&0xf],"inv")))
        || (!(stricmp(teregs[operand[0]&0xf],"inv"))))
            || (teregssize[(operand[0]>>4)&0xf]-teregssize[operand[0]&0xf])))
       {
        /* test if register codes are right */
        /* illegal register */
        for(sp=strlen(out);sp<OPNAMETAB;sp++)
         strcat(out," ");
        sprintf(out2,"FCB     ");
        strcat(out, out2);
        sprintf(out2, " $%02X", (UBYTE) opcode);
        strcat(out, out2);
        for (i=0;i<numoperands;i++)
        {
         sprintf(out2, ", $%02X" , (UBYTE) operand[i]);
         strcat(out, out2);
        }
        printf("; Illegal Opcode (illegal register code EXG/TFR), changing to data\n");
      }
      else if (((opcode==0x34)||(opcode==0x35)||(opcode==0x36)||(opcode==0x37))&&(operand[0]==0))
      {
       /* test if push/pull register codes are right */
       /* illegal register */
       for(sp=strlen(out);sp<OPNAMETAB;sp++)
        strcat(out," ");
       sprintf(out2,"FCB     ");
       strcat(out, out2);
       sprintf(out2, " $%02X", (UBYTE) opcode);
       strcat(out, out2);
       for (i=0;i<numoperands;i++)
       {
        sprintf(out2, ", $%02X" , (UBYTE) operand[i]);
        strcat(out, out2);
       }
       printf("; Illegal Opcode (illegal register code PUSH/PULL), changing to data\n");
      }
      else
      {
       if (pg1opcodes[i].mode!=IND)
       {
        for(sp=strlen(out);sp<OPNAMETAB;sp++)
         strcat(out," ");
        sprintf(out2,"%s",pg1opcodes[i].name);
        strcat(out,out2);
        for(sp=strlen(out);sp<OPERANDTAB;sp++)
         strcat(out," ");
       }
       printoperands(opcode,numoperands,operand, pg1opcodes[i].mode,fp,pg1opcodes[i].name,out,0);
      }
     }
    }
    else if(i==numops[0])
    {   /* not found in page 1 */
     for(sp=strlen(out);sp<OPNAMETAB;sp++)
      strcat(out," ");
     if (as9_comp == NO)
     {
      strcat(out, "Illegal Opcode, changing to data");
     }
     else
     {
      sprintf(out2,"FCB     ");
      strcat(out, out2);
      sprintf(out2, " $%02X ", (UBYTE) opcode);
      printf("; Illegal Opcode, changing to data\n");
      strcat(out, out2);
     }
    }
   }
   if ((as9_comp==YES)&&(commentlinenumber==YES))
   {
    for(sp=strlen(out);sp<LINENUMTAB;sp++)
     strcat(out," ");
    sprintf(out2, " <$%04X>", this_line_adress);
    strcat(out,out2);
   }
   docomment(count-1-numoperands,&curcomm,numcomm,numoperands+1);
   if(op)
   {
    if ((!stricmp(op->name,"bra"))||   /* extra space - branch */
       (!stricmp(op->name,"lbra"))||
       (!stricmp(op->name,"rts"))||
       (!stricmp(op->name,"jmp"))||
       (!stricmp(op->name,"rti"))||
       (!strnicmp(op->name,"pul",3)&&PC)||  /* PUL? PC=RTS */
       (!stricmp(op->name,"wai")))
    {
     if(strlen(out)&&(out[strlen(out)-1]!='\n'))
      strcat(out,"\n");
    }
   }
   if (diddata)
   {
    if(strlen(out)&&(out[strlen(out)-1]!='\n'))
     strcat(out,"\n");
   }
   printf("%s\n",out);
   PC=FALSE;
   diddata=FALSE;
  }
 }
 else
 {
  fprintf(stderr,"Can't open file '%s'\n",argv[1]);
  freearrays();
  exit(3);
 }
 fclose(fp);
 freearrays();
}
