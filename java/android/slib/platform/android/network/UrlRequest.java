/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

package slib.platform.android.network;

import java.io.FileOutputStream;
import java.io.InputStream;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.URL;
import java.net.URLConnection;
import java.util.Vector;

import slib.platform.android.Logger;

public class UrlRequest {

	final static int NETWORK_TIMEOUT = 30000;
	final static int PACKET_SIZE = 4096;

	boolean flagOpened = true;
	long instance;

	URLConnection connection = null;
	HttpURLConnection http = null;
	InputStream streamContent = null;
	OutputStream streamDownload = null;

	UrlRequest(long instance) {
		this.instance = instance;
	}

	public static void execute(long instance, String _url, String method, String[] headers, String[] additionalHeaders, byte[] uploadBody, String downloadFilePath) {

		if (downloadFilePath != null && downloadFilePath.length() == 0) {
			downloadFilePath = null;
		}

		UrlRequest request = new UrlRequest(instance);

		try {
			request.onInit();

			OutputStream streamDownload = null;
			if (downloadFilePath != null) {
				streamDownload = new FileOutputStream(downloadFilePath);
			}
			request.streamDownload = streamDownload;

			URL url = new URL(_url);
			URLConnection connection = url.openConnection();
			request.connection = connection;

			HttpURLConnection http = null;
			if (connection instanceof HttpURLConnection) {
				http = (HttpURLConnection) connection;
			}
			request.http = http;

			if (!(request.flagOpened)) {
				return;
			}
			if (http != null) {
				if (uploadBody != null && uploadBody.length > 0) {
					http.setDoOutput(true);
				}
				http.setDoInput(true);
				http.setConnectTimeout(NETWORK_TIMEOUT);
				http.setReadTimeout(NETWORK_TIMEOUT);
				http.setRequestMethod(method);
				if (headers != null) {
					for (int i = 0; i < headers.length - 1; i += 2) {
						http.setRequestProperty(headers[i], headers[i+1]);
					}
				}
				if (additionalHeaders != null) {
					for (int i = 0; i < additionalHeaders.length - 1; i += 2) {
						http.addRequestProperty(additionalHeaders[i], additionalHeaders[i+1]);
					}
				}
			}

			connection.connect();
			if (!(request.flagOpened)) {
				return;
			}

			if (http != null && uploadBody != null && uploadBody.length > 0) {
				OutputStream os = connection.getOutputStream();
				int pos = 0;
				while (pos < uploadBody.length) {
					if (!(request.flagOpened)) {
						return;
					}
					int len = PACKET_SIZE;
					if (pos + len > uploadBody.length) {
						len = uploadBody.length - pos;
					}
					os.write(uploadBody, pos, len);
					request.onUploadBody(len);
					pos += len;
				}
			}
			if (!(request.flagOpened)) {
				return;
			}

			if (http != null) {
				int statusCode = http.getResponseCode();
				String message = http.getResponseMessage();
				Vector<String> responseHeaders = new Vector<String>();
				for (int i = 0; i < 1024; i++) {
					String key = http.getHeaderFieldKey(i);
					String value = http.getHeaderField(i);
					if (key == null && value == null) {
						break;
					}
					responseHeaders.add(key);
					responseHeaders.add(value);
				}
				request.onResponse(statusCode, message, responseHeaders.toArray(new String[] {}));
			}
			if (!(request.flagOpened)) {
				return;
			}

			InputStream streamContent = connection.getInputStream();
			request.streamContent = streamContent;

			byte[] packet = new byte[PACKET_SIZE];
			while (true) {
				int n = streamContent.read(packet);
				if (n <= 0) {
					break;
				}
				if (downloadFilePath != null) {
					streamDownload.write(packet, 0, n);
					request.onDownloadContent(n);
				} else {
					request.onReceiveContent(packet, n);
				}
				if (!(request.flagOpened)) {
					return;
				}
			}

			request.close();

			request.onComplete();

		} catch (Exception e) {
			request.processError(e);
			return;
		}

	}

	public synchronized void close() {
		if (flagOpened) {
			flagOpened = false;
			if (streamDownload != null) {
				try {
					streamDownload.close();
				} catch (Exception e) {
					Logger.exception(e);
				}
				streamDownload = null;
			}
			connection = null;
			if (http != null) {
				try {
					http.disconnect();
				} catch (Exception e) {
					Logger.exception(e);
				}
				http = null;
				streamContent = null;
			} else {
				if (streamContent != null) {
					try {
						streamContent.close();;
					} catch (Exception e) {
						Logger.exception(e);
					}
					streamContent = null;
				}
			}
		}
	}

	void processError(Exception e) {
		close();
		if (e != null) {
			Logger.exception(e);
		}
		onError();
	}

	private native void nativeOnInit(long instance);
	protected void onInit() {
		nativeOnInit(instance);
	}

	private native void nativeOnUploadBody(long instance, int n);
	protected void onUploadBody(int n) {
		nativeOnUploadBody(instance, n);
	}

	private native void nativeOnResponse(long instance, int statusCode, String message, String[] headers);
	protected void onResponse(int statusCode, String message, String[] headers) {
		nativeOnResponse(instance, statusCode, message, headers);
	}

	private native void nativeOnReceiveContent(long instance, byte[] data, int n);
	protected void onReceiveContent(byte[] data, int n) {
		nativeOnReceiveContent(instance, data, n);
	}

	private native void nativeOnDownloadContent(long intance, int n);
	protected void onDownloadContent(int n) {
		nativeOnDownloadContent(instance, n);
	}

	private native void nativeOnComplete(long instance);
	protected void onComplete() {
		nativeOnComplete(instance);
	}

	private native void nativeOnError(long instance);
	protected void onError() {
		nativeOnError(instance);
	}

}
