#pragma once

#define _USE_MATH_DEFINES
#include <cmath>
#include <cstdint>

template< typename T >
struct matrix_t
{
	T data[16];
};

template< typename T >
struct vector3_t
{
	T x, y, z;
};

template< typename T >
class Quat
{
  public:
	const T* data() const { return m_value; }

  private:
	T m_value[4];

  public:
	Quat(T a, T b, T c, T d) : m_value{ b, c, d, a } {}

	Quat(T angle, bool inRadians, vector3_t< T > vect)
	{
		T normVect = std::sqrt(vect.x * vect.x + vect.y * vect.y + vect.z * vect.z);
		vect.x /= normVect;
		vect.y /= normVect;
		vect.z /= normVect;
		T halfAlpha = (T)((inRadians ? angle : angle * M_PI / 180) / 2);
		T sinHalfAlpha = std::sin(halfAlpha);
		m_value[3] = std::cos(halfAlpha);
		m_value[0] = vect.x * sinHalfAlpha;
		m_value[1] = vect.y * sinHalfAlpha;
		m_value[2] = vect.z * sinHalfAlpha;
	}

	Quat() : Quat(0, 0, 0, 0) {}

	Quat< T > operator+(const Quat< T >& that) const
	{
		Quat< T > res = *this;
		res += that;
		return res;
	}

	Quat< T >& operator+=(const Quat< T >& that)
	{
		const T* rightValue = that.m_value;
		for (uint8_t i = 0; i < 4; ++i)
			m_value[i] += rightValue[i];
		return *this;
	}

	Quat< T > operator-(const Quat< T >& that) const
	{
		Quat< T > res = *this;
		res -= that;
		return res;
	}

	Quat< T >& operator-=(const Quat< T >& that)
	{
		const T* rightValue = that.m_value;
		for (uint8_t i = 0; i < 4; ++i)
			m_value[i] -= rightValue[i];
		return *this;
	}

	Quat< T > operator*(const Quat< T >& that) const
	{
		const T* thatValue = that.m_value;
		T a1 = m_value[3], b1 = m_value[0], c1 = m_value[1], d1 = m_value[2];
		T a2 = thatValue[3], b2 = thatValue[0], c2 = thatValue[1], d2 = thatValue[2];
		Quat< T > res(
			a1 * a2 - b1 * b2 - c1 * c2 - d1 * d2,
			a1 * b2 + a2 * b1 + c1 * d2 - c2 * d1,
			a1 * c2 + a2 * c1 + d1 * b2 - d2 * b1,
			a1 * d2 + a2 * d1 + b1 * c2 - b2 * c1);
		return res;
	}

	Quat< T > operator~() const
	{
		Quat< T > res(m_value[3], -m_value[0], -m_value[1], -m_value[2]);
		return res;
	}

	bool operator==(const Quat< T >& that) const
	{
		const T* thatValue = that.m_value;
		for (uint8_t i = 0; i < 4; ++i)
			if (m_value[i] != thatValue[i])
				return false;
		return true;
	}

	bool operator!=(const Quat< T >& that) const { return !(*this == that); }

	explicit operator T() const
	{
		T val = 0;
		for (uint8_t i = 0; i < 4; ++i)
			val += m_value[i] * m_value[i];
		return std::sqrt(val);
	}

	Quat< T > operator*(T scalar) const
	{
		Quat< T > scalarQuat(scalar, 0, 0, 0);
		return *this * scalarQuat;
	}

	Quat< T > operator*(const vector3_t< T >& vect) const
	{
		Quat< T > vectQuat(0, vect.x, vect.y, vect.z);
		return *this * vectQuat;
	}

	matrix_t< T > rotation_matrix() const
	{
		Quat< T > normalized = *this * (1 / T(*this));
		T* normValue = normalized.m_value;
		T b = normValue[0], c = normValue[1], d = normValue[2], a = normValue[3];
		T bb = b * b, cc = c * c, dd = d * d, aa = a * a, bc = b * c, da = d * a, bd = b * d, ca = c * a, cd = c * d, ba = b * a;
		matrix_t< T > res;
		T* data = res.data;
		data[0] = 1 - 2 * cc - 2 * dd;
		data[1] = 2 * bc + 2 * da;
		data[2] = 2 * bd - 2 * ca;

		data[4] = 2 * bc - 2 * da;
		data[5] = 1 - 2 * bb - 2 * dd;
		data[6] = 2 * cd + 2 * ba;

		data[8] = 2 * bd + 2 * ca;
		data[9] = 2 * cd - 2 * ba;
		data[10] = 1 - 2 * bb - 2 * cc;

		data[3] = data[7] = data[11] = data[12] = data[13] = data[14] = 0;
		data[15] = 1;
		return res;
	}

	matrix_t< T > matrix() const
	{
		matrix_t< T > res;
		T* data = res.data;
		data[0] = data[5] = data[10] = data[15] = m_value[3];
		data[4] = data[14] = m_value[0];
		data[1] = data[11] = -m_value[0];
		data[7] = data[8] = m_value[1];
		data[2] = data[13] = -m_value[1];
		data[9] = data[12] = m_value[2];
		data[3] = data[6] = -m_value[2];
		return res;
	}

	T angle(bool inRadians = true) const
	{
		return (T)(std::acos(m_value[3] / T(*this)) * 2 * (inRadians ? 1 : 180 / M_PI));
	}

	vector3_t< T > apply(const vector3_t< T > vect) const
	{
		Quat< T > vectQuat(0, vect.x, vect.y, vect.z);
		Quat< T > normalized = *this * (1 / T(*this));
		Quat< T > resQuat = normalized * vectQuat * ~normalized;
		T* resVal = resQuat.m_value;
		vector3_t< T > res(resVal[0], resVal[1], resVal[2]);
		return res;
	}
};
