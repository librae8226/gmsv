#include "version.h"
#define __NPCCREATE__

#include <stdio.h>
#include <string.h>

#include "common.h"
#include "npccreate.h"
#include "npctemplate.h"
#include "util.h"
#include "buf.h"
#include "readmap.h"
#include "char_data.h"
#include "handletime.h"
#include "configfile.h"

INLINE int NPC_CHECKCREATEINDEX(int index)
{
    if(  NPC_createnum <= index || index < 0 )return FALSE;
    return TRUE;
}

INLINE int NPC_CHECKCREATEINTINDEX( int index)
{
    if(  NPC_CREATEINTNUM <= index || index < 0 )return FALSE;
    return TRUE;
}

INLINE int NPC_setCreateInt( int index , NPC_CREATEINT element, int data )
{
    int buf;
    buf = NPC_create[index].intdata[element];
    NPC_create[index].intdata[element] = data;
    return buf;
}

INLINE int NPC_getCreateInt( int index , NPC_CREATEINT element )
{
    return NPC_create[index].intdata[element];
}


BOOL NPC_initCreateArray( int createnum )
{
    NPC_createnum = createnum;
    NPC_create  = (NPC_Create*)allocateMemory( sizeof( NPC_Create ) * NPC_createnum );

	print( "init Char size:%d. createnum:%d.\n", sizeof( NPC_Create ), createnum);
    if( NPC_create == NULL ){
		return FALSE;
	}
    NPC_create_readindex = 0;
    return TRUE;
}

void NPC_setDefaultNPCCreate( NPC_Create*  cr )
{
    if( cr == NULL )return;

    cr->intdata[NPC_CREATEFLOORID]=0;
    cr->intdata[NPC_CREATEBORNLEFTUPX]=0;
    cr->intdata[NPC_CREATEBORNLEFTUPY]=0;
    cr->intdata[NPC_CREATEBORNRIGHTDOWNX]=0;
    cr->intdata[NPC_CREATEBORNRIGHTDOWNY]=0;
    cr->intdata[NPC_CREATEMOVELEFTUPX]=0;
    cr->intdata[NPC_CREATEMOVELEFTUPY]=0;
    cr->intdata[NPC_CREATEMOVERIGHTDOWNX]=0;
    cr->intdata[NPC_CREATEMOVERIGHTDOWNY]=0;

    cr->intdata[NPC_CREATEDIR]=0;
	//jeffrey 1231
#ifdef _ADD_ACTION
	cr->intdata[NPC_CREATEACTION] = 0;
#endif
    cr->intdata[NPC_CREATEBASEIMAGENUMBER]=-1;
    cr->intdata[NPC_CREATETIME]=0;
    cr->intdata[NPC_CREATEBORNNUM]=0;
    cr->intdata[NPC_CREATEENEMYNUM]=0;
    cr->intdata[NPC_CREATEBOUNDARY]=1;
    cr->intdata[NPC_CREATEIGNOREINVINCIBLE]=0;

    cr->intdata[NPC_CREATEDATE]=0;
    
    cr->intdata[NPC_CREATEFAMILY]=0;

    cr->chardata[NPC_CREATENAME].string[0]= '\0';

    cr->workdata[NPC_CREATEWORKENEMYNUM]=0;
    cr->workdata[NPC_CREATEWORKMAKESTARTSEC]=0;
    cr->workdata[NPC_CREATEWORKMAKESTARTUSEC]=0;
    cr->workdata[NPC_CREATEWORKNEVERMAKE]=0;

    
    {
        int i;
        for( i=0 ; i<arraysizeof(cr->templateindex) ; i++ ){
            cr->templateindex[i] = -1;
            cr->arg[i].string[0] = '\0';
        }
    }
}

BOOL NPC_IsNPCCreateFile( char* filename )
{
    FILE*   f;
    char    line1[128];
    char*   ret;

    /*  ~∆•±Œ‘¬∞◊—ÎƒÃªÔ∑¥ÿ¶ÿ∆±Â‘ ‘¬ */
    if( filename == NULL
        || strlen( filename ) < 1
        || filename[strlen(filename)-1] == '~'
        || filename[0] == '#'
        || strcmptail( filename, ".bak" ) == 0 )return FALSE;

    f= fopen( filename , "r" );
    if( f == NULL ) goto RETURNFALSE;

    ret = fgets( line1, sizeof( line1 ), f );
    if( ret == NULL )goto FCLOSERETURNFALSE;

    if( strcasecmp( NPC_CREATEFILEMAGIC, line1 ) == 0 ){
        fclose(f);
        return TRUE;
    }

FCLOSERETURNFALSE:
    fclose(f);
RETURNFALSE:
    return FALSE;
}

