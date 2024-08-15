#include "game.h"
#include <mem.h>
#include <stdlib.h>

// Zobrist

Zobrist::Zobrist() {
    p=rand();
	for(int y=0;y<BOARDSIZE;++y)
	    for(int x=0;x<BOARDSIZE/2;++x)
	 	    for(int color=0;color<2;++color)
	 		    for(int king=0;king<2;++king)
    	 			h[color][king][y][x]=rand();
}

uint32 Zobrist::Hash(const Game& game) {
    uint32 res=(uint32)(p^(-game.player));
	for(int player=0,j=0;player<2;++player,j+=NPIECE)
	    for(int k=0;k<game.nactive[player];++k)
	 	    res^=Hash(*game.active[j+k]);
    return res;
}

// Game

Game::Game() { for(int i=0;i<2*NPIECE;++i) pieces[i].Init(i); }

void Game::Init() {
    win=false;
	beats=0;nkilled=0;
	nturn=0;turns[0].Init();
    CalcMatVal();
    hash=zobrist.Hash(*this);
}

void Game::CalcMatVal() {
    matVal=0;
	for(int i=0,k=0;i<2;++i,k+=NPIECE)
	    for(int j=0;j<nactive[i];++j)
		    matVal+=(int16)(((i==player)?1:-1)*(active[k+j]->king?KINGWEIGHT:1));
}

void Game::InitBoard() { memset(board,0,sizeof(board)); }

void Game::Standard() {
    player=0;
	nactive[0]=nactive[1]=NPIECE;
	for(int y=NPIECE/(BOARDSIZE/2),i=0;--y>=0;)
	    for(int x=y&1;x<BOARDSIZE;x+=2,++i) {
		    Piece* piece=active[i]=&pieces[i];board[piece->y=y][piece->x=x]=piece;piece->king=false;
			piece=active[i+NPIECE]=&pieces[i+NPIECE];board[piece->y=BOARDSIZE-1-y][piece->x=BOARDSIZE-1-x]=piece;piece->king=false;
        }
}

void Game::Start() {
    InitBoard();
	Standard();
	Init();
}

void Game::Kill(Piece* piece) {
    int i=piece->ia,player=piece->Player(),j=player*NPIECE+(--nactive[player]);
	killed[nkilled++]=active[i];(active[i]=active[j])->ia=i;
	matVal+=piece->king?KINGWEIGHT:(int16)1; // update material value
	hash^=zobrist.Hash(*piece);
}

Piece* Game::UnKill() {
    Piece* piece=killed[--nkilled];
	int i=piece->ia,player=piece->Player(),j=player*NPIECE+(nactive[player]++);
	(active[j]=active[i])->ia=j;(active[i]=piece)->ia=i;
	matVal-=piece->king?KINGWEIGHT:(int16)1; // update material value
	board[piece->y][piece->x]=piece; // restore to board, because it can been overwritten
	hash^=zobrist.Hash(*piece);
	return piece;
}

void Game::Make(Turn& turn) {
    Piece* piece=board[turn.fromy][turn.fromx];

    hash^=zobrist.Hash(*piece);
	board[turn.fromy][turn.fromx]=0;
	board[piece->y=turn.toy][piece->x=turn.tox]=piece;
	piece->king|=turn.king;
	hash^=zobrist.Hash(*piece);
	if(turn.king) matVal+=(KINGWEIGHT-1);

	// enumerate bits set
    int e=(1-piece->Player())*NPIECE;

    // less significant bit E in uint16 A
    for(e=0,s=1<<8; a; s>>=1) {
        if(!(a&(s-1))) {
            a>>=s;
            e+=s;
        }
    }

    а тут надо постоянно убывающий S

	for(uint16 a=turn.beats>>e,b; a!=0;)
	    if(b=a&0xf) { // 4 lower bits
            static int lsb[16]={0,0,1,0,2,0,1,0,3,0,1,0,2,0,1,0}; // less significant bit
			int k=lsb[b];
			Kill(&pieces[e+k]);
			a^=(1<<k);
        }
        else {
		    a>>=4;e+=4;
        }

    beats^=turn.beats;
	PassTurn();
}

void Game::Undo(Turn& turn) {
    PassTurn();
	Piece* piece=board[turn.toy][turn.tox];
    hash^=zobrist.Hash(*piece);
	board[turn.toy][turn.tox]=0;
	board[piece->y=turn.fromy][piece->x=turn.fromx]=piece;
	piece->king=piece->king&&!turn.king;
    hash^=zobrist.Hash(*piece);
	if(turn.king) matVal-=(KINGWEIGHT-1);
	beats^=turn.beats;
	for(uint32 a=turn.beats; a!=0; a^=UnKill()->Mask());
}

void Game::PassTurn() {
    player=1-player;
	matVal=(int16)(-matVal);
    hash^=zobrist.p;
}

bool Game::CanMove(int x,int y,int dx,int dy,bool king) {
    return(abs(dx)==1&&(dy==Dy()||(dy==-Dy()&&king))&&Valid(x+dx)&&Valid(y+dy)&&!Live(board[y+dy][x+dx]));
}

bool Game::CanBeat(Piece* p,int x,int y,int dx,int dy,bool king) {
    if(!(abs(dx)==2&&(dy==2*Dy()||(dy==-2*Dy()&&king))&&Valid(x+dx)&&Valid(y+dy))) return false;
	Piece* targ=board[y+dy/2][x+dx/2];
	Piece* dest=board[y+dy][x+dx];
    return(targ&&targ->Player()!=player&&!targ->Masked(turns[nturn].beats|beats)&&(!Live(dest)||dest==p));
}

bool Game::CanMove(int x,int y,bool king) {
    for(int dy=-1;dy<=1;dy+=2)
	    for(int dx=-1;dx<=1;dx+=2)
		    if(CanMove(x,y,dx,dy,king))
			    return true;
    return false;
}

bool Game::CanBeat(Piece* p,int x,int y,bool king) {
    for(int dy=-2;dy<=2;dy+=4)
	    for(int dx=-2;dx<=2;dx+=4)
		    if(CanBeat(p,x,y,dx,dy,king))
			    return true;
    return false;
}

bool Game::CanMove(Piece* p) {
    return CanMove(p->x,p->y,p->king);
}

bool Game::CanBeat(Piece* p) {
    return CanBeat(p,p->x,p->y,p->king);
}

bool Game::CanMove() {
    for(int i=0;i<nactive[player];++i)
	    if(CanMove(active[i+player*NPIECE]))
            return true;
    return false;
}

bool Game::CanBeat() {
    for(int i=0;i<nactive[player];++i)
	    if(CanBeat(active[i+player*NPIECE]))
		    return true;
    return false;
}

bool Game::Lost() { return(!CanBeat()&&!CanMove()); }

