/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "../../../inc/slib/graphics/image.h"

#include "../../../inc/slib/core/file.h"
#include "../../../inc/slib/core/asset.h"
#include "../../../inc/slib/core/scoped.h"

#include "image_stb.h"

namespace slib
{

	ImageDesc::ImageDesc()
	: width(0), height(0), stride(0), colors(sl_null)
	{
	}

	ImageDesc::~ImageDesc()
	{
	}


	SLIB_DEFINE_OBJECT(Image, Bitmap)

	Image::Image()
	{
	}

	Image::~Image()
	{
	}

	Ref<Image> Image::createStatic(const ImageDesc& desc)
	{
		return createStatic(desc.width, desc.height, desc.colors, desc.stride, desc.ref.get());
	}

	Ref<Image> Image::createStatic(sl_uint32 width, sl_uint32 height, const Color* pixels, sl_int32 stride)
	{
		return createStatic(width, height, pixels, stride, sl_null);
	}

	Ref<Image> Image::createStatic(sl_uint32 width, sl_uint32 height, const Color* pixels, sl_int32 stride, Referable* ref)
	{
		Ref<Image> ret;
		if (width == 0 || height == 0 || pixels == sl_null) {
			return ret;
		}
		if (stride == 0) {
			stride = width;
		}
		ret = new Image;
		if (ret.isNotNull()) {
			ret->m_desc.colors = (Color*)pixels;
			ret->m_desc.width = width;
			ret->m_desc.height = height;
			ret->m_desc.stride = stride;
			ret->m_desc.ref = ref;
		}
		return ret;
	}

	Ref<Image> Image::create(const ImageDesc& desc)
	{
		return create(desc.width, desc.height, desc.colors, desc.stride);
	}

	Ref<Image> Image::create(sl_uint32 width, sl_uint32 height, const Color* pixels, sl_int32 stride)
	{
		if (pixels) {
			BitmapData bitmapData(width, height, pixels, stride);
			return create(bitmapData);
		} else {
			Ref<Image> ret;
			sl_uint32 size = (width*height) << 2;
			Memory mem = Memory::create(size);
			if (mem.isNotEmpty()) {
				ret = new Image;
				if (ret.isNotNull()) {
					ret->m_desc.width = width;
					ret->m_desc.height = height;
					ret->m_desc.stride = width;
					ret->m_desc.ref = mem.ref;
					ret->m_desc.colors = (Color*)(mem.getData());
					Base::zeroMemory(ret->m_desc.colors, size);
				}
			}
			return ret;

		}
	}

	Ref<Image> Image::create(const BitmapData& bitmapData)
	{
		Ref<Image> ret;
		sl_uint32 width = bitmapData.width;
		sl_uint32 height = bitmapData.height;
		if (width == 0 || height == 0) {
			return ret;
		}
		sl_uint32 size = (width*height) << 2;
		Memory mem = Memory::create(size);
		if (mem.isNotEmpty()) {
			ret = new Image;
			if (ret.isNotNull()) {
				ret->m_desc.width = width;
				ret->m_desc.height = height;
				ret->m_desc.stride = width;
				ret->m_desc.ref = mem.ref;
				ret->m_desc.colors = (Color*)(mem.getData());
				BitmapData dst(width, height, (Color*)(mem.getData()));
				if (BitmapFormats::isPrecomputedAlpha(bitmapData.format)) {
					dst.format = BitmapFormats::getPrecomputedAlphaFormat(dst.format);
				}
				dst.copyPixelsFrom(bitmapData);
			}
		}
		return ret;
	}

	Ref<Image> Image::create(const Ref<Bitmap>& bitmap, sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height)
	{
		Ref<Image> ret;
		if (bitmap.isNull()) {
			return ret;
		}
		sl_uint32 bw = bitmap->getWidth();
		sl_uint32 bh = bitmap->getHeight();
		if (x >= bw) {
			return ret;
		}
		if (y >= bh) {
			return ret;
		}
		if (width > bw - x) {
			return ret;
		}
		if (height > bh - y) {
			return ret;
		}
		Memory mem = Memory::create((width*height) << 2);
		if (mem.isNotEmpty()) {
			Color* buf = (Color*)(mem.getData());
			if (bitmap->readPixels(x, y, width, height, buf)) {
				ImageDesc desc;
				desc.width = width;
				desc.height = height;
				desc.stride = width;
				desc.colors = buf;
				desc.ref = mem.ref;
				return Image::createStatic(desc);
			}
		}
		return ret;
	}

	Ref<Image> Image::create(const Ref<Bitmap>& bitmap)
	{
		Ref<Image> ret;
		if (bitmap.isNull()) {
			return ret;
		}
		sl_uint32 width = bitmap->getWidth();
		sl_uint32 height = bitmap->getHeight();
		Memory mem = Memory::create((width*height) << 2);
		if (mem.isNotEmpty()) {
			Color* buf = (Color*)(mem.getData());
			if (bitmap->readPixels(0, 0, width, height, buf)) {
				ImageDesc desc;
				desc.width = width;
				desc.height = height;
				desc.stride = width;
				desc.colors = buf;
				desc.ref = mem.ref;
				return Image::createStatic(desc);
			}
		}
		return ret;
	}

	sl_uint32 Image::getWidth() const
	{
		return m_desc.width;
	}

	sl_uint32 Image::getHeight() const
	{
		return m_desc.height;
	}

	sl_bool Image::isEmpty() const
	{
		return m_desc.width == 0 || m_desc.height == 0;
	}

	sl_bool Image::isNotEmpty() const
	{
		return m_desc.width != 0 && m_desc.height != 0;
	}

	sl_int32 Image::getStride() const
	{
		return m_desc.stride;
	}

	Color* Image::getColors() const
	{
		return (Color*)(m_desc.colors);
	}

	Color* Image::getColorsAt(sl_int32 x, sl_int32 y) const
	{
		return getColors() + x + y * m_desc.stride;
	}

	Color& Image::getPixel(sl_int32 x, sl_int32 y) const
	{
		return getColors()[x + y * m_desc.stride];
	}

