/* Copyright (c) 2013 Quanta Research Cambridge, Inc
 * Original author John Ankcorn
 *
 * Permission is hereby granted, free of charge, to any person obtaining a
 * copy of this software and associated documentation files (the "Software"),
 * to deal in the Software without restriction, including without limitation
 * the rights to use, copy, modify, merge, publish, distribute, sublicense,
 * and/or sell copies of the Software, and to permit persons to whom the
 * Software is furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included
 * in all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS
 * OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
 * THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
 * FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
 * DEALINGS IN THE SOFTWARE.
 */

IMAGE_HEADER_TABLE     = 0x8c0
PARTITION_HEADER_TABLE = 0x980
DATA_AREA_START        = 0xa80
	.arm
	.data
1:
	b 1b
1:
	b 1b
1:
	b 1b
1:
	b 1b
1:
	b 1b
1:
	b 1b
1:
	b 1b
1:
	b 1b
        /* From TRM, Chapter 6: Boot and Configuration */
	.word 0xaa995566
	.word 0x584c4e58
	.word 0
	.word 0x1010000

	.word DATA_AREA_START
	.word 0       /* fill in with size later */
	.word 0
	.word 0
	.word 0       /* fill in with size later */
	.word 1
	.word 0xfc16a3f8
	.word 0
	.word 0, 0, 0, 0
	.word 0, 0, 0, 0
	.word 0, 0, 0, 0
	.word 0, 0, 0, 0
	.word 0
	.word 0
	.word IMAGE_HEADER_TABLE
	.word PARTITION_HEADER_TABLE
