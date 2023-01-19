// Default
#include <stdio.h>
#include "platform.h"
#include "xil_printf.h"

#include "xparameters.h"
#include "xil_io.h"

// for UART drivers
#include "xil_types.h"
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

        // infinite loop that continually receives from the UART one byte at a time 
	// until a carriage return byte (0x0D) is received
        while(1){
            rxBytes = XUartLite_Recv(&UartLiteInst, &RxBuffer[rxCount], 1);
            if ((rxBytes != 0) && (RxBuffer[rxCount] == 0x0D)){
                break;
            }
            rxCount += rxBytes;
        }

        /* 
        xil_printf("\nBytes received: %d\n", rxCount);
        xil_printf("Bytes received in hex: 0x");
        for (rxBytes=0; rxBytes<rxCount; rxBytes++){
            xil_printf("%02X", RxBuffer[rxBytes]);
        }
        xil_printf("\nBytes received in string: ");
        for (rxBytes=0; rxBytes<rxCount; rxBytes++){
            xil_printf("%c", RxBuffer[rxBytes]);
        }
            xil_printf("\n"); 
        */

        // initialize seed and length
        u8 seed = RxBuffer[0];
        u8 length = RxBuffer[1];

        // write data to data registers
        u32 data[7] = {0,0,0,0,0,0,0};
        int count = 0;
        int X = 0;

        for(i=0; i < rxCount; i++){
            data[X] += RxBuffer[i+2] << 8;
            count += 1;
            if (count==4){
                xil_printf(data[i])
                Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR + (X+2)*4, data[X]);
                X+=1;
            }
        }
        
        // set to COMPUTE state (control and status register --> enable=1, seed, length)
        u32 control_and_status = (control_and_status << 16) + (length << 8) + (seed << 8) + 0x01;
        xil_printf(control_and_status);
        Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR, control_and_status);

        
        // read control and status register if DONE 
        u32 regVal;
        regVal = Xil_In32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR + 4);    
        xil_printf("CRC register: %d",  regVal);


        // set CRC to IDLE state
        Xil_Out32(XPAR_CUSTOMCRC_0_S00_AXI_BASEADDR, 0x0);
        
    }

    cleanup_platform();
    return 0;
}
