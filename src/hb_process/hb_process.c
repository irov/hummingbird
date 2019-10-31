#include "hb_process.h"

#include "hb_log/hb_log.h"

#define WIN32_LEAN_AND_MEAN

#ifndef NOMINMAX
#define NOMINMAX
#endif

#include <Windows.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_process_run( const char * _name, const char * _command, hb_bool_t * _successful )
{
    wchar_t unicode_name[MAX_PATH] = {'\0'};

    int unicode_name_size = MultiByteToWideChar(
        CP_UTF8
        , MB_ERR_INVALID_CHARS
        , _name
        , strlen( _name )
        , unicode_name
        , MAX_PATH
    );

    unicode_name[unicode_name_size] = L'\0';

    wchar_t unicode_command[2048] = {L" "};

    int unicode_command_size = MultiByteToWideChar(
        CP_UTF8
        , MB_ERR_INVALID_CHARS
        , _command
        , strlen( _command )
        , unicode_command + 1
        , MAX_PATH
    );

    ++unicode_command_size;

    unicode_command[unicode_command_size] = L'\0';

    SECURITY_ATTRIBUTES sa;
    sa.nLength = sizeof( sa );
    sa.lpSecurityDescriptor = NULL;
    sa.bInheritHandle = TRUE;

    STARTUPINFOW startupInfo = {0};
    startupInfo.cb = sizeof( STARTUPINFOW );
    startupInfo.dwFlags = STARTF_USESTDHANDLES;
    startupInfo.hStdOutput = NULL;
    startupInfo.hStdError = NULL;
    startupInfo.hStdInput = NULL;

    PROCESS_INFORMATION processInfo = {0};
    if( CreateProcessW( unicode_name, unicode_command
        , NULL
        , NULL
        , TRUE
        , NORMAL_PRIORITY_CLASS | CREATE_NO_WINDOW, NULL
        , NULL
        , &startupInfo
        , &processInfo ) == FALSE )
    {
        DWORD le = GetLastError();

        hb_log_message( "process", HB_LOG_ERROR, "name '%ls' command '%ls' return error [%d]"
            , unicode_name
            , unicode_command
            , le
        );

        return HB_FAILURE;
    }

    WaitForSingleObject( processInfo.hProcess, INFINITE );

    DWORD exitCode = 0;
    BOOL result = GetExitCodeProcess( processInfo.hProcess, &exitCode );

    CloseHandle( processInfo.hProcess );
    CloseHandle( processInfo.hThread );

    if( result == FALSE )
    {
        hb_log_message( "process", HB_LOG_ERROR, "name '%ls' command '%ls' execute invalid get exit code"
            , unicode_name
            , unicode_command
        );

        return HB_FAILURE;
    }

    if( exitCode == 0 )
    {
        *_successful = HB_TRUE;
    }
    else
    {
        *_successful = HB_FALSE;
    }

    return HB_SUCCESSFUL;
}
