#include "ConsoleShell.h"
#include "Console.h"
#include "Keyboard.h"
#include "Utility.h"
#include "PIT.h"
#include "RTC.h"
#include "AssemblyUtility.h"
#include "Timer.h"

char command_history[10][300];
int count = 0;
int task_count = 0;
// Ŀ�ǵ� ���̺� ����
SHELLCOMMANDENTRY gs_vstCommandTable[] =
{
	{ "help", "Show Help", kHelp },
	{ "cls", "Clear Screen", kCls },
	{ "totalram", "Show Total RAM Size", kShowTotalRAMSize },
	{ "strtod", "String To Decial/Hex Convert", kStringToDecimalHexTest },
	{ "shutdown", "Shutdown And Reboot OS", kShutdown },
	{ "settimer", "Set PIT Controller Counter0, ex)settimer 10(ms) 1(periodic)", 
		kSetTimer },
	{ "wait", "Wait ms Using PIT, ex)wait 100(ms)", kWaitUsingPIT },
	{ "rdtsc", "Read Time Stamp Counter", kReadTimeStampCounter },
	{ "cpuspeed", "Measure Processor Speed", kMeasureProcessorSpeed },
	{ "date", "Show Date And Time", kShowDateAndTime },
	{ "createtask", "Create Task", kCreateTestTask },
	{ "time", "Measure Command time", kMeasureCommandTime},
};                                     

//==============================================================================
//  ���� ���� �����ϴ� �ڵ�
//==============================================================================
/**
 *  ���� ���� ����
 */
void kStartConsoleShell( void )
{
	char vcCommandBuffer[ CONSOLESHELL_MAXCOMMANDBUFFERCOUNT ];
	int iCommandBufferIndex = 0;
	BYTE bKey;
	int iCursorX, iCursorY;
	WORD startCounter=0;
	WORD endCounter=0;
	int current = count;
	int i;

	// ������Ʈ ���
	kPrintf( CONSOLESHELL_PROMPTMESSAGE );
	print_date_below_monitor();

	while( 1 )
	{
		// Ű�� ���ŵ� ������ ���
		bKey = kGetCh();
		// Backspace Ű ó��
		if( bKey == KEY_BACKSPACE )
		{
			if( iCommandBufferIndex > 0 )
			{
				// ���� Ŀ�� ��ġ�� �� �� ���� ������ �̵��� ���� ������ ����ϰ� 
				// Ŀ�ǵ� ���ۿ��� ������ ���� ����
				kGetCursor( &iCursorX, &iCursorY );
				kPrintStringXY( iCursorX - 1, iCursorY, " " );
				kSetCursor( iCursorX - 1, iCursorY );
				iCommandBufferIndex--;
			}
		}
		// ���� Ű ó��
		else if( bKey == KEY_ENTER )
		{
			kPrintf( "\n" );
			delete_date_below_monitor();

			if( iCommandBufferIndex > 0 )
			{
				// Ŀ�ǵ� ���ۿ� �ִ� ����� ����
				vcCommandBuffer[ iCommandBufferIndex ] = '\0';
				//startCounter = kReadCounter0();
				if(count != 10){
					kMemCpy(command_history[count++],vcCommandBuffer,iCommandBufferIndex);
					current = count;
				}
				else{
					count = 0;
					kMemCpy(command_history[count++],vcCommandBuffer,iCommandBufferIndex);
				}

				task_count = 0;
				kInitializePIT(0,FALSE);
				startCounter = kReadCounter0();
				kExecuteCommand( vcCommandBuffer );
				 endCounter = kReadCounter0();

				task_count = startCounter - endCounter;
				///	endCounter = kReadCounter0();
				//	kPrintf("measure : %d\n",endCounter - startCounter);
			}
			
			// ������Ʈ ��� �� Ŀ�ǵ� ���� �ʱ�ȭ
			kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );     
			print_date_below_monitor();
			kMemSet( vcCommandBuffer, '\0', CONSOLESHELL_MAXCOMMANDBUFFERCOUNT );
			iCommandBufferIndex = 0;
		}
		// ����Ʈ Ű, CAPS Lock, NUM Lock, Scroll Lock�� ����
		else if( ( bKey == KEY_LSHIFT ) || ( bKey == KEY_RSHIFT ) ||
				( bKey == KEY_CAPSLOCK ) || ( bKey == KEY_NUMLOCK ) ||
				( bKey == KEY_SCROLLLOCK ) )
		{
			;
		}
		else if(bKey==KEY_RIGHT){
			kGetCursor( &iCursorX, &iCursorY );
			if(iCommandBufferIndex+7!=iCursorX)
			kSetCursor( iCursorX+1, iCursorY );
		}
		else if(bKey==KEY_LEFT){
			kGetCursor( &iCursorX, &iCursorY );
			if(7<iCursorX)
			kSetCursor( iCursorX-1, iCursorY );
		}
		else if( bKey == KEY_UP){
			if(current != 0)
				current--;
			kGetCursor( &iCursorX, &iCursorY );
			for( i=80;i>0;--i){
				kPrintStringXY( i , iCursorY, " " );
				kSetCursor( i, iCursorY );
			}
			kSetCursor( 0, iCursorY );
			kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );
			kPrintf("%s",command_history[current]);

			iCommandBufferIndex = iCursorX;
			kMemCpy(vcCommandBuffer,command_history[current],iCommandBufferIndex);
		}
		else if( bKey == KEY_DOWN){

			kGetCursor( &iCursorX, &iCursorY );
			int tmpCursorX = iCursorX;
			for( i=80;i>0;--i){
				kPrintStringXY( i , iCursorY, " " );
				kSetCursor( i, iCursorY );
			}
			kSetCursor( 0, iCursorY );
			kPrintf( "%s", CONSOLESHELL_PROMPTMESSAGE );
			current++;
			if(current < count){
				kPrintf("%s",command_history[current]);
				iCommandBufferIndex = tmpCursorX;
				kMemCpy(vcCommandBuffer,command_history[current],iCommandBufferIndex);
			}
			else if(current == count )
				kPrintf("");
			else{
				current--;
				kPrintf("");
			}
		}
		else
		{
			// TAB�� �������� ��ȯ
			if( bKey == KEY_TAB )
			{
				bKey = ' ';
			}

			// ���ۿ� ������ �������� ���� ����
			if( iCommandBufferIndex < CONSOLESHELL_MAXCOMMANDBUFFERCOUNT )
			{
				vcCommandBuffer[ iCommandBufferIndex++ ] = bKey;
				kPrintf( "%c", bKey );
			}
		}
	}
}

