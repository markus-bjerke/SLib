#ifndef CHECKHEADER_SLIB_CORE_ARRAY2D
#define CHECKHEADER_SLIB_CORE_ARRAY2D

#include "definition.h"
#include "reference.h"
#include "array.h"

SLIB_NAMESPACE_BEGIN

template <class T>
class SLIB_EXPORT ArrayInfo2D
{
public:
	T* data;
	sl_size width;
	sl_size height;
	sl_reg stride;
	Ref<Referable> refer;

public:
	SLIB_INLINE T& item(sl_reg x, sl_reg y) const
	{
		return data[y * stride + x];
	}

};

template <class T>
class SLIB_EXPORT CArray2D : public Referable
{
	SLIB_DECLARE_ROOT_OBJECT(CArray2D)
public:
	typedef CArray2D<T> _Type;

protected:
	T* m_data;
	sl_size m_width;
	sl_size m_height;
	sl_reg m_stride;
	sl_bool m_flagStatic;
	Ref<Referable> m_refer;
	
protected:
	SLIB_INLINE CArray2D()
	{
	}

public:
	~CArray2D()
	{
		if (! m_flagStatic) {
			T* rp = m_data;
			if (rp) {				
				for (sl_size i = 0; i < m_height; i++) {
					T* p = rp;
					for (sl_size j = 0; j < m_width; j++) {
						p->~T();
						p ++;
					}
					rp += m_stride;
				}
				Base::freeMemory(m_data);
			}
		}
	}
	
public:
	static _Type* create(sl_size width, sl_size height, const T* data = sl_null, sl_reg strideSrc = 0)
	{
		if (width == 0 || height == 0) {
			return sl_null;
		}
		if (strideSrc == 0) {
			strideSrc = width;
		}
		sl_size size = width * height * sizeof(T);
		T* dataNew = (T*)(Base::createMemory(size));
		if (dataNew == sl_null) {
			return sl_null;
		}
		_Type* ret = new _Type;
		if (ret) {
			if (data) {
				T* rp = dataNew;
				const T* rq = data;
				for (sl_size i = 0; i < height; i++) {
					T* p = rp;
					const T* q = rq;
					for (sl_size j = 0; j < width; j++) {
						new (p) T(*q);
						p ++;
						q ++;
					}
					rp += width;
					rq += strideSrc;
				}
			} else {
				T* rp = dataNew;
				for (sl_size i = 0; i < height; i++) {
					T* p = rp;
					for (sl_size j = 0; j < width; j++) {
						new (p) T();
						p ++;
					}
					rp += width;
				}
			}
			ret->m_flagStatic = sl_false;
			ret->m_data = dataNew;
			ret->m_width = width;
			ret->m_height = height;
			ret->m_stride = width;
		} else {
			Base::freeMemory(dataNew);
		}
		return ret;
	}
	
	static _Type* createStatic(const T* data, sl_size width, sl_size height, sl_reg stride, const Ref<Referable>& refer) {
		if (data == sl_null || width == 0 || height == 0) {
			return sl_null;
		}
		if (stride == 0) {
			stride = width;
		}
		_Type* ret = new _Type();
		if (ret) {
			ret->m_flagStatic = sl_true;
			ret->m_data = (T*)data;
			ret->m_width = width;
			ret->m_height = height;
			ret->m_stride = stride;
			ret->m_refer = refer;
		}
		return ret;
	}
	
public:
	SLIB_INLINE T* getData() const
	{
		return m_data;
	}

	SLIB_INLINE T* data() const
	{
		return m_data;
	}

	SLIB_INLINE sl_size getWidth() const
	{
		return m_width;
	}

	SLIB_INLINE sl_size width() const
	{
		return m_width;
	}

	SLIB_INLINE sl_size getHeight() const
	{
		return m_height;
	}

	SLIB_INLINE sl_size height() const
	{
		return m_height;
	}

