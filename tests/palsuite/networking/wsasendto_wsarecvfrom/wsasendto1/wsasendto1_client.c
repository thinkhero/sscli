/*=============================================================================
**
** Source: wsasendto1_client.c (WSASendTo)
**
** Purpose: Test to ensure that WSASendTo returns the WSANOTINITIALISED
**          when a Cleanup is done calling WSASendTo.
** 
** Dependencies: PAL_Initialize
**               PAL_Terminate
**               Fail
**               WSAStartup
**               WSACleanup
**               WSASocketA
**               setsockopt
**               memset
**               GetLastError
**               bind
**  
** 
**  Copyright (c) 2006 Microsoft Corporation.  All rights reserved.
** 
**  The use and distribution terms for this software are contained in the file
**  named license.txt, which can be found in the root of this distribution.
**  By using this software in any fashion, you are agreeing to be bound by the
**  terms of this license.
** 
**  You must not remove this notice, or any other, from this software.
** 
**
**===========================================================================*/

#include <palsuite.h>
#include "wsacommon.h"

/**
 * Defines
 */

#define RECV_BUF_COUNT 1        /* number of recv buffer */
#define RECV_BUF_SIZE  255     /* buffer size max tcp:4328 */
#define NUM_OF_SOCKETS 1        /* number of sockets used in this test */

/**
 * Global variables
 */


const int       listenBacklog   = 1;    /* backlog for listen */

/**
 * main
 * 
 * executable entry point
 */
INT __cdecl main(INT argc, CHAR **argv)
{
    int     i;
    int     err;    
    struct  sockaddr_in mySockaddr;
    WSADATA wsaData;   

    /* Variables needed for WSASendTo */
    WSABUF        wsaBuf[RECV_BUF_COUNT];
    DWORD         dwNbrOfBuf  = RECV_BUF_COUNT;
    DWORD         dwNbrOfByteSent;
    DWORD         dwSendFlags = 0;
    WSAOVERLAPPED wsaOverlapped;

    char myBuffer[RECV_BUF_SIZE] = "violets are blue, roses are red"; 
    
    
    /* Sockets descriptor */
    const int numSockets = 1;    /* number of sockets used in this test */

    SOCKET testSockets[NUM_OF_SOCKETS];

    /* Socket DLL version */
    const WORD wVersionRequested = MAKEWORD(2,2);

    /* Sockets initialization to INVALID_SOCKET */
    for( i = 0; i < NUM_OF_SOCKETS; i++ )
    {
       testSockets[i] = INVALID_SOCKET;
    }

    /* PAL initialization */
    if( PAL_Initialize(argc, argv) != 0 )
    {
        return FAIL;
    }
   
    /* Initialize to use winsock2.dll */
    err = WSAStartup(wVersionRequested,
                     &wsaData);

    if(err != 0)
    {
         Fail( "ERROR: Unexpected failure: "
              "WSAStartup(%i) "
              "returned %d\n",
              wVersionRequested, 
              GetLastError() );
    }

    /* Confirm that the WinSock DLL supports 2.2.
       Note that if the DLL supports versions greater    
       than 2.2 in addition to 2.2, it will still return
       2.2 in wVersion since that is the version we      
       requested.                                        
    */
 
    if ( wsaData.wVersion != wVersionRequested ) 
    {
         
        Trace("ERROR: Unexpected failure "
              "to find a usable version of WinSock DLL\n");

         /* Do some cleanup */
        DoWSATestCleanup( testSockets,
                          numSockets );

        Fail("");
    }

    /* create an overlapped UDP socket in AF_INET domain */

    testSockets[0] = WSASocketA( AF_INET, 
                           SOCK_DGRAM, 
                           IPPROTO_UDP,
                           NULL, 
                           0, 
                           WSA_FLAG_OVERLAPPED ); 


    if( testSockets[0] == INVALID_SOCKET )

    {
        Trace("ERROR: Unexpected failure: "
              "WSASocketA"
              "(AF_INET,SOCK_DGRAM,IPPROTO_UDP,NULL,0,WSA_FLAG_OVERLAPPED)) "
              " returned %d\n",
              GetLastError());

        /* Do some cleanup */
        DoWSATestCleanup( testSockets,
                          numSockets );

        Fail("");
    }

    /* prepare the sockaddr structure */

    mySockaddr.sin_family           = AF_INET;
    mySockaddr.sin_port             = getRotorTestPort();
    mySockaddr.sin_addr.S_un.S_addr = inet_addr("127.0.0.1");

    memset( &(mySockaddr.sin_zero), 0, 8);    
    
    /* Initialize the WSAOVERLAPPED to 0 */
    memset(&wsaOverlapped, 0, sizeof(WSAOVERLAPPED));    

    /* Initialize the WSABUF structure */
    wsaBuf[0].buf = myBuffer;
    wsaBuf[0].len = RECV_BUF_SIZE;

    /* Close WSA services */
    err = WSACleanup( );
    if( err == SOCKET_ERROR )
    {
        Trace("ERROR: Unexpected failure: "
              "WSACleanup() "
              "returned %d\n",
              GetLastError() );
        
        Fail("");
    } 

    /* Send some data directly to server opened socket */  
    err=WSASendTo(
            testSockets[0],
            wsaBuf,
            dwNbrOfBuf,
            &dwNbrOfByteSent,
            dwSendFlags,
            (struct sockaddr FAR *)&mySockaddr,
            sizeof(mySockaddr),
            &wsaOverlapped,
            0);

    if(err != SOCKET_ERROR)      
    {
        
        Trace("ERROR: "
              "WSASendTo(...) "
              "returned %d, expected SOCKET_ERROR\n",
              err );       
        
        Fail("");
    }      
    else if( GetLastError()!=WSANOTINITIALISED)
    {
        
        Trace("ERROR: "
              "WSASendTo(...) "
              "returned %d, expected WSANOTINITIALISED\n",
              GetLastError() );       
        
        Fail("");
    }      
    
    PAL_Terminate();
    return PASS;
};

