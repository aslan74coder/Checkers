/*???
след партия виснет
ход со взятиями не рисуется
*/

#include <vcl.h>
#pragma hdrstop
#include "main.h"
#include "game.h"
#include "ai.h"

#pragma package(smart_init)
#pragma resource "*.dfm"

TfrmMain *frmMain;

TColor cellColors[]={clMaroon,clWhite};
TColor pieceColors[]={clSilver,clGreen};
Piece* selPiece=0; // selected piece for human player
Game game;
AI ai(game);

__fastcall TfrmMain::TfrmMain(TComponent* Owner) : TForm(Owner) {}

void __fastcall TfrmMain::FormCreate(TObject *Sender) {
    game.Start();
    game.players[0]=0;
	game.players[1]=1;
	cmbDepth->Visible=(game.players[0]>0||game.players[1]>0);
    ImageList1->GetBitmap(game.players[0],btnPlayer1->Glyph);
    ImageList1->GetBitmap(game.players[1],btnPlayer2->Glyph);
}

void __fastcall TfrmMain::btnNewGameClick(TObject *Sender) {
    game.Start();
	panBoard->Refresh();
	//??? timerTurn->Enabled=true;
}

void __fastcall TfrmMain::btnPlayer1Click(TObject *Sender) {
    TSpeedButton *btn=(TSpeedButton*)Sender;
    int i=btn->Tag;
	game.players[i]=1-game.players[i];
    btn->Glyph->Assign(0);
    ImageList1->GetBitmap(game.players[i],btn->Glyph);
	cmbDepth->Visible=(game.players[0]>0||game.players[1]>0);
}

bool Flip() { // flip board for human player
	return (0&& game.player>0&&game.players[game.player]==0); //???
}

void Circ(TCanvas *Canvas,int x,int y,int r,TColor ucolor,TColor dcolor) {
    Canvas->Pen->Color=ucolor;Canvas->Arc(x+r,y-r,x-r,y+r,x-r,y-r,x+r,y+r);
	Canvas->Pen->Color=dcolor;Canvas->Arc(x+r,y-r,x-r,y+r,x+r,y+r,x-r,y-r);
}

void DrawPiece(TCanvas *Canvas,int x,int y,int r,bool king,bool beaten,bool selected,bool canBeat,TColor color) {
    Canvas->Pen->Color=(selected||canBeat)?clRed:clBlack;
    Canvas->Pen->Width=selected?4:(canBeat?2:1);
    Canvas->Brush->Color=color;
//    Canvas->Ellipse(x-r,y-r,x+r,y+r);
    Canvas->Ellipse(x-r-1,y-r-1,x+r+1,y+r+1);
    Canvas->Pen->Width=1;
	if(king)
	    Circ(Canvas,x,y,r*2/3,clWhite,clBlack);
    else {
	    Circ(Canvas,x,y,r/3,clBlack,clWhite);
        Circ(Canvas,x,y,r*2/3,clWhite,clBlack);
    }
	if(beaten) {
	    int d=r*1/2;
        Canvas->Pen->Color=clRed;Canvas->Pen->Width=5;
		Canvas->MoveTo(x-d,y-d);Canvas->LineTo(x+d,y+d);
        Canvas->MoveTo(x-d,y+d);Canvas->LineTo(x+d,y-d);
    }
}

void __fastcall TfrmMain::panBoardPaint(TObject *Sender) {
    int w=panBoard->Width,size=w/BOARDSIZE;
	bool flip=Flip();
    TCanvas* Canvas=panBoard->Canvas;
    Turn& turn=game.turns[game.nturn-(!selPiece&&game.nturn>0)];
	for(int y=0;y<BOARDSIZE;++y) // draw board
        for(int x=0;x<BOARDSIZE;++x) {
            int X=(flip?BOARDSIZE-1-x:x)*size,Y=(flip?y:BOARDSIZE-1-y)*size;
            Canvas->Brush->Color=cellColors[(y^x)&1];
			Canvas->FillRect(TRect(X,Y,X+size,Y+size));
			Piece* p=game.board[y][x];
			if(game.Live(p))
			    DrawPiece(Canvas,X+size/2,Y+size/2,size*3/8,p->king,turn.Beaten(p),false,game.CanBeat(p),pieceColors[p->Player()]);
        }
    if(!selPiece&&game.nturn>0) { // draw prev turn
	    // connect neighbor points: start, end and s
        Canvas->Pen->Color=clRed;Canvas->Pen->Width=1;
        if(turn.beats==0) {
		    Canvas->MoveTo((flip?BOARDSIZE-1-turn.fromx:turn.fromx)*size+size/2,(flip?turn.fromy:BOARDSIZE-1-turn.fromy)*size+size/2);
            Canvas->LineTo((flip?BOARDSIZE-1-turn.tox:turn.tox)*size+size/2,(flip?turn.toy:BOARDSIZE-1-turn.toy)*size+size/2);
        }
		else {
		    static bool b[BOARDSIZE][BOARDSIZE];
			static TPoint ps[BOARDSIZE*BOARDSIZE/2];
            for(int y=0;y<BOARDSIZE;++y)
			    for(int x=0;x<BOARDSIZE;++x)
				    b[y][x]=false;
            ps[0]=TPoint(turn.fromx,turn.fromy);
			ps[1]=TPoint(turn.tox,turn.toy);
			TPoint P,p;
			for(int np=2,j,x,y;np>0;)
			    for(P=ps[--np],j=0,x=P.x,y=P.y; j<4; ++j) {
				    b[y][x]=true;
					int dx=((j&1)<<1)-1,dy=(j&2)-1,x1=x+dx,y1=y+dy,x2=x1+dx,y2=y1+dy;
					if(x2<0||x2>=BOARDSIZE||y2<0||y2>=BOARDSIZE) continue;
					if(b[y2,x2]) continue; // already visited
					Piece* pi=game.board[y1][x1];
					if(!pi||!turn.Beaten(pi)) continue;
					p=ps[np++]=TPoint(x2,y2);
                    Canvas->MoveTo((flip?BOARDSIZE-1-x:x)*size+size/2,(flip?y:BOARDSIZE-1-y)*size+size/2);
                    Canvas->LineTo((flip?BOARDSIZE-1-x2:x2)*size+size/2,(flip?y2:BOARDSIZE-1-y2)*size+size/2);
				}
        }
    }
    if(selPiece) // draw selected piece
        DrawPiece(Canvas,(flip?BOARDSIZE-1-turn.tox:turn.tox)*size+size/2,(flip?turn.toy:BOARDSIZE-1-turn.toy)*size+size/2,size*3/8,selPiece->king||turn.king,false,true,false,pieceColors[selPiece->Player()]);
}