	SLIB_INLINE sl_reg getStride() const
	{
		return m_stride;
	}

	SLIB_INLINE sl_reg stride() const
	{
		return m_stride;
	}

	SLIB_INLINE sl_bool isStatic() const
	{
		return m_flagStatic;
	}

	SLIB_INLINE const Ref<Referable>& getRefer() const
	{
		return m_refer;
	}

	SLIB_INLINE T& item(sl_reg x, sl_reg y) const
	{
		return m_data[y * m_stride + x];
	}

	SLIB_INLINE sl_bool getItem(sl_reg x, sl_reg y, T* _out = sl_null) const
	{
		if (x >= 0 && x < (sl_reg)m_width && y >= 0 && y < (sl_reg)m_height) {
			if (_out) {
				*_out = m_data[y * m_stride + x];
			}
			return sl_true;
		}
		return sl_false;
	}

	SLIB_INLINE T getItemValue(sl_reg x, sl_reg y, const T& def) const
	{
		if (x >= 0 && x < (sl_reg)m_width && y >= 0 && y < (sl_reg)m_height) {
			return m_data[y * m_stride + x];
		}
		return def;
	}

	SLIB_INLINE sl_bool setItem(sl_reg x, sl_reg y, const T& value) const
	{
		if (x >= 0 && x < (sl_reg)m_width && y >= 0 && y < (sl_reg)m_height) {
			m_data[y * m_stride + x] = value;
			return sl_true;
		}
		return sl_false;
	}

public:
	_Type* sub(sl_size x, sl_size y, sl_size width, sl_size height) const
	{
		sl_size widthParent = m_width;
		sl_size heightParent = m_height;
		if (x < widthParent && y < heightParent) {
			if (width > widthParent - x) {
				width = widthParent - x;
			}
			if (height > heightParent - y) {
				height = heightParent - y;
			}
			if (width > 0 && height > 0) {
				if (m_flagStatic) {
					return createStatic(m_data + (x + (sl_reg)y * m_stride), width, height, m_stride, m_refer);
				} else {
					return createStatic(m_data + (x + (sl_reg)y * m_stride), width, height, m_stride, this);
				}
			}
		}
		return sl_null;
	}

	template <class _T>
	sl_size read(sl_size x, sl_size y, sl_size width, sl_size height, _T* dataDst, sl_reg strideDst = 0) const
	{
		T* pSrc = m_data;
		_T* pDst = dataDst;
		if (pDst == sl_null || pSrc == sl_null) {
			return 0;
		}
		if (strideDst == 0) {
			strideDst = width;
		}
		sl_size widthSrc = m_width;
		sl_size heightSrc = m_height;
		if (x >= widthSrc || y >= heightSrc) {
			return 0;
		}
		if (width > widthSrc - x) {
			return 0;
		}
		if (height > heightSrc - y) {
			return 0;
		}
		sl_reg strideSrc = m_stride;
		T* rp = pSrc + (sl_reg)x + (sl_reg)y * strideSrc;
		_T* rq = pDst;
		for (sl_size i = 0; i < height; i++) {
			T* p = rp;
			_T* q = rq;
			for (sl_size j = 0; j < width; j++) {
				*(q) = *(p);
				p++;
				q++;
			}
			rp += strideSrc;
			rq += strideDst;
		}
		return width * height;
	}

	template <class _T>
	sl_size write(sl_size x, sl_size y, sl_size width, sl_size height, const _T* dataSrc, sl_reg strideSrc = 0) const
	{
		T* pDst = m_data
		const _T* pSrc = dataSrc;
		if (pDst == sl_null || pSrc == sl_null) {
			return 0;
		}
		if (strideSrc == 0) {
			strideSrc = width;
		}
		sl_size widthDst = m_width;
		sl_size heightDst = m_height;
		if (x >= widthDst || y >= heightDst) {
			return 0;
		}
		if (width > widthDst - x) {
			return 0;
		}
		if (height > heightDst - y) {
			return 0;
		}
		sl_reg strideDst = m_stride;
		T* rp = pDst + ((sl_reg)x + (sl_reg)y * strideDst);
		const _T* rq = pSrc;
		for (sl_size i = 0; i < height; i++) {
			T* p = rp;
			_T* q = rq;
			for (sl_size j = 0; j < width; j++) {
				*(p) = *(q);
				p++;
				q++;
			}
			rp += strideDst;
			rq += strideSrc;
		}
		return width * height;
	}

