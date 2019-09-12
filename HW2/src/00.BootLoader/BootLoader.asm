[ORG 0x00]
[BITS 16]

SECTION .text

jmp 0x07C0:START

;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;MINT64 OS에 관련된 환경설정 값
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

TOTALSECTORCOUNT: dw 0x02	;부트로더를 제외한 MINT64 OS 이미지 크기


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;	    코드영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

START:
	mov ax, 0x07C0
	mov ds, ax	;ds세그먼트 레지스터에 부트로더 시작어드레스 설정
	mov ax,0xB800
	mov es, ax	;es세그먼트 레지스터에 비디오메모리 시작 어드레스 설정
	
	;스택 초기화 코드
	mov ax, 0x0000
	mov ss, ax
	mov sp, 0xFFFE
	mov bp, 0xFFFE 

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 화면지우고 속성값을 녹색으로 설정
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	mov si,0	;i=0
	
.SCREENCLEARLOOP:		;화면 설정
	mov byte [ es: si ], 0		;검정바탕
	mov byte [ es: si+1 ], 0x0A	;초록글씨

	add si, 2	;i+=2
	cmp si, 80*25*2		;if문
	jl .SCREENCLEARLOOP	


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 메시지 출력함수 호출 부분
	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;시작메시지 출력
;	push MESSAGE1		;메시지 스택에 삽입
;	push 0			;y좌표 스택에 삽입
;	push 0			;x좌표 스택에 삽입
;	call PRINTMESSAGE
;	add sp,6		;삽입 파라미터 제거

	;OS 이미지 로딩한다는 메시지 출력
;	push IMAGELOADINGMESSAGE	;메시지 스택에 삽입
;	push 1
;	push 0
;	call PRINTMESSAGE
;	add sp,6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 디스크에서 OS이미지 로딩
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 디스크 읽기전 먼저 리셋
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
RESETDISK:
	;BIOS Reset Function 호출
	mov ax, 0
	mov dl, 0
	int 0x13
	jc HANDLEDISKERROR

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 현재 시간 출력	
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	
;	push TIMEMESSAGE
;	push 1
;	push 0
;	call PRINTMESSAGE
;	add sp,6

;	mov bx, TIMEBUF	;bx레지스터에 TIMEBUF주소값 넣기
;	mov ah, 0x02	;기능 번호는 02h
;	int 0x1a	;인터럽트 번호는 1ah
;	jc GETTIMEERROR	;에러 발생하면 이동
	
	;hour
;	mov al, ch	
;	call GETTIME
 
	;minute
;	mov al, cl
;	call GETTIME

	;second
;	mov al, dh
;	call GETTIME
	
	
;	push TIMEBUF
;	push 1
;	push 15
;	call PRINTMESSAGE
;	add sp, 6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 디스크에서 섹터를 읽음
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

	mov si, 0x1000	;물리적 주소(0x10000)을 세그먼트레지스터 값으로 변환

	mov es, si	;es세그먼트 레지스터에 값 설정
	mov bx, 0x0000	;bx레지스터에 0x0000을 설정하여 복사할 어드레스를 0x10000으로 설정

	mov di, word[ TOTALSECTORCOUNT ]	;복사할 OS이미지의 섹터 수를 DI레지스터에 설정