static int NPC_readCreateFile( char* filename )
{
    FILE*   f;
    char    line[512];
    int     linenum=0;
    int     start=OFF;
    NPC_Create  cr;
    int     enemyreadindex=0;

    typedef struct tagPOINT
    {
        int x,y;
    }POINT;
    typedef struct tagREC
    {
        int w,h;
    }REC;
    POINT   center[2]={{0,0},{0,0}};     /*  born ª• 0 ∆• move ª• 1 */
    REC     wh[2]    ={{0,0},{0,0}};      /*  born ª• 0 ∆• move ª• 1 */

    POINT   lu[2]    ={{0,0},{0,0}};     /*  born ª• 0 ∆• move ª• 1 */
    POINT   rd[2]    ={{0,0},{0,0}};     /*  born ª• 0 ∆• move ª• 1 */

    int     defborn=FALSE;              /*  born √´…¨¿√ÿ∆–◊æÆ…˝µ§æÆ */
    int     defmove=FALSE;              /*  move √´…¨¿√ÿ∆–◊æÆ…˝µ§æÆ */
    int     deflurd[2]={FALSE,FALSE};   /*  lu,rd ∆•…¨¿√ÿ∆–◊æÆ…˝µ§æÆ */
    char*   ret;


    if( NPC_create_readindex >= NPC_createnum ){
        print("Create Exceed configured buffer\n" );
        print("configured createnumber is %d\n",NPC_createnum);
        print("Unread File:%s\n",filename);
        return FALSE;
    }

    NPC_setDefaultNPCCreate( &cr );

    f= fopen( filename ,"r");
    if( f == NULL )return FALSE;

    ret = fgets( line, sizeof( line ), f );
    if( ret == NULL )goto FCLOSERETURNFALSE;
    if( strcmp( NPC_CREATEFILEMAGIC, line ) != 0 ){
        print( "This file is not a create file.\n" );
        goto FCLOSERETURNFALSE;
    }
    linenum = 1;

    while( fgets( line , sizeof( line ) , f ) ){

        linenum++;

        if( line[0] == '#' )continue;        /* comment */
        if( line[0] == '\n' )continue;       /* none    */
        chomp( line );

        switch( line[0] ){
        case '{':
            if( start == ON ){
                fprint( "Find {. But already START state. %s:%d\n",
                        filename, linenum);
                fprint( "exit\n" );
                goto FCLOSERETURNFALSE;

            }else{
                NPC_setDefaultNPCCreate( &cr );
                start = ON;
            }
            break;
        case '}':
            if( start == ON ){
                int     err = FALSE;
                if( enemyreadindex == 0 ){
                    err = TRUE;
                    print( "There is no enemydata  %s:%d\n",
                           filename,linenum);
                }else if( MAP_IsThereSpecificFloorid(
                    cr.intdata[NPC_CREATEFLOORID]) == FALSE ){
                    err = TRUE;
                    print( "Invalid Floor ID arround %s:%d-floor:%d\n",
                           filename,linenum, cr.intdata[NPC_CREATEFLOORID]);
                }else if( defborn == FALSE ){
                    err = TRUE;
                    print( "There is no born setting %s:%d\n",
                           filename,linenum);
                }

                if( err == FALSE ){
                    cr.intdata[NPC_CREATEENEMYNUM] = enemyreadindex;

                    if( deflurd[0] ){
                        cr.intdata[NPC_CREATEBORNLEFTUPX] =
                            min(lu[0].x,rd[0].x);
                        cr.intdata[NPC_CREATEBORNRIGHTDOWNX] =
                            max(lu[0].x,rd[0].x);
                        cr.intdata[NPC_CREATEBORNLEFTUPY] =
                            min(lu[0].y,rd[0].y);
                        cr.intdata[NPC_CREATEBORNRIGHTDOWNY] =
                            max(lu[0].y,rd[0].y);
                    }else{
                        cr.intdata[NPC_CREATEBORNLEFTUPX] =
                            center[0].x-wh[0].w/2;
                        cr.intdata[NPC_CREATEBORNRIGHTDOWNX] =
                            center[0].x+wh[0].w/2;
                        cr.intdata[NPC_CREATEBORNLEFTUPY] =
                            center[0].y-wh[0].h/2;
                        cr.intdata[NPC_CREATEBORNRIGHTDOWNY] =
                            center[0].y+wh[0].h/2;
                    }
					/* 99/01/21 ‘˙˜˙ [0]-->[1]±Âƒ§ø“ */
                    if( defmove ){
                        if( deflurd[0] ){
                            cr.intdata[NPC_CREATEMOVELEFTUPX] =
                                min(lu[1].x,rd[1].x);
                            cr.intdata[NPC_CREATEMOVERIGHTDOWNX] =
                                max(lu[1].x,rd[1].x);
                            cr.intdata[NPC_CREATEMOVELEFTUPY] =
                                min(lu[1].y,rd[1].y);
                            cr.intdata[NPC_CREATEMOVERIGHTDOWNY] =
                                max(lu[1].y,rd[1].y);
                        }else{
                            cr.intdata[NPC_CREATEMOVELEFTUPX] =
                                center[1].x-wh[1].w/2;
                            cr.intdata[NPC_CREATEMOVERIGHTDOWNX] =
                                center[1].x+wh[1].w/2;
                            cr.intdata[NPC_CREATEMOVELEFTUPY] =
                                center[1].y-wh[1].h/2;
                            cr.intdata[NPC_CREATEMOVERIGHTDOWNY] =
                                center[1].y+wh[1].h/2;
                        }

                    }else{
                        /*  born √´ŒÏÒ‚°ı‘ ‘¬ */
                        cr.intdata[NPC_CREATEMOVELEFTUPX] =
                            cr.intdata[NPC_CREATEBORNLEFTUPX];
                        cr.intdata[NPC_CREATEMOVERIGHTDOWNX] =
                            cr.intdata[NPC_CREATEBORNRIGHTDOWNX];
                        cr.intdata[NPC_CREATEMOVELEFTUPY] =
                            cr.intdata[NPC_CREATEBORNLEFTUPY];
                        cr.intdata[NPC_CREATEMOVERIGHTDOWNY] =
                            cr.intdata[NPC_CREATEBORNRIGHTDOWNY];
                    }

                    memcpy( &NPC_create[NPC_create_readindex], &cr,
                            sizeof( NPC_Create ) );


                    NPC_create_readindex++;

                    if( NPC_create_readindex >= NPC_createnum ){
                        print("Create Exceed configured buffer\n" );
                        print("configured createnumber is %d\n",
                              NPC_createnum);
                        print("Unread File:%s\n",filename);
                        goto FCLOSERETURNFALSE;
                    }

                }
                /*  ‚Ÿ”Â∞¿±Â  ÿ∆ªØ‘∆»   */
                NPC_setDefaultNPCCreate( &cr );
                enemyreadindex=0;
                defborn=FALSE;
                defmove=FALSE;
                deflurd[0]=FALSE;
                deflurd[1]=FALSE;

                start = OFF;
            }else{
                fprint( "Unresolved '}' at %s:%d\n",filename,linenum);
                goto FCLOSERETURNFALSE;
            }
            break;
        default:
        {
 
            char    firstToken[256];
            char    secondToken[256];
            int     ret;
            ret = getStringFromIndexWithDelim( line, "=",  1, firstToken,
                                               sizeof( firstToken ) );
            if( ret == FALSE ){
                print( "Find error at %s in line %d. Ignore\n",
                       filename , linenum);
                break;
            }
            ret = getStringFromIndexWithDelim( line, "=", 2, secondToken,
                                               sizeof( secondToken ) );
            if( ret == FALSE ){
                print( "Find error at %s in line %d. Ignore\n",
                       filename , linenum);
                break;
            }

            if( strcasecmp("floorid",firstToken) == 0 ){
                cr.intdata[NPC_CREATEFLOORID] = atoi(secondToken);

            }else if( strcasecmp("borncenter", firstToken) == 0 ){
                getFourIntsFromString(secondToken,&center[0].x,
                            &center[0].y,&wh[0].w,&wh[0].h );
                defborn = TRUE;
            }else if( strcasecmp("borncorner", firstToken) == 0 ){
                getFourIntsFromString(secondToken,&lu[0].x,&lu[0].y,
                            &rd[0].x,&rd[0].y );
                defborn = TRUE;
                deflurd[0]=TRUE;

            }else if( strcasecmp("movecenter", firstToken) == 0 ){
                getFourIntsFromString(secondToken,&center[1].x,
                            &center[1].y,&wh[1].w,&wh[1].h );
                defmove = TRUE;
            }else if( strcasecmp("movecorner", firstToken) == 0 ){
                getFourIntsFromString(secondToken,&lu[1].x,&lu[1].y,
                            &rd[1].x,&rd[1].y );
                defmove = TRUE;
                deflurd[1]=TRUE;

            }else if( strcasecmp("dir",firstToken) == 0 ){
                cr.intdata[NPC_CREATEDIR] = atoi( secondToken );

            }else if( strcasecmp("graphicname",firstToken) == 0 ){
                cr.intdata[NPC_CREATEBASEIMAGENUMBER]
                    = CHAR_seekGraphicNumberFromString(secondToken);

            }else if( strcasecmp("name",firstToken) == 0 ){
                strcpysafe( cr.chardata[NPC_CREATENAME].string,
                            sizeof(cr.chardata[NPC_CREATENAME].string),
                            secondToken );

            }else if( strcasecmp("time",firstToken) == 0 ){
                cr.intdata[NPC_CREATETIME] = atoi( secondToken );

            }else if( strcasecmp("date",firstToken) == 0 ){
                cr.intdata[NPC_CREATEDATE] = atoi( secondToken );

            }else if( strcasecmp("createnum",firstToken) == 0 ){
                cr.intdata[NPC_CREATEBORNNUM] = atoi( secondToken );

            }else if( strcasecmp( "boundary", firstToken) == 0 ){
                cr.intdata[NPC_CREATEBOUNDARY] = atoi( secondToken );

            }else if( strcasecmp( "ignoreinvincible", firstToken) == 0 ){
                cr.intdata[NPC_CREATEIGNOREINVINCIBLE] = atoi( secondToken );

		//jeffrey 1231
#ifdef _ADD_ACTION
			}else if( strcasecmp( "action", firstToken) == 0 ){
				cr.intdata[NPC_CREATEACTION] = atoi( secondToken );
#endif		
	    // Robin 0731
            }else if( strcasecmp( "family", firstToken) == 0 ){
                cr.intdata[NPC_CREATEFAMILY] = atoi( secondToken );

            }else if( strcasecmp("enemy", firstToken) == 0 ){
                /*  ¬¶–—√´  ¡˘‘¬∑Ωµ§±Â‘ ‘¬  */
                int     templateindex;
                char    enemyname[64];

                /*  —„ÓŒÀ≤√Òƒ·”¿€Õ  */
                if( enemyreadindex <= arraysizeof(cr.templateindex) )
                    /* OK */
                    ;
                else
                    break;

                /*  secondToken √´  ŸØ  ±Â€–›©‘ ‘¬    */
                getStringFromIndexWithDelim(secondToken,"|",1,enemyname,
                                            sizeof(enemyname) );
                templateindex= NPC_templateGetTemplateIndex(enemyname);
                if( templateindex != -1 ){
                    cr.templateindex[enemyreadindex] = templateindex;
                    {
                        /*  ¬¶–—√´÷   ‘ ‘¬  */
                        char    tmp[2];
                        int     ret;
                        ret = getStringFromIndexWithDelim( secondToken,
                                                           "|",2,tmp,
                                                           sizeof(tmp) );
                        if( ret == FALSE )
                            cr.arg[enemyreadindex].string[0] = '\0';
                        else
                            strcpysafe( cr.arg[enemyreadindex].string,
                                        sizeof( cr.arg[enemyreadindex].
                                                string ),
                                        secondToken+strlen(enemyname)+1 );
                    }
                    enemyreadindex++;
                }else
                    fprint( "No such template:\n[%s(%d)%s] enemy:%s\n",
                            filename ,linenum,secondToken, enemyname);
            }else{
                fprint( "No such entry %s %s:%d\n",firstToken,filename,
                        linenum );
            }
        }
        }
    }

    fclose(f);
    return TRUE;

FCLOSERETURNFALSE:
    fclose(f);
    return FALSE;
}

