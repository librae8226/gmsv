#include "version.h"
#include <string.h>
#include "char.h"
#include "object.h"
#include "char_base.h"
#include "npcutil.h"
#include "npc_memberman.h"
#include "lssproto_serv.h"
#include "saacproto_cli.h"
#include "readmap.h"
#include "battle.h"
#include "log.h"
#include "enemy.h"
#include "handletime.h"
#include "npc_eventaction.h"

//特地从天堂高薪挖角来的"传送师"
#ifdef _TRANSER_MAN
enum {
	WINDOW_START=2,
	WINDOW_SELECT=1,
	WINDOW_NEXT=8,
	WINDOW_NEXT1=15,
	WINDOW_NEXT2=22,
	WINDOW_NEXT3=29,
	WINDOW_NEXT4=36,
	WINDOW_NEXT5=43,
	WINDOW_WARP=4,
	WINDOW_END=3,
};
enum {
	NPC_WORK_CURRENTTIME = CHAR_NPCWORKINT1,
/*
	NPC_WORK_ROUTETOY = CHAR_NPCWORKINT2,
	NPC_WORK_ROUTEPOINT = CHAR_NPCWORKINT3,
	NPC_WORK_ROUNDTRIP = CHAR_NPCWORKINT4,
	NPC_WORK_MODE = CHAR_NPCWORKINT5,
	NPC_WORK_CURRENTROUTE = CHAR_NPCWORKINT6, 
	NPC_WORK_ROUTEMAX = CHAR_NPCWORKINT7,
	NPC_WORK_WAITTIME = CHAR_NPCWORKINT8,
	NPC_WORK_CURRENTTIME = CHAR_NPCWORKINT9,
	NPC_WORK_SEFLG = CHAR_NPCWORKINT10,
*/
};

static void NPC_MemberMan_selectWindow( int meindex, int toindex, int num,int select);
int MemberMan_GetMenuStr( int meindex, int toindex, char *npcarg, char *token, int num);

BOOL NPC_MemberManInit( int meindex )
{
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
		
	if(NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg))==NULL){
		print("TRANSER_MAN: GetArgStrErr!!");
		return FALSE;
	}

   	CHAR_setInt( meindex, CHAR_WHICHTYPE, CHAR_TRANSERMANS);

	//CHAR_setInt( meindex, CHAR_LOOPINTERVAL, NEWNPCMAN_STANDBY);
	CHAR_setWorkInt( meindex, NPC_WORK_CURRENTTIME, NowTime.tv_sec);
	return TRUE;
}
//CHAR_setWorkInt(talkerindex,CHAR_WORKSHOPRELEVANT,1);
void NPC_MemberManTalked( int meindex, int talkerindex, char *msg, int color )
{
	if( CHAR_getInt( talkerindex , CHAR_WHICHTYPE ) != CHAR_TYPEPLAYER )
    		return;
	if(NPC_Util_isFaceToFace( meindex ,talkerindex , 2) == FALSE) {
		if(NPC_Util_isFaceToChara( talkerindex, meindex, 1) == FALSE)
			return;
	}

	CHAR_setWorkInt( talkerindex, CHAR_WORKSHOPRELEVANT, 0);
	NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_START, 0);
}

void NPC_MemberManWindowTalked( int meindex, int talkerindex, int seqno,
								int select, char *data)
{
	if( select == WINDOW_BUTTONTYPE_CANCEL || select == WINDOW_BUTTONTYPE_NO)
		return;
	switch( seqno)	{
	case NPC_TRANSERMAN_START:
		break;
	case NPC_TRANSERMAN_SELECT:
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_SELECT, atoi( data));
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data));
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT1:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT1, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_SELECT, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+7);
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT2:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT2, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+14);
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT3:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT3, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT1, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+21);
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT4:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT4, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT2, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+28);
		break;
	case CHAR_WINDOWTYPE_MEMBER_NEXT5:
		if( select == WINDOW_BUTTONTYPE_NEXT ){						//下一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT5, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT3, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+35);
		break;
	case NPC_TRANSERMAN_WARP:
		if ( select == WINDOW_BUTTONTYPE_PREV ){		//上一页
			NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_NEXT4, atoi( data));
			return;
		}else if ( select == WINDOW_BUTTONTYPE_CANCEL ){	//CANCEL
			return;
		}
		NPC_MemberMan_selectWindow( meindex, talkerindex, WINDOW_WARP, atoi( data)+42);
		break;
	case NPC_TRANSERMAN_END:
		break;
	}
}