	void Image::getDesc(ImageDesc& desc) const
	{
		desc = m_desc;
	}


	sl_uint32 Image::getBitmapWidth()
	{
		return m_desc.width;
	}

	sl_uint32 Image::getBitmapHeight()
	{
		return m_desc.height;
	}

	sl_bool Image::readPixels(sl_uint32 x, sl_uint32 y, BitmapData& bmd)
	{
		ImageDesc& imd = m_desc;
		if (x >= imd.width || y >= imd.height) {
			return sl_false;
		}
		if (x + bmd.width > imd.width || y + bmd.height > imd.height) {
			return sl_false;
		}
		Color* colors = getColorsAt(x, y);
		BitmapData src(bmd.width, bmd.height, colors, imd.stride);
		bmd.copyPixelsFrom(src);
		return sl_true;
	}

	sl_bool Image::writePixels(sl_uint32 x, sl_uint32 y, const BitmapData& bmd)
	{
		ImageDesc& imd = m_desc;
		if (x >= imd.width || y >= imd.height) {
			return sl_false;
		}
		if (x + bmd.width > imd.width || y + bmd.height > imd.height) {
			return sl_false;
		}
		Color* colors = getColorsAt(x, y);
		BitmapData src(bmd.width, bmd.height, colors, imd.stride);
		src.copyPixelsFrom(bmd);
		return sl_true;
	}

	sl_bool Image::resetPixels(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height, const Color& color)
	{
		ImageDesc& imd = m_desc;
		if (x >= imd.width || y >= imd.height) {
			return sl_false;
		}
		if (x + width > imd.width) {
			width = imd.width - x;
		}
		if (y + height > imd.height) {
			height = imd.height - y;
		}
		Color* colors = getColorsAt(x, y);
		for (sl_uint32 yi = 0; yi < height; yi++) {
			Color* colorsDst = colors;
			for (sl_uint32 xi = 0; xi < width; xi++) {
				colorsDst[xi] = color;
			}
			colors += m_desc.stride;
		}
		return sl_true;
	}

	sl_bool Image::readPixels(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height, Color* colors, sl_int32 stride)
	{
		BitmapData bitmapData(width, height, colors, stride);
		return readPixels(x, y, bitmapData);
	}

	sl_bool Image::writePixels(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height, const Color* colors, sl_int32 stride)
	{
		BitmapData bitmapData(width, height, colors, stride);
		return writePixels(x, y, bitmapData);
	}

	sl_bool Image::resetPixels(const Color& color)
	{
		return resetPixels(0, 0, getWidth(), getHeight(), color);
	}

	Ref<Canvas> Image::getCanvas()
	{
		return sl_null;
	}

	void Image::fillColor(const Color& color)
	{
		Color* colorsDst;
		Color* colorsDstLine = m_desc.colors;
		for (sl_uint32 y = 0; y < m_desc.height; y++) {
			colorsDst = colorsDstLine;
			for (sl_uint32 x = 0; x < m_desc.width; x++) {
				colorsDst[x] = color;
			}
			colorsDstLine += m_desc.stride;
		}
	}

