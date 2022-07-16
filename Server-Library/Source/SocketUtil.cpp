#include <iostream>

#include "../Header/ServerShared.h"

bool SocketUtil::StaticInit()
{
#if _WIN32
	WSADATA wsaData;
	int iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if ( iResult != NO_ERROR )
	{
		ReportError ("Starting Up");
		return false;
	}
#endif
	return true;
}

void SocketUtil::CleanUp()
{
#if _WIN32
	WSACleanup();
#endif
}


void SocketUtil::ReportError( const char* inOperationDesc )
{
#if _WIN32
	LPVOID lpMsgBuf;
	DWORD errorNum = GetLastError();
	
	FormatMessage(
				  FORMAT_MESSAGE_ALLOCATE_BUFFER |
				  FORMAT_MESSAGE_FROM_SYSTEM |
				  FORMAT_MESSAGE_IGNORE_INSERTS,
				  NULL,
				  errorNum,
				  MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
				  (LPTSTR) &lpMsgBuf,
				  0, NULL );
	
	
	LOG( "Error %s: %d- %s", inOperationDesc, errorNum, lpMsgBuf );
#else
	LOG( "Error: %hs", inOperationDesc );
#endif
}

int SocketUtil::GetLastError()
{
#if _WIN32
	return WSAGetLastError();
#else
	return errno;
#endif
	
}

UDPSocketPtr SocketUtil::CreateUDPSocket( SocketAddressFamily inFamily )
{
	SOCKET s = socket( inFamily, SOCK_DGRAM, IPPROTO_UDP );
	
	if( s != INVALID_SOCKET )
	{
		return UDPSocketPtr( new UDPSocket( s ) );
	}
	else
	{
		ReportError( "SocketUtil::CreateUDPSocket" );
		return nullptr;
	}
}

TCPSocketPtr SocketUtil::CreateTCPSocket( SocketAddressFamily inFamily )
{
	SOCKET s = socket( inFamily, SOCK_STREAM, IPPROTO_TCP );
	
	if( s != INVALID_SOCKET )
	{
		return TCPSocketPtr( new TCPSocket( s ) );
	}
	else
	{
		ReportError( "SocketUtil::CreateTCPSocket" );
		return nullptr;
	}
}

fd_set* SocketUtil::FillSetFromVector( fd_set& outSet, const vector< TCPNetworkUserInfo >* inUserInfo, int& ioNaxNfds )
{
	if( inUserInfo )
	{
		FD_ZERO( &outSet );
		
		for( const auto& userInfo : *inUserInfo )
		{
			FD_SET( userInfo.GetNetworkID(), &outSet);
#if !_WIN32
			ioNaxNfds = std::max( ioNaxNfds, socket.mTcpSocketPtr->mSocket );
#endif
		}
		return &outSet;
	}
	else
	{
		return nullptr;
	}
}

void SocketUtil::FillVectorFromSet( vector< TCPNetworkUserInfo >* outUserInfo, const vector< TCPNetworkUserInfo >* inUserInfo, const fd_set& inSet )
{
	if( inUserInfo && outUserInfo )
	{
		outUserInfo->clear();
		for( const auto& userInfo : *inUserInfo )
		{
			if( FD_ISSET( userInfo.GetNetworkID(), &inSet))
			{
				outUserInfo->push_back( userInfo );
			}
		}
	}
}

int SocketUtil::Select( const vector< TCPNetworkUserInfo >* inReadSet,
					   vector< TCPNetworkUserInfo >* outReadSet,
					   const vector< TCPNetworkUserInfo >* inWriteSet,
					   vector< TCPNetworkUserInfo >* outWriteSet,
					   const vector< TCPNetworkUserInfo >* inExceptSet,
					   vector< TCPNetworkUserInfo >* outExceptSet )
{
	//build up some sets from our vectors
	fd_set read, write, except;

	int nfds = 0;
	
	fd_set *readPtr = FillSetFromVector( read, inReadSet, nfds );
	fd_set *writePtr = FillSetFromVector( write, inWriteSet, nfds );
	fd_set *exceptPtr = FillSetFromVector( except, inExceptSet, nfds );

	int toRet = select( nfds + 1, readPtr, writePtr, exceptPtr, nullptr );

	if( toRet > 0 )
	{
		FillVectorFromSet( outReadSet, inReadSet, read );
		FillVectorFromSet( outWriteSet, inWriteSet, write );
		FillVectorFromSet( outExceptSet, inExceptSet, except );
	}
	return toRet;
}
