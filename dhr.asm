              SECTION      text,CODE
__code:
main:
              MOVEM.L        D3/D7,-(A7)              ;48e7 1100 
___main__1:
              MOVE.L         #$1388,__MERGED(A4)      ;297c 0000 1388 0000 
              BRA.B          ___main__3               ;602c 
___main__2:
              MOVE.L         __MERGED(A4),D1          ;222c 0000 
              MOVE.L         D1,D0                    ;2001 
              ASL.L          #$2,D0                   ;e580 
              SUB.L          D1,D0                    ;9081 
              ADD.L          D0,D0                    ;d080 
              SUB.L          D1,D0                    ;9081 
              MOVE.L         D0,D3                    ;2600 
              ASL.L          #$4,D3                   ;e983 
              ADD.L          D0,D3                    ;d680 
              ADD.L          D3,D3                    ;d683 
              MOVE.L         D3,D1                    ;2203 
              ASL.L          #$5,D1                   ;eb81 
              SUB.L          D3,D1                    ;9283 
              ADDQ.L         #$1,D7                   ;5287 
              ASL.L          #$2,D1                   ;e581 
              ADD.L          D3,D1                    ;d283 
              ASL.L          #$3,D1                   ;e781 
              DIVS.L         D7,D1                    ;4c47 1801 
              MOVE.L         D1,__MERGED(A4)          ;2941 0000 
___main__3:
              BSR.W          Proc0                    ;6100 0000 
___main__4:
              MOVE.L         D0,D7                    ;2e00 
              CMPI.L         #$3a98,D0                ;0c80 0000 3a98 
              BLT.B          ___main__2               ;6dc6 
___main__5:
              MOVEQ.L        #$0,D0                   ;7000 
___main__6:
              MOVEM.L        (A7)+,D3/D7              ;4cdf 0088 
              RTS                                     ;4e75 
__const:
__strings:
fake:
___fake__1:
___fake__2:
              RTS                                     ;4e75 
Proc0:
              SUB.W          #$64,A7                  ;9efc 0064 
              MOVEM.L        D2/D3/D4/D5/D6/D7/A2/A3/A5,-(A7);48e7 3f34 
___Proc0__1:
              BSR.W          Forbid                   ;6100 0000 
              PEA            $74(A7)                  ;486f 0074 
              PEA            $7c(A7)                  ;486f 007c 
              BSR.W          CurrentTime              ;6100 0000 
              MOVEQ.L        #$7d,D1                  ;727d 
              LSL.L          #$3,D1                   ;e789 
              MOVE.L         $7c(A7),D0               ;202f 007c 
              DIVU.L         D1,D0                    ;4c41 0000 
              MOVE.L         $80(A7),D1               ;222f 0080 
              MOVE.L         D1,D2                    ;2401 
              ASL.L          #$5,D2                   ;eb82 
              SUB.L          D1,D2                    ;9481 
              ASL.L          #$2,D2                   ;e582 
              PEA            $7c(A7)                  ;486f 007c 
              ADD.L          D1,D2                    ;d481 
              ASL.L          #$3,D2                   ;e782 
              PEA            $84(A7)                  ;486f 0084 
              ADD.L          D0,D2                    ;d480 
              BSR.W          CurrentTime              ;6100 0000 
              MOVEQ.L        #$7d,D1                  ;727d 
              LSL.L          #$3,D1                   ;e789 
              MOVE.L         $84(A7),D0               ;202f 0084 
              DIVU.L         D1,D0                    ;4c41 0000 
              MOVE.L         $88(A7),D1               ;222f 0088 
              MOVE.L         D1,D3                    ;2601 
              ASL.L          #$5,D3                   ;eb83 
              SUB.L          D1,D3                    ;9681 
              ASL.L          #$2,D3                   ;e583 
              ADD.L          D1,D3                    ;d681 
              ASL.L          #$3,D3                   ;e783 
              ADD.L          D0,D3                    ;d680 
              SUB.L          D2,D3                    ;9682 
              MOVE.L         D3,$3c(A7)               ;2f43 003c 
              BSR.W          Permit                   ;6100 0000 
___Proc0__2:
              PEA            ($30).w                  ;4878 0030 
              BSR.W          malloc                   ;6100 0000 
              MOVE.L         D0,__MERGEDBSS+$297e(A4) ;2940 297e 
              PEA            ($30).w                  ;4878 0030 
              BSR.W          malloc                   ;6100 0000 
              LEA            $18(A7),A7               ;4fef 0018 
              MOVE.L         D0,__MERGEDBSS+$297a(A4) ;2940 297a 
              MOVE.L         D0,A0                    ;2040 
              MOVE.L         __MERGEDBSS+$297e(A4),(A0);20ac 297e 
              MOVE.L         __MERGEDBSS+$297a(A4),A0 ;206c 297a 
              LEA            $4(A0),A2                ;45e8 0004 
              MOVEQ.L        #$2,D0                   ;7002 
              CLR.L          (A2)+                    ;429a 
              MOVE.L         D0,(A2)+                 ;24c0 
              MOVEQ.L        #$28,D0                  ;7028 
              MOVE.L         D0,(A2)+                 ;24c0 
              LEA            $10(A0),A5               ;4be8 0010 
              LEA            __MERGED+$4(A4),A3       ;47ec 0004 
              DC.W           $c40                     ;0c40 
