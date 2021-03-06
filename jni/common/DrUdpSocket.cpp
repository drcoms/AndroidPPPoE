/*
 * DrUDPSocket.cpp
 *
 *  Created on: 2014年2月22日
 *      Author: Kingsley Yau
 *      Email: Kingsleyyau@gmail.com
 */

#include "DrUdpSocket.h"
#include "Arithmetic.h"
#include "DrLog.h"

DrUdpSocket::DrUdpSocket() {
	// TODO Auto-generated constructor stub
	m_iPort = -1;
	m_bBlocking = false;
}

DrUdpSocket::~DrUdpSocket() {
	// TODO Auto-generated destructor stub
}
int DrUdpSocket::Bind(unsigned int iPort, string ip, bool bBlocking) {
	int iRet = -1;
	if((m_Socket = socket(AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/)) < 0){
		// create socket error;
		showLog("Jni.DrUDPSocket.Bind", "create socket error");
	    return false;
	}

	int iBuffer;
	iRet = getsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, &iBuffer, NULL);
	if(iRet >= 0) {
		showLog("Jni.DrUDPSocket.Bind", "当前接收缓冲区大小:%d", iBuffer);
	}

	iRet = getsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF, &iBuffer, NULL);
	if(iRet >= 0) {
		showLog("Jni.DrUDPSocket.Bind", "当前发送缓冲区大小:%d", iBuffer);
	}
