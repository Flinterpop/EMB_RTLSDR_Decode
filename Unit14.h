//---------------------------------------------------------------------------

#ifndef Unit14H
#define Unit14H
//---------------------------------------------------------------------------
#include <System.Classes.hpp>
#include <Vcl.Controls.hpp>
#include <Vcl.StdCtrls.hpp>
#include <Vcl.Forms.hpp>
#include <IdBaseComponent.hpp>
#include <IdComponent.hpp>
#include <IdUDPBase.hpp>
#include <IdUDPClient.hpp>
#include <IdGlobal.hpp>
#include <IdSocketHandle.hpp>
#include <IdUDPServer.hpp>
#include <Vcl.ExtCtrls.hpp>
//---------------------------------------------------------------------------
#include <vector>
#include <set>


std::vector<bool> *BuildBitStringFromByteArray(const unsigned char* m, int len);

struct RTLSDR_Aircraft
{
	byte ICAO[3];
	char ICAO_s[10];

	int alt=-9990;
    bool Q_Bit;

	char flight[10];


	int CA;  //Capability  is 3 bits so 0 thru 7
	int lastTypeCode; //this is 5 bits so 0 thru 31

	int numMessages = 0;
    int age=0;

    std::set<int> typeCodesSeen;

};


class TForm14 : public TForm
{
__published:	// IDE-managed Components
	TMemo *MemoDebug;
	TButton *Button1;
	TIdUDPServer *IdUDPServer1;
	TButton *BN_Start;
	TMemo *Memo_ICAO;
	TTimer *Timer1;
	TMemo *MemoRaw;
	void __fastcall Button1Click(TObject *Sender);
	void __fastcall IdUDPServer1UDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
		  TIdSocketHandle *ABinding);
	void __fastcall BN_StartClick(TObject *Sender);
	void __fastcall Timer1Timer(TObject *Sender);
private:	// User declarations
public:		// User declarations
	__fastcall TForm14(TComponent* Owner);
	void  pme(const char* fmt, ...);
	void __fastcall HexDump(TIdBytes packet);
	void  __fastcall UpdateICAOList(RTLSDR_Aircraft *ac);

    void  __fastcall ParseTypeDF17(RTLSDR_Aircraft *a,const TIdBytes AData);
	void  __fastcall ParseTypeDF17TypeCode(RTLSDR_Aircraft *a,const TIdBytes AData);
	void PrintBitString(std::vector<bool> *bitstring);


	std::vector<RTLSDR_Aircraft*> TrackList;
	std::set<int> DFsSeen;
	int dropAfterAge = 3000;

	std::vector<bool> *bitstring;

    bool skip=false;

};
//---------------------------------------------------------------------------
extern PACKAGE TForm14 *Form14;
//---------------------------------------------------------------------------
#endif