___Proc0__3:
              MOVE.B         (A3)+,(A5)+              ;1adb 
___Proc0__4:
              TST.B          (A3)                     ;4a13 
              BNE.B          ___Proc0__3              ;66fa 
___Proc0__5:
              MOVEQ.L        #$a,D0                   ;700a 
              MOVE.L         D0,__MERGEDBSS+$752(A4)  ;2940 0752 
              PEA            $74(A7)                  ;486f 0074 
              PEA            $7c(A7)                  ;486f 007c 
              BSR.W          CurrentTime              ;6100 0000 
              MOVEQ.L        #$7d,D1                  ;727d 
              LSL.L          #$3,D1                   ;e789 
              MOVE.L         $7c(A7),D0               ;202f 007c 
              DIVU.L         D1,D0                    ;4c41 0000 
              MOVE.L         $80(A7),D1               ;222f 0080 
              MOVE.L         D1,D3                    ;2601 
              ASL.L          #$5,D3                   ;eb83 
              SUB.L          D1,D3                    ;9681 
              ASL.L          #$2,D3                   ;e583 
              ADD.L          D1,D3                    ;d681 
              ASL.L          #$3,D3                   ;e783 
              MOVEQ.L        #$0,D6                   ;7c00 
              ADD.L          D0,D3                    ;d680 
              MOVE.L         D3,$30(A7)               ;2f43 0030 
              BSR.W          Forbid                   ;6100 0000 
              ADDQ.W         #$8,A7                   ;504f 
              BRA.W          ___Proc0__55             ;6000 01f8 
___Proc0__6:
              MOVE.B         #$41,__MERGEDBSS+$8(A4)  ;197c 0041 0008 
              CLR.L          __MERGEDBSS+$4(A4)       ;42ac 0004 
              MOVE.B         #$42,__MERGEDBSS+$9(A4)  ;197c 0042 0009 
              MOVEQ.L        #$2,D0                   ;7002 
              MOVE.L         D0,$84(A7)               ;2f40 0084 
              LEA            $33(A7),A5               ;4bef 0033 
              LEA            __MERGED+$24(A4),A3      ;47ec 0024 
              DC.W           $c40                     ;0c40 
___Proc0__7:
              MOVE.B         (A3)+,(A5)+              ;1adb 
___Proc0__8:
              TST.B          (A3)                     ;4a13 
              BNE.B          ___Proc0__7              ;66fa 
___Proc0__9:
              MOVEQ.L        #$1,D0                   ;7001 
              MOVE.L         D0,$7c(A7)               ;2f40 007c 
              LEA            $53(A7),A5               ;4bef 0053 
              LEA            $33(A7),A3               ;47ef 0033 
              BRA.B          ___Proc0__12             ;6004 
___Proc0__10:
              TST.B          (A3)                     ;4a13 
              BNE.B          ___Proc0__13             ;6606 
___Proc0__11:
___Proc0__12:
              MOVE.B         (A5)+,D0                 ;101d 
              CMP.B          (A3)+,D0                 ;b01b 
              BEQ.B          ___Proc0__10             ;67f6 
___Proc0__13:
              MOVE.B         (A5),D0                  ;1015 
              SUB.B          (A3),D0                  ;9013 
              BLE.B          ___Proc0__15             ;6f04 
___Proc0__14:
              MOVEQ.L        #$1,D7                   ;7e01 
              DC.W           $c40                     ;0c40 
___Proc0__15:
              MOVEQ.L        #$0,D7                   ;7e00 
___Proc0__16:
              TST.L          D7                       ;4a87 
              SEQ            D0                       ;57c0 
              NEG.B          D0                       ;4400 
              EXTB.L         D0                       ;49c0 
              MOVE.L         D0,__MERGEDBSS+$4(A4)    ;2940 0004 
              BRA.B          ___Proc0__18             ;600e 
___Proc0__17:
              MOVE.L         $84(A7),D1               ;222f 0084 
              ADDQ.L         #$5,D1                   ;5a81 
              MOVE.L         D1,$80(A7)               ;2f41 0080 
              ADDQ.L         #$1,$84(A7)              ;52af 0084 
___Proc0__18:
              CMPI.L         #$3,$84(A7)              ;0caf 0000 0003 0084 
              BLT.B          ___Proc0__17             ;6de8 
