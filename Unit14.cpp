//---------------------------------------------------------------------------

#include <vcl.h>
#pragma hdrstop

#include "Unit14.h"
//---------------------------------------------------------------------------
#pragma package(smart_init)
#pragma resource "*.dfm"
TForm14 *Form14;


int _calcValue(int NumBits,std::vector<bool> *bitstring,int &curBit)
{
	int retVal = 0;
	//for (int x=0;x<NumBits;x++) retVal += (2^x) * (bitstring->at(curBit++)?1:0);
	for (int x = NumBits; x>0;x--) retVal += (  (1<<(x-1)) * (bitstring->at(curBit++)?1:0)  );
	return retVal;
}



int calcValue(int NumBits,std::vector<bool> *bitstring,int curBit)
{
	int retVal = 0;
	//for (int x=0;x<NumBits;x++) retVal += (0b1000 >> x) * (bitstring->at(curBit++)?1:0);
	for (int x = NumBits; x>0;x--) retVal += (  (1<<(x-1)) * (bitstring->at(curBit++)?1:0)  );
	return retVal;
}


//---------------------------------------------------------------------------
__fastcall TForm14::TForm14(TComponent* Owner)
	: TForm(Owner)
{
}
//---------------------------------------------------------------------------
void __fastcall TForm14::Button1Click(TObject *Sender)
{
	Close();
}


//---------------------------------------------------------------------------
void __fastcall TForm14::IdUDPServer1UDPRead(TIdUDPListenerThread *AThread, const TIdBytes AData,
          TIdSocketHandle *ABinding)
{
	if (skip) return;

	skip=true;
	HexDump(AData);

	//Message starts at index 9 of the AData
	//AData[9] is DF and CA
	//AData[10] thru [12] is ICAO
	//Adata[13] thru [19] is payload/ ME (Message, Extended Squitter)
	//AData[13] & 0b11111000)>>3; is the Type Code
	//PI Parity/Interogator ID is 20 thru 22

    const unsigned char *s = AData.data();
	bitstring = BuildBitStringFromByteArray(&s[9], 14);
	//PrintBitString(bitstring);

	//All messages have a DF and ICAO   Do they have a CA??
	byte DF = calcValue(5,bitstring,0);
	DFsSeen.insert(DF);

	RTLSDR_Aircraft *ac = new RTLSDR_Aircraft;
	ac->flight[0] = 0;
	ac->numMessages = 1;

	ac->CA = calcValue(3,bitstring,5);

	ac->ICAO[0]=AData[10];
	ac->ICAO[1]=AData[11];
	ac->ICAO[2]=AData[12];
	sprintf(ac->ICAO_s,"%02X%02X%02X",ac->ICAO[0],ac->ICAO[1],ac->ICAO[2]);


	switch (DF) {
		case 4:
			//pme("DF 4 for %s",DF,ac->ICAO_s);
			break;

		case 11:
			//pme("DF 11 for %s",DF,ac->ICAO_s);
			break;
		case 17:    //ADS-B Extended Squitter. Should always be 112 bits (14 bytes)
			{
			pme("DF 17 for %s. Length is %d",ac->ICAO_s,AData.size()-9);
			if ((AData.size()-9) < 14) {
				puts("DF17 message too small!!!!");
				return;
			}

			ac->lastTypeCode = calcValue(5,bitstring,32);
			ac->typeCodesSeen.insert(ac->lastTypeCode);

			ParseTypeDF17TypeCode(ac,AData);

			UpdateICAOList(ac);
			break;
			}
		case 18:
			//pme("DF 18 for %s",DF,ac->ICAO_s);
			break;

		case 20:
		case 21:
			//e("DF 20/21 Mode-S Enhanced Surveillance");
			break;
		default:
			//pme("DF %2d for %s",DF, ac->ICAO_s);
			break;
	}

    skip=false;
}
//---------------------------------------------------------------------------


void  __fastcall TForm14::ParseTypeDF17TypeCode(RTLSDR_Aircraft *a,const TIdBytes AData)
{
	const unsigned char *s = AData.data();

	char lookup[] = "#ABCDEFGHIJKLMNOPQRSTUVWXYZ##### ###############0123456789######";

	std::string cs="";
	switch (a->lastTypeCode)
	{
		case 4:   //aircraft identification message
			pme("\tParse DF=17, TC=4 aircraft identification message");

			//parse the flight (call sign). It is max 8 characters.
			for (int i = 0; i < 8; i++)
			{
				a->flight[i] = lookup[calcValue(6,bitstring,40 + i*6)];
			}
			a->flight[8]=0;//safety stop
			pme("\t\tFlight is %s\r\n",a->flight);
			break;
		case 9:
		case 10:
		case 11:
        {
			pme("\r\n---------------------------------\r\nParse DF=17, TC=%d  Aircraft Position", a->lastTypeCode);
			bool IsOddFrame = bitstring->at(54);
			if (true == IsOddFrame) pme("Position Odd Frame");
			else pme("Position Even Frame");
			int altLeft = calcValue(7,bitstring,40) << 4;
			a->Q_Bit = (bool)bitstring->at(47);
			int altRight = calcValue(4,bitstring,48);

			int inc = 100;
			if (true == a->Q_Bit) inc = 25;

			a->alt = (altLeft + altRight) * inc -1000;
			int CPR_Lat = calcValue(17,bitstring,54);
			int CPR_Lon = calcValue(17,bitstring,72);

			break;
			}
		case 19:
			{
			int subtype = AData[13] & 0b00000111;
			pme("Parse DF=17, TC=%d  Aircraft Velocity: Sub Type: %d", a->lastTypeCode,subtype);
			break;
            }

		default:
			pme("Parse DF=17, TC=%d", a->lastTypeCode);

	}
}