	class _ImageStretch_FillColor
	{
	public:
		template <class BLEND_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src)
		{
			Color* colorsDst = dst.colors;
			Color color = *(src.colors);
			for (sl_uint32 y = 0; y < dst.height; y++) {
				for (sl_uint32 x = 0; x < dst.width; x++) {
					BLEND_OP::blend(colorsDst[x], color);
				}
				colorsDst += dst.stride;
			}
		}
	};

	class _ImageStretch_Copy
	{
	public:
		template <class BLEND_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src)
		{
			Color* colorsDst = dst.colors;
			const Color* colorsSrc = src.colors;
			for (sl_uint32 y = 0; y < dst.height; y++) {
				for (sl_uint32 x = 0; x < dst.width; x++) {
					BLEND_OP::blend(colorsDst[x], colorsSrc[x]);
				}
				colorsDst += dst.stride;
				colorsSrc += src.stride;
			}
		}
	};

	class _ImageStretch_Nearest
	{
	public:
		template <class BLEND_OP>
		static void stretchX(ImageDesc& dst, const ImageDesc& src)
		{
			sl_uint32 dx, dy;
			
			SLIB_SCOPED_BUFFER(sl_uint32, 1024, mapx, dst.width);
			sl_uint32 tx = 0;
			for (dx = 0; dx < dst.width; dx++) {
				mapx[dx] = tx / dst.width;
				tx += src.width;
			}
			
			Color* colorsDst = dst.colors;
			const Color* colorsSrc = src.colors;
			for (dy = 0; dy < dst.height; dy++) {
				for (dx = 0; dx < dst.width; dx++) {
					BLEND_OP::blend(colorsDst[dx], colorsSrc[mapx[dx]]);
				}
				colorsDst += dst.stride;
				colorsSrc += src.stride;
			}
		}
		
		template <class BLEND_OP>
		static void stretchY(ImageDesc& dst, const ImageDesc& src)
		{
			sl_uint32 dx, dy;
			
			Color* colorsDst = dst.colors;
			for (dy = 0; dy < dst.height; dy++) {
				const Color* colorsSrc = src.colors + ((dy * src.height) / dst.height) * src.stride;
				for (dx = 0; dx < dst.width; dx++) {
					BLEND_OP::blend(colorsDst[dx], colorsSrc[dx]);
				}
				colorsDst += dst.stride;
			}
		}
		
		template <class BLEND_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src)
		{
			if (src.width == dst.width) {
				stretchX<BLEND_OP>(dst, src);
				return;
			}
			if (src.height == dst.height) {
				stretchY<BLEND_OP>(dst, src);
				return;
			}
			
			sl_uint32 dx, dy;
			
			SLIB_SCOPED_BUFFER(sl_uint32, 1024, mapx, dst.width);
			sl_uint32 tx = 0;
			for (dx = 0; dx < dst.width; dx++) {
				mapx[dx] = tx / dst.width;
				tx += src.width;
			}
			
			Color* colorsDst = dst.colors;
			for (dy = 0; dy < dst.height; dy++) {
				const Color* colorsSrc = src.colors + ((dy * src.height) / dst.height) * src.stride;
				for (dx = 0; dx < dst.width; dx++) {
					BLEND_OP::blend(colorsDst[dx], colorsSrc[mapx[dx]]);
				}
				colorsDst += dst.stride;
			}
		}
		
	};

	struct _ImageStretch_FilterParam
	{
		sl_bool flagBox;
		float filterSize;
	};

	class _ImageStretch_Smooth_LinearFilter
	{
	public:
		SLIB_INLINE static void getColorAt(Color& _out, const Color* colors, float fx, float fy, sl_uint32 stride, const _ImageStretch_FilterParam& px, const _ImageStretch_FilterParam& py)
		{
			const Color& c00 = *colors;
			const Color& c01 = colors[1];
			const Color& c10 = colors[stride];
			const Color& c11 = colors[stride + 1];
			float sx = 1 - fx;
			float ex = fx;
			float sy = 1 - fy;
			float ey = fy;
			float f00 = sx * sy;
			float f01 = ex * sy;
			float f10 = sx * ey;
			float f11 = ex * ey;
			float r = (float)(c00.r) * f00 + (float)(c01.r) * f01 + (float)(c10.r) * f10 + (float)(c11.r) * f11;
			float g = (float)(c00.g) * f00 + (float)(c01.g) * f01 + (float)(c10.g) * f10 + (float)(c11.g) * f11;
			float b = (float)(c00.b) * f00 + (float)(c01.b) * f01 + (float)(c10.b) * f10 + (float)(c11.b) * f11;
			float a = (float)(c00.a) * f00 + (float)(c01.a) * f01 + (float)(c10.a) * f10 + (float)(c11.a) * f11;
			_out.r = (sl_uint8)(r);
			_out.g = (sl_uint8)(g);
			_out.b = (sl_uint8)(b);
			_out.a = (sl_uint8)(a);
		}
		
		SLIB_INLINE static void getColorAtX(Color& _out, const Color* colors, float fx, const _ImageStretch_FilterParam& px)
		{
			const Color& c0 = *colors;
			const Color& c1 = colors[1];
			float f0 = 1 - fx;
			float f1 = fx;
			float r = (float)(c0.r) * f0 + (float)(c1.r) * f1;
			float g = (float)(c0.g) * f0 + (float)(c1.g) * f1;
			float b = (float)(c0.b) * f0 + (float)(c1.b) * f1;
			float a = (float)(c0.a) * f0 + (float)(c1.a) * f1;
			_out.r = (sl_uint8)(r);
			_out.g = (sl_uint8)(g);
			_out.b = (sl_uint8)(b);
			_out.a = (sl_uint8)(a);
		}
		
		SLIB_INLINE static void getColorAtY(Color& _out, const Color* colors, float fy, sl_uint32 stride, const _ImageStretch_FilterParam& py)
		{
			const Color& c0 = *colors;
			const Color& c1 = colors[stride];
			float f0 = 1 - fy;
			float f1 = fy;
			float r = (float)(c0.r) * f0 + (float)(c1.r) * f1;
			float g = (float)(c0.g) * f0 + (float)(c1.g) * f1;
			float b = (float)(c0.b) * f0 + (float)(c1.b) * f1;
			float a = (float)(c0.a) * f0 + (float)(c1.a) * f1;
			_out.r = (sl_uint8)(r);
			_out.g = (sl_uint8)(g);
			_out.b = (sl_uint8)(b);
			_out.a = (sl_uint8)(a);
		}

	};

	class _ImageStretch_Smooth_BoxFilter
	{
	public:
		SLIB_INLINE static void prepareFilter(float f, const _ImageStretch_FilterParam& p, float& s, float& e, int& n, float& area)
		{
			if (p.flagBox) {
				s = 1 - f;
				e = f + p.filterSize; // p.filterSize > 1
				n = (int)(e - 0.0001);
				e -= (float)n;
				n -= 1;
				area = p.filterSize;
			} else {
				s = 1 - f;
				e = f;
				n = 0;
				area = 1;
			}
		}
		
		SLIB_INLINE static void getColorAt(Color& _out, const Color* colors, float fx, float fy, sl_uint32 stride, const _ImageStretch_FilterParam& px, const _ImageStretch_FilterParam& py)
		{
			float sx, ex, ax; int nx;
			prepareFilter(fx, px, sx, ex, nx, ax);
			float sy, ey, ay; int ny;
			prepareFilter(fy, py, sy, ey, ny, ay);
			int ix, iy;
			float r = sx * (float)(colors[0].r) + ex * (float)(colors[nx + 1].r);
			float g = sx * (float)(colors[0].g) + ex * (float)(colors[nx + 1].g);
			float b = sx * (float)(colors[0].b) + ex * (float)(colors[nx + 1].b);
			float a = sx * (float)(colors[0].a) + ex * (float)(colors[nx + 1].a);
			for (ix = 0; ix < nx; ix++) {
				r += (float)(colors[1 + ix].r);
				g += (float)(colors[1 + ix].g);
				b += (float)(colors[1 + ix].b);
				a += (float)(colors[1 + ix].a);
			}
			r *= sy;
			g *= sy;
			b *= sy;
			a *= sy;
			colors += stride;
			for (iy = 0; iy < ny; iy++) {
				r += sx * (float)(colors[0].r) + ex * (float)(colors[nx + 1].r);
				g += sx * (float)(colors[0].g) + ex * (float)(colors[nx + 1].g);
				b += sx * (float)(colors[0].b) + ex * (float)(colors[nx + 1].b);
				a += sx * (float)(colors[0].a) + ex * (float)(colors[nx + 1].a);
				for (ix = 0; ix < nx; ix++) {
					r += (float)(colors[1 + ix].r);
					g += (float)(colors[1 + ix].g);
					b += (float)(colors[1 + ix].b);
					a += (float)(colors[1 + ix].a);
				}
				colors += stride;
			}
			r += ey * (sx * (float)(colors[0].r) + ex * (float)(colors[nx + 1].r));
			g += ey * (sx * (float)(colors[0].g) + ex * (float)(colors[nx + 1].g));
			b += ey * (sx * (float)(colors[0].b) + ex * (float)(colors[nx + 1].b));
			a += ey * (sx * (float)(colors[0].a) + ex * (float)(colors[nx + 1].a));
			for (ix = 0; ix < nx; ix++) {
				r += ey * (float)(colors[1 + ix].r);
				g += ey * (float)(colors[1 + ix].g);
				b += ey * (float)(colors[1 + ix].b);
				a += ey * (float)(colors[1 + ix].a);
			}
			float area = ax * ay;
			_out.r = (sl_uint8)(r / area);
			_out.g = (sl_uint8)(g / area);
			_out.b = (sl_uint8)(b / area);
			_out.a = (sl_uint8)(a / area);
		}
		
		SLIB_INLINE static void getColorAtX(Color& _out, const Color* colors, float fx, const _ImageStretch_FilterParam& px)
		{
			float sx, ex, ax; int nx;
			prepareFilter(fx, px, sx, ex, nx, ax);
			int ix;
			float r = sx * (float)(colors[0].r) + ex * (float)(colors[nx + 1].r);
			float g = sx * (float)(colors[0].g) + ex * (float)(colors[nx + 1].g);
			float b = sx * (float)(colors[0].b) + ex * (float)(colors[nx + 1].b);
			float a = sx * (float)(colors[0].a) + ex * (float)(colors[nx + 1].a);
			for (ix = 0; ix < nx; ix++) {
				r += (float)(colors[1 + ix].r);
				g += (float)(colors[1 + ix].g);
				b += (float)(colors[1 + ix].b);
				a += (float)(colors[1 + ix].a);
			}
			_out.r = (sl_uint8)(r / ax);
			_out.g = (sl_uint8)(g / ax);
			_out.b = (sl_uint8)(b / ax);
			_out.a = (sl_uint8)(a / ax);
		}
		
		SLIB_INLINE static void getColorAtY(Color& _out, const Color* colors, float fy, sl_uint32 stride, const _ImageStretch_FilterParam& py)
		{
			float sy, ey, ay; int ny;
			prepareFilter(fy, py, sy, ey, ny, ay);
			int iy;
			float r = sy * (float)(colors[0].r);
			float g = sy * (float)(colors[0].g);
			float b = sy * (float)(colors[0].b);
			float a = sy * (float)(colors[0].a);
			colors += stride;
			for (iy = 0; iy < ny; iy++) {
				r += (float)(colors[0].r);
				g += (float)(colors[0].g);
				b += (float)(colors[0].b);
				a += (float)(colors[0].a);
				colors += stride;
			}
			r += ey * (float)(colors[0].r);
			g += ey * (float)(colors[0].g);
			b += ey * (float)(colors[0].b);
			a += ey * (float)(colors[0].a);
			_out.r = (sl_uint8)(r / ay);
			_out.g = (sl_uint8)(g / ay);
			_out.b = (sl_uint8)(b / ay);
			_out.a = (sl_uint8)(a / ay);
		}
		
	};

	SLIB_INLINE static void _ImageStretch_Smooth_Prepare(sl_int32 sw, sl_int32 dw, float& step, float& sx_start, sl_int32& dx_start, sl_int32& dx_end, _ImageStretch_FilterParam& param)
	{
		if (sw >= dw) {
			step = (float)(sw) / (float)(dw);
			param.flagBox = sl_true;
			param.filterSize = step;
			sx_start = 0;
			dx_start = 0;
			dx_end = dw;
		} else {
			step = (float)(sw - 1) / (float)(dw - 1);
			param.flagBox = sl_false;
			param.filterSize = 1;
			sx_start = step;
			dx_start = 1;
			dx_end = dw - 1;
		}
	}

	template <class FILTER>
	class _ImageStretch_Smooth
	{
	public:
		template <class BLEND_OP>
		static void stretchX(ImageDesc& dst, const ImageDesc& src)
		{
			sl_int32 sw = src.width;
			sl_int32 dw = dst.width;
			float sx_step, sx_start;
			sl_int32 isx, dx_start, dx_end;
			_ImageStretch_FilterParam px;
			_ImageStretch_Smooth_Prepare(sw, dw, sx_step, sx_start, dx_start, dx_end, px);
			
			sl_int32 dx, dy;
			sl_int32 dh = dst.height;
			float sx;
			
			Color* colorsDst;
			const Color* colorsSrc;
			Color color;
			
			if (dx_start) {
				// left
				colorsDst = dst.colors;
				colorsSrc = src.colors;
				for (dy = 0; dy < dh; dy++) {
					BLEND_OP::blend(*colorsDst, *colorsSrc);
					colorsDst += dst.stride;
					colorsSrc += src.stride;
				}
				// right
				colorsDst = dst.colors + (dw - 1);
				colorsSrc = src.colors + (sw - 1);
				for (dy = 0; dy < dh; dy++) {
					BLEND_OP::blend(*colorsDst, *colorsSrc);
					colorsDst += dst.stride;
					colorsSrc += src.stride;
				}
			}
			
			colorsDst = dst.colors;
			colorsSrc = src.colors;
			for (dy = 0; dy < dh; dy++) {
				for (dx = dx_start; dx < dx_end; dx++) {
					sx = sx_start + (float)(dx) * sx_step;
					isx = (sl_int32)sx;
					FILTER::getColorAtX(color, colorsSrc + isx, sx - (float)isx, px);
					BLEND_OP::blend(colorsDst[dx], color);
				}
				colorsDst += dst.stride;
				colorsSrc += src.stride;
			}
		}
		
		template <class BLEND_OP>
		static void stretchY(ImageDesc& dst, const ImageDesc& src)
		{
			sl_int32 sh = src.height;
			sl_int32 dh = dst.height;
			float sy_step, sy_start;
			sl_int32 isy, dy_start, dy_end;
			_ImageStretch_FilterParam py;
			_ImageStretch_Smooth_Prepare(sh, dh, sy_step, sy_start, dy_start, dy_end, py);
			
			sl_int32 dx, dy;
			sl_int32 dw = dst.width;
			float sy;
			
			Color* colorsDst;
			const Color* colorsSrc;
			Color color;
			
			if (dy_start) {
				// top
				colorsDst = dst.colors;
				colorsSrc = src.colors;
				for (dx = 0; dx < dw; dx++) {
					BLEND_OP::blend(colorsDst[dx], colorsSrc[dx]);
				}
				// bottom
				colorsDst = dst.colors + (sl_uint32)(dh - 1) * dst.stride;
				colorsSrc = src.colors + (sl_uint32)(sh - 1) * src.stride;
				for (dx = 0; dx < dw; dx++) {
					BLEND_OP::blend(colorsDst[dx], colorsSrc[dx]);
				}
			}
			
			colorsDst = dst.colors + dy_start * dst.stride;
			colorsSrc = src.colors;
			for (dy = dy_start; dy < dy_end; dy++) {
				sy = sy_start + (float)(dy) * sy_step;
				isy = (sl_int32)sy;
				float fsy = sy - (float)isy;
				const Color* lineSrc = colorsSrc + (sl_uint32)isy * src.stride;
				for (dx = 0; dx < dw; dx++) {
					FILTER::getColorAtY(color, lineSrc + dx, fsy, src.stride, py);
					BLEND_OP::blend(colorsDst[dx], color);
				}
				colorsDst += dst.stride;
			}
		}
		
		template <class BLEND_OP>
		static void stretchOneRowSrc(ImageDesc& dst, const ImageDesc& src)
		{
			sl_int32 sw = src.width;
			sl_int32 dw = dst.width;
			float sx_step, sx_start;
			sl_int32 isx, dx_start, dx_end;
			_ImageStretch_FilterParam px;
			_ImageStretch_Smooth_Prepare(sw, dw, sx_step, sx_start, dx_start, dx_end, px);
			
			sl_int32 dx, dy;
			sl_int32 dh = dst.height;
			float sx;
			
			Color* colorsDst;
			Color color;
			
			if (dx_start) {
				// left
				colorsDst = dst.colors;
				color = *(src.colors);
				for (dy = 0; dy < dh; dy++) {
					BLEND_OP::blend(*colorsDst, color);
					colorsDst += dst.stride;
				}
				// right
				colorsDst = dst.colors + (dw - 1);
				color = *(src.colors + (sw - 1));
				for (dy = 0; dy < dh; dy++) {
					BLEND_OP::blend(*colorsDst, color);
					colorsDst += dst.stride;
				}
			}
			
			for (dx = dx_start; dx < dx_end; dx++) {
				sx = sx_start + (float)(dx) * sx_step;
				isx = (sl_int32)sx;
				FILTER::getColorAtX(color, src.colors + isx, sx - (float)isx, px);
				colorsDst = dst.colors + dx;
				for (dy = 0; dy < dh; dy++) {
					BLEND_OP::blend(*(colorsDst), color);
					colorsDst += dst.stride;
				}
			}
		}
		
		template <class BLEND_OP>
		static void stretchOneColSrc(ImageDesc& dst, const ImageDesc& src)
		{
			sl_int32 sh = src.height;
			sl_int32 dh = dst.height;
			float sy_step, sy_start;
			sl_int32 isy, dy_start, dy_end;
			_ImageStretch_FilterParam py;
			_ImageStretch_Smooth_Prepare(sh, dh, sy_step, sy_start, dy_start, dy_end, py);
			
			sl_int32 dx, dy;
			sl_int32 dw = dst.width;
			float sy;
			
			Color* colorsDst;
			Color color;
			
			if (dy_start) {
				// top
				colorsDst = dst.colors;
				color = *(src.colors);
				for (dx = 0; dx < dw; dx++) {
					BLEND_OP::blend(colorsDst[dx], color);
				}
				// bottom
				colorsDst = dst.colors + (dh - 1) * dst.stride;
				color = *(src.colors + (sh - 1) * src.stride);
				for (dx = 0; dx < dw; dx++) {
					BLEND_OP::blend(colorsDst[dx], color);
				}
			}
			
			colorsDst = dst.colors + dy_start * dst.stride;
			for (dy = dy_start; dy < dy_end; dy++) {
				sy = sy_start + (float)(dy) * sy_step;
				isy = (sl_int32)sy;
				float fsy = sy - (float)isy;
				FILTER::getColorAtY(color, src.colors + isy * src.stride, fsy, src.stride, py);
				for (dx = 0; dx < dw; dx++) {
					BLEND_OP::blend(colorsDst[dx], color);
				}
				colorsDst += dst.stride;
			}
		}
		
		template <class BLEND_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src)
		{
			if (src.width == dst.width) {
				stretchY<BLEND_OP>(dst, src);
				return;
			}
			if (src.height == dst.height) {
				stretchX<BLEND_OP>(dst, src);
				return;
			}
			if (src.width == 1) {
				stretchOneColSrc<BLEND_OP>(dst, src);
				return;
			}
			if (src.height == 1) {
				stretchOneRowSrc<BLEND_OP>(dst, src);
				return;
			}

			sl_int32 sw = src.width;
			sl_int32 dw = dst.width;
			float sx_step, sx_start;
			sl_int32 isx, dx_start, dx_end;
			_ImageStretch_FilterParam px;
			_ImageStretch_Smooth_Prepare(sw, dw, sx_step, sx_start, dx_start, dx_end, px);
			
			sl_int32 sh = src.height;
			sl_int32 dh = dst.height;
			float sy_step, sy_start;
			sl_int32 isy, dy_start, dy_end;
			_ImageStretch_FilterParam py;
			_ImageStretch_Smooth_Prepare(sh, dh, sy_step, sy_start, dy_start, dy_end, py);
			
			sl_int32 dx, dy;
			float sx, sy;
			
			Color color;
			Color* colorsDst;
			const Color* colorsSrc;
			
			// 4-corners
			if (dx_start && dy_start) {
				colorsDst = dst.colors;
				colorsSrc = src.colors;
				BLEND_OP::blend(colorsDst[0], colorsSrc[0]);
				BLEND_OP::blend(colorsDst[dw - 1], colorsSrc[sw - 1]);
				BLEND_OP::blend(colorsDst[(dh - 1) * dst.stride], colorsSrc[(sh - 1) * src.stride]);
				BLEND_OP::blend(colorsDst[(dh - 1) * dst.stride + dw - 1], colorsSrc[(sh - 1) * src.stride + sw - 1]);
			}
			
			if (dy_start) {
				// top
				colorsDst = dst.colors;
				colorsSrc = src.colors;
				for (dx = dx_start; dx < dx_end; dx++) {
					sx = sx_start + (float)(dx) * sx_step;
					isx = (sl_int32)sx;
					FILTER::getColorAtX(color, colorsSrc + isx, sx - (float)isx, px);
					BLEND_OP::blend(colorsDst[dx], color);
				}
				// bottom
				colorsDst = dst.colors + (dh - 1) * dst.stride;
				colorsSrc = src.colors + (sh - 1) * src.stride;
				for (dx = dx_start; dx < dx_end; dx++) {
					sx = sx_start + (float)(dx) * sx_step;
					isx = (sl_int32)sx;
					FILTER::getColorAtX(color, colorsSrc + isx, sx - (float)isx, px);
					BLEND_OP::blend(colorsDst[dx], color);
				}
			}
			if (dx_start) {
				// left
				colorsDst = dst.colors + dy_start * dst.stride;
				colorsSrc = src.colors;
				for (dy = dy_start; dy < dy_end; dy++) {
					sy = sy_start + (float)(dy) * sy_step;
					isy = (sl_int32)sy;
					FILTER::getColorAtY(color, colorsSrc + isy * src.stride, sy - (float)isy, src.stride, py);
					BLEND_OP::blend(*colorsDst, color);
					colorsDst += dst.stride;
				}
				// right
				colorsDst = dst.colors + (dy_start * dst.stride + dw - 1);
				colorsSrc = src.colors + (sw - 1);
				for (dy = dy_start; dy < dy_end; dy++) {
					sy = sy_start + (float)(dy) * sy_step;
					isy = (sl_int32)sy;
					FILTER::getColorAtY(color, colorsSrc + isy * src.stride, sy - (float)isy, src.stride, py);
					BLEND_OP::blend(*colorsDst, color);
					colorsDst += dst.stride;
				}
			}
			
			colorsDst = dst.colors + dy_start * dst.stride;
			colorsSrc = src.colors;
			for (dy = dy_start; dy < dy_end; dy++) {
				sy = sy_start + (float)(dy) * sy_step;
				isy = (sl_int32)sy;
				float fsy = sy - (float)isy;
				const Color* lineSrc = colorsSrc + isy * src.stride;
				for (dx = dx_start; dx < dx_end; dx++) {
					sx = sx_start + (float)(dx) * sx_step;
					isx = (sl_int32)sx;
					FILTER::getColorAt(color, lineSrc + isx, sx - (float)isx, fsy, src.stride, px, py);
					BLEND_OP::blend(colorsDst[dx], color);
				}
				colorsDst += dst.stride;
			}
		}
	};

	class _ImageStretch_Smooth_IntBox
	{
	public:
		template <class BLEND_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src)
		{
			sl_uint32 dw = dst.width;
			sl_uint32 sw = src.width;
			sl_uint32 fx = sw / dw;
			if (fx == 0) {
				return;
			}
			sl_uint32 dx, sx;
			
			sl_uint32 dh = dst.height;
			sl_uint32 sh = src.height;
			sl_uint32 fy = sh / dh;
			if (fy == 0) {
				return;
			}
			sl_uint32 dy, sy;
			
			Color* colorsDst = dst.colors;
			const Color* colorsSrc = src.colors;
			sl_uint32 ly = fy * src.stride;
			
			sl_uint32 area = fx * fy;
			sl_uint32 n = Math::getMostSignificantBits(area) - 1;
			if (area == (1U << n)) {
				for (dy = 0; dy < dst.height; dy++) {
					const Color* cs = colorsSrc;
					for (dx = 0; dx < dst.width; dx++) {
						sl_uint32 r = 0;
						sl_uint32 g = 0;
						sl_uint32 b = 0;
						sl_uint32 a = 0;
						const Color* c = cs;
						for (sy = 0; sy < fy; sy++) {
							for (sx = 0; sx < fx; sx++) {
								r += c[sx].r;
								g += c[sx].g;
								b += c[sx].b;
								a += c[sx].a;
							}
							c += src.stride;
						}
						cs += fx;
						Color& t = colorsDst[dx];
						t.r = (sl_uint8)(r >> n);
						t.g = (sl_uint8)(g >> n);
						t.b = (sl_uint8)(b >> n);
						t.a = (sl_uint8)(a >> n);
					}
					colorsDst += dst.stride;
					colorsSrc += ly;
				}
			} else {
				for (dy = 0; dy < dst.height; dy++) {
					const Color* cs = colorsSrc;
					for (dx = 0; dx < dst.width; dx++) {
						sl_uint32 r = 0;
						sl_uint32 g = 0;
						sl_uint32 b = 0;
						sl_uint32 a = 0;
						const Color* c = cs;
						for (sy = 0; sy < fy; sy++) {
							for (sx = 0; sx < fx; sx++) {
								r += c[sx].r;
								g += c[sx].g;
								b += c[sx].b;
								a += c[sx].a;
							}
							c += src.stride;
						}
						cs += fx;
						Color& t = colorsDst[dx];
						t.r = (sl_uint8)(r / area);
						t.g = (sl_uint8)(g / area);
						t.b = (sl_uint8)(b / area);
						t.a = (sl_uint8)(a / area);
					}
					colorsDst += dst.stride;
					colorsSrc += ly;
				}
			}
		}
		
	};

	class _ImageBlend_Copy
	{
	public:
		SLIB_INLINE static void blend(Color& dst, const Color& src)
		{
			dst = src;
		}
	};

	class _ImageBlend_SrcAlpha
	{
	public:
		SLIB_INLINE static void blend(Color& dst, const Color& src)
		{
			dst.blend_PA_NPA(src);
		}
	};

	class _ImageStretch
	{
	public:
		template <class STRETCH_OP>
		static void stretch(ImageDesc& dst, const ImageDesc& src, BlendMode blend)
		{
			switch (blend) {
				case BlendMode::Copy:
					STRETCH_OP::template stretch<_ImageBlend_Copy>(dst, src);
					break;
				case BlendMode::SrcAlpha:
					STRETCH_OP::template stretch<_ImageBlend_SrcAlpha>(dst, src);
					break;
			}
		}
	};

	void Image::draw(ImageDesc& dst, const ImageDesc& src, BlendMode blend, StretchMode stretch)
	{
		if (src.width == 0 || src.height == 0 || src.stride == 0 || src.colors == sl_null) {
			return;
		}
		if (dst.width == 0 || dst.height == 0 || dst.stride == 0 || dst.colors == sl_null) {
			return;
		}
		if (src.width == dst.width && src.height == dst.height) {
			_ImageStretch::template stretch<_ImageStretch_Copy>(dst, src, blend);
			return;
		}
		if (src.width == 1 && src.height == 1) {
			_ImageStretch::template stretch<_ImageStretch_FillColor>(dst, src, blend);
			return;
		}
		if (stretch == StretchMode::Nearest) {
			_ImageStretch::template stretch<_ImageStretch_Nearest>(dst, src, blend);
		} else if (stretch == StretchMode::Linear) {
			_ImageStretch::template stretch< _ImageStretch_Smooth<_ImageStretch_Smooth_LinearFilter> >(dst, src, blend);
		} else {
			if (src.width <= dst.width && src.height <= dst.height) {
				_ImageStretch::template stretch< _ImageStretch_Smooth<_ImageStretch_Smooth_LinearFilter> >(dst, src, blend);
				return;
			}
			if (src.width % dst.width == 0 && src.height % dst.height == 0) {
				_ImageStretch::template stretch<_ImageStretch_Smooth_IntBox>(dst, src, blend);
				return;
			}
			_ImageStretch::template stretch< _ImageStretch_Smooth<_ImageStretch_Smooth_BoxFilter> >(dst, src, blend);
		}
	}

	void Image::drawImage(sl_int32 dx, sl_int32 dy, sl_int32 dw, sl_int32 dh
		, const Ref<Image>& src, sl_int32 sx, sl_int32 sy, sl_int32 sw, sl_int32 sh
		, BlendMode blend, StretchMode stretch )
	{
		if (src.isNull()) {
			return;
		}
		if (dw <= 0 || dh <= 0) {
			return;
		}
		if (sw <= 0 || sh <= 0) {
			return;
		}
		sl_int32 dwx = getWidth();
		sl_int32 dhx = getHeight();
		sl_int32 swx = src->getWidth();
		sl_int32 shx = src->getHeight();
		if (dwx <= 0 || dhx <= 0) {
			return;
		}
		if (swx <= 0 || shx <= 0) {
			return;
		}
		sl_int32 dx2 = dx + dw;
		sl_int32 dy2 = dy + dh;
		sl_int32 sx2 = sx + sw;
		sl_int32 sy2 = sy + sh;
		if (dx < 0) {
			sx -= dx * sw / dw;
			dx = 0;
		}
		if (dy < 0) {
			sy -= dy * sh / dh;
			dy = 0;
		}
		if (dx2 > dwx) {
			sx2 -= (dwx - dx2) * sw / dw;
			dx2 = dwx;
		}
		if (dy2 > dhx) {
			sy2 -= (dhx - dy2) * sh / dh;
			dy2 = dhx;
		}
		if (sx < 0) {
			dx -= sx * dw / sw;
			sx = 0;
		}
		if (sy < 0) {
			dy -= sy * dh / sh;
			sy = 0;
		}
		if (sx2 > swx) {
			dx2 -= (swx - sx2) * dw / sw;
			sx2 = swx;
		}
		if (sy2 > shx) {
			dy2 -= (shx - sy2) * dh / sh;
			sy2 = shx;
		}
		ImageDesc descDst, descSrc;
		descDst.colors = getColorsAt(dx, dy);
		descDst.width = dx2 - dx;
		descDst.height = dy2 - dy;
		descDst.stride = getStride();
		descSrc.colors = src->getColorsAt(sx, sy);
		descSrc.width = sx2 - sx;
		descSrc.height = sy2 - sy;
		descSrc.stride = src->getStride();
		draw(descDst, descSrc, blend, stretch);
	}

	void Image::drawImage(const Rectanglei& rectDst
						, const Ref<Image>& src, const Rectanglei& rectSrc
						, BlendMode blend, StretchMode stretch)
	{
		drawImage(rectDst.left, rectDst.top, rectDst.getWidth(), rectDst.getHeight()
				, src, rectSrc.left, rectSrc.top, rectSrc.getWidth(), rectSrc.getHeight()
				, blend, stretch);
	}

	void Image::drawImage(sl_int32 dx, sl_int32 dy, sl_int32 dw, sl_int32 dh
						, const Ref<Image>& src, sl_int32 sx, sl_int32 sy
						, BlendMode blend, StretchMode stretch)
	{
		drawImage(dx, dy, dw, dh, src, sx, sy, dw, dh, blend, stretch);
	}

	Ref<Drawable> Image::subDrawable(sl_real x, sl_real y, sl_real width, sl_real height)
	{
		if (x >= 0 && y >= 0 && width > 0 && height > 0) {
			sl_real _w = (sl_real)(m_desc.width);
			sl_real _h = (sl_real)(m_desc.height);
			if (x + width <= _w && y + height <= _h) {
				return sub((sl_uint32)x, (sl_uint32)y, (sl_uint32)width, (sl_uint32)height);
			}
		}
		return Drawable::subDrawable(x, y, width, height);
	}

	Ref<Image> Image::sub(sl_uint32 x, sl_uint32 y, sl_uint32 width, sl_uint32 height) const
	{
		if (width > 0 && height > 0) {
			if (x < m_desc.width && y < m_desc.height) {
				if (width <= m_desc.width - x && height <= m_desc.height - y) {
					Ref<Image> ret = new Image;
					if (ret.isNotNull()) {
						ret->m_desc.colors = getColorsAt(x, y);
						ret->m_desc.width = width;
						ret->m_desc.height = height;
						ret->m_desc.stride = m_desc.stride;
						ret->m_desc.ref = m_desc.ref;
						return ret;
					}
				}
			}
		}
		return sl_null;
	}

	Ref<Image> Image::scale(sl_uint32 width, sl_uint32 height, StretchMode stretch) const
	{
		if (width > 0 && height > 0) {
			Ref<Image> ret = Image::create(width, height);
			if (ret.isNotNull()) {
				draw(ret->m_desc, m_desc, BlendMode::Copy, stretch);
			}
			return ret;
		}
		return sl_null;
	}

	Ref<Image> Image::scaleToSmall(sl_uint32 requiredWidth, sl_uint32 requiredHeight, StretchMode stretch) const
	{
		sl_uint32 width = SLIB_MIN(requiredWidth, m_desc.width);
		sl_uint32 height = SLIB_MIN(requiredHeight, m_desc.height);
		if (width > 0 && height > 0) {
			Ref<Image> ret = Image::create(width, height);
			if (ret.isNotNull()) {
				draw(ret->m_desc, m_desc, BlendMode::Copy, stretch);
			}
			return ret;
		}
		return sl_null;
	}

	ImageFileType Image::getFileType(const void* _mem, sl_size size)
	{
		sl_uint8* mem = (sl_uint8*)_mem;
		if (size > 4 && mem[0] == 0xFF && mem[1] == 0xD8) {
			return ImageFileType::JPEG;
		}
		if (size > 0x08 && mem[0] == 0x89 && mem[1] == 0x50 && mem[2] == 0x4E && mem[3] == 0x47 && mem[4] == 0x0D && mem[5] == 0x0A && mem[6] == 0x1A && mem[7] == 0x0A) {
			return ImageFileType::PNG;
		}
		if (size > 12 && mem[0] == 'B' && mem[1] == 'M') {
			return ImageFileType::BMP;
		}
		if (size > 4 && mem[0] == 'D' && mem[1] == 'D' && mem[2] == 'S' && mem[3] == ' ') {
			return ImageFileType::DDS;
		}
		return ImageFileType::Unknown;
	}

	ImageFileType Image::getFileType(Memory mem)
	{
		return getFileType(mem.getData(), mem.getSize());
	}

	Ref<Image> Image::loadFromMemory(const void* _mem, sl_size size, sl_uint32 width, sl_uint32 height)
	{
		Ref<Image> ret;
		sl_uint8* mem = (sl_uint8*)_mem;
		ImageFileType type = getFileType(mem, size);
		do {
#ifdef SLIB_GRAPHICS_IMAGE_SUPPORT_JPEG
			if (type == ImageFileType::JPEG) {
				ret = loadFromJPEG(mem, size);
				break;
			}
#endif
#ifdef SLIB_GRAPHICS_IMAGE_SUPPORT_PNG
			if (type == ImageFileType::PNG) {
				ret = loadFromPNG(mem, size);
				break;
			}
#endif
#ifdef SLIB_GRAPHICS_IMAGE_USE_STB
			ret = Image_STB::loadImage(mem, size);
			if (ret.isNotNull()) {
				break;
			}
#endif
		} while (0);
		if (ret.isNotNull()) {
			if (width == 0 || height == 0) {
				return ret;
			}
			if (ret->getWidth() != width && ret->getHeight() != height) {
				ret = ret->scale(width, height);
			}
			return ret;
		}
		return sl_null;
	}

	Ref<Image> Image::loadFromMemory(Memory mem, sl_uint32 width, sl_uint32 height)
	{
		return loadFromMemory(mem.getData(), mem.getSize(), width, height);
	}

	Ref<Image> Image::loadFromFile(const String& filePath, sl_uint32 width, sl_uint32 height)
	{
		Ref<Image> ret;
		Memory mem = File::readAllBytes(filePath);
		if (mem.isNotEmpty()) {
			ret = loadFromMemory(mem, width, height);
		}
		return ret;
	}

	Ref<Image> Image::loadFromAsset(const String& path, sl_uint32 width, sl_uint32 height)
	{
		Ref<Image> ret;
		Memory mem = Assets::readAllBytes(path);
		if (mem.isNotEmpty()) {
			ret = loadFromMemory(mem, width, height);
		}
		return ret;
	}

	Ref<Drawable> Image::getDrawableCache()
	{
		Ref<Drawable> drawableCached = m_drawableCached;
		if (drawableCached.isNotNull()) {
			return drawableCached;
		}
		drawableCached = PlatformDrawable::create(this);
		if (drawableCached.isNotNull()) {
			m_drawableCached = drawableCached;
		}
		return drawableCached;
	}

	void Image::onDraw(Canvas* canvas, const Rectangle& rectDst, const Rectangle& rectSrc, const DrawParam& param)
	{
		Ref<Drawable> drawableCached = getDrawableCache();
		if (drawableCached.isNotNull()) {
			drawableCached->onDraw(canvas, rectDst, rectSrc, param);
		}
	}

	void Image::onDrawAll(Canvas* canvas, const Rectangle& rectDst, const DrawParam& param)
	{
		Ref<Drawable> drawableCached = getDrawableCache();
		if (drawableCached.isNotNull()) {
			drawableCached->onDrawAll(canvas, rectDst, param);
		}
	}

}