___Proc0__19:
              MOVE.L         $84(A7),D5               ;2a2f 0084 
              ADDQ.L         #$5,D5                   ;5a85 
              MOVE.L         $80(A7),D0               ;202f 0080 
              LEA            __MERGEDBSS+$a(A4),A3    ;47ec 000a 
              MOVE.L         D0,$0(A3,D5.L*4)         ;2780 5c00 
              MOVE.L         D0,$4(A3,D5.L*4)         ;2780 5c04 
              MOVE.L         D5,D0                    ;2005 
              ASL.L          #$2,D0                   ;e580 
              MOVE.L         D5,$78(A3,D5.L*4)        ;2785 5c78 
              SUB.L          D5,D0                    ;9085 
              MOVE.L         D0,D1                    ;2200 
              ASL.L          #$4,D1                   ;e981 
              MOVE.L         $84(A7),D4               ;282f 0084 
              ADD.L          D0,D1                    ;d280 
              ASL.L          #$2,D1                   ;e581 
              LEA            __MERGEDBSS+$d6(A4),A3   ;47ec 00d6 
              ADD.L          D1,A3                    ;d7c1 
              ADDQ.L         #$6,D4                   ;5c84 
              MOVE.L         D5,D7                    ;2e05 
              LEA            $0(A3,D5.L*4),A1         ;43f3 5c00 
              BRA.B          ___Proc0__21             ;6004 
___Proc0__20:
              ADDQ.L         #$1,D7                   ;5287 
              MOVE.L         D5,(A1)+                 ;22c5 
___Proc0__21:
              CMP.L          D4,D7                    ;be84 
              BLE.B          ___Proc0__20             ;6ff8 
___Proc0__22:
              ADDQ.L         #$1,$fffffffc(A3,D5.L*4) ;52b3 5cfc 
              LEA            __MERGEDBSS+$a(A4),A1    ;43ec 000a 
              MOVE.L         $0(A1,D5.L*4),$ff0(A3,D5.L*4);27b1 5c00 5d20 0ff0 
              MOVEQ.L        #$5,D0                   ;7005 
              MOVE.L         D0,__MERGEDBSS(A4)       ;2940 0000 
              MOVE.L         __MERGEDBSS+$297a(A4),A5 ;2a6c 297a 
              MOVE.L         (A5),A1                  ;2255 
              MOVE.L         A5,A0                    ;204d 
              MOVEQ.L        #$30,D0                  ;7030 
              BSR.W          _CXAMEMCPY               ;6100 0000 
              MOVEQ.L        #$5,D0                   ;7005 
              MOVE.L         D0,$c(A5)                ;2b40 000c 
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         A0,(A0)                  ;2088 
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         (A0),A1                  ;2250 
              TST.L          __MERGEDBSS+$297a(A4)    ;4aac 297a 
              BEQ.B          ___Proc0__24             ;6708 
___Proc0__23:
              MOVE.L         ([__MERGEDBSS+$297a,A4],$0),(A1);22b4 0161 297a 
              BRA.B          ___Proc0__25             ;6006 
___Proc0__24:
              MOVEQ.L        #$64,D0                  ;7064 
              MOVE.L         D0,__MERGEDBSS(A4)       ;2940 0000 
___Proc0__25:
              MOVEQ.L        #$c,D0                   ;700c 
              ADD.L          __MERGEDBSS(A4),D0       ;d0ac 0000 
              MOVE.L         __MERGEDBSS+$297a(A4),A0 ;206c 297a 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         (A5),A0                  ;2055 
              TST.L          $4(A0)                   ;4aa8 0004 
              BNE.B          ___Proc0__43             ;6670 
___Proc0__26:
              MOVEQ.L        #$6,D0                   ;7006 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         $8(A5),D1                ;222d 0008 
              MOVE.L         D1,D0                    ;2001 
              LEA            $8(A0),A3                ;47e8 0008 
              MOVE.L         D1,(A3)                  ;2681 
              SUBQ.L         #$2,D0                   ;5580 
              BNE.B          ___Proc0__28             ;6604 
___Proc0__27:
              MOVEQ.L        #$1,D7                   ;7e01 
              DC.W           $c40                     ;0c40 
___Proc0__28:
              MOVEQ.L        #$0,D7                   ;7e00 
___Proc0__29:
              TST.L          D7                       ;4a87 
              BNE.B          ___Proc0__31             ;6604 
___Proc0__30:
              MOVEQ.L        #$3,D0                   ;7003 
              MOVE.L         D0,(A3)                  ;2680 
___Proc0__31:
              MOVE.L         D1,D0                    ;2001 
              CMPI.L         #$5,D0                   ;0c80 0000 0005 
              BCC.B          ___Proc0__42             ;6430 
___Proc0__32:
              MOVE.W         $6(PC,D0.W*2),D0         ;303b 0206 
              JMP            $4(PC,D0.W)              ;4efb 0004 
__switch_Proc0_32:
              DC.W           ___Proc0__33-__switch_Proc0_32
              DC.W           ___Proc0__34-__switch_Proc0_32
              DC.W           ___Proc0__37-__switch_Proc0_32
              DC.W           ___Proc0__42-__switch_Proc0_32
              DC.W           ___Proc0__39-__switch_Proc0_32
___Proc0__33:
              CLR.L          (A3)                     ;4293 
              BRA.B          ___Proc0__42             ;601a 
___Proc0__34:
              CMPI.L         #$64,__MERGEDBSS(A4)     ;0cac 0000 0064 0000 
              BLE.B          ___Proc0__36             ;6f04 
___Proc0__35:
              CLR.L          (A3)                     ;4293 
              BRA.B          ___Proc0__42             ;600c 