void print_date_below_monitor(){
	int i=0;
	int iCursorX,iCursorY;
	BYTE bSecond, bMinute, bHour;

	for(i=0;i<80;++i)
		kPrintStringXY(i,23,"=");
	
	kPrintTime(0,24,"Running Time : %d%d:%d%d",(task_count / 1000000)/10,(task_count / 1000000)%10,(task_count / 10000)/10,(task_count / 10000)%10);

	// RTC ��Ʈ�ѷ����� �ð� �� ���ڸ� ����
	kReadRTCTime( &bHour, &bMinute, &bSecond );
	kPrintTime(62,24, "Current : %d:%d:%d", bHour, bMinute, bSecond );
	
}

void delete_date_below_monitor(){

	int i=0;
	int iCursorX,iCursorY;
	kGetCursor( &iCursorX, &iCursorY );

	kSetCursor(0,23);
	for(i=0;i<77;++i)
		kPrintf(" ");
	kSetCursor(iCursorX,iCursorY);
}
/*
 *  Ŀ�ǵ� ���ۿ� �ִ� Ŀ�ǵ带 ���Ͽ� �ش� Ŀ�ǵ带 ó���ϴ� �Լ��� ����
 */
void kExecuteCommand( const char* pcCommandBuffer )
{
	int i, iSpaceIndex;
	int iCommandBufferLength, iCommandLength;
	int iCount;

	// �������� ���е� Ŀ�ǵ带 ����
	iCommandBufferLength = kStrLen( pcCommandBuffer );
	for( iSpaceIndex = 0 ; iSpaceIndex < iCommandBufferLength ; iSpaceIndex++ )
	{
		if( pcCommandBuffer[ iSpaceIndex ] == ' ' )
		{
			break;
		}
	}

	// Ŀ�ǵ� ���̺��� �˻��ؼ� ������ �̸��� Ŀ�ǵ尡 �ִ��� Ȯ��
	iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );
	for( i = 0 ; i < iCount ; i++ )
	{
		iCommandLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
		// Ŀ�ǵ��� ���̿� ������ ������ ��ġ�ϴ��� �˻�
		if( ( iCommandLength == iSpaceIndex ) &&
				( kMemCmp( gs_vstCommandTable[ i ].pcCommand, pcCommandBuffer,
						   iSpaceIndex ) == 0 ) )
		{
			gs_vstCommandTable[ i ].pfFunction( pcCommandBuffer + iSpaceIndex + 1 );
			break;
		}
	}

	// ����Ʈ���� ã�� �� ���ٸ� ���� ���
	if( i >= iCount )
	{
		kPrintf( "'%s' is not found.\n", pcCommandBuffer );
	}
}

