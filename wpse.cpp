#include <node.h>
#include <v8.h>
#include <windows.h>
#include <string>

#define BUFFER_SIZE 1024

using namespace v8;

Handle<Value> sexec(const Arguments& args) {
    HandleScope scope;
	Handle<Object> ret = Object::New();
	Local<String> lstrStdOut = String::Empty();
	Local<String> lstrStdErr = String::Empty();
	Local<String> lstrCmd = String::Empty();
    HANDLE hOutReadPipe = NULL;
    HANDLE hOutWritePipe = NULL;
	HANDLE hErrReadPipe = NULL;
	HANDLE hErrWritePipe = NULL;
	STARTUPINFOA si;
	PROCESS_INFORMATION pi;
	SECURITY_ATTRIBUTES sa;
	DWORD dwRead = 0;
	BOOL bReadSuccess = false;
	CHAR buffer[BUFFER_SIZE];
	ZeroMemory(&si, sizeof(si));
	ZeroMemory(&pi, sizeof(pi));
	ZeroMemory(&sa, sizeof(sa));

	sa.nLength = sizeof(sa);
	sa.bInheritHandle = TRUE;
	sa.lpSecurityDescriptor = NULL;
    
    if (!(CreatePipe(&hOutReadPipe, &hOutWritePipe, &sa, 0) && CreatePipe(&hErrReadPipe, &hErrWritePipe, &sa, 0))) {
		return ThrowException(String::New("Pipe not created"));
	}
	if (!(SetHandleInformation(hOutReadPipe, HANDLE_FLAG_INHERIT, 0) && SetHandleInformation(hOutReadPipe, HANDLE_FLAG_INHERIT, 0))) {
		return ThrowException(String::New("SetHandleInformation failed"));
	}

	si.cb = sizeof(si);
	si.dwFlags = STARTF_USESHOWWINDOW | STARTF_USESTDHANDLES;
	si.wShowWindow = SW_HIDE;
	si.hStdOutput = hOutWritePipe;
	si.hStdInput = NULL;
	si.hStdError = hErrWritePipe;

	for (int i = 0; i < args.Length(); i++) {
		lstrCmd = String::Concat(String::Concat(String::Concat(lstrCmd, String::New("\"")), args[i]->ToString()), String::New("\""));
		if (i < (args.Length() - 1)) {
			lstrCmd = String::Concat(lstrCmd, String::New(" "));
		}
	}

	if (!CreateProcess(NULL,   // No module name (use command line)
		*String::Utf8Value(lstrCmd),        // Command line
		NULL,           // Process handle not inheritable
		NULL,           // Thread handle not inheritable
		TRUE,          // Set handle inheritance to FALSE
		0,              // No creation flags
		NULL,           // Use parent's environment block
		NULL,           // Use parent's starting directory 
		&si,            // Pointer to STARTUPINFO structure
		&pi)           // Pointer to PROCESS_INFORMATION structure
		)
	{
		return ThrowException(String::New("Process exit with error"));
	}
	CloseHandle(hOutWritePipe);
	CloseHandle(hErrWritePipe);
	CloseHandle(pi.hProcess);
	CloseHandle(pi.hThread);

	ret->Set(
		String::NewSymbol("target"),
		lstrCmd
	);

	for (;;) {
		bReadSuccess = ReadFile(hOutReadPipe, buffer, BUFFER_SIZE, &dwRead, NULL);
		if (!bReadSuccess || dwRead == 0) {
			break;
		}
		lstrStdOut = String::Concat(lstrStdOut, String::New(buffer, (int)dwRead));
	}
	CloseHandle(hOutReadPipe);
	ret->Set(
		String::NewSymbol("out"),
		lstrStdOut
		);
	for (;;) { //Copy, paste... Shame on me!
		bReadSuccess = ReadFile(hErrReadPipe, buffer, BUFFER_SIZE, &dwRead, NULL);
		if (!bReadSuccess || dwRead == 0) {
			break;
		}
		lstrStdErr = String::Concat(lstrStdErr, String::New(buffer, (int)dwRead));
	}
	CloseHandle(hErrReadPipe);
	ret->Set(
		String::NewSymbol("err"),
		lstrStdErr
	);
	return scope.Close(ret);
}

void init(Handle<Object> target) {
    target->Set(String::New("exec"), FunctionTemplate::New(sexec)->GetFunction());
}

NODE_MODULE(pse, init);