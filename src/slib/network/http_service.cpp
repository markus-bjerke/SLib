#include "../../../inc/slib/network/http_service.h"

#include "../../../inc/slib/core/url.h"
#include "../../../inc/slib/core/resource.h"
#include "../../../inc/slib/core/file.h"
#include "../../../inc/slib/core/log.h"
#include "../../../inc/slib/core/content_type.h"

#define SERVICE_TAG "HTTP SERVICE"

SLIB_NETWORK_NAMESPACE_BEGIN


/**********************************************
	HttpServiceContext
**********************************************/
HttpServiceContext::HttpServiceContext()
{
	m_requestContentLength = 0;

	setClosingConnection(sl_false);
	setProcessingByThread(sl_true);
}

HttpServiceContext::~HttpServiceContext()
{
}

Ref<HttpService> HttpServiceContext::getService()
{
	Ref<HttpServiceConnection> connection = getConnection();
	if (connection.isNotNull()) {
		return connection->getService();
	}
	return Ref<HttpService>::null();
}

Ref<HttpServiceConnection> HttpServiceContext::getConnection()
{
	return m_connection;
}

Ref<AsyncStream> HttpServiceContext::getIO()
{
	Ref<HttpServiceConnection> connection = m_connection;
	if (connection.isNotNull()) {
		return connection->getIO();
	}
	return Ref<AsyncStream>::null();
}

Ref<AsyncLoop> HttpServiceContext::getAsyncLoop()
{
	Ref<HttpService> service = getService();
	if (service.isNotNull()) {
		return service->getAsyncLoop();
	}
	return Ref<AsyncLoop>::null();
}

const SocketAddress& HttpServiceContext::getLocalAddress()
{
	Ref<HttpServiceConnection> connection = getConnection();
	if (connection.isNotNull()) {
		return connection->getLocalAddress();
	} else {
		return SocketAddress::none();
	}
}

const SocketAddress& HttpServiceContext::getRemoteAddress()
{
	Ref<HttpServiceConnection> connection = getConnection();
	if (connection.isNotNull()) {
		return connection->getRemoteAddress();
	} else {
		return SocketAddress::none();
	}
}

Ref<HttpServiceContext> HttpServiceContext::create(const Ref<HttpServiceConnection>& _connection)
{
	Ref<HttpServiceContext> ret = new HttpServiceContext;
	Ref<HttpServiceConnection> connection = _connection;
	if (ret.isNotNull()) {
		ret->m_connection = connection;
	}
	return ret;
}

/******************************************************
	HttpServiceConnection
******************************************************/
#define SIZE_READ_BUF 0x10000
#define SIZE_COPY_BUF 0x10000

HttpServiceConnection::HttpServiceConnection()
{
	m_flagClosed = sl_false;
	m_flagReading = sl_false;
}

HttpServiceConnection::~HttpServiceConnection()
{
	close();
}

Ref<AsyncStream> HttpServiceConnection::getIO()
{
	return m_io;
}

Ref<HttpService> HttpServiceConnection::getService()
{
	return m_service;
}

Ref<HttpServiceConnection> HttpServiceConnection::create(HttpService* _service, AsyncStream* _io)
{
	Ref<HttpServiceConnection> ret;
	Ref<HttpService> service = _service;
	Ref<AsyncStream> io = _io;
	Ref<AsyncOutput> output = AsyncOutput::create(io, WeakRef<HttpServiceConnection>(ret), SIZE_COPY_BUF);
	if (service.isNotNull() && io.isNotNull() && output.isNotNull()) {
		Memory bufRead = Memory::create(SIZE_READ_BUF);
		if (bufRead.isNotEmpty()) {
			ret = new HttpServiceConnection;
			if (ret.isNotNull()) {
				ret->m_service = service;
				ret->m_io = io;
				ret->m_output = output;
				ret->m_bufRead = bufRead;
			}
		}
	}
	return ret;
}

