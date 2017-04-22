/*
 * Library: libcrc
 * File:    examples/tstcrc.c
 * Author:  Lammert Bies
 *
 * This file is licensed under the MIT License as stated below
 *
 * Copyright (c) 1999-2016 Lammert Bies
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in all
 * copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 * SOFTWARE.
 *
 * Description
 * -----------
 * The file tstx_crc.c contains a small sample program which demonstrates the
 * use of the functions for calculating the CRC-CCITT, CRC-16 and CRC-32 values
 * of data. The program calculates the three different CRC's for a file who's
 * name is either provided at the command line, or data typed in right the
 * program has started.
 */

#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <string.h>
#include <termios.h>
#include <unistd.h>



#include "../include/checksum.h"

#define MAX_STRING_SIZE	2048



#define DISPLAY_STRING

int set_interface_attribs(int fd, int speed)
{
        struct termios tty;

        if (tcgetattr(fd, &tty) < 0) {
                printf("Error from tcgetattr: %s\n", strerror(errno));
                return -1;
        }

        cfsetospeed(&tty, (speed_t)speed);
        cfsetispeed(&tty, (speed_t)speed);

        tty.c_cflag |= (CLOCAL | CREAD);    /* ignore modem controls */
        tty.c_cflag &= ~CSIZE;
        tty.c_cflag |= CS8;         /* 8-bit characters */
        tty.c_cflag &= ~PARENB;     /* no parity bit */
        tty.c_cflag &= ~CSTOPB;     /* only need 1 stop bit */
        tty.c_cflag &= ~CRTSCTS;    /* no hardware flowcontrol */

                                                                /* setup for non-canonical mode */
        tty.c_iflag &= ~(IGNBRK | BRKINT | PARMRK | ISTRIP | INLCR | IGNCR | ICRNL | IXON);
        tty.c_lflag &= ~(ECHO | ECHONL | ICANON | ISIG | IEXTEN);
        tty.c_oflag &= ~OPOST;

        /* fetch bytes as they become available */
        tty.c_cc[VMIN] = 1;
        tty.c_cc[VTIME] = 1;

        if (tcsetattr(fd, TCSANOW, &tty) != 0) {
                //printf("Error from tcsetattr: %s\n", strerror(errno));
                return -1;
        }
        return 0;
}

void set_mincount(int fd, int mcount)
{
        struct termios tty;

        if (tcgetattr(fd, &tty) < 0) {
                //printf("Error tcgetattr: %s\n", strerror(errno));
                return;
        }

        tty.c_cc[VMIN] = mcount ? 1 : 0;
        tty.c_cc[VTIME] = 5;        /* half second timer */

        if (tcsetattr(fd, TCSANOW, &tty) < 0)
		{
			//printf("Error tcsetattr: %s\n", strerror(errno));
		}
            
}

uint32_t calc_crc32(char input_string[], int length)
{

 

	unsigned char *ptr;
	unsigned char *dest;

	uint32_t crc_32_val;

	bool do_ascii;
	bool do_hex;


	do_ascii = true;
	do_hex   = false;

	
            
	if ( do_ascii ) {

		ptr = (unsigned char *) input_string;
		while ( *ptr) ptr++;
		*ptr = 0;
	}

	if ( do_hex ) {

		ptr  = (unsigned char *) input_string;
		dest = (unsigned char *) input_string;

		while( *ptr ) {

			if ( *ptr >= '0'  &&  *ptr <= '9' ) *dest++ = (unsigned char) ( (*ptr) - '0'      );
			if ( *ptr >= 'A'  &&  *ptr <= 'F' ) *dest++ = (unsigned char) ( (*ptr) - 'A' + 10 );
			if ( *ptr >= 'a'  &&  *ptr <= 'f' ) *dest++ = (unsigned char) ( (*ptr) - 'a' + 10 );

			ptr++;
		}

		* dest    = '\x80';
		*(dest+1) = '\x80';
	}



	
		crc_32_val = 0xffffffffL;



		if ( do_ascii ) {
			ptr       = (unsigned char *) input_string;

			int i = 0;
			for (i=0; i < length; i++) {

				crc_32_val = update_crc_32(crc_32_val,*ptr);
				
				ptr++;
			}
		}
		crc_32_val ^= 0xffffffffL;
		printf( input_string);
		printf("\n%d",sizeof(input_string));
		printf( "\n%d\n",length);
		printf( "CRC32 = %08" PRIX32, crc_32_val);
		printf( "CRC32 = 0x%08" PRIX32 "  /  %" PRIu32 "\n", crc_32_val, crc_32_val);
        

	return crc_32_val;


}