/**
 *  �Ķ���� �ڷᱸ���� �ʱ�ȭ
 */
void kInitializeParameter( PARAMETERLIST* pstList, const char* pcParameter )
{
	pstList->pcBuffer = pcParameter;
	pstList->iLength = kStrLen( pcParameter );
	pstList->iCurrentPosition = 0;
}

/**
 *  �������� ���е� �Ķ������ ����� ���̸� ��ȯ
 */
int kGetNextParameter( PARAMETERLIST* pstList, char* pcParameter )
{
	int i;
	int iLength;

	// �� �̻� �Ķ���Ͱ� ������ ����
	if( pstList->iLength <= pstList->iCurrentPosition )
	{
		return 0;
	}

	// ������ ���̸�ŭ �̵��ϸ鼭 ������ �˻�
	for( i = pstList->iCurrentPosition ; i < pstList->iLength ; i++ )
	{
		if( pstList->pcBuffer[ i ] == ' ' )
		{
			break;
		}
	}

	// �Ķ���͸� �����ϰ� ���̸� ��ȯ
	kMemCpy( pcParameter, pstList->pcBuffer + pstList->iCurrentPosition, i );
	iLength = i - pstList->iCurrentPosition;
	pcParameter[ iLength ] = '\0';

	// �Ķ������ ��ġ ������Ʈ
	pstList->iCurrentPosition += iLength + 1;
	return iLength;
}

//==============================================================================
//  Ŀ�ǵ带 ó���ϴ� �ڵ�
//==============================================================================
/**
 *  �� ������ ���
 */
void kHelp( const char* pcCommandBuffer )
{
	int i;
	int iCount;
	int iCursorX, iCursorY;
	int iLength, iMaxCommandLength = 0;


	kPrintf( "=========================================================\n" );
	kPrintf( "                    MINT64 Shell Help                    \n" );
	kPrintf( "=========================================================\n" );

	iCount = sizeof( gs_vstCommandTable ) / sizeof( SHELLCOMMANDENTRY );

	// ���� �� Ŀ�ǵ��� ���̸� ���
	for( i = 0 ; i < iCount ; i++ )
	{
		iLength = kStrLen( gs_vstCommandTable[ i ].pcCommand );
		if( iLength > iMaxCommandLength )
		{
			iMaxCommandLength = iLength;
		}
	}

	// ���� ���
	for( i = 0 ; i < iCount ; i++ )
	{
		kPrintf( "%s", gs_vstCommandTable[ i ].pcCommand );
		kGetCursor( &iCursorX, &iCursorY );
		kSetCursor( iMaxCommandLength, iCursorY );
		kPrintf( "  - %s\n", gs_vstCommandTable[ i ].pcHelp );
	}
}

/**
 *  ȭ���� ���� 
 */
void kCls( const char* pcParameterBuffer )
{
	// �� ������ ����� ������ ����ϹǷ� ȭ���� ���� ��, ���� 1�� Ŀ�� �̵�
	kClearScreen();
	kSetCursor( 0, 1 );
}

/**
 *  �� �޸� ũ�⸦ ���
 */
void kShowTotalRAMSize( const char* pcParameterBuffer )
{
	kPrintf( "Total RAM Size = %d MB\n", kGetTotalRAMSize() );
}

/**
 *  ���ڿ��� �� ���ڸ� ���ڷ� ��ȯ�Ͽ� ȭ�鿡 ���
 */
