;主引导程序
section mbr vstart=0x7c00
    mov ax, cs
    mov ds, ax
    mov es, ax
    mov ss, ax
    mov fs, ax
    mov sp, 0x7c00
    mov ax, 0xb800
    mov gs, ax

;清屏利用0x06号功能，上卷全部行则可清屏
;INT 0x10 功能号：0x06 功能描述：上卷窗口
;输入
;ah 功能号=0x06
;al = 上卷行数（为0表示全部）
;bh = 上卷行属性
;(cl,ch) = 窗口左上角的（x,y）位置
;(dl,dh) = 窗口右下角的（x,y）位置
;无返回值
    mov ax, 0x600
    mov bx, 0x700
    mov cx, 0           ; 左上角：(0,0)
    mov dx, 0x184f      ; 右下角：(80,25)，VGA文本模式中一行只能容纳80字符，共25行，因此0x18=24，0x4f=79
    int 0x10

;打印字符串；输出背景色绿色，前景色红色并且跳动的字符串
    mov byte [gs:0x00], '1'
    mov byte [gs:0x01], 0xa4    ;a标示绿色背景闪烁，4表示红色前景
    mov byte [gs:0x02], ' '
    mov byte [gs:0x03], 0xa4
    mov byte [gs:0x04], 'M'
    mov byte [gs:0x05], 0xa4
    mov byte [gs:0x06], 'B'
    mov byte [gs:0x07], 0xa4
    mov byte [gs:0x08], 'R'
    mov byte [gs:0x09], 0xa4

    jmp $               ; 使程序悬停

    message db "1 MBR"
    times 510-($-$$) db 0
    db 0x55,0xaa

