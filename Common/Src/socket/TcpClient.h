/*
 * Copyright: JessMA Open Source (ldcsaa@gmail.com)
 *
 * Version	: 3.2.1
 * Author	: Bruce Liang
 * Website	: http://www.jessma.org
 * Project	: https://github.com/ldcsaa
 * Blog		: http://www.cnblogs.com/ldcsaa
 * Wiki		: http://www.oschina.net/p/hp-socket
 * QQ Group	: 75375912
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */
 
#pragma once

#include "SocketHelper.h"
#include "../Event.h"
#include "../bufferptr.h"
#include "../bufferpool.h"
#include "../CriticalSection.h"

class CTcpClient : public ITcpClient
{
public:
	virtual BOOL Start	(LPCTSTR pszRemoteAddress, USHORT usPortt, BOOL bAsyncConnect = FALSE);
	virtual BOOL Stop	();
	virtual BOOL Send	(const BYTE* pBuffer, int iLength, int iOffset = 0);
	virtual BOOL			HasStarted			()	{return m_enState == SS_STARTED || m_enState == SS_STARTING;}
	virtual EnServiceState	GetState			()	{return m_enState;}
	virtual CONNID			GetConnectionID		()	{return m_dwConnID;};
	virtual BOOL			GetLocalAddress		(LPTSTR lpszAddress, int& iAddressLen, USHORT& usPort);
	virtual BOOL GetPendingDataLength			(int& iPending) {iPending = m_iPending; return HasStarted();}
	virtual EnSocketError	GetLastError		()	{return m_enLastError;}
	virtual LPCTSTR			GetLastErrorDesc	()	{return ::GetSocketErrorDesc(m_enLastError);}

public:
	virtual void SetSocketBufferSize	(DWORD dwSocketBufferSize)	{m_dwSocketBufferSize	= dwSocketBufferSize;}
	virtual void SetKeepAliveTime		(DWORD dwKeepAliveTime)		{m_dwKeepAliveTime		= dwKeepAliveTime;}
	virtual void SetKeepAliveInterval	(DWORD dwKeepAliveInterval)	{m_dwKeepAliveInterval	= dwKeepAliveInterval;}

	virtual DWORD GetSocketBufferSize	()	{return m_dwSocketBufferSize;}
	virtual DWORD GetKeepAliveTime		()	{return m_dwKeepAliveTime;}
	virtual DWORD GetKeepAliveInterval	()	{return m_dwKeepAliveInterval;}

	virtual void SetFreeBufferPoolSize	(DWORD dwFreeBufferPoolSize)	{m_dwFreeBufferPoolSize = dwFreeBufferPoolSize;}
	virtual void SetFreeBufferPoolHold	(DWORD dwFreeBufferPoolHold)	{m_dwFreeBufferPoolHold = dwFreeBufferPoolHold;}

	virtual DWORD GetFreeBufferPoolSize	()	{return m_dwFreeBufferPoolSize;}
	virtual DWORD GetFreeBufferPoolHold	()	{return m_dwFreeBufferPoolHold;}

protected:
	virtual EnHandleResult FirePrepareConnect(CONNID dwConnID, SOCKET socket)
		{return m_psoListener->OnPrepareConnect(dwConnID, socket);}
	virtual EnHandleResult FireConnect(CONNID dwConnID)
		{return m_psoListener->OnConnect(dwConnID);}
	virtual EnHandleResult FireSend(CONNID dwConnID, const BYTE* pData, int iLength)
		{return m_psoListener->OnSend(dwConnID, pData, iLength);}
	virtual EnHandleResult FireReceive(CONNID dwConnID, const BYTE* pData, int iLength)
		{return m_psoListener->OnReceive(dwConnID, pData, iLength);}
	virtual EnHandleResult FireReceive(CONNID dwConnID, int iLength)
		{return m_psoListener->OnReceive(dwConnID, iLength);}
	virtual EnHandleResult FireClose(CONNID dwConnID)
		{return m_psoListener->OnClose(dwConnID);}
	virtual EnHandleResult FireError(CONNID dwConnID, EnSocketOperation enOperation, int iErrorCode)
		{return m_psoListener->OnError(dwConnID, enOperation, iErrorCode);}

	virtual void Reset();

private:
	BOOL CheckParams();
	BOOL CheckStarting();
	BOOL CheckStoping();
	BOOL CreateClientSocket();
	BOOL ConnectToServer(LPCTSTR pszRemoteAddress, USHORT usPort);
	BOOL CreateWorkerThread();
	BOOL ProcessNetworkEvent();
	BOOL ReadData();
	BOOL SendData();
	BOOL DoSendData(TItem* pItem);
	TItem* GetSendBuffer();
	void WaitForWorkerThreadEnd(DWORD dwCurrentThreadID);

	void SetLastError(EnSocketError code, LPCSTR func, int ec);

	static 
#ifndef _WIN32_WCE
	UINT
#else
	DWORD
#endif
	 WINAPI WorkerThreadProc(LPVOID pv);

private:
	static const DWORD DEFAULT_SOCKET_BUFFER_SIZE;
	static const DWORD DEFAULT_FREE_BUFFER_POOL_SIZE;
	static const DWORD DEFAULT_FREE_BUFFER_POOL_HOLD;
	static const DWORD DEFALUT_KEEPALIVE_TIME;
	static const DWORD DEFALUT_KEEPALIVE_INTERVAL;

public:
	CTcpClient(ITcpClientListener* psoListener)
	: m_psoListener			(psoListener)
	, m_lsSend				(m_itPool)
	, m_soClient			(INVALID_SOCKET)
	, m_evSocket			(nullptr)
	, m_dwConnID			(0)
	, m_hWorker				(nullptr)
	, m_dwWorkerID			(0)
	, m_bAsyncConnect		(FALSE)
	, m_iPending			(0)
	, m_enState				(SS_STOPED)
	, m_enLastError			(SE_OK)
	, m_dwSocketBufferSize	(DEFAULT_SOCKET_BUFFER_SIZE)
	, m_dwFreeBufferPoolSize(DEFAULT_FREE_BUFFER_POOL_SIZE)
	, m_dwFreeBufferPoolHold(DEFAULT_FREE_BUFFER_POOL_HOLD)
	, m_dwKeepAliveTime		(DEFALUT_KEEPALIVE_TIME)
	, m_dwKeepAliveInterval	(DEFALUT_KEEPALIVE_INTERVAL)
	{
		ASSERT(m_psoListener);
	}

	virtual ~CTcpClient()	{if(HasStarted()) Stop();}

protected:
	CItemPool		m_itPool;

private:
	CInitSocket		m_wsSocket;
	SOCKET			m_soClient;
	HANDLE			m_evSocket;
	CONNID			m_dwConnID;

	CEvt			m_evWorker;
	HANDLE			m_hWorker;

#ifndef _WIN32_WCE
	UINT
#else
	DWORD
#endif
					m_dwWorkerID;

	BOOL			m_bAsyncConnect;

	CEvt			m_evBuffer;
	CCriSec			m_scSend;
	TItemList		m_lsSend;
	CBufferPtr		m_rcBuffer;

	volatile int			m_iPending;
	volatile EnServiceState	m_enState;
	volatile EnSocketError	m_enLastError;

private:
	ITcpClientListener*	m_psoListener;

private:
	DWORD m_dwSocketBufferSize;
	DWORD m_dwFreeBufferPoolSize;
	DWORD m_dwFreeBufferPoolHold;
	DWORD m_dwKeepAliveTime;
	DWORD m_dwKeepAliveInterval;
};