void kStringToDecimalHexTest( const char* pcParameterBuffer )
{
	char vcParameter[ 100 ];
	int iLength;
	PARAMETERLIST stList;
	int iCount = 0;
	long lValue;

	// �Ķ���� �ʱ�ȭ
	kInitializeParameter( &stList, pcParameterBuffer );

	while( 1 )
	{
		// ���� �Ķ���͸� ����, �Ķ������ ���̰� 0�̸� �Ķ���Ͱ� ���� ���̹Ƿ�
		// ����
		iLength = kGetNextParameter( &stList, vcParameter );
		if( iLength == 0 )
		{
			break;
		}

		// �Ķ���Ϳ� ���� ������ ����ϰ� 16�������� 10�������� �Ǵ��Ͽ� ��ȯ�� ��
		// ����� printf�� ���
		kPrintf( "Param %d = '%s', Length = %d, ", iCount + 1, 
				vcParameter, iLength );

		// 0x�� �����ϸ� 16����, �׿ܴ� 10������ �Ǵ�
		if( kMemCmp( vcParameter, "0x", 2 ) == 0 )
		{
			lValue = kAToI( vcParameter + 2, 16 );
			kPrintf( "HEX Value = %q\n", lValue );
		}
		else
		{
			lValue = kAToI( vcParameter, 10 );
			kPrintf( "Decimal Value = %d\n", lValue );
		}

		iCount++;
	}
}

/**
 *  PC�� �����(Reboot)
 */
void kShutdown( const char* pcParamegerBuffer )
{
	kPrintf( "System Shutdown Start...\n" );

	// Ű���� ��Ʈ�ѷ��� ���� PC�� �����
	kPrintf( "Press Any Key To Reboot PC..." );
	kGetCh();
	kReboot();
}

/**
 *  PIT ��Ʈ�ѷ��� ī���� 0 ����
 */
void kSetTimer( const char* pcParameterBuffer )
{
	char vcParameter[ 100 ];
	PARAMETERLIST stList;
	long lValue;
	BOOL bPeriodic;

	// �Ķ���� �ʱ�ȭ
	kInitializeParameter( &stList, pcParameterBuffer );

	// milisecond ����
	if( kGetNextParameter( &stList, vcParameter ) == 0 )
	{
		kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
		return ;
	}
	lValue = kAToI( vcParameter, 10 );

	// Periodic ����
	if( kGetNextParameter( &stList, vcParameter ) == 0 )
	{
		kPrintf( "ex)settimer 10(ms) 1(periodic)\n" );
		return ;
	}    
	bPeriodic = kAToI( vcParameter, 10 );

	kInitializePIT( MSTOCOUNT( lValue ), bPeriodic );
	kPrintf( "Time = %d ms, Periodic = %d Change Complete\n", lValue, bPeriodic );
}

/**
 *  PIT ��Ʈ�ѷ��� ���� ����Ͽ� ms ���� ���  
 */
void kWaitUsingPIT( const char* pcParameterBuffer )
{
	char vcParameter[ 100 ];
	int iLength;
	PARAMETERLIST stList;
	long lMillisecond;
	int i;
	BYTE bHour, bMinute, before;
    kReadRTCTime(&bHour, &bMinute, &before);
	// �Ķ���� �ʱ�ȭ
	kInitializeParameter( &stList, pcParameterBuffer );
	if( kGetNextParameter( &stList, vcParameter ) == 0 )
	{
		kPrintf( "ex)wait 100(ms)\n" );
		return ;
	}

	lMillisecond = kAToI( pcParameterBuffer, 10 );
	kPrintf( "%d ms Sleep Start...\n", lMillisecond );

	// ���ͷ�Ʈ�� ��Ȱ��ȭ�ϰ� PIT ��Ʈ�ѷ��� ���� ���� �ð��� ����
	kDisableInterrupt();
	for( i = 0 ; i < lMillisecond / 30 ; i++ )
	{
		before = printCurrentTime(before);
		kWaitUsingDirectPIT( MSTOCOUNT( 30 ) );
	}
	kWaitUsingDirectPIT( MSTOCOUNT( lMillisecond % 30 ) );   
	kEnableInterrupt();
	kPrintf( "%d ms Sleep Complete\n", lMillisecond );

	// Ÿ�̸� ����
	kInitializePIT( MSTOCOUNT( 1 ), TRUE );
}

/**
 *  Ÿ�� ������ ī���͸� ����  
 */
void kReadTimeStampCounter( const char* pcParameterBuffer )
{
	QWORD qwTSC;

	qwTSC = kReadTSC();
	kPrintf( "Time Stamp Counter = %q\n", qwTSC );
}

/**
 *  ���μ����� �ӵ��� ����
 */
