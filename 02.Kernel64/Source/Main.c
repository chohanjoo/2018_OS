#include "Types.h"

// 함수 선언
void kPrintString( int iX, int iY, const char* pcString );
BOOL ReadTest();
BOOL WriteTest();
// 아래 함수는 C 언어 커널의 시작 부분
void Main( void )
{
    kPrintString( 0, 11, "Switch To IA-32e Mode Success~!!" );
    kPrintString( 0, 12, "IA-32e C Language Kernel Start..............[Pass]" );
    kPrintString( 0, 13, "This message is printed through the video memory relocated to 0xAB8000" );

    kPrintString( 0, 14, "Test the Read-only Page Funtion: Read.......[    ]" );
    if(ReadTest())
        kPrintString( 45, 14, "Pass");
    
    kPrintString( 0, 15, "Test the Read-only Page Funtion: Write......[    ]" );
    if(WriteTest())
        kPrintString( 45, 15, "Pass");
    
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
