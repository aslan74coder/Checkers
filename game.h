#ifndef __game_H
#define __game_H

#define NPIECE      12
#define BOARDSIZE   8
#define KINGWEIGHT  3

typedef unsigned char   byte;
typedef unsigned int    uint32;
typedef short           int16;
typedef unsigned short  uint16;

struct Piece {
    int id; // index in pieces[], used for beat masks
    int ia; // index in active[], used for restoring from killed[]
    int x,y;
    bool king;
    void Init(int i) { id=ia=i; };
	inline int  Player() const { return (ia<NPIECE)?0:1; }
	inline uint32 Mask() const { return (1<<id); }
	inline bool Masked(uint32 mask) const { return (Mask()&mask)!=0; }
};

struct Turn {
    byte fromx,fromy; // from
	byte tox,toy; // to
	uint32 beats; // bit mask of beaten pieces
	bool king;
	int16 val; // value of resulting position for AI

    //??? byte from, to; // по 4 бит
    //??? uint32 mask; // старш бит - king

    Turn() { Init(); }
    inline void Init() { tox=fromx;toy=fromy;beats=0;king=false; }
    inline void Init(int x,int y,int x2,int y2,bool king,uint32 beats) {
        fromx=x;fromy=y;
        tox=x2;toy=y2;
    	this->king=king;
        this->beats=beats;
    }
    inline bool Empty() { return (tox==fromx&&toy==fromy); }
    inline bool Beaten(Piece* piece) { return piece->Masked(beats); }
    inline void Beat(Piece* piece)   { beats|=piece->Mask(); }
};

struct Zobrist {	// Zobrist hash calculation
    uint32 p;
	uint32 h[2][2][BOARDSIZE][BOARDSIZE/2]; // hashes of pieces
    Zobrist();
    inline uint32 Hash(const Piece& piece) { return h[piece.Player()][piece.king?1:0][piece.y][piece.x>>1]; }
    uint32 Hash(const struct Game& game);   // hash of current position
};

struct Game {
    int player;
    int players[2]; // player type -  0: human, 1: AI
    int nkilled;
    int nturn;
    int nactive[2];
    uint32 beats;
    int16  matVal; // material value of current position
    uint32 hash; // Zobrist hash of current position
    bool   win;
    Piece  pieces[2*NPIECE];
    Piece* active[2*NPIECE];
    Piece* killed[2*NPIECE];
    Piece* board[BOARDSIZE][BOARDSIZE];
    Zobrist zobrist;
    Turn turns[1000];

    inline int Dy() { return 1-2*player; } // where pawn can move
    inline static bool Valid(int x) { return (0<=x&&x<BOARDSIZE); } // valid coord
    inline int KingLine() { return (BOARDSIZE-1)*(1-player); } // king line

    Game();
    void Init();
    void CalcMatVal();
    void InitBoard();
    void Standard();
    void Start();
    inline bool Live(Piece* piece) { return piece&&!piece->Masked(beats); }
    inline void Kill(Piece* piece); // remove piece from active[] to deleted[]
    inline Piece* UnKill(); // restore last removed piece from deleted[] to active[]
    //??? inline
    void Make(Turn& turn);
 	//??? inline
    void Undo(Turn& turn);
    //??? inline
    void PassTurn();

    bool CanMove(int x,int y,int dx,int dy,bool king); // piece can move
    bool CanBeat(Piece* p,int x,int y,int dx,int dy,bool king); // piece can beat
    bool CanMove(int x,int y,bool king); // piece can move
    bool CanBeat(Piece* p,int x,int y,bool king); // piece can beat
    bool CanMove(Piece* p); // piece can move
    bool CanBeat(Piece* p); // piece can beat
    bool CanMove(); // player can move
    bool CanBeat(); // player can beat
    bool Lost();    // player lost
};

#endif