	template <class _T>
	sl_size copy(sl_size xTarget, sl_size yTarget, const CArray2D<_T>* source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		_T* pSrc = source->getData();
		if (pSrc == sl_null) {
			return 0;
		}
		sl_size widthSrc = source->getWidth();
		sl_size heightSrc = source->getHeight();
		if (xSource >= widthSrc || ySource >= heightSrc) {
			return 0;
		}
		if (width > widthSrc - xSource) {
			width = widthSrc - xSource;
		}
		if (height > heightSrc - ySource) {
			height = heightSrc - ySource;
		}
		sl_reg strideSrc = source->getStride();
		return write<_T>(xTarget, yTarget, width, height, pSrc + ((sl_reg)xSource + (sl_reg)ySource * strideSrc), strideSrc);
	}

	template <class _T>
	SLIB_INLINE sl_size copy(const CArray2D<_T>* source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		return copy<_T>(0, 0, source, xSource, ySource, width, height);
	}

	SLIB_INLINE _Type* duplicate() const
	{
		return create(m_width, m_height, m_data, m_stride);
	}
};

template <class T>
class SafeArray2D;

/** auto-referencing object **/
template <class T>
class SLIB_EXPORT Array2D
{
	typedef Array2D<T> _Type;
	typedef CArray2D<T> _Obj;
	typedef Ref<_Obj> _Ref;
	SLIB_DECLARE_OBJECT_TYPE_FROM(_Type, _Obj)
	SLIB_DECLARE_OBJECT_WRAPPER(Array2D, _Type, _Obj, _Ref)
	
public:
	SLIB_INLINE Array2D(sl_size width, sl_size height, const T* dataIn = sl_null, sl_reg strideIn = 0) : m_object(_Obj::create(width, height, dataIn, strideIn))
	{
	}

	SLIB_INLINE Array2D(const T* dataIn, sl_size width, sl_size height, sl_reg strideIn, const Ref<Referable>& refer) : m_object(_Obj::createStatic(strideIn, width, height, dataIn, refer))
	{
	}

public:
	Array2D(const SafeArray2D<T>& other);

	_Type& operator=(const SafeArray2D<T>& other);

	SLIB_INLINE _Obj* getObject() const
	{
		return m_object.get();
	}

public:
	SLIB_INLINE static _Type create(sl_size width, sl_size height, const T* dataIn = sl_null, sl_reg strideIn = 0)
	{
		return _Type(width, height, dataIn, strideIn);
	}

	SLIB_INLINE static Array2D<T> createStatic(const T* data, sl_size width, sl_size height, sl_reg stride = 0)
	{
		return _Type(data, width, height, stride, Ref<Referable>::null());
	}

	SLIB_INLINE static Array2D<T> createStatic(const T* data, sl_size width, sl_size height, const Ref<Referable>& refer)
	{
		return _Type(data, width, height, width, refer);
	}