___Proc0__36:
              MOVEQ.L        #$3,D0                   ;7003 
              BRA.B          ___Proc0__41             ;6006 
___Proc0__37:
              MOVEQ.L        #$1,D0                   ;7001 
              DC.W           $c40                     ;0c40 
___Proc0__38:
___Proc0__39:
              MOVEQ.L        #$2,D0                   ;7002 
___Proc0__40:
___Proc0__41:
              MOVE.L         D0,(A3)                  ;2680 
___Proc0__42:
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         ([__MERGEDBSS+$297a,A4],$0),(A0);20b4 0161 297a 
              MOVE.L         (A5),A0                  ;2055 
              MOVEQ.L        #$c,D0                   ;700c 
              ADD.L          D0,$c(A0)                ;d1a8 000c 
              BRA.B          ___Proc0__44             ;600a 
___Proc0__43:
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         A5,A1                    ;224d 
              MOVEQ.L        #$30,D0                  ;7030 
              BSR.W          _CXAMEMCPY               ;6100 0000 
___Proc0__44:
              MOVE.B         #$41,$73(A7)             ;1f7c 0041 0073 
              MOVE.B         __MERGEDBSS+$9(A4),D0    ;102c 0009 
              MOVE.B         D0,$24(A7)               ;1f40 0024 
              BRA.B          ___Proc0__51             ;6026 
___Proc0__45:
              MOVEQ.L        #$43,D0                  ;7043 
              CMP.B          $73(A7),D0               ;b02f 0073 
              BEQ.B          ___Proc0__47             ;6704 
___Proc0__46:
              MOVEQ.L        #$0,D7                   ;7e00 
              DC.W           $c40                     ;0c40 
___Proc0__47:
              MOVEQ.L        #$1,D7                   ;7e01 
___Proc0__48:
              MOVE.L         $7c(A7),D0               ;202f 007c 
              CMP.L          D7,D0                    ;b087 
              BNE.B          ___Proc0__50             ;660c 
___Proc0__49:
              LEA            $7c(A7),A5               ;4bef 007c 
              MOVEQ.L        #$3,D0                   ;7003 
              CLR.L          (A5)                     ;4295 
              MOVE.L         D0,(A5)                  ;2a80 
              CLR.L          (A5)+                    ;429d 
___Proc0__50:
              ADDQ.B         #$1,$73(A7)              ;522f 0073 
___Proc0__51:
              MOVE.B         $73(A7),D0               ;102f 0073 
              CMP.B          $24(A7),D0               ;b02f 0024 
              BLE.B          ___Proc0__45             ;6fd0 
___Proc0__52:
              MOVE.L         $84(A7),D0               ;202f 0084 
              MOVE.L         D0,D1                    ;2200 
              ASL.L          #$2,D1                   ;e581 
              MOVEQ.L        #$a,D7                   ;7e0a 
              SUB.L          D0,D1                    ;9280 
              ADD.L          D0,D7                    ;de80 
              MOVEQ.L        #$41,D0                  ;7041 
              MOVE.L         D1,$80(A7)               ;2f41 0080 
              CMP.B          __MERGEDBSS+$8(A4),D0    ;b02c 0008 
              BNE.B          ___Proc0__54             ;660a 
___Proc0__53:
              SUBQ.L         #$1,D7                   ;5387 
              SUB.L          __MERGEDBSS(A4),D7       ;9eac 0000 
              MOVE.L         D7,$84(A7)               ;2f47 0084 
___Proc0__54:
              ADDQ.L         #$1,D6                   ;5286 
___Proc0__55:
              CMP.L          __MERGED(A4),D6          ;bcac 0000 
              BLT.W          ___Proc0__6              ;6d00 fe04 
___Proc0__56:
              BSR.W          Permit                   ;6100 0000 
              PEA            $74(A7)                  ;486f 0074 
              PEA            $7c(A7)                  ;486f 007c 
              BSR.W          CurrentTime              ;6100 0000 
              MOVEQ.L        #$7d,D1                  ;727d 
              LSL.L          #$3,D1                   ;e789 
              MOVE.L         $7c(A7),D0               ;202f 007c 
              DIVU.L         D1,D0                    ;4c41 0000 
              MOVE.L         $80(A7),D2               ;242f 0080 
              MOVE.L         D2,D3                    ;2602 
              ASL.L          #$5,D3                   ;eb83 
              SUB.L          D2,D3                    ;9682 
              ASL.L          #$2,D3                   ;e583 
              MOVEQ.L        #$7d,D1                  ;727d 
              ADD.L          D2,D3                    ;d682 
              ASL.L          #$3,D3                   ;e783 
              LSL.L          #$3,D1                   ;e789 
              ADD.L          D0,D3                    ;d680 
              SUB.L          $30(A7),D3               ;96af 0030 
              SUB.L          $34(A7),D3               ;96af 0034 
              MOVE.L         D3,D0                    ;2003 
              DIVS.L         D1,D0                    ;4c41 0800 
              MOVE.L         D3,D1                    ;2203 
              DIVS.L         #$3e8,D1:D2              ;4c7c 1802 0000 03e8 
              MOVE.L         D2,(A7)                  ;2e82 
              MOVE.L         D0,-(A7)                 ;2f00 
              MOVE.L         __MERGED(A4),-(A7)       ;2f2c 0000 
              PEA            __MERGED+$44(A4)         ;486c 0044 
              BSR.W          printf                   ;6100 0000 
