#include <Windows.h>
#include <string>
#include <sstream>

#include "application.h"
#include "gi_logic.h"
#include "exceptions.h"
#include "scene.h"

using ::std::wstring;
using ::std::wstringstream;

void CopyToClipboard(const wstring &);

class NullComponent : public Component{

public:

	NullComponent(int value){

		value_ = value;

	}

	int value_;

};

class HuehueComponent : public NullComponent{

public:

	HuehueComponent() : NullComponent(42){}

};

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd){

	SceneObject so;

	so.AddComponent<NullComponent>(47);
	so.AddComponent<NullComponent>(52);
	so.AddComponent<NullComponent>(96);
	so.AddComponent<HuehueComponent>();

	so.RemoveComponents<HuehueComponent>();

	auto p = so.GetComponent<HuehueComponent>();

	if (p != so.GetEnd<HuehueComponent>()){

		int huehuheu = 89;

	}
	
	auto q = so.GetComponent<NullComponent>();

	if (q != so.GetEnd<NullComponent>()){

		auto & x = *q;

		int i = 0;

		(++i)++;
	}


	GILogic gi_logic;

	try{

		Application::AddLogic(hInstance, gi_logic);

		Application::Run();

	}catch (const RuntimeException & e){

		//Unhandled exception!
		wstringstream stream;
		
		stream << e.GetErrorMessage()
			   << std::endl
			   << std::endl
			   << e.GetStackTrace()
			   << std::endl
			   << "Stack trace has been copied to the clipboard";
		
		auto message = stream.str();

		CopyToClipboard(message);

		MessageBox(NULL,
				   message.c_str(),
				   L"Unhandled exception",
				   MB_OK | MB_ICONERROR);

	}

}

void CopyToClipboard(const wstring & text){

	auto output = text.c_str();
	
	auto len = (text.length() + 1) * sizeof(wchar_t);

	//Allocate the text globally
	auto hMem = GlobalAlloc(GMEM_MOVEABLE, len);
	memcpy(GlobalLock(hMem), output, len);
	GlobalUnlock(hMem);

	//Send the data to the clipboard
	OpenClipboard(NULL);
	EmptyClipboard();
	SetClipboardData(CF_UNICODETEXT, hMem);
	CloseClipboard();

}