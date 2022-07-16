#pragma once
#include "TCPNetworkUserInfo.h"

enum SocketAddressFamily
{
	INET = AF_INET,
	INET6 = AF_INET6
};

class SocketUtil
{
public:

	static bool			StaticInit();
	static void			CleanUp();

	static void			ReportError( const char* inOperationDesc );
	static int			GetLastError();

	static int			Select( const vector< TCPNetworkUserInfo >* inReadSet,
								vector< TCPNetworkUserInfo >* outReadSet,
								const vector< TCPNetworkUserInfo >* inWriteSet,
								vector< TCPNetworkUserInfo >* outWriteSet,
								const vector< TCPNetworkUserInfo >* inExceptSet,
								vector< TCPNetworkUserInfo >* outExceptSet );

	static UDPSocketPtr	CreateUDPSocket( SocketAddressFamily inFamily );
	static TCPSocketPtr	CreateTCPSocket( SocketAddressFamily inFamily );

private:

	inline static fd_set* FillSetFromVector( fd_set& outSet, const vector< TCPNetworkUserInfo >* inUserInfo, int& ioNaxNfds );
	inline static void FillVectorFromSet( vector< TCPNetworkUserInfo >* outUserInfo, const vector< TCPNetworkUserInfo >* inUserInfo, const fd_set& inSet );
};