BOOL NPC_readNPCCreateFiles( char* topdirectory ,int createsize)
{
    STRING64    *filenames;
    int         filenum;
    int         i;
    filenames = (STRING64 *)allocateMemory( sizeof( STRING64 ) *
                                              (int)getFilesearchnum( ) );
	if( filenames == NULL ){
	    print( "Reading NPC Create files... Memory Error\n" );
		return FALSE;
	}
    filenum = rgetFileName( topdirectory ,filenames ,getFilesearchnum( ) );
    if( filenum == -1 ){
	    freeMemory( filenames );
    	return FALSE;
    }
	if( filenum >= getFilesearchnum( ) ){
		print( "filessearchnum  failure %d\n", getFilesearchnum( ) );
		while( 1 );
	}
    if( !NPC_initCreateArray( createsize ) ){
        fprint( "initCreateArray Error\n" );
	    freeMemory( filenames );
        return FALSE;
    }
    print( "Reading NPC Create files...   " );
    for( i = 0 ; i < filenum ; i ++ )
        if( NPC_IsNPCCreateFile( filenames[i].string ))
            if( NPC_readCreateFile( filenames[i].string ) == -1 )
                break;
    print( "There is %d valid create\n" , NPC_create_readindex );
     NPC_createnum=NPC_create_readindex;
#ifdef DEBUG
	/* 99/4/8 By Kawata csv‰ﬂ÷ø±Âÿ∆ªØ÷ß‘¬ */
	print( "Npc_Create's\n");
    for( i = 0 ; i < NPC_createnum ; i ++ ){
        int     j;
        /*print( "NPC_create[%d]:\n", i);*/
        print( "%d,", i);
        for( j = 0 ; j < NPC_CREATECHARNUM ; j ++ )
            print("%s," , NPC_create[i].chardata[j].string );
        /*print( "\tIntdata\t" );*/
        for( j = 0 ; j < NPC_CREATEINTNUM ; j ++ )
            print("%d," , NPC_create[i].intdata[j] );
        /***print("\n" );
        print( "\tChardata\t" );***/
        for( j = 0 ; j < NPC_create[i].intdata[NPC_CREATEENEMYNUM]; j ++ )
            print("%d" , NPC_create[i].templateindex[j] );
        /*print("\n" );*/
        print("\n" );
    }
#endif /*DEBUG*/

    freeMemory( filenames );
    return TRUE;
}