/*
 * int main( int argc, char *argv[] );
 *
 * The function main() is the entry point of the example program which
 * calculates several CRC values from the contents of files, or data from
 * stdin.
 */

int main(void) {





        const char *portname = "/dev/ttyGS0";
        int fd;
        unsigned int wlen;

        fd = open(portname, O_RDWR | O_NOCTTY | O_SYNC);
        if (fd < 0) {
                printf("Error opening %s: %s\n", portname, strerror(errno));
                return -1;
        }
        /*baudrate 115200, 8 bits, no parity, 1 stop bit */
        set_interface_attribs(fd, B115200);
        //set_mincount(fd, 0);                /* set to pure timed read */




        /* simple output */
        wlen = write(fd, "%READY%", 7);
        if (wlen != 7) {
                printf("Error from write: %d, %d\n", wlen, errno);
        }
        tcdrain(fd);    /* delay for output */





        char bufsm2[8];
        printf(" [1] ");
        unsigned int rdlensm2 = read(fd, bufsm2, sizeof(bufsm2));
        if (rdlensm2 < 1)
        {
        }
        while ((strncmp("%IGNORE%", bufsm2, 8) == 0) ||
                (strncmp("IGNORE%%", bufsm2, 8) == 0) ||
                (strncmp("GNORE%%I", bufsm2, 8) == 0) ||
                (strncmp("NORE%%IG", bufsm2, 8) == 0) ||
                (strncmp("ORE%%IGN", bufsm2, 8) == 0) ||
                (strncmp("RE%%IGNO", bufsm2, 8) == 0) ||
                (strncmp("E%%IGNOR", bufsm2, 8) == 0) ||
                (strncmp("%%IGNORE", bufsm2, 8) == 0))
        {
                printf(" [2] ");
                rdlensm2 = read(fd, bufsm2, sizeof(bufsm2));
                if (rdlensm2 < 1)
                {
                }
        }
        int totalBytes = 0;

        char buf[4096];
        char smallbuf[8];
        char tinybuf[8];

        printf(" [3] ");
        while (1)
        {
			    printf("\n");
                buf[sizeof(buf) - 26] = ' ';
                buf[sizeof(buf) - 25] = ' ';
                buf[sizeof(buf) - 24] = ' ';

                char bufsm[6] = "";
                //printf("4");
                unsigned int rdlensm = read(fd, bufsm, sizeof(bufsm));
                if (rdlensm < 1)
                {
                }
                //printf(bufsm);

                int tries = 0;
                while ((strncmp("!rtppl", bufsm, 6) != 0) &&
                        (strncmp("rtppla", bufsm, 6) != 0) &&
                        (strncmp("tpplay", bufsm, 6) != 0) &&
                        (strncmp("pplay1", bufsm, 6) != 0) &&
                        (strncmp("play1.", bufsm, 6) != 0) &&
                        (strncmp("lay1.0", bufsm, 6) != 0))
                {
//                      printf(bufsm);
//                        printf(" %d ",(strcmp("!rtppl", bufsm)));
//                        printf(" %d ",(strcmp("rtppla", bufsm)));
//                        printf(" %d ",(strcmp("tpplay", bufsm)));
//                        printf(" %d ",(strcmp("pplay1", bufsm)));
//                        printf(" %d ",(strcmp("play1.", bufsm)));
//                        printf(" %d ",(strcmp("lay1.0", bufsm)));

//                        printf("\n");
                        rdlensm = read(fd, bufsm, sizeof(bufsm));
                        if (rdlensm < 1)
                        {
                        }
                        tries += 1;
                        if (tries == 100)
                        {
                                printf(" ALLTRIES ");
                                wlen = write(fd, "00000000 CRC ERROR", 18);
                                tcdrain(fd);    /* delay for output */
                                wlen = write(fd, "%READY%", 7);
                                tcdrain(fd);
                                tries = 0;
                        }

                }

                int read_bytes;

                if ((strncmp("!rtppl", bufsm, 6) == 0)) {
                        read_bytes = read(fd, bufsm, 5);
                        if (read_bytes < 1)
                        {
                        }
                }
                else if ((strncmp("rtppla", bufsm, 6) == 0)) {
                        read_bytes = read(fd, bufsm, 4);
                        if (read_bytes < 1)
                        {
                        }
                }
                else if ((strncmp("tpplay", bufsm, 6) == 0)) {
                        read_bytes = read(fd, bufsm, 3);
                        if (read_bytes < 1)
                        {
                        }
                }
                else if ((strncmp("pplay1", bufsm, 6) == 0)) {
                        read_bytes = read(fd, bufsm, 2);
                        if (read_bytes < 1)
                        {
                        }
                }
                else if ((strncmp("play1.", bufsm, 6) == 0)) {
                        read_bytes = read(fd, bufsm, 1);
                        if (read_bytes < 1)
                        {
                        }
                }


                printf(" [START] ");

                /* simple noncanonical input */
                int i = 0, j = 0;

                int readEnough = 0, crcString = 0;
                while (i < ((int)sizeof(buf) - (int)sizeof(smallbuf)) && !readEnough)
                {
						

                        int rdlen;

                        rdlen = read(fd, smallbuf, (int)sizeof(smallbuf));
                        if (rdlen > 0) {

                                for (j = 0; j<rdlen; j++) {
                                        buf[i + j] = smallbuf[j];
                                }
                                for (j = rdlen; j< (int)sizeof(smallbuf); j++) {
                                        smallbuf[j] = ' ';
                                }

                                // copy first 8 bytes

                                tinybuf[0] = smallbuf[0];
								tinybuf[1] = smallbuf[1];
								tinybuf[2] = smallbuf[2];
								tinybuf[3] = smallbuf[3];
								tinybuf[4] = smallbuf[4];
								tinybuf[5] = smallbuf[5];
								tinybuf[6] = smallbuf[6];
								tinybuf[7] = smallbuf[7];


                                // issue: want to increase the small buffer
                                if ((strncmp("%IGNORE%", tinybuf, 8) == 0) ||
                                        (strncmp("IGNORE%%", tinybuf, 8) == 0) ||
                                        (strncmp("GNORE%%I", tinybuf, 8) == 0) ||
                                        (strncmp("NORE%%IG", tinybuf, 8) == 0) ||
                                        (strncmp("ORE%%IGN", tinybuf, 8) == 0) ||
                                        (strncmp("RE%%IGNO", tinybuf, 8) == 0) ||
                                        (strncmp("E%%IGNOR", tinybuf, 8) == 0) ||
                                        (strncmp("%%IGNORE", tinybuf, 8) == 0))
                                {
                                        printf(" [BREAK] ");
                                        break;
                                }
                                else
                                {
                                        i += rdlen;
                                        //printf("Read %d (%d): \"%s\"\n", rdlen, i, smallbuf);
                                        if (i >= ((int)(sizeof(buf) - 16)))
                                        {
                                                readEnough = 1;
                                                if ((char)buf[sizeof(buf) - 26] == 'C' && (char)buf[sizeof(buf) - 25] == 'R' && (char)buf[sizeof(buf) - 24] == 'C')
                                                {
                                                        crcString = 1;
                                                }
                                        }

                                }


                                //buf[rdlen] = 0;

                        }
                        else if (rdlen < 0) {
                                //printf("Error from read: %d: %s\n", rdlen, strerror(errno));
                        }
                        /* repeat read to get full message */



                }

                if (crcString)
                {
                        totalBytes += sizeof(buf);

                        //printf("(%d) ", totalBytes);
						
			            //printf(buf,(sizeof(buf) - 16));
                        uint32_t crcResult = calc_crc32(buf, sizeof(buf) - 26);
                        printf("\n%d\n",sizeof(buf) - 26);
			printf("%d",crcResult);

                        wlen = write(fd, "00000000 CRC ERROR", 18);
                        tcdrain(fd);    /* delay for output */
                        wlen = write(fd, "%READY%", 7);
                        tcdrain(fd);


                }
                else
                {


                }
        }


}  /* main (tstcrc.c) */
