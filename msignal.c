#include "version.h"
#include <signal.h>
#include <errno.h>
#include <unistd.h>

#include <stdio.h>

#include "buf.h"
#include "net.h"
#include "char.h"
#include "item.h"
#include "object.h"
#include "configfile.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "log.h"
#include "petmail.h"

/*------------------------------------------------------------
 * lsprotocol 及伐□平件弘    毛荸  允月楮醒毛裟少［
 * 娄醒｝忒曰袄
 *  卅仄
 ------------------------------------------------------------*/
static void endlsprotocol( void )
{
    lssproto_CleanupServer();
    saacproto_CleanupClient();
}

/*------------------------------------------------------------
 * 允屯化及忡绣仄卅仃木壬卅日卅中犯□正毛母件皿允月楮醒
 * 公木冗木及乒斥亘□伙及楮醒毛裟少及心［
 * 娄醒｝忒曰袄
 *  卅仄
 ------------------------------------------------------------*/
static void allDataDump( void )
{
    closeAllLogFile();
#ifndef _SIMPLIFY_ITEMSTRING
	storeObjects( getStoredir() );
	storePetmail();
#endif
	storeCharaData();
}

/*------------------------------------------------------------
 * 皿夫弘仿丞及蔽  质  及凶户卞裟太请今木月［
 * 娄醒｝忒曰袄
 *  卅仄
 ------------------------------------------------------------*/
void shutdownProgram( void )
{
    close( acfd );
    close( bindedfd );
    endlsprotocol();
    endConnect();
    memEnd();
}

char *DebugFunctionName = NULL;
//char *DebugFunctionName_CHAR = NULL;
//char *DebugFunctionName_ITEM = NULL;
int DebugPoint = 0;

void sigshutdown( int number )
{
    print( "Received signal : %d\n" , number  );
    if( number == 0 )print( "\ngmsv normal down\n" );
    print( "\nDebugPoint (%d)\n", DebugPoint );
    print( "\nLastFunc (%s)\n", DebugFunctionName );

	remove( "gmsvlog.err2");
	rename( "gmsvlog.err1", "gmsvlog.err2" );
	rename( "gmsvlog.err", "gmsvlog.err1" );
	rename( "gmsvlog", "gmsvlog.err" );
#if USE_MTIO
    {
        void MTIO_join(void);
        MTIO_join();
    }
#endif
    allDataDump();


    signal( SIGINT , SIG_IGN );
    signal( SIGQUIT, SIG_IGN );
    signal( SIGKILL, SIG_IGN );
    signal( SIGSEGV, SIG_IGN );
    signal( SIGPIPE, SIG_IGN );
    signal( SIGTERM, SIG_IGN );
		signal( SIGALRM, SIG_IGN );

    shutdownProgram();

    exit(number);
}

extern jztimeout;

void jztime_out( int number )
{
	print("超时机制生效");
	jztimeout=TRUE;
}
void signalset( void )
{
    // CoolFish: Test Signal 2001/10/26
    print("\nCoolFish Get Signal..\n");

	print("SIGINT:%d\n", SIGINT);
	print("SIGQUIT:%d\n", SIGQUIT);
	print("SIGKILL:%d\n", SIGKILL);
	print("SIGSEGV:%d\n", SIGSEGV);
	print("SIGPIPE:%d\n", SIGPIPE);
	print("SIGTERM:%d\n", SIGTERM);
  print("SIGALRM:%d\n", SIGALRM);

    signal( SIGINT , sigshutdown );  //终端中断符
    signal( SIGQUIT, sigshutdown );	 //终端退出符
    signal( SIGKILL, sigshutdown );	 //终止
    signal( SIGSEGV, sigshutdown );	 //无效储存访问
    signal( SIGPIPE, SIG_IGN );			 //写至无读进程的管道
    signal( SIGTERM, sigshutdown );	 //终止
    signal( SIGALRM, jztime_out  );	 //超时
}
