#include "ai.h"
#include "main.h"

#define MAXVAL NPIECE*KINGWEIGHT

AI::Position& AI::Positions::Get(uint32 hash) {
    Position *head=ppositions[hash&(NHASH-1)],*pos=head;
    while(pos&&pos->hash!=hash) pos=pos->next;
    if(!pos) { // position not found, create one
        if(!freePos) Error("Out of positions");
        pos=freePos;
        freePos=freePos->next;
        pos->hash=hash;
    	pos->depth=-1; //??? зачем
        pos->iturn=;
        pos->next=head;
    }
    return *pos;
}

AI::AI(Game& _game) : game(_game) {};

void AI::Move(int x,int y,bool king,Position& position,int& nturn,Turn* turns) {
    for(int dy=-1,y2;dy<2;dy+=2)
	    if((dy==game.Dy()||king)&&game.Valid(y2=y+dy))
		    for(int dx=-1,x2;dx<2;dx+=2)
                if(game.Valid(x2=x+dx)&&!game.Live(game.board[y2][x2]))
                    turns[nturn++].Init(x,y,x2,y2,!king&&y2==game.KingLine(),0);
}

// returns if possible to beat from (x,y)
bool AI::Beat(Piece* p,int x,int y,bool king,uint32 beats,Position& position,int& nturn,Turn* turns) {
    bool res=false,king2;
	for(int dy=-1,y1,y2;dy<2;dy+=2)
	    if((dy==game.Dy()||king)&&game.Valid(y2=(y1=y+dy)+dy))
		    for(int dx=-1,x1,x2;dx<2;dx+=2)
				if(game.Valid(x2=(x1=x+dx)+dx)) {
				    Piece *p1=game.board[y1][x1],*p2=game.board[y2][x2];
					if(p1&&p1->Player()!=game.player&&!p1->Masked(beats|game.beats)&&(!p2||p2==p||p2->Masked(game.beats))) {
					    res=true;king2=king||y2==game.KingLine();
						uint32 beats2=beats|p1->Mask();
						if(!Beat(p,x2,y2,king2,beats2,position,nturn,turns)) // no beat further, stop here
                            turns[nturn++].Init(p->x,p->y,x2,y2,king2!=p->king,beats2);
                    }
                }
    return res;
}

// Calculate value of position and best turn
int16 AI::Eval(int depth,int16 alfa,int16 beta,int nturn,Turn *turns,bool progress) {
    Position* pos=positions.Get(game.hash);

    if(!pos)

    if(pos.depth>=depth) return pos.val;

    ??? зачем присв pos.val

	if(depth<=0) { ++npos; return pos.val=game.matVal; }; // return material value

	// collect turns list
	int nturn2=nturn,n=game.nactive[game.player],i,j;
	for(i=n,j=game.player*NPIECE;--i>=0;++j) { // try beat
	    Piece& p=*game.active[j];
		Beat(&p,p.x,p.y,p.king,0,pos,nturn2,turns);
    }

	if(nturn2==nturn) // can not beat
	for(i=n,j=game.player*NPIECE;--i>=0;++j) { // try move
	    Piece& p=*game.active[j];
		Move(p.x,p.y,p.king,pos,nturn2,turns);
    }

    //??? замен bubble sort на heap sort
    //??? строить heap, потом в цикле альфа-бета извлекать top

	// sort turns by value of resulting position
	for(i=nturn;i<nturn2;++i) {
	    Turn& turn=turns[i];
        game.Make(turn);
        turn.val=positions.Get(game.hash).val;
        game.Undo(turn); // value of resulting positions

		for(j=i;--j>=nturn&&turn.val>turns[j].val;) turns[j+1]=turns[j]; // shift previous right
		turns[j+1]=turn; // insert to its place
    }

    ??? может +depth, чтоб оттянуть поражение
	pos.val=(int16)(-MAXVAL-depth);

	for(i=j=nturn;i<nturn2;++i) {
        if(progress) {
    	    frmMain->barAI->Position=100*(i-nturn)/(nturn2-nturn);
	    	frmMain->barAI->Refresh();
        }

	    if(depth>=5) {
            AnsiString s=IntToStr(npos);
            for(int i=s.Length()-2;i>0;i-=3)
                s.Insert(" ",i);
		    frmMain->labNumPos->Caption=s;
			frmMain->labNumPos->Refresh();
            Application->ProcessMessages();
        }

        game.Make(turns[i]);
    	int16 alfa2=-Eval(depth-1,-beta,-alfa,nturn2,turns,false);
	    game.Undo(turns[i]);

		if(alfa2>pos.val) pos.val=alfa2; // update position value
    	if(alfa2>alfa) {
	        alfa=alfa2; j=i;
		    if(alfa>=beta) break;
        }
    }

    ??? а если просто turns[nturn]=turns[j]; // place best turn first
    
	Turn tt=turns[nturn];turns[nturn]=turns[j];turns[j]=tt; // place best turn first
	return alfa;
}

void AI::Eval(int depth,Turn& turn) {
    npos=0;
	Eval(depth,-MAXVAL,MAXVAL,0,turns,true);
	turn=turns[0];
}

