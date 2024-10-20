         ;代码清单7-1
         ;文件名：c07_mbr.asm
         ;文件说明：硬盘主引导扇区代码
         ;创建日期：2011-4-13 18:02
         
         jmp near start
	
 message db '1+2+3+...+100='



 start:
         mov ax,0x7c0           ;设置数据段的段基地址 
         mov ds,ax    ;将数据段寄存器设置成0x7c0


         mov ax,0xb800          ;设置附加段基址到显示缓冲区
         mov es,ax    ;将附加段设置成0xb800,显存的地址？

         ;以下显示字符串 
         mov si,message  ;将si寄存器保存message
         mov di,0 ;di用作目标索引寄存器，常用于字符串操作和数组索引。
         mov cx,start-message ;
     @g:
         mov al,[si]
         mov [es:di],al
         inc di
         mov byte [es:di],0x07 ;显示属性
         inc di
         inc si
         loop @g

         ;以下计算1到100的和 
         xor ax,ax ;归零
         mov cx,1 ;把cx寄存器存储1
     @f:
         add ax,cx ;传给ax
         inc cx ;inc是啥
         cmp cx,100 ;cmp有是啥
         jle @f

         ;以下计算累加和的每个数位 
         xor cx,cx              ;设置堆栈段的段基地址
         mov ss,cx
         mov sp,cx

         mov bx,10
         xor cx,cx
     @d:
         inc cx
         xor dx,dx
         div bx
         or dl,0x30
         push dx
         cmp ax,0
         jne @d

         ;以下显示各个数位 
     @a:
         pop dx
         mov [es:di],dl
         inc di
         mov byte [es:di],0x07
         inc di
         loop @a
       
         jmp near $ 
       

times 510-($-$$) db 0
                 db 0x55,0xaa
