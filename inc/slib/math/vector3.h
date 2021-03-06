/*
 *  Copyright (c) 2008-2017 SLIBIO. All Rights Reserved.
 *
 *  This file is part of the SLib.io project.
 *
 *  This Source Code Form is subject to the terms of the Mozilla Public
 *  License, v. 2.0. If a copy of the MPL was not distributed with this
 *  file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef CHECKHEADER_SLIB_MATH_VECTOR3
#define CHECKHEADER_SLIB_MATH_VECTOR3

#include "definition.h"

#include "vector2.h"

#include "../core/interpolation.h"

namespace slib
{
	
	template <class T, class FT = T>
	class SLIB_EXPORT Vector3T
	{
	public:
		T x;
		T y;
		T z;
	
	public:
		SLIB_INLINE Vector3T() = default;

		constexpr Vector3T(const Vector3T<T, FT>& other) : x(other.x), y(other.y), z(other.z) {}
	
		template <class O, class FO>
		constexpr Vector3T(const Vector3T<O, FO>& other) : x((T)(other.x)), y((T)(other.y)), z((T)(other.z)) {}
	
		constexpr Vector3T(T _x, T _y, T _z) : x(_x), y(_y), z(_z) {}
	
	public:
		static const Vector3T<T, FT>& zero();

		static const Vector3T<T, FT>& fromArray(const T arr[3]);

		static Vector3T<T, FT>& fromArray(T arr[3]);

		static Vector3T<T, FT> fromLocation(const Vector2T<T>& v);

		static Vector3T<T, FT> fromDirection(const Vector2T<T>& v);

		T dot(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT> cross(const Vector3T<T, FT>& other) const;

		T getLength2p() const;

		FT getLength() const;

		T getLength2p(const Vector3T<T, FT>& other) const;

		FT getLength(const Vector3T<T, FT>& other) const;

		void normalize();

		Vector3T<T, FT> getNormalized();

		FT getCosBetween(const Vector3T<T, FT>& other) const;

		FT getAngleBetween(const Vector3T<T, FT>& other) const;

		sl_bool equals(const Vector3T<T, FT>& other) const;

		sl_bool isAlmostEqual(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT> lerp(const Vector3T<T, FT>& target, float factor) const;
	
	public:
		Vector3T<T, FT>& operator=(const Vector3T<T, FT>& other) = default;

		template <class O, class FO>
		Vector3T<T, FT>& operator=(const Vector3T<O, FO>& other);

		Vector3T<T, FT> operator+(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT>& operator+=(const Vector3T<T, FT>& other);

		Vector3T<T, FT> operator-(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT>& operator-=(const Vector3T<T, FT>& other);

		Vector3T<T, FT> operator*(T f) const;

		Vector3T<T, FT>& operator*=(T f);

		Vector3T<T, FT> operator*(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT>& operator*=(const Vector3T<T, FT>& other);

		Vector3T<T, FT> operator/(T f) const;

		Vector3T<T, FT>& operator/=(T f);

		Vector3T<T, FT> operator/(const Vector3T<T, FT>& other) const;

		Vector3T<T, FT>& operator/(const Vector3T<T, FT>& other);

		Vector3T<T, FT> operator-() const;

		sl_bool operator==(const Vector3T<T, FT>& other) const;

		sl_bool operator!=(const Vector3T<T, FT>& other) const;

	private:
		static T _zero[3];
	
	};
	
	extern template class Vector3T<float>;
	extern template class Vector3T<double>;
	extern template class Vector3T<sl_int32, float>;
	extern template class Vector3T<sl_int64, double>;
	typedef Vector3T<sl_real> Vector3;
	typedef Vector3T<float> Vector3f;
	typedef Vector3T<double> Vector3lf;
	typedef Vector3T<sl_int32, float> Vector3i;
	typedef Vector3T<sl_int64, double> Vector3li;
	
	template <class T, class FT>
	Vector3T<T, FT> operator*(T f, const Vector3T<T, FT>& v);
	
	template <class T, class FT>
	Vector3T<T, FT> operator/(T f, const Vector3T<T, FT>& v);
	
	template <class T, class FT>
	class Interpolation< Vector3T<T, FT> >
	{
	public:
		static Vector3T<T, FT> interpolate(const Vector3T<T, FT>& a, const Vector3T<T, FT>& b, float factor);
	};

}

#include "detail/vector3.h"

#endif