void HttpServiceConnection::close()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	m_flagClosed = sl_true;
	Ref<HttpService> service = getService();
	if (service.isNotNull()) {
		service->closeConnection(this);
	}
	m_service.setNull();
	Ref<AsyncStream> io = m_io;
	if (io.isNotNull()) {
		io->close();
	}
	m_io.setNull();
	Ref<AsyncOutput> output = m_output;
	if (output.isNotNull()) {
		output->close();
	}
	m_output.setNull();
}

void HttpServiceConnection::start(const void* data, sl_uint32 size)
{
	m_contextCurrent.setNull();
	if (data && size > 0) {
		_processInput(data, size);
	} else {
		_read();
	}
}

void HttpServiceConnection::_processInput(const void* _data, sl_uint32 size)
{
	Ref<HttpService> service = getService();
	if (service.isNull()) {
		return;
	}
	if (m_flagClosed) {
		return;
	}
	
	const HttpServiceParam& param = service->getParam();
	sl_uint64 maxRequestHeadersSize = param.maxRequestHeadersSize;
	sl_uint64 maxRequestBodySize = param.maxRequestBodySize;

	char* data = (char*)_data;
	Ref<HttpServiceContext> _context = getCurrentContext();
	if (_context.isNull()) {
		_context = HttpServiceContext::create(this);
		if (_context.isNull()) {
			sendResponse_ServerError();
			return;
		}
		m_contextCurrent = _context;
		_context->setProcessingByThread(param.flagProcessByThreads);
	}
	HttpServiceContext* context = _context.get();
	if (context->m_requestHeader.isEmpty()) {
		sl_size posBody;
		if (context->m_requestHeaderReader.add(data, size, posBody)) {
			context->m_requestHeader = context->m_requestHeaderReader.mergeHeader();
			if (context->m_requestHeader.isEmpty()) {
				sendResponse_ServerError();
				return;
			}
			if (posBody > size) {
				sendResponse_ServerError();
				return;
			}
			context->m_requestHeaderReader.clear();
			sl_reg iRet = context->parseRequestPacket(context->m_requestHeader.getBuf(), context->m_requestHeader.getSize());
			if (iRet != context->m_requestHeader.getSize()) {
				sendResponse_BadRequest();
				return;
			}
			context->m_requestContentLength = context->getRequestContentLengthHeader();
			if (context->m_requestContentLength > maxRequestBodySize) {
				sendResponse_BadRequest();
				return;
			}
			context->m_requestBody = Memory::create(data + posBody, size - (sl_uint32)posBody);
			if (!(context->m_requestBody.isEmpty())) {
				sendResponse_ServerError();
				return;
			}
			if (!(context->m_requestBodyBuffer.add(context->m_requestBody))) {
				sendResponse_ServerError();
				return;
			}
			context->applyQueryToParameters();
			if (service->preprocessRequest(context)) {
				return;
			}
		} else {
			if (context->m_requestHeaderReader.getHeaderSize() > maxRequestHeadersSize) {
				sendResponse_BadRequest();
				return;
			}
		}
	} else {
		if (!(context->m_requestBodyBuffer.add(data, size))) {
			sendResponse_ServerError();
			return;
		}
	}
	if (context->m_requestHeader.isNotEmpty()) {
		if (context->m_requestBodyBuffer.getSize() >= context->m_requestContentLength) {

			m_contextCurrent.setNull();

			context->m_requestBody = context->m_requestBodyBuffer.merge();
			if (context->m_requestBody.isNotEmpty()) {
				sendResponse_ServerError();
				return;
			}
			context->m_requestBodyBuffer.clear();

			if (context->getMethodUppercase() == SLIB_HTTP_METHOD_POST) {
				if (context->getRequestContentType() == SLIB_HTTP_CONTENT_TYPE_FORM) {
					context->applyPostParameters(context->m_requestBody.getBuf(), context->m_requestBody.getSize());
				}
			}
			SLIB_STATIC_STRING(s, SLIB_HTTP_CONTENT_TYPE_TEXT_HTML_UTF8);
			context->setResponseContentType(s);
				
			if (context->isProcessingByThread()) {
				Ref<AsyncLoop> loop = service->getAsyncLoop();
				if (loop.isNotNull()) {
					loop->addTask(SLIB_CALLBACK_WEAKREF(HttpServiceConnection, _processContext, this, _context));
				} else {
					sendResponse_ServerError();
					return;
				}
			} else {
				_processContext(context);
			}
		}
	}
	_read();
}

