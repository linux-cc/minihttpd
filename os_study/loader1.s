%include "boot.inc"
section loader vstart=LOADER_BASE_ADDR

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
