#include "InterruptHandler.h"
#include "PIC.h"
#include "Keyboard.h"
#include "Page.h"
/**
 *  공통으로 사용하는 예외 핸들러
 */
void kCommonExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 3 ] = { 0, };

    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 0 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 1 ] = '0' + iVectorNumber % 10;

    kPrintString( 0, 0, "====================================================" );
    kPrintString( 0, 1, "                 Exception Occur~!!!!               " );
    kPrintString( 0, 2, "                    Vector:                         " );
    kPrintString( 27, 2, vcBuffer );
    kPrintString( 0, 3, "====================================================" );

    while( 1 ) ;
}

void kSetPageEntryData( PTENTRY* pstEntry, DWORD dwUpperBaseAddress, 
        DWORD dwLowerBaseAddress, DWORD dwLowerFlags, DWORD dwUpperFlags )
{
    pstEntry->dwAttributeAndLowerBaseAddress = dwLowerBaseAddress | dwLowerFlags;
    pstEntry->dwUpperBaseAddressAndEXB = ( dwUpperBaseAddress & 0xFF ) |
        dwUpperFlags;
}

void kDistinguishException(int iVectorNumber, QWORD qwErrorCode)
{
	int page_mask = 0x00000001;
	int protection_mask = 0x00000002;
	
	if((qwErrorCode & page_mask) == 0 )
	{
		kPageFaultExceptionHandler(iVectorNumber,qwErrorCode);
	}
	else if((qwErrorCode & protection_mask) == 2)
	{
		kProtectionFaultExceptionHandler(iVectorNumber,qwErrorCode);
	}

}
void kPageFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 7 ] = { 0, };
	int number = 0;
	int mask = 0x00F00000;
	PTENTRY* pstPTEntry;
	

	for(int i=0;i<6;i++){
		if((number = ((iVectorNumber & mask)>>(20-i*4)))<=9)
			vcBuffer[ i ] = '0' + number;
		else
			vcBuffer[ i ] = 87 + number;
		mask >>= 4;
	}

	
    kPrintString( 0, 0, "====================================================" );
    kPrintString( 0, 1, "                 Page Fault Occur~!!!!               " );
    kPrintString( 0, 2, "                    Address:                         " );
	kPrintString( 28, 2, "0x" );
    kPrintString( 30, 2, vcBuffer );
    kPrintString( 0, 3, "====================================================" );

	pstPTEntry = (PTENTRY*)0x142000;
	kSetPageEntryData(&(pstPTEntry[511]),0,0x1FF000,0x00000003,0);
	invlpg(&iVectorNumber);
}
void kProtectionFaultExceptionHandler( int iVectorNumber, QWORD qwErrorCode )
{
    char vcBuffer[ 7 ] = { 0, };
	int number = 0;
	int mask = 0x00F00000;
	PTENTRY* pstPTEntry;
	

	for(int i=0;i<6;i++){
		if((number = ((iVectorNumber & mask)>>(20-i*4)))<=9)
			vcBuffer[ i ] = '0' + number;
		else
			vcBuffer[ i ] = 87 + number;
		mask >>= 4;
	}

	
    kPrintString( 0, 0, "====================================================" );
    kPrintString( 0, 1, "                 Protection Fault Occur~!!!!               " );
    kPrintString( 0, 2, "                    Address:                         " );
	kPrintString( 28, 2, "0x" );
    kPrintString( 30, 2, vcBuffer );
    kPrintString( 0, 3, "====================================================" );

	pstPTEntry = (PTENTRY*)0x142000;
	kSetPageEntryData(&(pstPTEntry[511]),0,0x1FF000,0x00000003,0);
	invlpg(&iVectorNumber);
}
/**
 *  공통으로 사용하는 인터럽트 핸들러
 */
void kCommonInterruptHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iCommonInterruptCount = 0;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 오른쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iCommonInterruptCount;
    g_iCommonInterruptCount = ( g_iCommonInterruptCount + 1 ) % 10;
    kPrintString( 70, 0, vcBuffer );
    //=========================================================================

    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

/**
 *  키보드 인터럽트의 핸들러
 */
void kKeyboardHandler( int iVectorNumber )
{
    char vcBuffer[] = "[INT:  , ]";
    static int g_iKeyboardInterruptCount = 0;
    BYTE bTemp;

    //=========================================================================
    // 인터럽트가 발생했음을 알리려고 메시지를 출력하는 부분
    // 인터럽트 벡터를 화면 왼쪽 위에 2자리 정수로 출력
    vcBuffer[ 5 ] = '0' + iVectorNumber / 10;
    vcBuffer[ 6 ] = '0' + iVectorNumber % 10;
    // 발생한 횟수 출력
    vcBuffer[ 8 ] = '0' + g_iKeyboardInterruptCount;
    g_iKeyboardInterruptCount = ( g_iKeyboardInterruptCount + 1 ) % 10;
    kPrintString( 0, 0, vcBuffer );
    //=========================================================================

    // 키보드 컨트롤러에서 데이터를 읽어서 ASCII로 변환하여 큐에 삽입
    if( kIsOutputBufferFull() == TRUE )
    {
        bTemp = kGetKeyboardScanCode();
        kConvertScanCodeAndPutQueue( bTemp );
    }

    // EOI 전송
    kSendEOIToPIC( iVectorNumber - PIC_IRQSTARTVECTOR );
}

static inline void invlpg(void *m)
{
	asm volatile("invlpg (%0)" : : "b"(m) : "memory");
}