void HttpServiceConnection::_read()
{
	MutexLocker lock(getLocker());
	if (m_flagClosed) {
		return;
	}
	if (m_flagReading) {
		return;
	}
	m_flagReading = sl_true;
	if (!(m_io->readToMemory(m_bufRead, (WeakRef<HttpServiceConnection>)this))) {
		m_flagReading = sl_false;
		close();
	}
}

void HttpServiceConnection::onRead(AsyncStream* stream, void* data, sl_uint32 size, Referable* ref, sl_bool flagError)
{
	m_flagReading = sl_false;
	if (flagError) {
		close();
	} else {
		_processInput(data, size);
	}
}

void HttpServiceConnection::_processContext(Ref<HttpServiceContext> context)
{
	Ref<HttpService> service = getService();
	if (service.isNull()) {
		return;
	}
	if (context->getMethodUppercase() == SLIB_HTTP_METHOD_CONNECT) {
		sendConnectResponse_Failed();
		return;
	} else {
		service->processRequest(context.get());
		SLIB_STATIC_STRING(s, "Content-Length");
		context->setResponseHeader(s, String::fromUint64(context->getResponseContentLength()));
	}
	Memory header = context->makeResponsePacket();
	if (header.isEmpty()) {
		close();
		return;
	}
	if (!(m_output->write(header))) {
		close();
		return;
	}
	m_output->mergeBuffer(&(context->m_bufferOutput));
	m_output->startWriting();
	start();
}

void HttpServiceConnection::onAsyncOutputError(AsyncOutput* output)
{
	close();
}

void HttpServiceConnection::sendResponse(const Memory& _mem)
{
	Memory mem = _mem;
	if (mem.isNotEmpty()) {
		if (m_io->writeFromMemory(mem, Ptr<IAsyncStreamListener>::null())) {
			return;
		}
	}
	close();
}

void HttpServiceConnection::sendResponseAndRestart(const Memory& _mem)
{
	Memory mem = _mem;
	if (mem.isNotEmpty()) {
		if (m_io->writeFromMemory(mem, Ptr<IAsyncStreamListener>::null())) {
			start();
			return;
		}
	}
	close();
}

class _HttpServiceConnection_SendResponseAndCloseListener : public Referable, public IAsyncStreamListener
{
public:
	WeakRef<HttpServiceConnection> m_connection;

	_HttpServiceConnection_SendResponseAndCloseListener(HttpServiceConnection* connection)
	{
		m_connection = connection;
	}

	void onWrite(AsyncStream* stream, void* data, sl_uint32 sizeWritten, Referable* ref, sl_bool flagError)
	{
		Ref<HttpServiceConnection> connection = m_connection;
		if (connection.isNotNull()) {
			connection->close();
		}
	}
};

void HttpServiceConnection::sendResponseAndClose(const Memory& _mem)
{
	Memory mem = _mem;
	if (mem.isNotEmpty()) {
		Ref<_HttpServiceConnection_SendResponseAndCloseListener> listener(new _HttpServiceConnection_SendResponseAndCloseListener(this));
		if (m_io->writeFromMemory(mem, listener)) {
			return;
		}
	}
	close();
}

void HttpServiceConnection::sendResponse_BadRequest()
{
	SLIB_STATIC_STRING8(s, "HTTP/1.1 400 Bad Request\r\n\r\n");
	sendResponseAndRestart(Memory::create(s.getBuf(), s.getLength()));
}

void HttpServiceConnection::sendResponse_ServerError()
{
	SLIB_STATIC_STRING8(s, "HTTP/1.1 500 Internal Server Error\r\n\r\n");
	sendResponseAndRestart(Memory::create(s.getBuf(), s.getLength()));
}

void HttpServiceConnection::sendConnectResponse_Successed()
{
	SLIB_STATIC_STRING8(s, "HTTP/1.1 200 Connection established\r\n\r\n");
	sendResponse(Memory::create(s.getBuf(), s.getLength()));
}