___Proc0__57:
              MOVE.L         __MERGED(A4),D0          ;202c 0000 
              MOVE.L         D0,D1                    ;2200 
              ASL.L          #$5,D1                   ;eb81 
              SUB.L          D0,D1                    ;9280 
              ADDQ.L         #$1,D3                   ;5283 
              ASL.L          #$2,D1                   ;e581 
              ADD.L          D0,D1                    ;d280 
              ASL.L          #$3,D1                   ;e781 
              DIVS.L         D3,D1                    ;4c43 1801 
              MOVE.L         D1,(A7)                  ;2e81 
              PEA            __MERGED+$6e(A4)         ;486c 006e 
              BSR.W          printf                   ;6100 0000 
              LEA            $18(A7),A7               ;4fef 0018 
              MOVE.L         D3,D0                    ;2003 
___Proc0__58:
              MOVEM.L        (A7)+,D2/D3/D4/D5/D6/D7/A2/A3/A5;4cdf 2cfc 
              ADD.W          #$64,A7                  ;defc 0064 
              RTS                                     ;4e75 
timer:
              SUBQ.W         #$8,A7                   ;514f 
              MOVE.L         D3,-(A7)                 ;2f03 
___timer__1:
              PEA            $4(A7)                   ;486f 0004 
              PEA            $c(A7)                   ;486f 000c 
              BSR.W          CurrentTime              ;6100 0000 
              MOVEQ.L        #$7d,D1                  ;727d 
              LSL.L          #$3,D1                   ;e789 
              MOVE.L         $c(A7),D0                ;202f 000c 
              DIVU.L         D1,D0                    ;4c41 0000 
              MOVE.L         $10(A7),D1               ;222f 0010 
              MOVE.L         D1,D3                    ;2601 
              ASL.L          #$5,D3                   ;eb83 
              SUB.L          D1,D3                    ;9681 
              ASL.L          #$2,D3                   ;e583 
              ADD.L          D1,D3                    ;d681 
              ASL.L          #$3,D3                   ;e783 
              ADD.L          D0,D3                    ;d680 
              ADDQ.W         #$8,A7                   ;504f 
              MOVE.L         D3,D0                    ;2003 
___timer__2:
              MOVE.L         (A7)+,D3                 ;261f 
              ADDQ.W         #$8,A7                   ;504f 
              RTS                                     ;4e75 
Proc1:
              MOVEM.L        D6/A3/A5,-(A7)           ;48e7 0214 
___Proc1__1:
              MOVE.L         $10(A7),A5               ;2a6f 0010 
              MOVE.L         (A5),A1                  ;2255 
              MOVE.L         __MERGEDBSS+$297a(A4),A0 ;206c 297a 
              MOVEQ.L        #$30,D0                  ;7030 
              BSR.W          _CXAMEMCPY               ;6100 0000 
              MOVEQ.L        #$5,D0                   ;7005 
              MOVE.L         D0,$c(A5)                ;2b40 000c 
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         A0,(A0)                  ;2088 
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         (A0),A1                  ;2250 
              TST.L          __MERGEDBSS+$297a(A4)    ;4aac 297a 
              BEQ.B          ___Proc1__3              ;6708 
___Proc1__2:
              MOVE.L         ([__MERGEDBSS+$297a,A4],$0),(A1);22b4 0161 297a 
              BRA.B          ___Proc1__4              ;6006 
___Proc1__3:
              MOVEQ.L        #$64,D0                  ;7064 
              MOVE.L         D0,__MERGEDBSS(A4)       ;2940 0000 
___Proc1__4:
              MOVEQ.L        #$c,D0                   ;700c 
              ADD.L          __MERGEDBSS(A4),D0       ;d0ac 0000 
              MOVE.L         __MERGEDBSS+$297a(A4),A0 ;206c 297a 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         (A5),A0                  ;2055 
              TST.L          $4(A0)                   ;4aa8 0004 
              BNE.B          ___Proc1__22             ;6670 
___Proc1__5:
              MOVEQ.L        #$6,D0                   ;7006 
              MOVE.L         D0,$c(A0)                ;2140 000c 
              MOVE.L         $8(A5),D1                ;222d 0008 
              MOVE.L         D1,D0                    ;2001 
              LEA            $8(A0),A3                ;47e8 0008 
              MOVE.L         D1,(A3)                  ;2681 
              SUBQ.L         #$2,D0                   ;5580 
              BNE.B          ___Proc1__7              ;6604 
___Proc1__6:
              MOVEQ.L        #$1,D6                   ;7c01 
              DC.W           $c40                     ;0c40 
___Proc1__7:
              MOVEQ.L        #$0,D6                   ;7c00 
___Proc1__8:
              TST.L          D6                       ;4a86 
              BNE.B          ___Proc1__10             ;6604 