/*------------------------------------------------------------
 * createindex æÆ»’ boundary ª•…¨¿√ΩÒƒæªØ÷–‘¬æÆ…˝µ§æÆ√´Œ≠‘¬
 * ¬¶–—
 *  createindex         int         createº∞ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  …¨¿√ΩÒƒæªØ÷–‘¬          TRUE(1)
 *  …¨¿√ΩÒƒæªØ÷–ÿ¶÷–        FALSE(0)
 ------------------------------------------------------------*/
BOOL NPC_isBoundarySet( int createindex )
{
    if( NPC_CHECKCREATEINDEX(createindex) == FALSE )return FALSE;
    return NPC_create[createindex].intdata[NPC_CREATEBOUNDARY];
}
/*------------------------------------------------------------
 * createindex æÆ»’  ÿÍ‘¬  «¯º∞RECT √´∑∆ªß‘¬£€
 * ¬¶–—
 *  createindex         int           º˛√Û“¡°ı–˛º∞ƒÃº˛∑∏”¿€Õµ©
 *  r                   RECT*       ∞¿√´∞æÿÍŒÁ‘¬RECT º∞∫ÃƒÃº˛’˝
 * ﬂØ‘ª∞¿
 *  ‘¿      TRUE(1)
 *  ¡√      FALSE(0)
 ------------------------------------------------------------*/