READDATA:		;디스크 읽는 코드 시작
	cmp di, 0	;if문(di값이 0이 되면 다 읽은 것)
	je READEND
	sub di, 0x1	; sector--

	;;;;;;;;;;;;;;;;;;;;;;
	; BIOS Read함수 호출
	;;;;;;;;;;;;;;;;;;;;;;
	mov ah, 0x02	; BIOS 서비스 번호 2(섹터 읽기)
	mov al, 0x1	; 읽을 섹터 수는 1
	mov ch, byte[ TRACKNUMBER ]	;읽을 트랙 번호 설정
	mov cl, byte[ SECTORNUMBER ]	;읽을 섹터 번호 설정
	mov dh, byte[ HEADNUMBER ]	;읽을 헤드 번호 설정
	mov dl, 0x00			;읽을 드라이브 번호(플로피디스크는 0)
	int 0x13			;디스크 io서비스를 사용하기 위해 인터럽트발생
	jc HANDLEDISKERROR		;에러 발생했다면 이동

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	;복사할 주소와 트랙,헤드,섹터주소 계산
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	add si, 0x0020
	mov es, si	;0x200바이트만큼 읽었으므로 es세그먼트 레지스터에 더해줌

	mov al, byte[ SECTORNUMBER ]
	add al, 0x01			
	mov byte[ SECTORNUMBER ],al	;SECTORNUMBER++
	cmp al, 19			;섹터 18까지 썼나?
	jl READDATA	
	
	;섹터를 18까지 읽었으면 헤드를 토글(0->1 1->0)하고 섹터를 1로 설정
	xor byte[ HEADNUMBER ], 0x01
	mov byte[ SECTORNUMBER ], 0x01

	;헤드가 1->0이 된경우엔 트랙번호+1
	cmp byte[ HEADNUMBER ], 0x00
	jne READDATA
	add byte[ TRACKNUMBER ], 0x01
	jmp READDATA
READEND:

	cli
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; OS이미지 로딩이 완료되었다는 메시지 출력
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push LOADINGCOMPLETEMESSAGE
	push 1
	push 20
	call PRINTMESSAGE
	add sp,6

	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; OS이미지 체크
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	push IMAGECHECKINGMESSAGE
	push 2
	push 0
	call PRINTMESSAGE
	add sp,6

	call CHECKOS


	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	; 로딩한 가상 OS이미지 실행
	;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
	jmp 0x1000:0x0000


;-----------------------------------------------------------------------------------------


;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 함수 코드 영역
;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;

STOREHASH:
	push bp
	mov bp,sp
	push si
 	push dx
	mov si,0

.STOREHASHLOOP:
		add si,1
		cmp si,5
		jge .STOREHASHEND

		mov ax,0
		mov al, byte[bp +si+3]
		push ax

		mov ax,si
		sub ax,1
		mov bx,2
		mul bx
		push ax

		call STORECHA
		add sp,4
		jmp .STOREHASHLOOP

.STOREHASHEND:
	pop dx
	pop si
	pop bp
	ret

STORECHA:
	push bp
	mov bp,sp
	push si
	push cx
	push dx

	mov ax, word[bp+6]
	mov si,0

.STORECHALOOP:
	add si,1
	cmp si,2
	je .SECOND
	cmp si,3
	jge .STORECHAEND
	shr al, 4
	jmp .COMMON

.SECOND:
	mov ax, word[bp+6]
	and al, 0xf
.COMMON:
	cmp al, 10
	jge .OVERTEN
.OVERTENRETURN:
	add al, 0x30
	mov bx, word[bp+4]
	mov cx,si
	mov si,bx
	add si,cx
	dec si
	mov byte[HASHBUF +si],al
	mov si,cx
	jmp .STORECHALOOP

.OVERTEN:
	add al, 0x07
	jmp .OVERTENRETURN

.STORECHAEND:
        pop dx
	pop cx
	pop si
	pop bp
	ret







;디스크 에러 처리함수
HANDLEDISKERROR:	;예외처리
	push DISKERRORMESSAGE	
	push 1
	push 20
	call PRINTMESSAGE

	jmp $

;시간 에러 처리함수
;GETTIMEERROR:
;	push TIMEERRORMESSAGE
;	push 1
;	push 20
;	call PRINTMESSAGE

;	jmp $

;시간을 얻는 함수
;GETTIME:
;	mov ah, al		
;	and ax, 0xf00f		;ah레지스터에는 십의자리, al레지스터에는 일의자리가 있으므로 필요한 부분만 남겨둠
;	shr ah, 4		;ah레지스터를 오른쪽으로 4비트 shift
;	add ax, 0x3030		;아스키문자로 변환(48씩 더하기)
;
;	mov byte[ bx ], ah	;십의 자리
;	add bx, 1
;	mov byte[ bx ], al	;일의 자리
;	add bx, 2
;	ret