	SLIB_INLINE static Array2D<T> createStatic(const T* data, sl_size width, sl_size height, sl_reg stride, const Ref<Referable>& refer)
	{
		return _Type(data, width, height, stride, refer);
	}

public:
	SLIB_INLINE T* getData() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->data();
		}
		return sl_null;
	}

	SLIB_INLINE T* data() const
	{
		return getData();
	}

	SLIB_INLINE sl_size getWidth() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->width();
		}
		return 0;
	}

	SLIB_INLINE sl_reg width() const
	{
		return getWidth();
	}

	SLIB_INLINE sl_size getHeight() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->height();
		}
		return 0;
	}

	SLIB_INLINE sl_size height() const
	{
		return getHeight();
	}

	SLIB_INLINE sl_reg getStride() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->stride();
		}
		return 0;
	}

	SLIB_INLINE sl_reg stride() const
	{
		return getStride();
	}

	SLIB_INLINE sl_bool isEmpty() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->width() == 0 || obj->height() == 0;
		}
		return sl_true;
	}

	SLIB_INLINE sl_bool isNotEmpty() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->width() != 0 && obj->height() != 0;
		}
		return sl_false;
	}

	SLIB_INLINE T& item(sl_reg x, sl_reg y) const
	{
		return (m_object->data())[y * m_object->stride() + x];
	}

	SLIB_INLINE sl_bool getItem(sl_reg x, sl_reg y, T* _out = sl_null) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->getItem(x, y, _out);
		}
		return sl_false;
	}

	SLIB_INLINE T getItemValue(sl_reg x, sl_reg y, const T& def) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->getItemValue(x, y, def);
		}
		return def;
	}

	SLIB_INLINE sl_bool setItem(sl_reg x, sl_reg y, const T& value) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->setItem(x, y, value);
		}
		return sl_false;
	}

public:
	SLIB_INLINE _Type sub(sl_size x, sl_size y, sl_size width, sl_size height) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->sub(x, y, width, height);
		}
		return _Type::null();
	}

	template <class _T>
	SLIB_INLINE sl_size read(sl_size x, sl_size y, sl_size width, sl_size height, _T* dataDst, sl_reg strideDst = 0) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->read(x, y, width, height, dataDst, strideDst);
		}
		return 0;
	}
	
	template <class _T>
	SLIB_INLINE sl_size write(sl_size x, sl_size y, sl_size width, sl_size height, const _T* dataSrc, sl_reg strideSrc = 0) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->write(x, y, width, height, dataSrc, strideSrc);
		}
		return 0;
	}
	
	template <class _T>
	SLIB_INLINE sl_size copy(sl_size xTarget, sl_size yTarget, const Array2D<_T>& source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->copy(xTarget, yTarget, source.getObject(), xSource, ySource, width, height);
		}
		return 0;
	}
	
	template <class _T>
	SLIB_INLINE sl_size copy(const Array2D<_T>& source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		return copy<_T>(0, 0, source, xSource, ySource, width, height);
	}

	SLIB_INLINE _Type duplicate() const
	{
		_Obj* obj = m_object.get();
		if (obj) {
			return obj->duplicate();
		}
		return _Type::null();
	}

	sl_bool getInfo(ArrayInfo2D<T>& info)
	{
		_Obj* obj = m_object.get();
		if (obj) {
			info.data = obj->data();
			info.width = obj->width();
			info.height = obj->height();
			info.stride = obj->stride();
			if (obj->isStatic()) {
				info.refer = obj->getRefer();
			} else {
				info.refer = obj;
			}
			return sl_true;
		} else {
			info.data = sl_null;
			info.width = 0;
			info.height = 0;
			info.stride = 0;
			info.refer.setNull();
			return sl_false;
		}
	}
	
};

/** auto-referencing object **/
template <class T>
class SLIB_EXPORT SafeArray2D
{
	typedef SafeArray2D<T> _Type;
	typedef CArray2D<T> _Obj;
	typedef SafeRef<_Obj> _Ref;
	typedef Array2D<T> _LocalType;
	typedef Ref<_Obj> _LocalRef;
	SLIB_DECLARE_OBJECT_TYPE_FROM(_Type, _Obj)
	SLIB_DECLARE_OBJECT_WRAPPER(SafeArray2D, _Type, _Obj, _Ref)

public:
	SLIB_INLINE SafeArray2D(sl_size count) : m_object(_Obj::create(count))
	{
	}