BOOL NPC_createGetRECT( int createindex,RECT*   r )
{
    if( NPC_CHECKCREATEINDEX(createindex) == FALSE )return FALSE;

    r->x = NPC_create[createindex].intdata[NPC_CREATEMOVELEFTUPX];
    r->y = NPC_create[createindex].intdata[NPC_CREATEMOVELEFTUPY];
    r->width
        = NPC_create[createindex].intdata[NPC_CREATEMOVERIGHTDOWNX]
        - NPC_create[createindex].intdata[NPC_CREATEMOVELEFTUPX];
    r->height
        = NPC_create[createindex].intdata[NPC_CREATEMOVERIGHTDOWNY]
        - NPC_create[createindex].intdata[NPC_CREATEMOVELEFTUPY];
    return TRUE;
}



/*------------------------------------------------------------
 * ¡›√ﬁæÆ»’◊€‘»ªØ ÷÷–÷–æÆ…˝µ§æÆ√Òƒ·”¿€Õ‘ ‘¬£€
 * ¬¶–—
 *  cindex      int     ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  BOOL    ◊€‘»ªØ÷–÷–ËÎƒ˛  TRUE(1)
 *  BOOL    ◊€‘»ªØ÷–ÿÍÿ¶÷–ËÎƒ˛  FALSE(0)
 ------------------------------------------------------------*/