void HttpServiceConnection::sendConnectResponse_Failed()
{
	SLIB_STATIC_STRING8(s, "HTTP/1.1 500 Tunneling is not supported\r\n\r\n");
	sendResponseAndClose(Memory::create(s.getBuf(), s.getLength()));
}

void HttpServiceConnection::sendProxyResponse_Failed()
{
	SLIB_STATIC_STRING8(s, "HTTP/1.1 500 Internal Error\r\n\r\n");
	sendResponseAndRestart(Memory::create(s.getBuf(), s.getLength()));
}

/******************************************************
	HttpServiceConnectionProvider
******************************************************/
Ref<HttpService> HttpServiceConnectionProvider::getService()
{
	return m_service;
}

void HttpServiceConnectionProvider::setService(const Ref<HttpService>& service)
{
	m_service = service;
}

class _DefaultHttpServiceConnectionProvider : public HttpServiceConnectionProvider, public IAsyncTcpServerListener
{
public:
	Ref<AsyncTcpServer> m_server;
	Ref<AsyncLoop> m_loop;

	_DefaultHttpServiceConnectionProvider()
	{
	}

	~_DefaultHttpServiceConnectionProvider()
	{
		release();
	}

	void release()
	{
		MutexLocker lock(getLocker());
		if (m_server.isNotNull()) {
			m_server->close();
			m_server.setNull();
		}
	}

	void onAccept(AsyncTcpServer* socketListen, const Ref<Socket>& socketAccept, const SocketAddress& address)
	{
		Ref<HttpService> service = getService();
		if (service.isNotNull()) {
			Ref<AsyncLoop> loop = m_loop;
			if (loop.isNull()) {
				return;
			}
			Ref<AsyncTcpSocket> stream = AsyncTcpSocket::create(socketAccept, loop);
			if (stream.isNotNull()) {
				SocketAddress addrLocal;
				socketAccept->getLocalAddress(addrLocal);
				service->addConnection(stream.get(), address, addrLocal);
			}
		}
	}

	void onError(AsyncTcpServer* socketListen)
	{
		SLIB_LOG_ERROR(SERVICE_TAG, "Accept Error");
	}

	static Ref<HttpServiceConnectionProvider> create(HttpService* service, const SocketAddress& addressListen, const Ref<AsyncLoop>& loop)
	{
		Ref<_DefaultHttpServiceConnectionProvider> ret;
		Ref<AsyncTcpServer> server = AsyncTcpServer::create(addressListen, loop);
		if (server.isNotNull()) {
			ret = new _DefaultHttpServiceConnectionProvider;
			if (ret.isNotNull()) {
				ret->m_loop = server->getLoop();
				ret->m_server = server;
				ret->setService(service);
				server->start((WeakRef<_DefaultHttpServiceConnectionProvider>)(ret));
			}
		}
		return Ref<HttpServiceConnectionProvider>::from(ret);
	}
};

/******************************************************
	HttpService
******************************************************/
HttpService::HttpService()
{
	m_flagRunning = sl_false;
}

HttpService::~HttpService()
{
	release();
}

void HttpService::release()
{
	MutexLocker lock(getLocker());

	m_flagRunning = sl_false;
	{
		ListLocker< Ref<HttpServiceConnectionProvider> > cp(m_connectionProviders);
		for (sl_size i = 0; i < cp.count(); i++) {
			cp[i]->release();
		}
	}
	m_connectionProviders.clear();

	if (m_loop.isNotNull()) {
		m_loop->release();
		m_loop.setNull();
	}
	m_connections.clear();
}

sl_bool HttpService::start(const HttpServiceParam& param)
{
	MutexLocker lock(getLocker());

	if (m_flagRunning) {
		return sl_false;
	}

	Ref<AsyncLoop> loop = AsyncLoop::create();
	if (loop.isNotNull()) {
		loop->setMaximumThreadsCount(param.maxThreadsCount);
		m_loop = loop;
		m_param = param;
		if (param.port) {
			if (! (addHttpService(param.addressBind, param.port))) {
				m_loop.setNull();
				return sl_false;
			}
		}
		return sl_true;
	}
	return sl_false;
}

