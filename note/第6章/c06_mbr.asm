      ;代码清单6-1
         ;文件名：c06_mbr.asm
         ;文件说明：硬盘主引导扇区代码
         ;创建日期：2011-4-12 22:12 
      

;section .data
  mytext db 'l',0x07,'a',0x07,'b',0x07,'e',0x07,'l',0x07,' ',0x07,'o',0x07, 'f',0x07,'f',0x07,'s',0x07,'e',0x07,'t',0x07,':',0x07               ;char arr[13]={'L','a','b','e','l',' ','o','f','f','s','e','t',':'};
                               
  number db 0,0,0,0,0                                                                      ;int arr[5] = {0};
  jmp near _start
;section .text
  _start:
;     SECTION MBR vstart=0x7c00         
;     mov ax,cs                   ;此时cs寄存器为0，自然可以用来将ax寄存器置0
;     mov ds,ax
;     mov es,ax
;     mov ss,ax
;     mov fs,ax
;     mov sp,0x7c00
         mov ax,0x7c0                  ;设置数据段基地址                                   
         mov ds,ax                    ;把ds给设成0x7c0                                                     ;将数据段寄存器ds设置为ax的值

         mov ax,0xb800                 ;设置附加段基地址                                   ;设置视屏内存基段地址
         mov es,ax                       ;es设成0xb800                                                  ;将附加寄存器as设置为ax的值
         
         cld                               
         mov si,mytext                                                                     ;将arr的地址放入si
         mov di,0                                                                          ;将di寄存器清0,用于记录视屏偏移
         mov cx,(number-mytext)/2      ;实际上等于 13                                      ;int cx = sizeof(arr)/sizeof(char)
         rep movsw                                                                         ;将cx个字从Si复制到DI
                     ; rep movsb 指令会把 DS:SI 指向的数据复制到 ES:DI 指向的内存中。
                     ; 这里就打印了？不是只是给显存了

         ;得到标号所代表的偏移地址
         mov ax,number                                                                     ;将arr[5]地址存入ax
         
         ;计算各个数位
         mov bx,ax                                                                        
         mov cx,5                      ;循环次数   ;int cx=5;                                       
         mov si,10                     ;除数   ;int n=10;
  digit: 
         xor dx,dx           ;int dx = 0;int index=5;  
       ; 这条指令将寄存器 DX 的值与自身进行异或运算。
       ; 由于任何数与自身异或的结果为 0，因此这条指令的效果是将 DX 清零。                                                            
         div si     
         ;  div si 指令将 AX 寄存器中的值除以 SI 寄存器中的值，
         ;  并将结果存储在 AL 和 AH 寄存器中。   
         ;div 指令的被除数默认是ax       
         ;这里把结果保存到dl里面的啊                                                       ;arr/si
         mov [bx],dl                   ;保存数位                                           ;bx[index]=arr%si;
         inc bx                                                                            ;index -= 1;
         loop digit                                                                        ;if(index!=0) 循环
         
         ;显示各个数位
         mov bx,number                                                                     ;将bx设置为number的地址
         mov si,4                                                                          ;int i = 4;  用来遍历数组
   show:
         mov al,[bx+si]                                                                    ;将bx+i个字符放入AL
         add al,0x30                                                                       ;字符+'30'
         mov ah,0x04                                                                       ;设置属性
         mov [es:di],ax                                                                    ;将ax值放入ES:Di指向的内存
         add di,2                                                                          ;向后跳两个
         dec si                                                                            ;i -= 1
         jns show                                                                          ;if(i>0) 循环
         
         mov word [es:di],0x0744                                                           ;显示字符'D'

         jmp near $  ; $当前地址  ;$$ 当前段地址


  times 510-($-$$) db 0 ;这里把整个段前510字节的都变成0
                   db 0x55,0xaa ;最后两个是
;重复生成、
;($-$$) 偏移量
;