void __fastcall TfrmMain::panBoardMouseDown(TObject *Sender,TMouseButton Button, TShiftState Shift, int X, int Y) {
    if(game.win||game.players[game.player]>0) return; // non-human player
	int w=panBoard->Width,size=w/BOARDSIZE;
	bool flip=Flip();
	int x=X/size,y=Y/size; // board coords
	x=flip?BOARDSIZE-1-x:x;y=flip?y:BOARDSIZE-1-y; // flip
	Piece* p=game.board[y][x];
    Turn& turn=game.turns[game.nturn];
	bool canBeat=game.CanBeat()||turn.beats; // player can and must beat
	// note: player whether can beat or already marked some pieces as beaten
	if(game.Live(p)&&p!=selPiece) { // click piece to select
	    if(p->Player()!=game.player||(canBeat?!game.CanBeat(p):!game.CanMove(p))) return;
		selPiece=p;turn.fromx=x;turn.fromy=y;turn.Init();
    }
	else if(selPiece) { // click empty square
	    int x0=turn.tox,y0=turn.toy,dx=x-x0,dy=y-y0;
		if(canBeat?!game.CanBeat(selPiece,x0,y0,dx,dy,selPiece->king||turn.king):
                   !game.CanMove(x0,y0,dx,dy,selPiece->king||turn.king))
            return;
        turn.tox=x;turn.toy=y;turn.king=(turn.king||y==game.KingLine())&&!selPiece->king;
		if(canBeat)
		    turn.Beat(game.board[y0+dy/2][x0+dx/2]);
        if(!canBeat||!game.CanBeat(selPiece,x,y,selPiece->king||turn.king)) { // no further move, stop here
		    selPiece=0;
			EndTurn();
        }
    }
	panBoard->Refresh();

}

void __fastcall TfrmMain::panPlayerPaint(TObject *Sender) {
    DrawPiece(panPlayer->Canvas,panPlayer->Width/2,panPlayer->Height/2,0.4*panPlayer->Width,false,false,false,false,pieceColors[game.player]);
}

AnsiString TurnString(const Turn& turn) {
    return AnsiString('A'+turn.fromx)+('1'+turn.fromy)+":"+('A'+turn.tox)+('1'+turn.toy);
}

void TfrmMain::EndTurn() {
    Turn& turn=game.turns[game.nturn++];
	game.Make(turn);
	labTurn->Caption=TurnString(turn);
	timerTurn->Enabled=true;
}

void __fastcall TfrmMain::timerTurnTimer(TObject *Sender) {
    timerTurn->Enabled=false;
	Turn& turn=game.turns[game.nturn];
	AnsiString s=IntToStr(game.nturn+1);
	if(game.Lost()) {
	    game.PassTurn();
		s+=": WIN";
		panPlayer->Refresh();
		game.win=true;
    }
	else {
	    turn.Init();
		panPlayer->Refresh();
		if(game.players[game.player]==0) // human player
		    s+=": MOVE";
        else { // AI player
		    s+=": THINK";
		    barAI->Show();
    		ai.Eval(cmbDepth->ItemIndex+1,turn);
	    	barAI->Hide();
            AnsiString s=IntToStr(ai.NPos());
            for(int i=s.Length()-2;i>0;i-=3)
                s.Insert(" ",i);
		    labNumPos->Caption=s;
    		EndTurn();
	    	panBoard->Refresh();
        }
    }
    labStatus->Caption=s;
}

void __fastcall TfrmMain::btnUndoClick(TObject *Sender) {
    Turn* turn=&game.turns[game.nturn];
	if(turn->Empty()&&game.nturn>0) { // no moves made - undo prev turn
	    turn=&game.turns[--game.nturn];
        game.Undo(*turn);
		labStatus->Caption=IntToStr(game.nturn+1)+((game.players[game.player]==0)?": MOVE":": THINK");
        labTurn->Caption=TurnString(*turn);
    }
	turn->Init();
    selPiece=0;
	game.win=false;
	panBoard->Refresh();
	panPlayer->Refresh();
}

void __fastcall TfrmMain::btnExitClick(TObject *Sender) {
    Close();
}

