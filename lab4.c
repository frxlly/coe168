/******************************************************************************
*
* Copyright (C) 2009 - 2014 Xilinx, Inc.  All rights reserved.
*
* Permission is hereby granted, free of charge, to any person obtaining a copy
* of this software and associated documentation files (the "Software"), to deal
* in the Software without restriction, including without limitation the rights
* to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
* copies of the Software, and to permit persons to whom the Software is
* furnished to do so, subject to the following conditions:
*
* The above copyright notice and this permission notice shall be included in
* all copies or substantial portions of the Software.
*
* Use of the Software is limited solely to applications:
* (a) running on a Xilinx device, or
* (b) that interact with a Xilinx device through a bus or interconnect.
*
* THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
* IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
* FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
* XILINX  BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY,
* WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF
* OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
* SOFTWARE.
*
* Except as contained in this notice, the name of the Xilinx shall not be used
* in advertising or otherwise to promote the sale, use or other dealings in
* this Software without prior written authorization from Xilinx.
*
******************************************************************************/

/*
 * helloworld.c: simple test application
 *
 * This application configures UART 16550 to baud rate 9600.
 * PS7 UART (Zynq) is not initialized by this application, since
 * bootrom/bsp configures it to baud rate 115200
 *
 * ------------------------------------------------
 * | UART TYPE   BAUD RATE                        |
 * ------------------------------------------------
 *   uartns550   9600
 *   uartlite    Configurable only in HW design
 *   ps7_uart    115200 (configured by bootrom/bsp)
 */

// Default
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"
#include "xil_io.h"

// for UART drivers
#include "xil_types.h"
#include "xparameters.h"
#include "xuartlite.h"

// UART Buffer size and declared instance for UART Lite Peripheral
#define BUFFER_SIZE 32
XUartLite UartLiteInst;

// Definitions for driver CUSTOMCRC
#define XPAR_CUSTOMCRC_NUM_INSTANCES 1

// Definitions for peripheral CUSTOMCRC_0
#define XPAR_CUSTOMCRC_0_DEVICE_ID 0
#define XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR 0X44A00000
#define XPAR_CUSTOMCRC_0_S00_AXI_HIGHADDR 0X44A0FFFF


int main(){
    init_platform();


    /* UART initialization */
    int Status;
    Status = XUartLite_Initialize(&UartLiteInst, XPAR_AXI_UARTLITE_0_DEVICE_ID);
    if (Status != XST_SUCCESS){
        xil_printf("UART initialization failed\n");
    }
    Status = XUartLite_SelfTest(&UartLiteInst);
    if (Status != XST_SUCCESS){
        xil_printf("UART self-test failed\n");
    }


    u8 RxBuffer[BUFFER_SIZE];
    int rxCount, rxBytes;

    while(1){
        /* Initialize RxBuffer */
        rxCount = 0;
        RxBuffer[0] = 0x00;

        // infinite loop that continually receives from the UART one byte 
        // at a time until a carriage return byte (0x0D) is received
        while(1){
            rxBytes = XUartLite_Recv(&UartLiteInst, &RxBuffer[rxCount], 1);
            if ((rxBytes != 0) && (RxBuffer[rxCount] == 0x0D)){
                break;
            }
            rxCount += rxBytes;
        }

        // Initialize seed and length
        u8 seed = RxBuffer[0];
        u8 length = RxBuffer[1];


        // Write data to data registers
        u32 data[7] = {0,0,0,0,0,0,0};
        int count = 3;
        int X = 0;

        for(int i=0; i < rxCount - 2; i++){
        	data[X] += (RxBuffer[i+2] << 8*count);
            count--;
            Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR + (X+2)*4, data[X]);
            if (count<0){
            	X+=1;
            	count=3;
            }
        }


        // Set CRC IP to COMPUTE State
        u32 control_and_status = (0xFF << 24) + (length << 16) + (seed << 8) + 0x01;
        Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR, control_and_status);

        // Read CRC register
        u16 CRC = Xil_In32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR + 4);
        xil_printf("CRC: 0x%x\n",  CRC);

        //Reset DATA registers
        int j=0;
        for(j = 0; j < 8; j++){
        	Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR + (j+2)*4, 0x0);
        }

        //Reset Control and Status Register to IDLE State
        Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR, 0x0);
    }

    cleanup_platform();
    return 0;
}