___Proc1__9:
              MOVEQ.L        #$3,D0                   ;7003 
              MOVE.L         D0,(A3)                  ;2680 
___Proc1__10:
              MOVE.L         D1,D0                    ;2001 
              CMPI.L         #$5,D0                   ;0c80 0000 0005 
              BCC.B          ___Proc1__21             ;6430 
___Proc1__11:
              MOVE.W         $6(PC,D0.W*2),D0         ;303b 0206 
              JMP            $4(PC,D0.W)              ;4efb 0004 
__switch_Proc1_11:
              DC.W           ___Proc1__12-__switch_Proc1_11
              DC.W           ___Proc1__13-__switch_Proc1_11
              DC.W           ___Proc1__16-__switch_Proc1_11
              DC.W           ___Proc1__21-__switch_Proc1_11
              DC.W           ___Proc1__18-__switch_Proc1_11
___Proc1__12:
              CLR.L          (A3)                     ;4293 
              BRA.B          ___Proc1__21             ;601a 
___Proc1__13:
              CMPI.L         #$64,__MERGEDBSS(A4)     ;0cac 0000 0064 0000 
              BLE.B          ___Proc1__15             ;6f04 
___Proc1__14:
              CLR.L          (A3)                     ;4293 
              BRA.B          ___Proc1__21             ;600c 
___Proc1__15:
              MOVEQ.L        #$3,D0                   ;7003 
              BRA.B          ___Proc1__20             ;6006 
___Proc1__16:
              MOVEQ.L        #$1,D0                   ;7001 
              DC.W           $c40                     ;0c40 
___Proc1__17:
___Proc1__18:
              MOVEQ.L        #$2,D0                   ;7002 
___Proc1__19:
___Proc1__20:
              MOVE.L         D0,(A3)                  ;2680 
___Proc1__21:
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         ([__MERGEDBSS+$297a,A4],$0),(A0);20b4 0161 297a 
              MOVE.L         (A5),A0                  ;2055 
              MOVEQ.L        #$c,D0                   ;700c 
              ADD.L          D0,$c(A0)                ;d1a8 000c 
              BRA.B          ___Proc1__23             ;600a 
___Proc1__22:
              MOVE.L         (A5),A0                  ;2055 
              MOVE.L         A5,A1                    ;224d 
              MOVEQ.L        #$30,D0                  ;7030 
              BSR.W          _CXAMEMCPY               ;6100 0000 
___Proc1__23:
              MOVEM.L        (A7)+,D6/A3/A5           ;4cdf 2840 
              RTS                                     ;4e75 
Proc2:
___Proc2__1:
              MOVE.L         $4(A7),A0                ;206f 0004 
              MOVEQ.L        #$a,D1                   ;720a 
              MOVEQ.L        #$41,D0                  ;7041 
              ADD.L          (A0),D1                  ;d290 
              CMP.B          __MERGEDBSS+$8(A4),D0    ;b02c 0008 
              BNE.B          ___Proc2__3              ;6608 
___Proc2__2:
              SUBQ.L         #$1,D1                   ;5381 
              SUB.L          __MERGEDBSS(A4),D1       ;92ac 0000 
              MOVE.L         D1,(A0)                  ;2081 
___Proc2__3:
              RTS                                     ;4e75 
Proc3:
___Proc3__1:
              MOVE.L         $4(A7),A1                ;226f 0004 
              TST.L          __MERGEDBSS+$297a(A4)    ;4aac 297a 
              BEQ.B          ___Proc3__3              ;6708 
___Proc3__2:
              MOVE.L         ([__MERGEDBSS+$297a,A4],$0),(A1);22b4 0161 297a 
              BRA.B          ___Proc3__4              ;6006 
___Proc3__3:
              MOVEQ.L        #$64,D0                  ;7064 
              MOVE.L         D0,__MERGEDBSS(A4)       ;2940 0000 
___Proc3__4:
              MOVEQ.L        #$c,D0                   ;700c 
              ADD.L          __MERGEDBSS(A4),D0       ;d0ac 0000 
              MOVE.L         __MERGEDBSS+$297a(A4),A0 ;206c 297a 
              MOVE.L         D0,$c(A0)                ;2140 000c 
___Proc3__5:
              RTS                                     ;4e75 
Proc4:
___Proc4__1:
              MOVE.B         #$42,__MERGEDBSS+$9(A4)  ;197c 0042 0009 
              MOVEQ.L        #$0,D0                   ;7000 
___Proc4__2:
              RTS                                     ;4e75 
Proc5:
___Proc5__1:
              MOVE.B         #$41,__MERGEDBSS+$8(A4)  ;197c 0041 0008 
              CLR.L          __MERGEDBSS+$4(A4)       ;42ac 0004 
___Proc5__2:
              RTS                                     ;4e75 
Proc6:
              MOVEM.L        D6/A5,-(A7)              ;48e7 0204 
___Proc6__1:
              MOVE.L         $c(A7),D1                ;222f 000c 
              MOVE.L         D1,D0                    ;2001 
              MOVE.L         $10(A7),A5               ;2a6f 0010 
              MOVE.L         D1,(A5)                  ;2a81 
              SUBQ.L         #$2,D0                   ;5580 
              BNE.B          ___Proc6__3              ;6604 
