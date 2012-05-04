#ifndef _MEMBER_MAN_H
#define _MEMBER_MAN_H

void NPC_MemberManTalked( int meindex, int talkerindex, char *msg, int color );
BOOL NPC_MemberManInit( int meindex );
void NPC_MemberManWindowTalked( int meindex, int talkerindex, int seqno, int select, char *data);

#endif