void  __fastcall TForm14::UpdateICAOList(RTLSDR_Aircraft *ac)
{
	for (auto a :TrackList)
	{
		if (0 == strcmp(a->ICAO_s, ac->ICAO_s)) {  //found a match so update existing track
			//update existing track with metadata updates and updates from the recived track
			a->age = 0;
			a->numMessages++;

			a->lastTypeCode = ac->lastTypeCode;
			a->typeCodesSeen.insert(ac->lastTypeCode);

			if ( a->lastTypeCode==11)
			{
				a->alt = ac->alt;
				a->Q_Bit = ac->Q_Bit;
			}
			if (ac->flight[0]!=0) {
				a->flight[0]=ac->flight[0];
				a->flight[1]=ac->flight[1];
				a->flight[2]=ac->flight[2];
				a->flight[3]=ac->flight[3];
				a->flight[4]=ac->flight[4];
				a->flight[5]=ac->flight[5];
				a->flight[6]=ac->flight[6];
				a->flight[7]=ac->flight[7];
				a->flight[8]=ac->flight[8];
			}
			return;
		}
	}

    //track doesn't already exist in TrackList so add it
	TrackList.push_back(ac);
}


 //this is a printf for the Debug Form
void  TForm14::pme(const char* fmt, ...)
{
	//Check length and clear after 1000 lines
	if (MemoDebug->Lines->Count > 1000) MemoDebug->Lines->Clear();

	va_list args;
	va_start(args, fmt);
	char buf[200];
	vsprintf(buf,fmt,args);

	MemoDebug->Lines->Add(buf);

	va_end(args);

	//scroll to bottom of text
	MemoDebug->SelStart = MemoDebug->Lines->Text.Length();
	MemoDebug->SelLength = 1;
	MemoDebug->ClearSelection();
}
//---------------------------------------------------------------------------

void __fastcall TForm14::BN_StartClick(TObject *Sender)
{
	if (IdUDPServer1->Active==true)
	{
		IdUDPServer1->Active=false;
		BN_Start->Caption="Start";
	}
	else
	{
		IdUDPServer1->Active=true;
		BN_Start->Caption="Stop";
	}
}
//---------------------------------------------------------------------------




void __fastcall TForm14::HexDump(TIdBytes packet)
{
	if (MemoDebug->Lines->Count > 1000) MemoDebug->Lines->Clear();

	char buf[80];
	char asciibuf[80];
	int size = packet.Length;
	String s;
	sprintf(buf, "[%02d] ",size);
	s = buf;
	for (int x=0;x<packet.size();x++)
		{
			sprintf(buf, "%02X ",packet[x]);
			s += buf;
		}
		MemoRaw->Lines->Add(s);

		//scroll to bottom of text
		MemoRaw->SelStart = MemoRaw->Lines->Text.Length();
		MemoRaw->SelLength = 1;
		MemoRaw->ClearSelection();
}
//

void __fastcall TForm14::Timer1Timer(TObject *Sender)
{
	Memo_ICAO->Clear();
	char buf[100];

	Memo_ICAO->Lines->Add("DF17 Decode");

	Memo_ICAO->Lines->Add("ICAO   Flight   Alt  inc  CA   Last-Type-Code  #Messages  Age Type Codes Seen");
	Memo_ICAO->Lines->Add("-----------------------------------------------------------------");
	for (auto a :TrackList)
	{
		sprintf(buf,"%02X%02X%02X %8s '%d'   %d   %d    %2d             %3d         %d",a->ICAO[0],a->ICAO[1],a->ICAO[2],a->flight, a->alt, (a->Q_Bit)?100:25, a->CA,a->lastTypeCode,a->numMessages,a->age);
		String s = buf;
		s += " ";
		for (int i:a->typeCodesSeen) {
			sprintf(buf,"%2d ",i);
			s+=buf;
		}
		Memo_ICAO->Lines->Add(s);
	}

	for (int x = 0; x < TrackList.size(); x++)
	{
		if (++TrackList[x]->age > dropAfterAge) TrackList.erase(TrackList.begin() + x);
	}


	String s = "Downlink Format (DF) Seen: ";
	char b[5];
	for (auto a :DFsSeen)
	{
		sprintf(b,"%d ", a);
		s += b;
	}
	Memo_ICAO->Lines->Add(s);

}
//---------------------------------------------------------------------------


//First step in parsing a VMF message presented as a byte array
std::vector<bool> *BuildBitStringFromByteArray(const unsigned char* m, int byteArrayLen)
{
	std::vector<bool> *bitstring = new std::vector<bool>;

	for (int i = 0; i < byteArrayLen; i++) {
		byte cb = m[i];
		for (int x=0; x < 8; x++) {
			//if (cb & (0b00000001 <<x)) {
			if (cb & (0b10000000 >>x)) {
				bitstring->push_back(true);
			}
			else bitstring->push_back(false);
		}
	}
	return bitstring;
}


void TForm14::PrintBitString(std::vector<bool> *bitstring)
{
	std::string  s = "";
	for (int i=0; i<bitstring->size();i++) {
		 if (bitstring->at(i)) s.append("1");
			else s.append("0");
			if (!((i+1)%8)) s.append(" ");

	}
	s.append("\r\n");
	MemoDebug->Lines->Append(s.c_str());
}