___Proc6__2:
              MOVEQ.L        #$1,D6                   ;7c01 
              DC.W           $c40                     ;0c40 
___Proc6__3:
              MOVEQ.L        #$0,D6                   ;7c00 
___Proc6__4:
              TST.L          D6                       ;4a86 
              BNE.B          ___Proc6__6              ;6604 
___Proc6__5:
              MOVEQ.L        #$3,D0                   ;7003 
              MOVE.L         D0,(A5)                  ;2a80 
___Proc6__6:
              MOVE.L         D1,D0                    ;2001 
              CMPI.L         #$5,D0                   ;0c80 0000 0005 
              BCC.B          ___Proc6__17             ;6430 
___Proc6__7:
              MOVE.W         $6(PC,D0.W*2),D0         ;303b 0206 
              JMP            $4(PC,D0.W)              ;4efb 0004 
__switch_Proc6_7:
              DC.W           ___Proc6__8-__switch_Proc6_7
              DC.W           ___Proc6__9-__switch_Proc6_7
              DC.W           ___Proc6__12-__switch_Proc6_7
              DC.W           ___Proc6__17-__switch_Proc6_7
              DC.W           ___Proc6__14-__switch_Proc6_7
___Proc6__8:
              CLR.L          (A5)                     ;4295 
              BRA.B          ___Proc6__17             ;601a 
___Proc6__9:
              CMPI.L         #$64,__MERGEDBSS(A4)     ;0cac 0000 0064 0000 
              BLE.B          ___Proc6__11             ;6f04 
___Proc6__10:
              CLR.L          (A5)                     ;4295 
              BRA.B          ___Proc6__17             ;600c 
___Proc6__11:
              MOVEQ.L        #$3,D0                   ;7003 
              BRA.B          ___Proc6__16             ;6006 
___Proc6__12:
              MOVEQ.L        #$1,D0                   ;7001 
              DC.W           $c40                     ;0c40 
___Proc6__13:
___Proc6__14:
              MOVEQ.L        #$2,D0                   ;7002 
___Proc6__15:
___Proc6__16:
              MOVE.L         D0,(A5)                  ;2a80 
___Proc6__17:
              MOVEQ.L        #$0,D0                   ;7000 
___Proc6__18:
              MOVEM.L        (A7)+,D6/A5              ;4cdf 2040 
              RTS                                     ;4e75 
Proc7:
___Proc7__1:
              MOVE.L         $8(A7),D0                ;202f 0008 
              MOVE.L         $4(A7),D1                ;222f 0004 
              ADD.L          D1,D0                    ;d081 
              ADDQ.L         #$2,D0                   ;5480 
              MOVE.L         $c(A7),A0                ;206f 000c 
              MOVE.L         D0,(A0)                  ;2080 
              MOVEQ.L        #$0,D0                   ;7000 
___Proc7__2:
              RTS                                     ;4e75 
Proc8:
              MOVEM.L        D2/D7/A2,-(A7)           ;48e7 2120 
___Proc8__1:
              MOVE.L         $18(A7),D7               ;2e2f 0018 
              MOVE.L         D7,D0                    ;2007 
              MOVE.L         $1c(A7),D1               ;222f 001c 
              ADDQ.L         #$5,D0                   ;5a80 
              MOVE.L         $10(A7),A2               ;246f 0010 
              MOVE.L         $14(A7),A0               ;206f 0014 
              MOVE.L         D1,$0(A2,D0.L*4)         ;2581 0c00 
              MOVE.L         D1,$4(A2,D0.L*4)         ;2581 0c04 
              MOVE.L         D0,D1                    ;2200 
              ASL.L          #$2,D1                   ;e581 
              SUB.L          D0,D1                    ;9280 
              MOVE.L         D1,D2                    ;2401 
              ASL.L          #$4,D2                   ;e982 
              MOVE.L         D0,$78(A2,D0.L*4)        ;2580 0c78 
              ADD.L          D1,D2                    ;d481 
              ASL.L          #$2,D2                   ;e582 
              ADD.L          D2,A0                    ;d1c2 
              LEA            $0(A0,D0.L*4),A1         ;43f0 0c00 
              MOVE.L         D0,(A1)                  ;2280 
              MOVE.L         D0,$18(A0,D7.L*4)        ;2180 7c18 
              ADDQ.L         #$1,$fffffffc(A1)        ;52a9 fffc 
              MOVE.L         $0(A2,D0.L*4),$ff0(A1)   ;2372 0c00 0ff0 
              MOVEQ.L        #$5,D0                   ;7005 
              MOVE.L         D0,__MERGEDBSS(A4)       ;2940 0000 
___Proc8__2:
              MOVEM.L        (A7)+,D2/D7/A2           ;4cdf 0484 
              RTS                                     ;4e75 
Func1:
              MOVEM.L        D6/D7,-(A7)              ;48e7 0300 
___Func1__1:
              MOVE.B         $13(A7),D6               ;1c2f 0013 
              MOVE.B         $f(A7),D7                ;1e2f 000f 
              CMP.B          D6,D7                    ;be06 
              BEQ.B          ___Func1__3              ;6704 