	SLIB_INLINE SafeArray2D(const T* data, sl_size count) : m_object(_Obj::create(data, count))
	{
	}

	SLIB_INLINE SafeArray2D(const T* data, sl_size count, const Ref<Referable>& refer) : m_object(_Obj::create(data, count, refer))
	{
	}

public:
	SLIB_INLINE SafeArray2D(const _LocalType& other) : m_object(other.getReference())
	{
	}

	SLIB_INLINE _Type& operator=(const _LocalType& other)
	{
		m_object = other.getReference();
		return *this;
	}

public:
	SLIB_INLINE sl_size getWidth() const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->width();
		}
		return 0;
	}

	SLIB_INLINE sl_reg width() const
	{
		return getWidth();
	}

	SLIB_INLINE sl_size getHeight() const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->height();
		}
		return 0;
	}

	SLIB_INLINE sl_size height() const
	{
		return getHeight();
	}

	SLIB_INLINE sl_bool isEmpty() const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->width() == 0 || obj->height() == 0;
		}
		return sl_true;
	}

	SLIB_INLINE sl_bool isNotEmpty() const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->width() != 0 && obj->height() != 0;
		}
		return sl_false;
	}

	SLIB_INLINE sl_bool getItem(sl_reg x, sl_reg y, T* _out = sl_null) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->getItem(x, y, _out);
		}
		return sl_false;
	}

	SLIB_INLINE T getItemValue(sl_reg x, sl_reg y, const T& def) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->getItemValue(x, y, def);
		}
		return def;
	}

	SLIB_INLINE sl_bool setItem(sl_reg x, sl_reg y, const T& value) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->setItem(x, y, value);
		}
		return sl_false;
	}

public:
	SLIB_INLINE _LocalType sub(sl_size x, sl_size y, sl_size width, sl_size height) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->sub(x, y, width, height);
		}
		return _LocalType::null();
	}

	template <class _T>
	SLIB_INLINE sl_size read(sl_size x, sl_size y, sl_size width, sl_size height, _T* dataDst, sl_reg strideDst = 0) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->read(x, y, width, height, dataDst, strideDst);
		}
		return 0;
	}

	template <class _T>
	SLIB_INLINE sl_size write(sl_size x, sl_size y, sl_size width, sl_size height, const _T* dataSrc, sl_reg strideSrc = 0) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->write(x, y, width, height, dataSrc, strideSrc);
		}
		return 0;
	}

	template <class _T>
	SLIB_INLINE sl_size copy(sl_size xTarget, sl_size yTarget, const Array2D<_T>& source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->copy(xTarget, yTarget, source.getObject(), xSource, ySource, width, height);
		}
		return 0;
	}

	template <class _T>
	SLIB_INLINE sl_size copy(const Array2D<_T>& source, sl_size xSource = 0, sl_size ySource = 0, sl_size width = SLIB_SIZE_MAX, sl_size height = SLIB_SIZE_MAX) const
	{
		return copy<_T>(0, 0, source, xSource, ySource, width, height);
	}

	SLIB_INLINE _LocalType duplicate() const
	{
		_LocalRef obj(m_object);
		if (obj.isNotNull()) {
			return obj->duplicate();
		}
		return _LocalType::null();
	}

	sl_bool getInfo(ArrayInfo2D<T>& info);
};

template <class T>
SLIB_INLINE Array2D<T>::Array2D(const SafeArray2D<T>& other) : m_object(other.getReference())
{
}

template <class T>
SLIB_INLINE Array2D<T>& Array2D<T>::operator=(const SafeArray2D<T>& other)
{
	m_object = other.getReference();
	return *this;
}

template <class T>
SLIB_INLINE sl_bool SafeArray2D<T>::getInfo(ArrayInfo2D<T>& info)
{
	_LocalType obj(*this);
	return obj.getInfo(info);
}

SLIB_NAMESPACE_END

#endif
