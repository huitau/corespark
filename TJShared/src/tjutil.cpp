#include "../include/tjcore.h"
#include <math.h>
#include <iomanip>
using namespace tj::shared;

const wchar_t* Bool::KTrue = L"yes";
const wchar_t* Bool::KFalse = L"no";

float Util::RandomFloat() { 
	#ifdef _CRT_RAND_S
		unsigned int pr = 0;
		if(rand_s(&pr)!=0) {
			Throw(L"Rand_s failed", ExceptionTypeError);
		}
		return float(pr)/float(UINT_MAX);
	#else
		return rand()/(float)RAND_MAX;
	#endif
}

/* rand() returns a number between 0 and RAND_MAX, which is guaranteed to be at least
32767, which is 7FFF (so we have 15 random bits). So just call this function twice and we
get a random number between 0 and 0x3FFFFFF (30 bits). This means that the int this function
returns is always positive, since the left-most bit is always 0.

The rand()&0x7FFF make sure that this also works correctly on systems with a RAND_MAX higher than
0x7FFF (we just cut the remaining bits off)
*/
int Util::RandomInt() {
	#ifdef _CRT_RAND_S
		unsigned int pr = 0;
		if(rand_s(&pr)!=0) {
			Throw(L"Rand_s failed", ExceptionTypeError);
		}
		return (int)(pr >> 1);
	#else
		return (rand() & 0x7FFF) | ((rand() & 0x7FFF) << 15);
	#endif
}

char* Util::CopyString(const char* str) {
	char* buf = new char[strlen(str)+1];
	strcpy_s(buf,strlen(str)+1,str);
	return buf;
}

wchar_t* Util::IntToWide(int x) {
	wchar_t* str = new wchar_t[33];
	_itow_s(x,str,33,10);
	return str;
}

std::wstring Util::IPToString(in_addr ip) {
	return Wcs(std::string(inet_ntoa(ip)));
}

std::wstring Util::GetSizeString(Bytes bytes) {
	std::wstring x;

	const static Bytes BytesInAGigaByte = 1024*1024*1024;
	const static Bytes BytesInAMegaByte = 1024*1024;
	const static Bytes BytesInAKiloByte = 1024;

	if(bytes>BytesInAGigaByte) {
		x = Stringify(int(bytes/BytesInAGigaByte)) + std::wstring(L" GB");
	}

	else if(bytes>BytesInAMegaByte) { // 
		x = Stringify(int(bytes/BytesInAMegaByte)) + std::wstring(L" MB");
	}

	else if(bytes>BytesInAKiloByte) {
		x = Stringify(int(bytes/BytesInAKiloByte)) + std::wstring(L" kB");
	}
	else {
		x = Stringify(bytes)+ std::wstring(L" B");
	}

	return x;
}

std::wstring& Util::StringToLower(std::wstring& r) {
	transform(r.begin(), r.end(), r.begin(), tolower);
	return r;
}

namespace tj {
	namespace shared {
		template<> bool StringTo(const std::wstring& s, const bool& def) {
			std::wstring ln = s;
			std::transform(ln.begin(), ln.end(), ln.begin(), tolower);
			return def ? (ln!=Bool::KFalse) : (ln==Bool::KTrue);
		}

		template<> std::wstring StringTo(const std::wstring& s, const std::wstring& def) {
			return s;
		}

		template<> std::wstring Stringify(const bool& x) {
			std::wostringstream o;
			o << x ? Bool::KTrue : Bool::KFalse;
			return o.str();
		}

		template<> std::wstring Stringify(const int& x) {
			wchar_t buffer[33];
			_itow_s(x, buffer, (size_t)16, 10);
			return std::wstring(buffer);
		}

		Time::Time(const std::wstring& txt) {
			std::wistringstream is(txt);
			Time time;
			is >> time;
			_time = time._time;
		}

		std::wstring Time::Format() const {
			// Value 0 seems to cause some trouble (displays weird values) so we handle that separately then...
			if(_time==0) {
				return L"00:00:00/000";
			}

			int time = abs(_time);
			float ft = float(time);
			float seconds = ft/1000.0f;
			unsigned int ms = (time%1000);

			float mseconds = floor(fmod(seconds,60.0f));
			float minutes = floor(fmod(seconds/60.0f, 60.0f));
			float hours = floor(seconds/3600.0f);

			// TJShow's format is hour:minute:second/ms
			std::wostringstream os;
			os.fill('0');
			if(_time<0) {
				os << L'-';
			}
			os  << hours << L':' << std::setw(2) << minutes << L':' << std::setw(2) << mseconds  << L'/' << std::setw(3)  << float(ms);

			return os.str();
		} 

		std::wistream& operator>>(std::wistream& strm, Time& time) {
			// {hour,minute,seconds,ms}
			int data[4] = {0,0,0,0};

			// The idea is that we put every new value we find in data[3], while shifting the existing value
			// in data[3] to the left.
			for(int a=0;a<5;a++) {
				if(strm.eof()) break;
				int val = 0;
				strm >> val;

				for(int b=0;b<4;b++) {
					data[b] = data[b+1];
				}
				data[3] = val;

				wchar_t delim;
				if(strm.eof()) break;
				strm >> delim;
			}

			time = Time(data[0]*3600*1000 + data[1]*60*1000 + data[2]*1000 + data[3]);
			return strm;
		}

		std::istream& operator>>(std::istream& strm, Time& time) {
			int x;
			strm >> x;
			time = Time(x);
			return strm;
		}

		std::wostream& operator<<( std::wostream& strm, const Time& time ) {
			strm << time.ToInt();
			return strm;
		}

		std::ostream& operator<<( std::ostream& strm, const Time& time ) {
			strm << time.ToInt();
			return strm;
		}
	}
}

// Clipboard (Windows implementation)
#ifdef _WIN32
	void Clipboard::SetClipboardText(const std::wstring& text) {
		ZoneEntry ze(Zones::ClipboardZone);
		EmptyClipboard();

		HGLOBAL mem = GlobalAlloc(GMEM_MOVEABLE, sizeof(wchar_t)*(text.length()+1));
		if(mem) {
			wchar_t* textBuf = (wchar_t*)GlobalLock(mem);
			if(textBuf!=0) {
				memcpy(textBuf, text.c_str(), sizeof(wchar_t)*(text.length()+1));
				GlobalUnlock(mem);
			}

			SetClipboardData(CF_UNICODETEXT, mem);
			CloseClipboard();

			// We must not free the object until CloseClipboard is called.
			if(mem!=0) {
				GlobalFree(mem);
			}
		}
	}

	bool Clipboard::GetClipboardText(std::wstring& text) {
		ZoneEntry ze(Zones::ClipboardZone);

		if(OpenClipboard(NULL)) {
			HANDLE handle = GetClipboardData(CF_UNICODETEXT);
			if(handle!=0) {
				wchar_t* textPointer = (wchar_t*)GlobalLock(handle);
				if(textPointer!=0) {
					text = std::wstring(textPointer);
					GlobalUnlock(handle);
				}
				else {
					CloseClipboard();
					return false;
				}
			}
			CloseClipboard();
			return true;
		}
		return false;
	}
#endif