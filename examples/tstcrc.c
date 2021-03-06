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
	
			
	#define WHITESPACE 64
	#define EQUALS     65
	#define INVALID    66

	static const unsigned char d[] = {
		66,66,66,66,66,66,66,66,66,66,64,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,62,66,66,66,63,52,53,
		54,55,56,57,58,59,60,61,66,66,66,65,66,66,66, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9,
		10,11,12,13,14,15,16,17,18,19,20,21,22,23,24,25,66,66,66,66,66,66,26,27,28,
		29,30,31,32,33,34,35,36,37,38,39,40,41,42,43,44,45,46,47,48,49,50,51,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,66,
		66,66,66,66,66,66
	};

	int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen) { 
		char *end = in + inLen;
		char iter = 0;
		size_t buf2 = 0;
		size_t len = 0;
		
		while (in < end) {
			unsigned char c = d[(int)(*in)];
			in++;
			//printf("%c",c);
			switch (c) {
			case WHITESPACE: continue;   /* skip whitespace */
			case INVALID:    return 1;   /* invalid input, return error */
			case EQUALS:                 /* pad character, end of data */
				in = end;
				continue;
			default:
				buf2 = buf2 << 6 | c;
				iter++; // increment the number of iteration
				/* If the buffer is full, split it into bytes */
				if (iter == 4) {
					if ((len += 3) > *outLen) return 99; /* buffer overflow */
					*(out++) = (buf2 >> 16) & 255;
					*(out++) = (buf2 >> 8) & 255;
					*(out++) = buf2 & 255;
					buf2 = 0; iter = 0;

				}   
			}
		}
	   
		if (iter == 3) {
			if ((len += 2) > *outLen) return 1; /* buffer overflow */
			*(out++) = (buf2 >> 10) & 255;
			*(out++) = (buf2 >> 2) & 255;
		}
		else if (iter == 2) {
			if (++len > *outLen) return 1; /* buffer overflow */
			*(out++) = (buf2 >> 4) & 255;
		}

		*outLen = len; /* modify to reflect the actual output size */
		return 0;
	}
				
	

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
		do_hex = false;



		if (do_ascii) {

			ptr = (unsigned char *)input_string;
			while (*ptr) ptr++;
			*ptr = 0;
		}

		if (do_hex) {

			ptr = (unsigned char *)input_string;
			dest = (unsigned char *)input_string;

			while (*ptr) {

				if (*ptr >= '0'  &&  *ptr <= '9') *dest++ = (unsigned char)((*ptr) - '0');
				if (*ptr >= 'A'  &&  *ptr <= 'F') *dest++ = (unsigned char)((*ptr) - 'A' + 10);
				if (*ptr >= 'a'  &&  *ptr <= 'f') *dest++ = (unsigned char)((*ptr) - 'a' + 10);

				ptr++;
			}

			*dest = '\x80';
			*(dest + 1) = '\x80';
		}




		crc_32_val = 0xffffffffL;



		if (do_ascii) {
			ptr = (unsigned char *)input_string;

			int i = 0;
			for (i = 0; i < length; i++) {

				crc_32_val = update_crc_32(crc_32_val, *ptr);

				ptr++;
			}
		}
		crc_32_val ^= 0xffffffffL;
		//printf(input_string);
		//printf("\n%d", sizeof(input_string));
		//printf("\n%d\n", length);
		//printf("CRC32 = %08" PRIX32, crc_32_val);
		//printf("CRC32 = 0x%08" PRIX32 "  /  %" PRIu32 "\n", crc_32_val, crc_32_val);


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





		char bufsm2[8] = {0};
		char buf[1056 + 12] = {0};
		char smallbuf[8] = {0};
		char crc[18] = {0};
		char crcPassedIn[8] = {0};
		char crcsmall[8] = {0};
		char setCount[12] = {0};
		char prevSetCount[12] = {0};

		
		//printf(" [1] ");
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
			//printf(" [2] ");
			rdlensm2 = read(fd, bufsm2, sizeof(bufsm2));
			if (rdlensm2 < 1)
			{
			}
		}
		int totalBytes = 0;


		
		int first = 1;

		
		
		while (1)
		{
			
			//printf(" [3] ");
			//printf("\n");
			//buf[sizeof(buf) - 26] = ' ';
			//buf[sizeof(buf) - 25] = ' ';
			//buf[sizeof(buf) - 24] = ' ';

			buf[0] = ' ';
			buf[1] = ' ';
			buf[2] = ' ';
			buf[3] = ' ';		
			
			char bufsm[6] = "";
			
			unsigned int rdlensm = read(fd, bufsm, sizeof(bufsm));
			if (rdlensm < 1)
			{
			}
			

			int tries = 0;
			while ((strncmp("UklGRn", bufsm, 6) != 0) &&
				(strncmp("klGRn7", bufsm, 6) != 0) &&
				(strncmp("lGRn7I", bufsm, 6) != 0) &&
				(strncmp("GRn7IJ", bufsm, 6) != 0) &&
				(strncmp("Rn7IJA", bufsm, 6) != 0) &&
				(strncmp("n7IJA1", bufsm, 6) != 0))
			{

				rdlensm = read(fd, bufsm, sizeof(bufsm));
				//printf("%s\n",bufsm);
				if (rdlensm < 1)
				{
				}
				tries += 1;
				if (tries == 100)
				{
					//printf(" ALLTRIES\n ");
					//printf("00000000 CRC ERROR\n");
					wlen = write(fd, "00000000 CRC ERROR", 18);
					tcdrain(fd);    /* delay for output */
					wlen = write(fd, "%READY%", 7);
					tcdrain(fd);
					tries = 0;
				}

			}
			
			// 6 bytes read already
			// read up to the full 11 bytes UklGRn7IJA1

			int read_bytes;

			if ((strncmp("UklGRn", bufsm, 6) == 0)) {
				read_bytes = read(fd, bufsm, 5);
				if (read_bytes < 1)
				{
				}
			}
			else if ((strncmp("klGRn7", bufsm, 6) == 0)) {
				read_bytes = read(fd, bufsm, 4);
				if (read_bytes < 1)
				{
				}
			}
			else if ((strncmp("lGRn7I", bufsm, 6) == 0)) {
				read_bytes = read(fd, bufsm, 3);
				if (read_bytes < 1)
				{
				}
			}
			else if ((strncmp("GRn7IJ", bufsm, 6) == 0)) {
				read_bytes = read(fd, bufsm, 2);
				if (read_bytes < 1)
				{
				}
			}
			else if ((strncmp("Rn7IJA", bufsm, 6) == 0)) {
				read_bytes = read(fd, bufsm, 1);
				if (read_bytes < 1)
				{
				}
			}
			
			// read another 5 bytes to make up to 16
			read_bytes = read(fd, bufsm, 5);						
			
			while (1)
			{


				/* simple noncanonical input */
				int i = 0, j = 0;
				for (i = 0; i < (int)sizeof(buf); i++)
				{
					buf[i] = '-';
				}
				i = 0;
				
				if (first){
					first = 0;
					buf[0] = 'U';
					buf[1] = 'k';
					buf[2] = 'l';
					buf[3] = 'G';
					buf[4] = 'R';
					buf[5] = 'n';
					buf[6] = '7';
					buf[7] = 'I';
					buf[8] = 'J';
					buf[9] = 'A';
					buf[10] = '1';
					buf[11] = 'B';
					buf[12] = 'V';
					buf[13] = 'k';
					buf[14] = 'k';
					buf[15] = 'g';
					
					i = 16;
				}


				int readEnough = 0, crcString = 0;
				int crccheckcount = 0;

				while (!readEnough)
				{			
					
					int rdlen;
					
					for (j = 0; j < (int)sizeof(smallbuf); j++) {
						smallbuf[j] = '-';
					}
					j = 0;

					rdlen = read(fd, smallbuf, (int)sizeof(smallbuf));
					if (rdlen > 0) {

						for (j = 0; j < rdlen; j++) {
							buf[i + j] = smallbuf[j];
						}

						// issue: want to increase the small buffer
						if ((strncmp("%IGNORE%", smallbuf, 8) == 0) ||
							(strncmp("IGNORE%%", smallbuf, 8) == 0) ||
							(strncmp("GNORE%%I", smallbuf, 8) == 0) ||
							(strncmp("NORE%%IG", smallbuf, 8) == 0) ||
							(strncmp("ORE%%IGN", smallbuf, 8) == 0) ||
							(strncmp("RE%%IGNO", smallbuf, 8) == 0) ||
							(strncmp("E%%IGNOR", smallbuf, 8) == 0) ||
							(strncmp("%%IGNORE", smallbuf, 8) == 0))
						{
							//printf("b");
							i = 0;
							break;
						}
						else
						{

							// get to the starting position
							
							i += rdlen;

							
							if (i >= ((int)(sizeof(buf))))
							{
								//printf("\nREADENOUGH\n");
								//printf(buf);

								if ((char)buf[sizeof(buf) - 12 - 12] == 'C' && (char)buf[sizeof(buf) - 11 - 12] == 'R' && (char)buf[sizeof(buf) - 10 - 12] == 'C')
								{
									crccheckcount++;
									if (crccheckcount == 3){
										crcString = 1;
										readEnough = 1;
									}									
									//printf("2\n");
								}
								else
								{
									i = 0;
									break;									
								}
								
							}
							//printf("0");
						}
						//buf[rdlen] = 0;

					}
					else if (rdlen < 0) {
						printf("Error from read: %d: %s\n", rdlen, strerror(errno));
					}
					/* repeat read to get full message */
				}

				if (crcString)
				{
					totalBytes += sizeof(buf);
					
					//if (!first){ minus3 = 16;}
					uint32_t crcResult = calc_crc32(buf, sizeof(buf) - 12 - 12);
					

					//printf("\n%d\n", sizeof(buf) - 26);
					//printf("%d", crcResult);

					//write crcResult into crcsmall as a string
					sprintf(crcsmall, "%08lX", (unsigned long)crcResult);

					crcPassedIn[0] = buf[sizeof(buf) - 8 - 12];
					crcPassedIn[1] = buf[sizeof(buf) - 7 - 12];
					crcPassedIn[2] = buf[sizeof(buf) - 6 - 12];
					crcPassedIn[3] = buf[sizeof(buf) - 5 - 12];
					crcPassedIn[4] = buf[sizeof(buf) - 4 - 12];
					crcPassedIn[5] = buf[sizeof(buf) - 3 - 12];
					crcPassedIn[6] = buf[sizeof(buf) - 2 - 12];
					crcPassedIn[7] = buf[sizeof(buf) - 1 - 12];
					int ok = strncmp(crcsmall, crcPassedIn, 8) == 0;
					if (ok) {

						sprintf(crc, "%08lX CRC OK!!!", (unsigned long)crcResult);
						//printf("THISCRC %s\n", crc);
						wlen = write(fd, crc, 18);
						//minus3 = 15;
						

						//int index = 0;
						//for (index = 0; index< (int)sizeof(buf) - 12; index++)
						//{
							
							//int base64decode (char *in, size_t inLen, unsigned char *out, size_t *outLen)
							unsigned char *out = (unsigned char *)malloc((int)sizeof(buf) + 1);
							size_t sizeout1 = (int)sizeof(buf) + 1;
							size_t * sizeout = &sizeout1;
							size_t len = (size_t)sizeof(buf) - 12 - 12;
							//char* c = (char*)malloc(6*sizeof(char));
							//strcpy(c, "hello\0");
							char buf3[sizeof(buf) - 12 - 12 + 1] = {0};
							int cnt = 0;
 							for (cnt = 0; cnt< (int)sizeof(buf) - 12 - 12; cnt++)
							{
								buf3[cnt] = buf[cnt];
							}
							char * instr = &buf3[0];
							//printf("%s\n",instr);
							int retval = base64decode(instr, len, out, sizeout);

							
							setCount[0] = buf[sizeof(buf) - 12];
							setCount[1] = buf[sizeof(buf) - 11];
							setCount[2] = buf[sizeof(buf) - 10];
							setCount[3] = buf[sizeof(buf) - 9];
							
							if (strncmp(setCount, prevSetCount, 12) != 0)
							{
								if (retval == 0)
								{
									int charcnt = 0;
									for (charcnt = 0; charcnt < sizeout1; charcnt++){
										putchar(out[charcnt]);
									}
								}							
							}
							
							prevSetCount[0] = setCount[0];
							prevSetCount[1] = setCount[1];
							prevSetCount[2] = setCount[2];
							prevSetCount[3] = setCount[3];
							
							
							//printf("%d\n",retval);
							//printf("sizeout:%d\n",*sizeout);
							//printf("%s",out);
							
							//printf("%c",buf[index]);
						//}
					

					}
					else {
						//printf("\nExpected %s, got %s  \n", crcsmall, crcPassedIn);
						wlen = write(fd, "00000000 CRC ERROR", 18);
					}

					tcdrain(fd);    /* delay for output */
					wlen = write(fd, "%READY%", 7);
					tcdrain(fd);
				}
				else
				{
					wlen = write(fd, "00000000 CRC ERROR", 18);
					tcdrain(fd);    /* delay for output */
					wlen = write(fd, "%READY%", 7);
					tcdrain(fd);
				}
			}
		}

		


	}  /* main (tstcrc.c) */

