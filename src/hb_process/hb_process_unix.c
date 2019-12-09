#include "hb_process.h"

#include "hb_log/hb_log.h"

#include <unistd.h>
#include <spawn.h>
#include <sys/wait.h>

#include <stdio.h>
#include <stdlib.h>

//////////////////////////////////////////////////////////////////////////
hb_result_t hb_process_run( const char * _name, hb_sharedmemory_handle_t * _handle, hb_bool_t * _successful )
{
    uint32_t sharedmemory_id = hb_sharedmemory_get_id( _handle );

    char command[64];
    int command_size = sprintf( command, "%u"
        , sharedmemory_id
    );

    pid_t pid = fork();

    switch( pid )
    {
    case -1:
        {
            exit(1);
        }break;
    case 0:
        {
            char * args[] = {"--sm", command, NULL};
            int code = execv("_name", args);
            exit(code);
        }break;
    default:
        {
            int status;
            if( waitpid(pid, &status, 0) == -1 )
            {
                HB_LOG_MESSAGE_ERROR( "process", "name '%s' execute invalid get exit code"
                    , _name
                );

                return HB_FAILURE;
            }

            if( status == 0 )
            {
               *_successful = HB_TRUE;
            }
            else
            {
                *_successful = HB_FALSE;
            }
        }break;
    }

    return HB_SUCCESSFUL;
}
