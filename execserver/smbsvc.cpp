#include <stdio.h>
#include <Windows.h>
#define SERVICE_NAME "SMBSVC3"
#pragma comment(lib, "advapi32.lib")
#define __DEBUG__

void debug( char * str )
{
#ifdef __DEBUG__
    OutputDebugString( str );
    printf( str );
#endif
}

SERVICE_STATUS_HANDLE ghStatusHandle;
SERVICE_STATUS gStatus;

HANDLE ghServiceStopEvent;

void SetServiceStop( void )
{
    gStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gStatus.dwCurrentState = SERVICE_STOPPED;
    gStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    gStatus.dwWin32ExitCode = 79;
    gStatus.dwCheckPoint = 0;
    gStatus.dwWaitHint = 0;
    SetServiceStatus( ghStatusHandle, &gStatus );
}

void SetServiceRuning( void )
{
    gStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gStatus.dwCurrentState = SERVICE_RUNNING;
    gStatus.dwControlsAccepted = SERVICE_ACCEPT_STOP;
    gStatus.dwWin32ExitCode = NO_ERROR;
    gStatus.dwCheckPoint = 0;
    gStatus.dwWaitHint = 0;

    SetServiceStatus( ghStatusHandle, &gStatus );
}
// why pending
void SetServicePending( void )
{
    gStatus.dwServiceType = SERVICE_WIN32_OWN_PROCESS;
    gStatus.dwCurrentState = SERVICE_STOP_PENDING;
    gStatus.dwControlsAccepted = 0;
    gStatus.dwWin32ExitCode = NO_ERROR;
    gStatus.dwCheckPoint = 2;
    gStatus.dwWaitHint = 0;

    SetServiceStatus( ghStatusHandle, &gStatus );
}

void WINAPI ServiceControl( DWORD dwOpcode )
{
    switch ( dwOpcode )
    {
    case SERVICE_CONTROL_STOP:
        SetEvent( ghServiceStopEvent );
        SetServiceStop();

        break;
        
    case SERVICE_CONTROL_INTERROGATE:
        SetServiceStatus( ghStatusHandle, &gStatus );
        break;
    }
}

void ServiceRunning( DWORD dwArgc, LPSTR * lpszArgv )
{
    ghServiceStopEvent = CreateEvent( NULL,
                                      TRUE,
                                      FALSE,
                                      NULL );
    if ( !ghServiceStopEvent )
    {
        debug( "Create Event error" );
        SetServiceStop();
        return;
    }

    SetServiceRuning();

    while ( 1 )
    {
        OutputDebugString( "I'm here" );
        WaitForSingleObject( ghServiceStopEvent, 0 );

        Sleep( 1000 );
    }

    return;
}

void WINAPI ServiceMain( DWORD dwArgc, LPSTR * lpszArgv )
{
    ghStatusHandle = RegisterServiceCtrlHandler( SERVICE_NAME, ServiceControl );
    if ( !ghStatusHandle )
        return;

    ServiceRunning( dwArgc, lpszArgv );
}

#ifdef __DEBUG__

void ServiceInstall()
{
    SC_HANDLE schSCManager;
    SC_HANDLE schService;

    char szPath[MAX_PATH] = {0};
    if ( !GetModuleFileName( NULL, szPath, MAX_PATH ) )
        return;

    schSCManager = OpenSCManager( NULL, NULL, SC_MANAGER_ALL_ACCESS );
    if ( !schSCManager )
        return;

    schService = CreateService( schSCManager,
                                SERVICE_NAME,
                                SERVICE_NAME,
                                SERVICE_ALL_ACCESS,
                                SERVICE_WIN32_OWN_PROCESS,
                                SERVICE_AUTO_START,
                                SERVICE_ERROR_NORMAL,
                                szPath,
                                NULL,
                                NULL,
                                NULL,
                                NULL,
                                NULL );
    if ( !schService )
    {
        CloseServiceHandle( schSCManager );
        return;
    }

    CloseServiceHandle( schSCManager );
    CloseServiceHandle( schService );
}
#endif

int main( int argc, char **argv )
{
   
    if ( argc > 2 && argv[1][0] == 'i' )
        ServiceInstall();

    SERVICE_TABLE_ENTRY DispatchTable[] = 
    {
        { SERVICE_NAME, ServiceMain },
        { NULL, NULL }
    };

    if( !StartServiceCtrlDispatcher( DispatchTable ) )
    {
        int a = GetLastError();
        char buf[1024] = {0};
        _itoa( a, buf, 10 );
        debug( buf );
        
        return -1;
    }

    return 0;
}