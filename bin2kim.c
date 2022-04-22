//--------------------------------------------------------------------------
// bin2kim
//
// Fabio Sturman
//
// fabio.sturman@gmail.com
//
// 21/04/22 20:55:37
//
// convert binary files to kim format
//
//  bin2kim <file_in> [<offset_hex>]
//    output to stdout
//    default offset 0x200
//--------------------------------------------------------------------------
/*
The paper tape LOAD and DUMP routines store and retrieve data in
a specific format designed to insure error free recovery.  Each byte
of data to be stored is converted to two half bytes.  The half bytes
(whose possible values are 0 to FHEX) are translated into their ASCII
equivalents and written out onto paper tape in this form.

     Each record outputted begins with a ";" character (ASCII 3B) to
mark the start of a valid record.  The next byte transmitted (18HEX) or
(2410) is the number of data bytes contained in the record.  The record's
starting address High (1 byte, 2 characters), starting address Lo (1 byte,
2 characters), and data (24 bytes, 48 characters) follow.  Each record is
terminated by the record's check-sum (2 bytes, 4 characters), a carriage
return (ASCII OD), line feed (ASCII øA), and six "NULL" characters
(ASCII øø).

     The last record transmitted has zero data bytes (indicated by ;00)
The starting address field is replaced by a four digit Hex number repre-
senting the total number of data records contained in the transmission,
followed by the records usual check-sum digits.  A "XOFF" character ends
the transmission.

  len  adl adh data                                                  chk
; 18   00  00  FF EE DD CC BBAA009988776655443322112233445566778899  0AFC
  len  num_of_data_records  chk
; 00  0001                  0001



History
The KIM-1 single-board computer specified a file format for magnetic tape and a
format for paper tape. The paper tape format was adapted slightly and has been
used to interchange files for computers based on the MOS Technology 6502
microprocessor.

The open-source Srecord package simplified this tape format by eliminating the
<NUL> and XOFF characters.

Format
Each record begins with a semicolon (;), followed by two hexadecimal digits
denoting the length of the data in the record. The next two bytes represent
the starting address of the data, in big-endian (most-significant byte first)
hexadecimal. Up to 24 bytes of data follow. Then, there is a 2-byte
(4-character) checksum: the sum of the other non-; data in the record.
Finally, a record ends with a carriage return (<CR>), a line break (<LF>),
and six null characters (<NUL>).

The last record on the paper tape is empty (its length field is 0000), with the
starting address field representing the total number of data bytes contained in
the transmission. The file ends with a XOFF.

;0A03B02041626F7274696E67240437


;03 12EE 4C 49 42 01DA
all in chk

;08  05 70  30 30 30 31 0D 0A 0D 0A  016C


;18  02 00  38 AD 04 01 E9 1D 29 80 85 08 AD 05 01 E9 04 85 09 
38 A5 08 E9 00 85 08  07 C9
all (-;) in chk
*/

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <unistd.h>
#include <getopt.h>

int getrec(unsigned char buf[], int max_len, FILE *f)
{
    int i;
    i=0;
    if(feof(f)) return -1;
    while(!feof(f) && i<max_len) buf[i++]=fgetc(f);
    if(feof(f)) i--;
    return(i);
}

int main(int argc, char *argv[])
{
  FILE *f;
  int tofs, ofs, n, i, eof, chk, reclen, ndat;
  unsigned char buf[32];
  ofs=0x200;
  if(argc<2)
  {
    printf("bin2kim <file_in> [<offset_hex>]\n");
    printf("  output to stdout\n");
    printf("  default offset 0x200\n");
    return 1;
  }
  if(argc==3) sscanf(argv[2],"%X",&ofs);
  f=fopen(argv[1],"r");
  if(f==NULL)
  {
    printf("error: %s file not found\n",argv[2]);
    return 2;
  }
  reclen=0x18;
  ndat=0;
  tofs=ofs;
  eof=0;
  while(!eof)
  {
    n=getrec(buf,reclen,f);
    if(n==-1) eof=1;
    else
    {
      chk=0;
      putchar(';');
      printf("%02X",n&0xff);
      chk+=(n&0xff);
      printf("%04X",tofs);
      chk+=(tofs&0xff);
      chk+=(tofs/256)&0xff;
      for(i=0;i<n;i++)
      {
        chk+=buf[i]&0xff;
        printf("%02X",buf[i]);
      }
      printf("%04X\r\n",chk&0xffff);
      tofs+=n;
      ndat+=n;
    }
  }
  //if(ndat>0) printf(";00%04X%04X\n",ndat,ndat);  // last record
  return 0;
}

