//---------------------------------------------------------------------------
#ifndef mainH
#define mainH
//---------------------------------------------------------------------------
#include <Classes.hpp>
#include <Controls.hpp>
#include <StdCtrls.hpp>
#include <Forms.hpp>
#include <Buttons.hpp>
#include <ComCtrls.hpp>
#include <ExtCtrls.hpp>
#include <ImgList.hpp>
//---------------------------------------------------------------------------
class TfrmMain : public TForm
{
__published:	// IDE-managed Components
    TPaintBox *panBoard;
    TSpeedButton *btnNewGame;
    TSpeedButton *btnExit;
    TSpeedButton *btnPlayer1;
    TSpeedButton *btnPlayer2;
    TPaintBox *panPlayer;
    TLabel *labStatus;
    TSpeedButton *btnUndo;
    TLabel *labTurn;
    TLabel *labNumPos;
    TComboBox *cmbDepth;
    TProgressBar *barAI;
    TImageList *ImageList1;
    TTimer *timerTurn;
    void __fastcall FormCreate(TObject *Sender);
    void __fastcall btnNewGameClick(TObject *Sender);
    void __fastcall btnPlayer1Click(TObject *Sender);
    void __fastcall btnExitClick(TObject *Sender);
    void __fastcall panBoardPaint(TObject *Sender);
    void __fastcall panBoardMouseDown(TObject *Sender, TMouseButton Button,
          TShiftState Shift, int X, int Y);
    void __fastcall panPlayerPaint(TObject *Sender);
    void __fastcall timerTurnTimer(TObject *Sender);
    void __fastcall btnUndoClick(TObject *Sender);
private:	// User declarations
    void EndTurn();
public:		// User declarations
    __fastcall TfrmMain(TComponent* Owner);
};
//---------------------------------------------------------------------------
extern PACKAGE TfrmMain *frmMain;
//---------------------------------------------------------------------------
#endif
