#ifndef __ai_H
#define __ai_H
#include "game.h"
#include <vector>

#define NPOSITIONS (1<<26)
#define NHASH NPOSITIONS/4

class AI {

    // если iturn < текущий номер хода (не внутри ф-ции перебора), то выкин позицию
    // в эл-те таблицы связ список

    struct Position {
        uint32      hash;  // use for lookup in hash table and to differ positions also
        uint32      val;   // value = +/-NPIECE*KINGWEIGHT
    	int16       depth; // if <= PosVal depth then return val
        int16       iturn; // max turn number, from analysis of which this pos appeared
        Position*   next;  // next Position
        
	    Position() : hash(0),val(0),depth(-1) {}
    };

    class Positions {  // hash table
        std::vector<Position> positions;
        std::vector<Position*> ppositions;
        Position* freePos; // free positions list
    public:
        Positions() : positions(NPOSITIONS),ppositions(NHASH) {};
        Position& Get(uint32 hash);
    };

    int npos;
    Turn turns[2000];
    Positions positions;
    Game& game;

public:
    AI(Game& _game);
    void Move(int x,int y,bool king,Position& position,int& nturn,Turn* turns);
    bool Beat(Piece* p,int x,int y,bool king,uint32 beats,Position& position,int& nturn,Turn* turns);
    int16 Eval(int depth,int16 alfa,int16 beta,int nturn,Turn *turns,bool progress);
    void Eval(int depth,Turn& turn);
    int NPos() { return npos; }
};

#endif

