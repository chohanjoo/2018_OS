#include "Types.h"
#include "Keyboard.h"
#include "Descriptor.h"
#include "PIC.h"

// 함수 선언
void kPrintString( int iX, int iY, const char* pcString );
BOOL ReadTest();
BOOL WriteTest();
// 아래 함수는 C 언어 커널의 시작 부분
void Main( void )
{
	char vcTemp[2] = {0,};
	BYTE bFlags;
	BYTE bTemp;
	int i=0;
	KEYDATA stData;


	kPrintString( 0, 11, "Switch To IA-32e Mode Success~!!" );
	kPrintString( 0, 12, "IA-32e C Language Kernel Start..............[Pass]" );

	kPrintString( 0, 13, "GDT Initialize And Switch For IA-32e Mode...[    ]" );
	kInitializeGDTTableAndTSS();
	kLoadGDTR( GDTR_STARTADDRESS );
	kPrintString( 45, 13, "Pass" );

	kPrintString( 0, 14, "TSS Segment Load............................[    ]" );
	kLoadTR( GDT_TSSSEGMENT );
	kPrintString( 45, 14, "Pass" );

	kPrintString( 0, 15, "IDT Initialize..............................[    ]" );
	kInitializeIDTTables();    
	kLoadIDTR( IDTR_STARTADDRESS );
	kPrintString( 45, 15, "Pass" );
	kPrintString( 0, 16, "Keyboard Activate And Queue Initialize......[    ]" );



	// 키보드를 활성화
	if( kInitializeKeyboard()  == TRUE )
	{
		kPrintString(45,16,"Pass");
		kChangeKeyboardLED(FALSE, FALSE, FALSE);
	}
	else{
		kPrintString(45,16,"Fail");
		while(1);
	}

	kPrintString( 0, 17, "This message is printed through the video memory relocated to 0xAB8000" );

	kPrintString( 0, 18, "Test the Read-only Page Funtion: Read.......[    ]" );
	if(ReadTest())
		kPrintString( 45, 18, "Pass");

	kPrintString( 0, 19, "Test the Read-only Page Funtion: Write......[    ]" );
	if(WriteTest())
		kPrintString( 45, 19, "Pass");

	kPrintString(0,20,"PIC Controller And Interrupt Initialize.....[    ]");
	// PIC 컨트롤러 초기화 및 모든 인터럽트 활성화
	kInitializePIC();
	kMaskPICInterrupt(0);
	kEnableInterrupt();
	kPrintString(45,20,"Pass");

	while(1)
	{
		// 키 큐에 데이터가 있음녀 키를 처
		if( kGetKeyFromKeyQueue( &stData ) == TRUE)
		{

			// 키가 눌러졌으면 키의 ASCII 코드 값을 화면에 출력
			if( stData.bFlags & KEY_FLAGS_DOWN)
			{
				vcTemp[0] = stData.bASCIICode;
				kPrintString(i++,21,vcTemp);
				//0이 입력되면 변수를 0으로 나누어 Divide Error 예외(벡터 0번)를 발생시킴
				if(vcTemp[0] == '0')
				{
					//아래 코드를 수행하면 Divide Error 예외가 발생하여
					//커널의 임시 핸들러가 수행됨
					bTemp = bTemp / 0;
				}
			}
		}

	}


}

// 문자열 출력 함수
void kPrintString( int iX, int iY, const char* pcString )
{
	CHARACTER* pstScreen = ( CHARACTER* ) 0xAB8000;
	int i;

	pstScreen += ( iY * 80  ) + iX;
	for( i = 0 ; pcString[ i ] != 0 ; i++ )
	{
		pstScreen[ i ].bCharacter = pcString[ i ];
	}
}

// Read_Only  Read 기능을 테스트하는 함수
BOOL ReadTest( void )
{
	DWORD testValue = 0xFF;

	DWORD* pdwCurrentAddress = ( DWORD* ) 0x1ff000;

	testValue = *pdwCurrentAddress;

	if(testValue != 0xFF)
	{
		return TRUE;
	}
	return FALSE;
}

// Read_Only  Write 기능을 테스트하는 함수
BOOL WriteTest( void )
{
	DWORD* pdwCurrentAddress = ( DWORD* ) 0x1ff000;
	*pdwCurrentAddress =0x12345678;

	if(*pdwCurrentAddress !=0x12345678){
		return FALSE;
	}
	return TRUE;
}
