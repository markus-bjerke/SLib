/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_MEDIA_AUDIO_CODEC
#define CHECKHEADER_SLIB_MEDIA_AUDIO_CODEC

#include "definition.h"

#include "audio_data.h"

#include "../core/object.h"
#include "../core/memory.h"

namespace slib
{
	class SLIB_EXPORT AudioEncoder : public Object
	{
		SLIB_DECLARE_OBJECT
		
	public:
		AudioEncoder();
		
		~AudioEncoder();
		
	public:
		virtual Memory encode(const AudioData& input) = 0;
		
	public:
		sl_uint32 getSamplesCountPerSecond() const;
		
		sl_uint32 getChannelsCount() const;
		
		sl_uint32 getBitrate();

		virtual void setBitrate(sl_uint32 bitrate);
		
	protected:
		sl_uint32 m_nSamplesPerSecond;
		sl_uint32 m_nChannels;
		sl_uint32 m_bitrate;
		
	};
	
	class SLIB_EXPORT AudioDecoder : public Object
	{
		SLIB_DECLARE_OBJECT
		
	public:
		AudioDecoder();
		
		~AudioDecoder();
		
	public:
		// returns the count of samples (per channel) actually decoded
		virtual sl_uint32 decode(const void* input, sl_uint32 sizeInput, const AudioData& output) = 0;
		
	public:
		sl_uint32 getSamplesCountPerSecond() const;
		
		sl_uint32 getChannelsCount() const;
		
	protected:
		sl_uint32 m_nSamplesPerSecond;
		sl_uint32 m_nChannels;
		
	};
}

#endif
