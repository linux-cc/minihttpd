%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR
LOADER_STACK_TOP equ LOADER_BASE_ADDR
jmp loader_start

;构建GDT及其内部的描述符
GDT_BASE:
    dd 0
    dd 0
CODE_DESC:
    dd 0xffff
    dd DESC_CODE_HIGH4
DATA_DESC:
    dd 0xffff
    dd DESC_DATA_HIGH4
VIDEO_DESC:
    dd 0x80000007           ;limit=(0xbffff-0xb8000)/4k=0x7
    dd DESC_VIDEO_HIGH4     ;此时dpl=0

    GDT_SIZE equ $ - GDT_BASE
    GDT_LIMIT equ GDT_SIZE - 1
    times 60 dq 0           ;此处预留60个描述符的空位
    SELECTOR_CODE equ ()
;打印字符串；输出背景色绿色，前景色红色并且跳动的字符串
    mov byte [gs:0x00], '2'
    mov byte [gs:0x01], 0xa4    ;a标示绿色背景闪烁，4表示红色前景
    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xa4
    mov byte [gs:0x04], 'L'
    mov byte [gs:0x05], 0xa4
    mov byte [gs:0x06], 'O'
    mov byte [gs:0x07], 0xa4
    mov byte [gs:0x08], 'A'
    mov byte [gs:0x09], 0xa4
    mov byte [gs:0x08], 'D'
    mov byte [gs:0x09], 0xa4
    mov byte [gs:0x0a], 'E'
    mov byte [gs:0x0b], 0xa4
    mov byte [gs:0x0c], 'R'
    mov byte [gs:0x0d], 0xa4

    jmp $