//
//	iRet = getsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF, &i, &iLen);
//	if(iRet >= 0) {
//		showLog("Jni.DrUDPSocket.Bind", "getsockopt SOL_SOCKET SO_RCVBUF:%d iLen:%d", i, iLen);
//	}
//
//	int size = 8;
//	iRet = setsockopt(m_Socket, SOL_SOCKET, SO_SNDBUF,  &size, sizeof(int));
//	if(iRet < 0) {
//		showLog("Jni.DrUDPSocket.Bind", "setsockopt SOL_SOCKET SO_SNDBUF error");
//		return false;
//	}
//
//	iRet = setsockopt(m_Socket, SOL_SOCKET, SO_RCVBUF,  &size, sizeof(int));
//	if(iRet < 0) {
//		showLog("Jni.DrUDPSocket.Bind", "setsockopt SOL_SOCKET SO_RCVBUF error");
//		return false;
//	}

	m_bBlocking = bBlocking;
	if(m_bBlocking) {
		// set to blocking
		int iFlag = 1;
		if ((iFlag = fcntl(m_Socket, F_GETFL, 0)) == -1) {
			showLog("Jni.DrUDPSocket.Bind", "fcntl F_GETFL O_NONBLOCK socket error");
			return iRet;
		}
		if (fcntl(m_Socket, F_SETFL, iFlag & ~O_NONBLOCK) == -1) {
			showLog("Jni.DrUDPSocket.Bind", "fcntl F_SETFL iFlag & ~O_NONBLOCK socket error");
			return iRet;
		}
		showLog("Jni.DrUDPSocket.Bind", "set socket to blocking");
	}
	else {
		// set to nonblocking
		int iFlag = 1;
		if ((iFlag = fcntl(m_Socket, F_GETFL, 0)) == -1) {
			showLog("Jni.DrUDPSocket.RecvData", "fcntl F_GETFL O_NONBLOCK socket error");
			return iRet;
		}
		if (fcntl(m_Socket, F_SETFL, iFlag | O_NONBLOCK) == -1) {
			showLog("Jni.DrUDPSocket.RecvData", "fcntl F_SETFL iFlag | O_NONBLOCK socket error");
			return iRet;
		}
		showLog("Jni.DrUDPSocket.Bind", "set socket to nonblocking");
	}

	struct sockaddr_in localAddr;
	bzero(&localAddr, sizeof(localAddr));
	localAddr.sin_family = AF_INET;
	if(ip.length() > 0) {
		inet_aton(ip.c_str(), &localAddr.sin_addr);
	}
	else {
		localAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	}

	localAddr.sin_port = htons(iPort);
	if(bind(m_Socket, (struct sockaddr *)&localAddr, sizeof(localAddr)) < 0) {
		// bind socket error
		showLog("Jni.DrUDPSocket.Bind", "bind socket error");
		return false;
	}

	m_iPort = iPort;

	return true;
}
int DrUdpSocket::SendData(string ip, unsigned int iPort, char* pBuffer, unsigned int uiSendLen) {
	int iRet = -1, iRetS = -1, iSendedLen = 0;
	int iOrgLen = uiSendLen;
	fd_set wset;

	struct sockaddr_in remoteAddr;
	bzero(&remoteAddr, sizeof(remoteAddr));
	socklen_t iremoteAddrLen = sizeof(struct sockaddr_in);
	remoteAddr.sin_family = AF_INET;
	inet_aton(ip.c_str(), &remoteAddr.sin_addr);
	remoteAddr.sin_port = htons(iPort);

	string log = Arithmetic::AsciiToHexWithSep(pBuffer, uiSendLen);
	showLog("jni.DrUdpSocket::SendData", "准备向(%s:%d)发送数据，Hex编码后:\n%s(%d)", ip.c_str(), iPort, log.c_str(), uiSendLen);

	socket_type sendSocket;
	if(m_Socket > 0) {
		sendSocket = m_Socket;
	}
	else {
		if((sendSocket = socket(AF_INET, SOCK_DGRAM, 0/*IPPROTO_UDP*/)) < 0){
			// create socket error;
			showLog("Jni.DrUDPSocket.SendData", "create socket error");
		    return iRet;
		}
	}
	iRet = sendto(sendSocket, pBuffer, uiSendLen, 0, (struct sockaddr *)&remoteAddr, iremoteAddrLen);

	showLog("jni.DrUdpSocket::SendData", "成功发送字节数:(%d)", iRet);

//	int iLeft;
//	iRet = ioctl(m_Socket, SIOCOUTQ, &iLeft);
//	if(iRet >= 0) {
//		showLog("Jni.DrUDPSocket.SendData", "缓冲区剩余包:%d", iLeft);
//	}

	return iRet;
}
int DrUdpSocket::RecvData(char* pBuffer, unsigned int uiRecvLen, unsigned int uiTimeout, struct sockaddr_in* remoteAddr) {
	int iRet = -1, iRetS = -1;
	int iOrgLen = uiRecvLen;
	fd_set rset;

	if(-1 == m_Socket) {
		return iRet;
	}

	struct sockaddr_in sendAddr;
	bzero(&sendAddr, sizeof(sendAddr));
	int iSendAddrLen = sizeof(struct sockaddr_in);

	if(NULL != remoteAddr) {
		if(sizeof(*remoteAddr) != iSendAddrLen) {
			return iRet;
		}
	}

	if(m_bBlocking) {
		// blocking recv
		bzero(pBuffer, sizeof(pBuffer));
		iRet = recvfrom(m_Socket, pBuffer, uiRecvLen, 0, (struct sockaddr *)&sendAddr , &iSendAddrLen);
		memcpy((void *)remoteAddr, &sendAddr, iSendAddrLen);

		string ip = DrSocket::IpToString(sendAddr.sin_addr.s_addr);
		int iPort = ntohs(sendAddr.sin_port);
		string log = Arithmetic::AsciiToHexWithSep(pBuffer, iRet);
		showLog("jni.DrUdpSocket::RecvData", "收到来自(%s:%d)访问，成功接收字节数:(%d) Hex编码后:\n%s", ip.c_str(), iPort, iRet, log.c_str());
	}
	else {
		// nonblocking recv
		timeval tout;
		tout.tv_sec = uiTimeout / 1000;
		tout.tv_usec = (uiTimeout % 1000) * 1000;

		FD_ZERO(&rset);
		FD_SET(m_Socket, &rset);
		iRetS = select(m_Socket + 1, &rset, NULL, NULL, &tout);

		if(iRetS > 0) {
			bzero(pBuffer, sizeof(pBuffer));
			iRet = recvfrom(m_Socket, pBuffer, uiRecvLen, 0, (struct sockaddr *)&sendAddr , &iSendAddrLen);
			memcpy((void *)remoteAddr, &sendAddr, iSendAddrLen);

			string ip = DrSocket::IpToString(sendAddr.sin_addr.s_addr);
			int iPort = ntohs(sendAddr.sin_port);
			string log = Arithmetic::AsciiToHexWithSep(pBuffer, iRet);
			showLog("jni.DrUdpSocket::RecvData", "收到来自(%s:%d)访问，成功接收字节数:(%d) Hex编码后:\n%s", ip.c_str(), iPort, iRet, log.c_str());
		}
		else if(iRetS == 0){
			showLog("jni.DrUdpSocket::RecvData", "%d秒超时返回！", uiTimeout / 1000);
		}
		else {
			showLog("jni.DrUdpSocket::RecvData", "socket错误返回！");
		}
	}



	return iRet;
}
int DrUdpSocket::Close() {
	if (m_Socket != -1) {
		shutdown(m_Socket, SHUT_RDWR);
		close(m_Socket);
		m_Socket = -1;
	}
	return 1;
}