___Func1__2:
              MOVEQ.L        #$0,D0                   ;7000 
              DC.W           $c40                     ;0c40 
___Func1__3:
              MOVEQ.L        #$1,D0                   ;7001 
___Func1__4:
              MOVEM.L        (A7)+,D6/D7              ;4cdf 00c0 
              RTS                                     ;4e75 
Func2:
___Func2__1:
              MOVE.L         $8(A7),A0                ;206f 0008 
              MOVE.L         $4(A7),A1                ;226f 0004 
              BRA.B          ___Func2__4              ;6004 
___Func2__2:
              TST.B          (A0)                     ;4a10 
              BNE.B          ___Func2__5              ;6606 
___Func2__3:
___Func2__4:
              MOVE.B         (A1)+,D0                 ;1019 
              CMP.B          (A0)+,D0                 ;b018 
              BEQ.B          ___Func2__2              ;67f6 
___Func2__5:
              MOVE.B         (A1),D0                  ;1011 
              SUB.B          (A0),D0                  ;9010 
              BLE.B          ___Func2__7              ;6f04 
___Func2__6:
              MOVEQ.L        #$1,D0                   ;7001 
              RTS                                     ;4e75 
___Func2__7:
              MOVEQ.L        #$0,D0                   ;7000 
___Func2__8:
              RTS                                     ;4e75 
Func3:
___Func3__1:
              MOVE.L         $4(A7),D0                ;202f 0004 
              SUBQ.L         #$2,D0                   ;5580 
              BNE.B          ___Func3__3              ;6604 
___Func3__2:
              MOVEQ.L        #$1,D0                   ;7001 
              RTS                                     ;4e75 
___Func3__3:
              MOVEQ.L        #$0,D0                   ;7000 
___Func3__4:
              RTS                                     ;4e75 
strcpy:
___strcpy__1:
              MOVE.L         $8(A7),A0                ;206f 0008 
              MOVE.L         $4(A7),A1                ;226f 0004 
              DC.W           $c40                     ;0c40 
___strcpy__2:
              MOVE.B         (A0)+,(A1)+              ;12d8 
___strcpy__3:
              TST.B          (A0)                     ;4a10 
              BNE.B          ___strcpy__2             ;66fa 
___strcpy__4:
___strcpy__5:
              RTS                                     ;4e75 
strcmp:
___strcmp__1:
              MOVE.L         $8(A7),A0                ;206f 0008 
              MOVE.L         $4(A7),A1                ;226f 0004 
              BRA.B          ___strcmp__4             ;6004 
___strcmp__2:
              TST.B          (A0)                     ;4a10 
              BNE.B          ___strcmp__5             ;6606 
___strcmp__3:
___strcmp__4:
              MOVE.B         (A1)+,D0                 ;1019 
              CMP.B          (A0)+,D0                 ;b018 
              BEQ.B          ___strcmp__2             ;67f6 
___strcmp__5:
              MOVE.B         (A1),D0                  ;1011 
              SUB.B          (A0),D0                  ;9010 
              EXTB.L         D0                       ;49c0 
___strcmp__6:
              RTS                                     ;4e75 
              XREF           Forbid
              XREF           CurrentTime
              XREF           Permit
              XREF           malloc
              XREF           _CXAMEMCPY
              XREF           printf
              XDEF           main
              XDEF           fake
              XDEF           Proc0
              XDEF           timer
              XDEF           Proc1
              XDEF           Proc2
              XDEF           Proc3
              XDEF           Proc4
              XDEF           Proc5
              XDEF           Proc6
              XDEF           Proc7
              XDEF           Proc8
              XDEF           Func1
              XDEF           Func2
              XDEF           Func3
              XDEF           strcpy
              XDEF           strcmp

              SECTION        __MERGED,DATA
__MERGED
LOOPS:
              DC.L           $0000C350
              DC.B           'DHRYSTONE PROGRAM, SOME STRING'
              DC.B           $00
              DC.B           $00
              DC.B           'DHRYSTONE PROGRAM, 2''ND STRING'
              DC.B           $00
              DC.B           $00
              DC.B           'Dhrystone time for %ld passes = %ld.%03d'
              DC.B           $0a
              DC.B           $00
              DC.B           'This machine benchmarks at %ld dhrystones/sec'
              DC.B           'ond'
              DC.B           $0a
              DC.B           $00
              XDEF           LOOPS

              SECTION        __MERGED,BSS
__MERGEDBSS
IntGlob:
              DS.B           4
BoolGlob:
              DS.B           4
Char1Glob:
              DS.B           1
Char2Glob:
              DS.B           1
Array1Glob:
              DS.B           204
Array2Glob:
              DS.B           10404
PtrGlb:
              DS.B           4
PtrGlbNext:
              DS.B           4
              XDEF           IntGlob
              XDEF           BoolGlob
              XDEF           Char1Glob
              XDEF           Char2Glob
              XDEF           Array1Glob
              XDEF           Array2Glob
              XDEF           PtrGlb
              XDEF           PtrGlbNext
              END