;메시지 출력 함수(PARAM: X좌표, Y좌표, 문자열)
PRINTMESSAGE:
	push bp		;원래 bp값 저장
	mov bp, sp	;bp로 파라미터에 접근하기 위해

	push es		;레지스터 값을 함수 호출 전과 같이 유지하기 위해
	push si
	push di
	push ax
	push cx
	push dx

	mov ax, 0xB800	; es세그먼트 레지스터에 비디오 모드 어드레스 설정
	mov es, ax

	;x,y좌표를 이용해  비디오 메모리의 어드레스 계산(y먼저)
	mov ax, word[ bp+6 ]
	mov si, 160	; 한 라인의 바이트 수(2*80)를 si레지스터에 설정
	mul si		; ax레지스터와 si레지스터 곱하여 Y어드레스 계산
	mov di, ax	; Y어드레스를 di레지스터에 설정
	
	mov ax, word[ bp+4 ]
	mov si, 2	; 한문자를 나타내는 바이트수(2)를 si레지스터에 설정
	mul si		; X어드레스 계산
	add di, ax	; di레지스터에 총 좌표값을 설정

	mov si,  word[ bp+8 ]	;출력할 문자열주소를 si레지스터에 설정

.MESSAGELOOP:
	mov cl, byte[ si ]	;CL레지스터는 CX레지스터의 하위 1바이트(문자열은 1바이트로 충분)
	cmp cl,0
	je .MESSAGEEND

	mov byte[ es:di ], cl	;비디오 메모리 어드레스 0xB800:di에 문자 출력
	
	add si,1	; 다음 문자열로 이동
	add di,2	; 다음 좌표값으로 이동(문자, 속성으로 2바이트임)

	jmp .MESSAGELOOP


.MESSAGEEND:
	pop dx
	pop cx
	pop ax
	pop di
	pop si
	pop es
	pop bp
	ret	

; OS이미지 체크 함수
CHECKOS:
	mov si, 0x1000
	mov es, si					;0x10000번지부터 커널이미지 있음
	mov si, 4					;si 레지스터를 인덱스로 사용

	mov cx, word [ es : si ]	; 처음 4byte에는 hash값이 있으므로
	mov dx, word [ es : si+2 ]	; hash값 계산은 4번지 부터

.HASHLOOP:
	add si,4

	cmp si, 1024
	je .ENDHASHLOOP

	mov ax, word[es:si]

	mov bx,word[es:si+2]
	xor cx,ax
	xor dx,bx

	jmp .HASHLOOP

.ENDHASHLOOP:
;	mov bx,0x0a00
;	xor dx,bx
;	mov bx,cx
;	and bh,0xf
;	add bh,0x30
;	mov ax,0xB800
;	mov ds,ax
;	mov byte[0x00],bh

;	mov si,0
;	mov ax,word[es:si]
;
	mov si,0
;	mov ax, dx
;	mov bx, 0xB800
;	mov es,bx
;	shr ax,4
;	and ax,0x000F
;	shr ax,1
;	add al,0x30


;	mov byte[es:(160*6)],al

	mov ax, 0x1000
	mov es,ax
;	mov si,0

	mov ax,word[es:si]
	mov bx,word[es:si+2]

	cmp ax,cx
	jne .FAIL

	cmp bx,dx
	jne .FAIL
	jmp .SUCCESS

.SUCCESS:
;	push CHECKINGCOMPLETEMESSAGE
;	push 2
;	push 21
;	call PRINTMESSAGE
;	add sp,6
	
	ret
.FAIL:
;	push CHECKINGERRORMESSAGE
;	push 4
;	push 0
;	call PRINTMESSAGE
;	add sp,6

	push word[es:si]
	push word[es:si+2]
	call STOREHASH
	add sp,4


	push HASHBUF
	push 5
	push 0
	call PRINTMESSAGE
	add sp,6

      

	push dx
	push cx
	call STOREHASH
	add sp,4


	push HASHBUF
	push 6
	push 0
	call PRINTMESSAGE
	add sp,6
	jmp $
;.CHECKOSERROR:

;	push CHECKINGERRORMESSAGE
;	push 3
;	push 21
;	call PRINTMESSAGE
;	add sp, 6
;
;	push LOADEDHASHVALUEMESSAGE	;cx,dx값 넣기
;	push 4
;	push 0
;	call PRINTMESSAGE
;	add sp,6
;
;	push ax
;
;	mov ax, 0x0000
;	mov al, ch
;	mov ah, ch
;   shr ah, 4
;	and al, 0x0f