BOOL NPC_createCheckGenerateFromTime( int cindex )
{
    struct timeval  old;
    if( !NPC_CHECKCREATEINDEX(cindex) )
        return FALSE;

    if( NPC_create[cindex].workdata[NPC_CREATEWORKNEVERMAKE] )
        return FALSE;

    if( NPC_create[cindex].intdata[NPC_CREATEBORNNUM]
        <=  NPC_create[cindex].workdata[NPC_CREATEWORKENEMYNUM] )
        return FALSE;

    if( NPC_create[cindex].intdata[NPC_CREATETIME] < 0 )return FALSE;
    old.tv_sec = NPC_create[cindex].workdata[NPC_CREATEWORKMAKESTARTSEC];
    old.tv_usec= NPC_create[cindex].workdata[NPC_CREATEWORKMAKESTARTUSEC];

    if( time_diff_us( NowTime , old ) <=
        NPC_create[cindex].intdata[NPC_CREATETIME]*1000 )
        return FALSE;

    return TRUE;
}




/*------------------------------------------------------------
 * ◊€‘¬¡›º∞¡›√ﬁº∞…¨¿√√´‚Ÿ”Âº¿‘ ‘¬£€
 * ¬¶–—
 *  tindex      int     ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  ÿ¶ÿ∆
 ------------------------------------------------------------*/
void NPC_createInitTime( int index )
{
    /*  ◊€‘¬“«±Âÿ¶‘»–◊æÆ»’∞¿√´…¨¿√  */
    NPC_create[index].workdata[NPC_CREATEWORKMAKESTARTSEC] =
        NowTime.tv_sec;
    NPC_create[index].workdata[NPC_CREATEWORKMAKESTARTUSEC] =
        NowTime.tv_usec;
}
/*------------------------------------------------------------
 * ≥ƒ√´◊€‘»–◊  ±Â –À¸º˛–˛–—√´  ÷ß‘ Ë˙–—
 * ¬¶–—
 *  index       int         ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  ÿ¶ÿ∆
 ------------------------------------------------------------*/
void NPC_createIncreaseEnemynum( int index )
{
    if( !NPC_CHECKCREATEINDEX(index) )return;
    NPC_create[index].workdata[NPC_CREATEWORKENEMYNUM] ++;
}
/*------------------------------------------------------------
 * ≥ƒª•…ÿ œ∆• –À¸º˛–˛–—√´”º»’‘ Ë˙–—
 * ¬¶–—
 *  index       int         ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  ÿ¶ÿ∆
 ------------------------------------------------------------*/
void NPC_createDecreaseEnemynum( int index )
{
    if( !NPC_CHECKCREATEINDEX(index) )return;
    NPC_create[index].workdata[NPC_CREATEWORKENEMYNUM] --;
}
/*------------------------------------------------------------
 *  –À¸º˛–˛–—ª•∆·    æÆ∆©ÕÕ‘¬Ë˙–—
 * ¬¶–—
 *  index       int         ƒÃº˛∑∏”¿€Õµ©
 * ﬂØ‘ª∞¿
 *  TRUE:MAX  FALSE:‡®µ§
 ------------------------------------------------------------*/
BOOL NPC_createCheckMaxEnemynum( int index )
{

    if( NPC_create[index].intdata[NPC_CREATEBORNNUM]
        <=  NPC_create[index].workdata[NPC_CREATEWORKENEMYNUM] )
        return TRUE;
	else
		return FALSE;
}
