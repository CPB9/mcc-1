#include "mcc/crashdump/CrashDump.h"

//HACK: cp1251 encoding

#include <windows.h>
#include <dbghelp.h>
#include <Strsafe.h>
#include <intrin.h>
#include <stdlib.h>
#include <new.h>
#include <eh.h>
#include <signal.h>
#include <tchar.h>
#include <exception>

static LONG WINAPI reportException(PEXCEPTION_POINTERS ptrs)
{
    TCHAR tszFileName[1024] = { 0 };

    SYSTEMTIME stTime = { 0 };
    GetSystemTime(&stTime);

    StringCbPrintf(tszFileName,
        sizeof(tszFileName),
#ifdef MCC_BUILD_NUMBER
        _T("%s_%s_%d_%s_%4d.%02d.%02d_%02d%02d%02d.dmp"),
        _T("CrashDump"),
		_T(MCC_BRANCH),
		MCC_BUILD_NUMBER,
		_T(MCC_COMMIT_HASH),
#else
        _T("%s_%4d.%02d.%02d_%02d%02d%02d.dmp"),
        _T("CrashDump"),
#endif
        stTime.wYear,
        stTime.wMonth,
        stTime.wDay,
        stTime.wHour,
        stTime.wMinute,
        stTime.wSecond);

    HANDLE file = CreateFile(tszFileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, FILE_ATTRIBUTE_NORMAL, NULL);
    if (file == NULL)
        return EXCEPTION_EXECUTE_HANDLER;

    if (file == INVALID_HANDLE_VALUE) {
        CloseHandle(file);
        return EXCEPTION_EXECUTE_HANDLER;
    }

    MINIDUMP_EXCEPTION_INFORMATION info;
    info.ThreadId = GetCurrentThreadId();
    info.ExceptionPointers = ptrs;
    info.ClientPointers = TRUE;

    MiniDumpWriteDump(GetCurrentProcess(), GetCurrentProcessId(), file, MiniDumpNormal, &info, NULL, NULL);
    CloseHandle(file);

    MessageBox(NULL, _T("Произошла критическая ошибка.\nФайл с диагностической информацией сохранен."),
               _T("Критическая ошибка"), MB_OK | MB_ICONERROR);

    return EXCEPTION_EXECUTE_HANDLER;
}

static void getExceptionPointers(DWORD exceptionCode, EXCEPTION_POINTERS* ptrs)
{
    CONTEXT contextRecord;
    memset(&contextRecord, 0, sizeof(CONTEXT));

#ifdef _X86_
    __asm {
        mov dword ptr[contextRecord.Eax], eax
        mov dword ptr[contextRecord.Ecx], ecx
        mov dword ptr[contextRecord.Edx], edx
        mov dword ptr[contextRecord.Ebx], ebx
        mov dword ptr[contextRecord.Esi], esi
        mov dword ptr[contextRecord.Edi], edi
        mov word ptr[contextRecord.SegSs], ss
        mov word ptr[contextRecord.SegCs], cs
        mov word ptr[contextRecord.SegDs], ds
        mov word ptr[contextRecord.SegEs], es
        mov word ptr[contextRecord.SegFs], fs
        mov word ptr[contextRecord.SegGs], gs
        pushfd
        pop[contextRecord.EFlags]
    }

    contextRecord.ContextFlags = CONTEXT_CONTROL;

#pragma warning(push)
#pragma warning(disable:4311)
    contextRecord.Eip = (ULONG)_ReturnAddress();
    contextRecord.Esp = (ULONG)_AddressOfReturnAddress();
#pragma warning(pop)
    contextRecord.Ebp = *((ULONG *)_AddressOfReturnAddress() - 1);
#elif defined (_IA64_) || defined (_AMD64_)
    RtlCaptureContext(&contextRecord);
#else
    ZeroMemory(&contextRecord, sizeof(contextRecord));
#endif

    memcpy(ptrs->ContextRecord, &contextRecord, sizeof(CONTEXT));
    ZeroMemory(ptrs->ExceptionRecord, sizeof(EXCEPTION_RECORD));
    ptrs->ExceptionRecord->ExceptionCode = exceptionCode;
    ptrs->ExceptionRecord->ExceptionAddress = _ReturnAddress();
}

static LONG WINAPI reporExceptionCode(DWORD code)
{
    EXCEPTION_RECORD exceptionRecord;
    CONTEXT contextRecord;
    EXCEPTION_POINTERS exceptionPointers;
    exceptionPointers.ExceptionRecord = &exceptionRecord;
    exceptionPointers.ContextRecord = &contextRecord;
    getExceptionPointers(code, &exceptionPointers);
    reportException(&exceptionPointers);
    return EXCEPTION_EXECUTE_HANDLER;
}

static DWORD WINAPI stackOverflowHandler(LPVOID data)
{
    PEXCEPTION_POINTERS ptrs = (PEXCEPTION_POINTERS)data;
    reportException(ptrs);
    return EXCEPTION_EXECUTE_HANDLER;
}

static LONG WINAPI sehHandler(PEXCEPTION_POINTERS ptrs)
{
    if (ptrs->ExceptionRecord != 0 && ptrs->ExceptionRecord->ExceptionCode == EXCEPTION_STACK_OVERFLOW)
    {
        HANDLE thread = ::CreateThread(0, 0, &stackOverflowHandler, ptrs, 0, 0);
        ::WaitForSingleObject(thread, INFINITE);
        ::CloseHandle(thread);
        TerminateProcess(GetCurrentProcess(), 1);
    }

    reportException(ptrs);
    TerminateProcess(GetCurrentProcess(), 1);
    return EXCEPTION_EXECUTE_HANDLER;
}

static void __cdecl terminateHandler()
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl pureCallHandler()
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static int __cdecl newHandler(size_t)
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
    return 0;
}

static void __cdecl unexpectedHandler()
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl invalidParameterHandler(const wchar_t* expression, const wchar_t* function, const wchar_t* file,
                                            unsigned int line, uintptr_t reserved)
{
    (void)expression;
    (void)function;
    (void)file;
    (void)line;
    (void)reserved;
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigabrtHandler(int)
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigintHandler(int)
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigtermHandler(int)
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigfpeHandler(int)
{
    reportException((PEXCEPTION_POINTERS)_pxcptinfoptrs);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigillHandler(int)
{
    reporExceptionCode(EXCEPTION_ILLEGAL_INSTRUCTION);
    TerminateProcess(GetCurrentProcess(), 1);
}

static void __cdecl sigsegvHandler(int)
{
    reporExceptionCode(0);
    TerminateProcess(GetCurrentProcess(), 1);
}

namespace mcccrashdump {

void installProcessCrashHandlers()
{
    SetUnhandledExceptionFilter(sehHandler);
    _set_purecall_handler(pureCallHandler);
    _set_new_mode(1);
    _set_new_handler(newHandler);
    _set_invalid_parameter_handler(invalidParameterHandler);
    _set_abort_behavior(_CALL_REPORTFAULT, _CALL_REPORTFAULT);
    signal(SIGABRT, sigabrtHandler);
    signal(SIGINT, sigintHandler);
    signal(SIGTERM, sigtermHandler);
}

void installThreadCrashHandlers()
{
    set_terminate(terminateHandler);
    set_unexpected(unexpectedHandler);
    signal(SIGFPE, sigfpeHandler);
    signal(SIGILL, sigillHandler);
    signal(SIGSEGV, sigsegvHandler);
}
}