static void NPC_MemberMan_selectWindow( int meindex, int toindex, int num,int select)
{
	char npcarg[NPC_UTIL_GETARGSTR_BUFSIZE];
	char token[256];

	int buttontype = 0, windowtype = 0, windowno = 0;
	int fd = getfdFromCharaIndex( toindex);
	int runint;
	windowtype = WINDOW_MESSAGETYPE_MESSAGE;

	if(NPC_Util_isFaceToFace( meindex ,toindex , 2) == FALSE) {
		if(NPC_Util_isFaceToChara( toindex, meindex, 1) == FALSE)
			return;
	}

	memset( npcarg, 0, sizeof( npcarg));
	if(NPC_Util_GetArgStr( meindex, npcarg, sizeof(npcarg))==NULL){
		print("TRANSER_MAN: GetArgStrErr!!");
		return;
	}
	memset( token, 0, sizeof( token));

	switch( num)	{
	case WINDOW_START:
		if( NPC_Util_GetStrFromStrWithDelim( npcarg, "START_MSG", token, sizeof( token) ) == NULL) {
			print("MemberMan Get START_MSG ERROR !");
			return;
		}
		CHAR_setWorkInt( toindex, CHAR_WORKSHOPRELEVANT, WINDOW_START);
		buttontype = WINDOW_BUTTONTYPE_YESNO;
		windowno = NPC_TRANSERMAN_SELECT;
		break;
	case WINDOW_SELECT:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_SELECT);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT;
		break;
	case WINDOW_NEXT:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT1;
		break;
	case WINDOW_NEXT1:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT1);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT2;
		break;
	case WINDOW_NEXT2:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT2);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT3;
		break;
	case WINDOW_NEXT3:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT3);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT4;
		break;
	case WINDOW_NEXT4:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT4);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		if (runint == 2)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_NEXT | WINDOW_BUTTONTYPE_CANCEL;
		if (runint == 1)buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = CHAR_WINDOWTYPE_MEMBER_NEXT5;
		break;
	case WINDOW_NEXT5:
		runint = MemberMan_GetMenuStr( meindex, toindex, npcarg, token,WINDOW_NEXT5);
		if( runint == 0 )	{
			print( "MemberMan GetMenu ERROR !!");
			return;
		}
		windowtype = WINDOW_MESSAGETYPE_SELECT;
		buttontype = WINDOW_BUTTONTYPE_PREV | WINDOW_BUTTONTYPE_CANCEL;
		windowno = NPC_TRANSERMAN_WARP;
		break;
	case WINDOW_WARP:
		if( ActionNpc_CheckMenuFree( meindex, toindex, npcarg, select) == FALSE ){
			memset( token, 0, sizeof( token));
			if( NPC_Util_GetStrFromStrWithDelim( npcarg, "FAIL_MSG", token, sizeof( token) ) != NULL) {
				CHAR_talkToCli( toindex, meindex, token, CHAR_COLORYELLOW);
			}
		}
		return;

	case WINDOW_END:
		break;
	}

	lssproto_WN_send( fd, windowtype, buttontype, windowno,
		CHAR_getWorkInt( meindex, CHAR_WORKOBJINDEX), token);
}

int MemberMan_GetMenuStr( int meindex, int toindex, char *npcarg, char *token, int num)
{
	char buf1[1024], buf2[256], buf3[256];
	BOOL FINDS=0;
	int talkNo=num;
	if( npcarg == NULL ) return FALSE;
	
	if( NPC_Util_GetStrFromStrWithDelim( npcarg, "MenuHead", buf1, sizeof( buf1) ) == NULL){
		print("MemberMan Get MenuHead ERROR!");
		return FALSE;
	}
	int MemberPoin = CHAR_getInt(toindex, CHAR_MEMBERORDER);
	sprintf( token,"%s %d \n", buf1,MemberPoin);
	//printf(token);
	while( getStringFromIndexWithDelim( npcarg,"}",talkNo, buf1, sizeof( buf1)) != FALSE )	{
		talkNo++;
		if (talkNo>(num+7))
			{
				FINDS = 2;
				break;
			}
		memset( buf2, 0, sizeof( buf2));
		if( NPC_Util_GetStrFromStrWithDelim( buf1, "MenuStr", buf2, sizeof( buf2)) == NULL  )
			continue;
		FINDS = 1;

		sprintf( buf3, "  %s\n", buf2);
		strcat( token, buf3);
	}

	return FINDS;
}

#endif


