;主引导程序
%include "boot.inc"
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

    mov eax, LOADER_START_SECTOR    ;起始扇区lba地址
    mov bx, LOADER_BASE_ADDR        ;写入的地址
    mov cx, 4                       ;待读入的扇区数
    call rd_disk_m_16               ;读取程序的起始部分，一个扇区

    jmp LOADER_BASE_ADDR

;功能：读取硬盘n个扇区
;eax=lba扇区号
;bx=将数据写入的内存地址
;cx=读入的扇区数
rd_disk_m_16:
    mov esi, eax            ;备份eax
    mov di, cx              ;备份cx
    ;读写硬盘
    ;第一步：设置要读取的扇区数
    mov dx, 0x1f2
    mov al, cl
    out dx, al              ;读取的扇区数
    mov eax, esi            ;恢复eax
    ;第二步：将lba地址存入0x1f3---0x1f6
    ;lba地址0-7位写入端口0x1f3
    mov dx, 0x1f3
    out dx, al
    ;lba地址8-15位写入端口0x1f4
    mov cl, 8
    shr eax, cl
    mov dx, 0x1f4
    out dx, al
    ;lba地址16-23位写入端口0x1f5
    shr eax, cl
    mov dx, 0x1f5
    out dx, al
    ;lba地址第24-27位
    shr eax, cl
    and al, 0x0f
    or al, 0xe0         ;设置4-7位为1110，表示lba模式
    mov  dx, 0x1f6
    out dx, al
    ;第三步：向0x1f7端口写入读命令0x20
    mov dx, 0x1f7
    mov al, 0x20
    out dx, al

    ;第四步：检测硬盘状态
    .not_ready:
    ;同一端口，写时表示写入命令字，读时表示读入硬盘状态
    nop
    in al, dx
    and al, 0x88        ;第3位为1表示已准备好数据传输，第7位为1表示硬盘忙
    cmp al, 0x8
    jnz .not_ready      ;未准备好，继续等
    ;第五步：从0x1f0端口读数据
    ;di为要读取的扇区数，一个扇区512字节，每次读入一个字，共di*512/2即di*256次
    mov ax, di
    mov dx, 256
    mul dx
    mov cx, ax
    mov dx, 0x1f0

    .go_on_read:
    in ax, dx
    mov [bx], ax
    add bx, 2
    loop .go_on_read
    ret


    times 510-($-$$) db 0
    db 0x55,0xaa

