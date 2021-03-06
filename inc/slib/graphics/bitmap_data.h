/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_GRAPHICS_BITMAP_DATA
#define CHECKHEADER_SLIB_GRAPHICS_BITMAP_DATA

#include "definition.h"

#include "bitmap_format.h"
#include "color.h"

#include "../core/ref.h"

namespace slib
{
	
	class SLIB_EXPORT ColorComponentBuffer
	{
	public:
		sl_uint32 width;
		sl_uint32 height;
		void* data;
		sl_int32 sample_stride; // bytes offset between samples on a row
		sl_int32 pitch; // bytes of a physical row
		Ref<Referable> ref;

	public:
		ColorComponentBuffer();

		~ColorComponentBuffer();

	};
	
	class SLIB_EXPORT BitmapData
	{
	public:
		sl_uint32 width;
		sl_uint32 height;
		BitmapFormat format;

		// plane0
		void* data; // samples
		sl_int32 pitch; // number of bytes of a row
		Ref<Referable> ref; // reference for samples

		// plane1
		void* data1; // samples
		sl_int32 pitch1; // number of bytes of a row
		Ref<Referable> ref1; // reference for samples
	
		// plane2
		void* data2; // samples
		sl_int32 pitch2; // number of bytes of a row
		Ref<Referable> ref2; // reference for samples

		// plane3
		void* data3; // samples
		sl_int32 pitch3; // number of bytes of a row
		Ref<Referable> ref3; // reference for samples
	
	public:
		BitmapData();

		BitmapData(const BitmapData& other);

		BitmapData(sl_uint32 width, sl_uint32 height, const Color* colors, sl_int32 stride = 0);

		~BitmapData();

	public:
		void*& planeData(sl_uint32 plane);

		void* const& planeData(sl_uint32 plane) const;
	
		sl_int32& planePitch(sl_uint32 plane);

		sl_int32 const& planePitch(sl_uint32 plane) const;

		static sl_int32 calculatePitchAlign1(sl_uint32 width, sl_uint32 bitsPerSample);

		static sl_int32 calculatePitchAlign2(sl_uint32 width, sl_uint32 bitsPerSample);

		static sl_int32 calculatePitchAlign4(sl_uint32 width, sl_uint32 bitsPerSample);

		static sl_int32 calculatePitchAlign8(sl_uint32 width, sl_uint32 bitsPerSample);

		static sl_int32 calculatePitchAlign16(sl_uint32 width, sl_uint32 bitsPerSample);
	
		void fillDefaultValues();

		sl_size getTotalSize() const;

		// returns the number of color components
		sl_uint32 getColorComponentBuffers(ColorComponentBuffer* buffers) const;

		void copyPixelsFrom(const BitmapData& other) const;

		void setFromColors(sl_uint32 width, sl_uint32 height, const Color* colors, sl_int32 stride = 0);

	public:
		BitmapData& operator=(const BitmapData& other);
	
	};

}

#endif