void kMeasureProcessorSpeed( const char* pcParameterBuffer )
{
	int i;
	QWORD qwLastTSC, qwTotalTSC = 0;
	BYTE bHour, bMinute, before;
    kReadRTCTime(&bHour, &bMinute, &before);

	kPrintf( "Now Measuring." );

	// 10�� ���� ��ȭ�� Ÿ�� ������ ī���͸� �̿��Ͽ� ���μ����� �ӵ��� ���������� ����
	kDisableInterrupt();    
	for( i = 0 ; i < 200 ; i++ )
	{
		before = printCurrentTime(before);
		qwLastTSC = kReadTSC();
		kWaitUsingDirectPIT( MSTOCOUNT( 50 ) );
		qwTotalTSC += kReadTSC() - qwLastTSC;

		kPrintf( "." );
	}
	// Ÿ�̸� ����
	kInitializePIT( MSTOCOUNT( 1 ), TRUE );    
	kEnableInterrupt();

	kPrintf( "\nCPU Speed = %d MHz\n", qwTotalTSC / 10 / 1000 / 1000 );
}

/**
 *  RTC ��Ʈ�ѷ��� ����� ���� �� �ð� ������ ǥ��
 */
void kShowDateAndTime( const char* pcParameterBuffer )
{
	BYTE bSecond, bMinute, bHour;
	BYTE bDayOfWeek, bDayOfMonth, bMonth;
	WORD wYear;

	// RTC ��Ʈ�ѷ����� �ð� �� ���ڸ� ����
	kReadRTCTime( &bHour, &bMinute, &bSecond );
	kReadRTCDate( &wYear, &bMonth, &bDayOfMonth, &bDayOfWeek );

	kPrintf( "Date: %d/%d/%d %s, ", wYear, bMonth, bDayOfMonth,
			kConvertDayOfWeekToString( bDayOfWeek ) );
	kPrintf( "Time: %d:%d:%d\n", bHour, bMinute, bSecond );
}

// TCB �ڷᱸ���� ���� ����
static TCB gs_vstTask[ 2 ] = { 0, };
static QWORD gs_vstStack[ 1024 ] = { 0, };

/**
 *  �½�ũ ��ȯ�� �׽�Ʈ�ϴ� �½�ũ
 */
void kTestTask( void )
{
	int i = 0;

	while( 1 )
	{
		// �޽����� ����ϰ� Ű �Է��� ���
		kPrintf( "[%d] This message is from kTestTask. Press any key to switch "
				"kConsoleShell~!!\n", i++ );
		kGetCh();

		// ������ Ű�� �ԷµǸ� �½�ũ�� ��ȯ
		kSwitchContext( &( gs_vstTask[ 1 ].stContext ), &( gs_vstTask[ 0 ].stContext ) );
	}
}

/**
 *  �½�ũ�� �����ؼ� ��Ƽ �½�ŷ ����
 */
void kCreateTestTask( const char* pcParameterBuffer )
{
	KEYDATA stData;
	int i = 0;

	// �½�ũ ����
	kSetUpTask( &( gs_vstTask[ 1 ] ), 1, 0, ( QWORD ) kTestTask, &( gs_vstStack ), 
			sizeof( gs_vstStack ) );

	// 'q' Ű�� �Էµ��� ���� ������ ����
	while( 1 )
	{
		// �޽����� ����ϰ� Ű �Է��� ���
		kPrintf( "[%d] This message is from kConsoleShell. Press any key to "
				"switch TestTask~!!\n", i++ );
		if( kGetCh() == 'q' )
		{
			break;
		}
		// ������ Ű�� �ԷµǸ� �½�ũ�� ��ȯ
		kSwitchContext( &( gs_vstTask[ 0 ].stContext ), &( gs_vstTask[ 1 ].stContext ) );
	}
}

void kMeasureCommandTime( const char* pcParameterBuffer )
{
	QWORD startCounter=0;
	QWORD endCounter=0;
	WORD counter = 0;
	WORD hour, min, sec,msec,nsec,micsec;
	unsigned long ini, end=0;


	startCounter = kReadTSC();
	kExecuteCommand( pcParameterBuffer );
	endCounter  = kReadTSC();
	counter = (endCounter - startCounter);

	min = counter / 60000;
	sec = counter / 10000;
	msec = (counter % 10000)/10;
	// kPrintf("measure command time : %d\n",counter);
	nsec = (counter *100000)% 1000;
	micsec = (counter * 100 )% 1000;
	kPrintf("real %d:%d:%d:%d:%d\n",min,sec,msec,micsec,nsec);
}