;	cmp ah, 0x0a
;	jge .HOVERTEN
;	add ah, 0x30
;
;	cmp al, 0x0a
;	jge .LOVERTEN
;	add al, 0x30
;
;	mov si, LHASHBUF
;	mov word[ si ], ax
;	add si, 2
;
;	mov ax, 0x0000
;	mov al, cl
;	mov ah, cl
 ;   shr ah, 4
;	and al, 0x0f
;
;	cmp ah, 0x0a
;	jge .HOVERTEN
;	add ah, 0x30
;
;	cmp al, 0x0a
;	jge .LOVERTEN
;	add al, 0x30
;
;	mov word[ si ], ax
;	add si, 2
;	
;	mov ax, 0x0000
;	mov al, dh
;	mov ah, dh
 ;   shr ah, 4
;	and al, 0x0f
;
;	cmp ah, 0x0a
;	jge .HOVERTEN
;	add ah, 0x30
;
;	cmp al, 0x0a
;	jge .LOVERTEN
;;	add al, 0x30
;
;	mov word[ si ], ax
;	add si, 2
;
;	mov ax, 0x0000
;	mov al, dl
;	mov ah, dl
 ;   shr ah, 4
;	and al, 0x0f

;;	cmp ah, 0x0a
;	jge .HOVERTEN
;	add ah, 0x30
;
;	cmp al, 0x0a
;;	jge .LOVERTEN
;	add al, 0x30
;;
;	mov word[ si ], ax
;	add si, 2

;	pop ax

;	push LHASHBUF
;	push 4
;	push 21
;	call PRINTMESSAGE
;	add sp, 6
;	
;	push CALCULATEDHASHVALUEMESSAGE ;ax,bx값 넣기
;	push 5
;;	push 0
;	call PRINTMESSAGE
;	add sp, 6
;
;	mov si, CHASHBUF
;	mov word[ si ], ax
;	add si, 4
;	mov word[ si ], bx
;
;	push CHASHBUF
;	push 5
;	push 21
;	call PRINTMESSAGE
;	add sp, 6
;
;	jmp $

;.CHECKOSEND:
;	push CHECKINGCOMPLETEMESSAGE
;	push 3
;	push 21
;	call PRINTMESSAGE
;	add sp, 6
;
;	pop es
;	pop si
;	pop dx
;	pop cx
;	pop bx
;	pop ax
;	ret

;.LOVERTEN:
;	add al, 0x11
;	ret
;.HOVERTEN:
;	add ah, 0x11
;	ret

;-----------------------------------------------------------------------------------------


;;;;;;;;;;;;;;;;;;;;;;;;;;;;
; 데이터 영역(변수 등)
;;;;;;;;;;;;;;;;;;;;;;;;;;;

;메시지 - 마지막을 0으로 해서 MESSAGEEND로 감
MESSAGE1: db '1', 0 
DISKERRORMESSAGE: db '2', 0
;TIMEMESSAGE: db '3',0
IMAGELOADINGMESSAGE: db '4', 0
LOADINGCOMPLETEMESSAGE: db '5', 0
;TIMEERRORMESSAGE: db 'TIME ERROR~', 0
IMAGECHECKINGMESSAGE: db '6', 0
CHECKINGCOMPLETEMESSAGE: db '9', 0
CHECKINGERRORMESSAGE: db '-1', 0
;LOADEDHASHVALUEMESSAGE: db 'Loaded Hash Value: ', 0
;CALCULATEDHASHVALUEMESSAGE: db 'Calculated Hash Value: ', 0

;시간 관련 변수
;TIMEBUF: db '00:00:00', 0

;hash 관련 변수
HASHBUF: db '00000000', 0
;CHASHBUF: db '00000000', 0

;디스크 읽기 관련 변수
SECTORNUMBER: db 0x02	;첫번째 섹터는 부트로더이므로 OS이미지는 2번째 섹터부터 시작
HEADNUMBER: db 0x00	;OS이미지의 시작헤드번호
TRACKNUMBER: db 0x00	;OS이미지의 시작트랙번호



times 510 - ($ - $$)    db    0x00

db 0x55
db 0xAA