sl_bool HttpService::preprocessRequest(HttpServiceContext* context)
{
	return sl_false;
}

void HttpService::processRequest(HttpServiceContext* context)
{
	Ref<HttpServiceConnection> connection = context->getConnection();
	if (connection.isNull()) {
		return;
	}
	if (m_param.flagLogDebug) {
		SLIB_LOG(SERVICE_TAG, "[" + String::fromPointerValue(connection.get()) + "] Method="
			+ context->getMethod()
			+ " Path=" + context->getPath()
			+ " Query=" + context->getQuery()
			+ " Host=" + context->getHost());
	}
	ListLocker< Ptr<IHttpServiceProcessor> > processors(m_processors.duplicate());
	for (sl_size i = 0; i < processors.count(); i++) {
		PtrLocker<IHttpServiceProcessor> processor(processors[i]);
		if (processor.isNotNull()) {
			if (processor->onHttpRequest(context)) {
				return;
			}
		}
	}
	if (m_param.flagUseResource) {
		if (context->getMethodUppercase() == "GET") {
			String path = context->getPath();
			if (path.startsWith('/')) {
				path = path.substring(1);
			}
			path = m_param.prefixResource + path;
			if (processResource(context, path)) {
				return;
			}
			if (processResource(context, path + "/index.html")) {
				return;
			}
		}
	}
	context->setResponseCode(SLIB_HTTP_STATUS_404_NOT_FOUND);
}

sl_bool HttpService::processResource(HttpServiceContext* context, String path)
{
	FilePathSegments seg;
	seg.parsePath(path);
	if (seg.parentLevel == 0) {
		String ext = File::getFileExtension(path);
		String contentType = ContentType::fromFileExtension(ext);
		if (contentType.isEmpty()) {
			contentType = ContentType::getOctetStream();
		}
		context->setResponseContentType(contentType);
		if (Resource::isBasedOnFileSystem()) {
			String filePath = Resource::getFilePath(path);
			if (File::getSize(filePath) > 0x100000) {
				context->copyFromFile(filePath);
				return sl_true;
			}
		}
		Memory mem = Resource::readAllBytes(path);
		if (mem.isNotEmpty()) {
			context->write(mem);
			return sl_true;
		}
	}
	return sl_false;
}

Ref<HttpServiceConnection> HttpService::addConnection(const Ref<AsyncStream>& _stream, const SocketAddress& remoteAddress, const SocketAddress& localAddress)
{
	Ref<AsyncStream> stream = _stream;
	Ref<HttpServiceConnection> connection = HttpServiceConnection::create(this, stream.get());
	if (connection.isNotNull()) {
		if (m_param.flagLogDebug) {
			SLIB_LOG(SERVICE_TAG, "[" + String::fromPointerValue(connection.get()) + "] Connection Created - Address: " + remoteAddress.toString());
		}
		connection->setRemoteAddress(remoteAddress);
		connection->setLocalAddress(localAddress);
		m_connections.put(connection.get(), connection);
		connection->start();
	}
	return connection;
}

void HttpService::closeConnection(HttpServiceConnection* connection)
{
	if (m_param.flagLogDebug) {
		SLIB_LOG(SERVICE_TAG, "[" + String::fromPointerValue(connection) + "] Connection Closed");
	}
	m_connections.remove(connection);
}

sl_bool HttpService::addHttpService(const SocketAddress& addr)
{
	Ref<HttpServiceConnectionProvider> provider = _DefaultHttpServiceConnectionProvider::create(this, addr, m_loop);
	if (provider.isNotNull()) {
		addConnectionProvider(provider);
		return sl_true;
	}
	return sl_false;
}

Ref<HttpService> HttpService::create(const HttpServiceParam& param)
{
	Ref<HttpService> ret = new HttpService;
	if (ret.isNotNull()) {
		if (ret->start(param)) {
			return ret;
		}
		ret.setNull();
	}
	return ret;
}

SLIB_NETWORK_NAMESPACE_END
