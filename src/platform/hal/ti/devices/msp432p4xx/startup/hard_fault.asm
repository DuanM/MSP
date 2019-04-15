
        PUBLIC  HardFault_Handler

        EXTERN  hal_hardfault_print
;********************************************************************************************************
;                                      CODE GENERATION DIRECTIVES
;********************************************************************************************************

        RSEG CODE:CODE:NOROOT(2)
        THUMB
        
HardFault_Handler
        TST LR, #4
        ITE EQ
        MRSEQ R0, MSP
        MRSNE R0,PSP
        MOV R1, LR
        B hal_hardfault_print
        
        END
        
