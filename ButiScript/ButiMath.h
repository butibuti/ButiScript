#pragma once
#ifndef BUTI_MATH_H
#define BUTI_MATH_H
namespace ButiEngine {
	static const float BM_PI = 3.141592654f;
	static const float BM_2PI = 6.283185307f;
	static const float BM_1DIVPI = 0.318309886f;
	static const float BM_1DIV2PI = 0.159154943f;
	static const float BM_PIDIV2 = 1.570796327f;
	static const float BM_PIDIV4 = 0.785398163f;
	static const char BM_CHAR_MAX = 127;
	static const short int BM_SHORT_MAX = 32767;
	static const int BM_INT_MAX = 2147483647;
	static const unsigned char BM_UCHAR_MAX = 256;
	static const unsigned short int BM_USHORT_MAX = 65535;
	static const unsigned int BM_UINT_MAX = 4294967295;


	namespace MathHelper {

		static void SinCos(float& ref_Sin, float& ref_Cos, const float  Value)
		{
			float quotient = BM_1DIV2PI * Value;
			if (Value >= 0.0f)
			{
				quotient = static_cast<float>(static_cast<int>(quotient + 0.5f));
			}
			else
			{
				quotient = static_cast<float>(static_cast<int>(quotient - 0.5f));
			}
			float y = Value - BM_2PI * quotient;

			float sign;
			if (y > BM_PIDIV2)
			{
				y = BM_PI - y;
				sign = -1.0f;
			}
			else if (y < -BM_PIDIV2)
			{
				y = -BM_PI - y;
				sign = -1.0f;
			}
			else
			{
				sign = +1.0f;
			}

			float y2 = y * y;

			ref_Sin = (((((-2.3889859e-08f * y2 + 2.7525562e-06f) * y2 - 0.00019840874f) * y2 + 0.0083333310f) * y2 - 0.16666667f) * y2 + 1.0f) * y;

			float p = ((((-2.6051615e-07f * y2 + 2.4760495e-05f) * y2 - 0.0013888378f) * y2 + 0.041666638f) * y2 - 0.5f) * y2 + 1.0f;
			ref_Cos = sign * p;
		}
		static double Ceil(const double dSrc,const int iLen)
		{
			double	dRet;

			dRet = dSrc * pow(10.0, iLen);
			dRet = (double)(int)(dRet + 0.9);

			return dRet * pow(10.0, -iLen);
		}
		static double Floor(const double dSrc,const int iLen)
		{
			double dRet;

			dRet = dSrc * pow(10.0, iLen);
			dRet = (double)(int)(dRet);

			return dRet * pow(10.0, -iLen);
		}
		static double Round(const double dSrc,const int iLen)
		{
			double	dRet;

			dRet = dSrc * pow(10.0, iLen);
			dRet = (double)(int)(dRet + 0.5);

			return dRet * pow(10.0, -iLen);
		}

		static float ToRadian(const float deg) {

			return deg * BM_PI / 180.0f;
		}
		static float ToDegree(const float rad) {

			return rad * 180.0f / BM_PI;
		}

		static char GetByteSize(const int arg_check) {

			if (arg_check <= BM_CHAR_MAX) {
				return 1;
			}
			else if (arg_check <=BM_SHORT_MAX ) {
				return 2;
			}
			else if (arg_check <=BM_INT_MAX ) {
				return 4;
			}

			return 8;
		}
		static char GetUnsignedByteSize(const UINT arg_check) {

			if (arg_check <= BM_UCHAR_MAX) {
				return 1;
			}
			else if (arg_check <= BM_USHORT_MAX) {
				return 2;
			}
			else if (arg_check <= BM_UINT_MAX) {
				return 4;
			}
			return 8;
		}
	}
	struct Vector2;
	struct Vector3;
	struct Vector4;
	struct Quat;
	struct Matrix4x4;

	struct UInt2   {
		inline UInt2(int arg_x, int arg_y) {
			x = arg_x;
			y = arg_y;
		}
		inline UInt2() {
			x = 0;
			y = 0;
		}
#ifdef USE_DIRECTXMATH

		operator DirectX::XMUINT2() {
			return *(XMUINT2*)((void*)this);
		}
#endif // USE_DIRECTXMATH


		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(x, y);
		}
		unsigned int x, y;
	};

	struct Matrix4x4 
	{
		explicit inline Matrix4x4() {
			_11 = 1.0f;	_12 = 0.0f;	_13 = 0.0f;	_14 = 0.0f;
			_21 = 0.0f;	_22 = 1.0f;	_23 = 0.0f;	_24 = 0.0f;
			_31 = 0.0f;	_32 = 0.0f;	_33 = 1.0f;	_34 = 0.0f;
			_41 = 0.0f;	_42 = 0.0f;	_43 = 0.0f;	_44 = 1.0f;
		}
		explicit inline Matrix4x4(const Quat& quat);
		bool ShowUI() { return false; }
		inline Vector4 operator*(const Vector4& other);
		inline Vector3 operator*(const Vector3& other);

		inline Matrix4x4 operator*(const Matrix4x4& other)const {
			Matrix4x4 output;
			float x = this->_11;
			float y = this->_12;
			float z = this->_13;
			float w = this->_14;

			output._11 = (other._11 * x) + (other._21 * y) + (other._31 * z) + (other._41 * w);
			output._12 = (other._12 * x) + (other._22 * y) + (other._32 * z) + (other._42 * w);
			output._13 = (other._13 * x) + (other._23 * y) + (other._33 * z) + (other._43 * w);
			output._14 = (other._14 * x) + (other._24 * y) + (other._34 * z) + (other._44 * w);
			

			x = this->_21;
			y = this->_22;
			z = this->_23;
			w = this->_24;
			output._21 = (other._11 * x) + (other._21 * y) + (other._31 * z) + (other._41 * w);
			output._22 = (other._12 * x) + (other._22 * y) + (other._32 * z) + (other._42 * w);
			output._23 = (other._13 * x) + (other._23 * y) + (other._33 * z) + (other._43 * w);
			output._24 = (other._14 * x) + (other._24 * y) + (other._34 * z) + (other._44 * w);
			x = this->_31;
			y = this->_32;
			z = this->_33;
			w = this->_34;
			output._31 = (other._11 * x) + (other._21 * y) + (other._31 * z) + (other._41 * w);
			output._32 = (other._12 * x) + (other._22 * y) + (other._32 * z) + (other._42 * w);
			output._33 = (other._13 * x) + (other._23 * y) + (other._33 * z) + (other._43 * w);
			output._34 = (other._14 * x) + (other._24 * y) + (other._34 * z) + (other._44 * w);
			x = this->_41;
			y = this->_42;
			z = this->_43;
			w = this->_44;
			output._41 = (other._11 * x) + (other._21 * y) + (other._31 * z) + (other._41 * w);
			output._42 = (other._12 * x) + (other._22 * y) + (other._32 * z) + (other._42 * w);
			output._43 = (other._13 * x) + (other._23 * y) + (other._33 * z) + (other._43 * w);
			output._44 = (other._14 * x) + (other._24 * y) + (other._34 * z) + (other._44 * w);

			return output;
		}
		inline Matrix4x4 operator*=(const Matrix4x4& other) {
			*this = (*this) * other;
			return *this;
		}
		inline bool operator==(const Matrix4x4& other) const{
			

			return (this->_11 == other._11&& this->_12 == other._12&& this->_13 == other._13&& this->_14 == other._14&&
				this->_21 == other._21 && this->_22 == other._22 && this->_23 == other._23 && this->_24 == other._24&&
				this->_31 == other._31 && this->_32 == other._32 && this->_33 == other._33 && this->_34 == other._34&&
				this->_41 == other._41 && this->_42 == other._42 && this->_43 == other._43 && this->_44 == other._44
				);
		}

		inline Vector4& operator [](const unsigned int idx);


		inline Vector4 operator [](const unsigned int idx) const;

		inline Matrix4x4& Identity()
		{
			_11 = 1.0f;	_12 = 0.0f;	_13 = 0.0f;	_14 = 0.0f;
			_21 = 0.0f;	_22 = 1.0f;	_23 = 0.0f;	_24 = 0.0f;
			_31 = 0.0f;	_32 = 0.0f;	_33 = 1.0f;	_34 = 0.0f;
			_41 = 0.0f;	_42 = 0.0f;	_43 = 0.0f;	_44 = 1.0f;
			return *this;
		}
		inline Matrix4x4& Transpose() {

			Matrix4x4 temp = *this;

			this->_12 = temp._21; this->_13 = temp._31; this->_14 = temp._41;
			this->_21 = temp._12; this->_23 = temp._32; this->_24 = temp._42;
			this->_31 = temp._13; this->_32 = temp._23; this->_34 = temp._43;
			this->_41 = temp._14; this->_42 = temp._24; this->_43 = temp._34;

			return *this;
		}
		inline Matrix4x4 GetTranspose() const {
			Matrix4x4 output;

			output._12 = this->_21; output._13 = this->_31; output._14 = this->_41;
			output._21 = this->_12; output._23 = this->_32; output._24 = this->_42;
			output._31 = this->_13; output._32 = this->_23; output._34 = this->_43;
			output._41 = this->_14; output._42 = this->_24; output._43 = this->_34;
			output._11 = this->_11; output._22= this->_22; output._33 = this->_33; output._44 = this->_44;
			return output;
		}
		inline Matrix4x4& Inverse() {

			float a =this->_11,b = this->_12, c = this->_13, d = this->_14,
				e = this->_21, f = this->_22, g = this->_23, h = this->_24,
				i = this->_31, j = this->_32, k = this->_33, l = this->_34,
				m = this->_41, n = this->_42, o = this->_43, p = this->_44,
				q = a * f - b * e, r = a * g - c * e,
				s = a * h - d * e, t = b * g - c * f,
				u = b * h - d * f, v = c * h - d * g,
				w = i * n - j * m, x = i * o - k * m,
				y = i * p - l * m, z = j * o - k * n,
				A = j * p - l * n, B = k * p - l * o,
				ivd = 1 / (q * B - r * A + s * z + t * y - u * x + v * w);
			this->_11 = (f * B - g * A + h * z) * ivd;
			this->_12 = (-b * B + c * A - d * z) * ivd;
			this->_13 = (n * v - o * u + p * t) * ivd;
			this->_14= (-j * v + k * u - l * t) * ivd;
			this->_21= (-e * B + g * y - h * x) * ivd;
			this->_22= (a * B - c * y + d * x) * ivd;
			this->_23= (-m * v + o * s - p * r) * ivd;
			this->_24= (i * v - k * s + l * r) * ivd;
			this->_31= (e * A - f * y + h * w) * ivd;
			this->_32= (-a * A + b * y - d * w) * ivd;
			this->_33 = (m * u - n * s + p * q) * ivd;
			this->_34 = (-i * u + j * s - l * q) * ivd;
			this->_41 = (-e * z + f * x - g * w) * ivd;
			this->_42 = (a * z - b * x + c * w) * ivd;
			this->_43 = (-m * t + n * r - o * q) * ivd;
			this->_44 = (i * t - j * r + k * q) * ivd;


			return *this;
		}
		inline Matrix4x4 GetInverse()const {

			Matrix4x4 output;
			float a = this->_11, b = this->_12, c = this->_13, d = this->_14,
				e = this->_21, f = this->_22, g = this->_23, h = this->_24,
				i = this->_31, j = this->_32, k = this->_33, l = this->_34,
				m = this->_41, n = this->_42, o = this->_43, p = this->_44,
				q = a * f - b * e, r = a * g - c * e,
				s = a * h - d * e, t = b * g - c * f,
				u = b * h - d * f, v = c * h - d * g,
				w = i * n - j * m, x = i * o - k * m,
				y = i * p - l * m, z = j * o - k * n,
				A = j * p - l * n, B = k * p - l * o,
				ivd = 1 / (q * B - r * A + s * z + t * y - u * x + v * w);
			output._11 = (f * B - g * A + h * z) * ivd;
			output._12 = (-b * B + c * A - d * z) * ivd;
			output._13 = (n * v - o * u + p * t) * ivd;
			output._14 = (-j * v + k * u - l * t) * ivd;
			output._21 = (-e * B + g * y - h * x) * ivd;
			output._22 = (a * B - c * y + d * x) * ivd;
			output._23 = (-m * v + o * s - p * r) * ivd;
			output._24 = (i * v - k * s + l * r) * ivd;
			output._31 = (e * A - f * y + h * w) * ivd;
			output._32 = (-a * A + b * y - d * w) * ivd;
			output._33 = (m * u - n * s + p * q) * ivd;
			output._34 = (-i * u + j * s - l * q) * ivd;
			output._41 = (-e * z + f * x - g * w) * ivd;
			output._42 = (a * z - b * x + c * w) * ivd;
			output._43 = (-m * t + n * r - o * q) * ivd;
			output._44 = (i * t - j * r + k * q) * ivd;
			return output;
		}
		inline Matrix4x4 GetInValidYZ()const {
			Matrix4x4 output = *this;
			output._11 = 1.0f;
			output._21 = 0.0f;
			output._31 = 0.0f;
			return output;
		}
		inline Matrix4x4 GetInValidXZ()const {
			Matrix4x4 output = *this;
			output._12 = 0.0f;
			output._22 = 1.0f;
			output._32 = 0.0f;
			return output;
		}
		inline Matrix4x4 GetInValidXY()const {
			Matrix4x4 output = *this;
			output._13 = 0.0f;
			output._23 = 0.0f;
			output._33 = 1.0f;
			return output;
		}
		inline Matrix4x4& InValidYZ() {
			this->_11 = 1.0f;
			this->_21 = 0.0f;
			this->_31 = 0.0f;
			return *this;
		}
		inline Matrix4x4& InValidXZ() {
			this->_12 = 0.0f;
			this->_22 = 1.0f;
			this->_32 = 0.0f;
			return *this;
		}
		inline Matrix4x4& InValidXY() {

			this->_13 = 0.0f;
			this->_23 = 0.0f;
			this->_33 = 1.0f;
			return *this;
		}
		inline Quat ToQuat()const;

		inline Vector3 GetEulerOneValue()const;
		inline Vector3 GetEulerOneValue_local()const;


		inline Matrix4x4& CreateFromEuler(const Vector3& arg_euler);
		inline Matrix4x4& CreateFromEuler_local(const Vector3& arg_euler);
		inline bool Same(const Matrix4x4& other, const float epsilon = 0.001f) const {

			for (int i = 0; i < 4; i++) {
				for (int j = 0; j < 4; j++) {
					if (abs(this->m[i][j] - other.m[i][j]) > epsilon) {
						return false;
					}
				}
			}

			return true;
		}

		inline Vector3 GetPosition()const;
		inline Vector3 GetPosition_Transpose()const;
		inline void PositionFloor();
		inline Matrix4x4 GetPositionFloor()const;
		inline void PositionFloor_transpose();
		inline void PositionXFloor_transpose();
		inline void PositionYFloor_transpose();
		inline void PositionZFloor_transpose();
		inline void PositionXYFloor_transpose();
		inline void PositionYZFloor_transpose();
		inline void PositionXZFloor_transpose();
		inline Matrix4x4 GetPositionFloor_transpose()const;
		inline Matrix4x4 GetPositionXFloor_transpose()const;
		inline Matrix4x4 GetPositionYFloor_transpose()const;
		inline Matrix4x4 GetPositionZFloor_transpose()const;
		inline Matrix4x4 GetPositionXYFloor_transpose()const;
		inline Matrix4x4 GetPositionYZFloor_transpose()const;
		inline Matrix4x4 GetPositionXZFloor_transpose()const;
		inline Matrix4x4 GetOnlyRotation_transpose()const;
		inline void RemovePosition();
		static inline Matrix4x4 Translate(const Vector3& arg_position);
		static inline Matrix4x4 Scale(const Vector3& arg_scale);

		static inline Matrix4x4 RollX(const float angle) {
			float    fSinAngle;
			float    fCosAngle;
			MathHelper::SinCos(fSinAngle, fCosAngle, angle);

			Matrix4x4 output;
			output._11 = 1.0f;
			output._12 = 0.0f;
			output._13 = 0.0f;
			output._14= 0.0f;

			output._21 = 0.0f;
			output._22 = fCosAngle;
			output._23 = fSinAngle;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = -fSinAngle;
			output._33 = fCosAngle;
			output._34 = 0.0f;

			output._41 = 0.0f;
			output._42 = 0.0f;
			output._43 = 0.0f;
			output._44 = 1.0f;
			return output;
		}
		static inline Matrix4x4 RollY(const float angle) {
			float    fSinAngle;
			float    fCosAngle;
			MathHelper::SinCos(fSinAngle, fCosAngle, angle);

			Matrix4x4 output;
			output._11 = fCosAngle;
			output._12 = 0.0f;
			output._13 = -fSinAngle;
			output._14 =  0.0f;

			output._21 = 0.0f;
			output._22 = 1.0f;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = fSinAngle;
			output._32 = 0.0f;
			output._33 = fCosAngle;
			output._34 = 0.0f;

			output._41 = 0.0f;
			output._42 = 0.0f;
			output._43 = 0.0f;
			output._44 = 1.0f;
			return output;
		}
		static inline Matrix4x4 RollZ(const float angle) {
			float    fSinAngle;
			float    fCosAngle;
			MathHelper::SinCos(fSinAngle, fCosAngle, angle);

			Matrix4x4 output;
			output._11 = fCosAngle;
			output._12 = fSinAngle;
			output._13 = 0.0f;
			output._14 = 0.0f;

			output._21 = -fSinAngle;
			output._22 = fCosAngle;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = 0.0f;
			output._33 = 1.0f;
			output._34 = 0.0f;

			output._41 = 0.0f;
			output._42 = 0.0f;
			output._43 = 0.0f;
			output._44 = 1.0f;
			return output;
		}

		static Matrix4x4 PersepectiveFovLH(const float fovAngleY, const float aspectRate, const float nearClip, const float farClip) {
			float    SinFov;
			float    CosFov;
			MathHelper::SinCos(SinFov, CosFov, fovAngleY * 0.5f);

			float Height = CosFov / SinFov;
			float Width = Height / aspectRate;
			float fRange = farClip / (farClip - nearClip);

			Matrix4x4 output;
			output._11 = Width;
			output._12 = 0.0f;
			output._13 = 0.0f;
			output._14 = 0.0f;

			output._21 = 0.0f;
			output._22 = Height;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = 0.0f;
			output._33 = fRange;
			output._34 = 1.0f;

			output._41 = 0.0f;
			output._42 = 0.0f;
			output._43 = -fRange * nearClip;
			output._44 = 0.0f;
			return output;
		}
		static Matrix4x4 PersepectiveFovRH(const float fovAngleY, const float aspectRate, const float nearClip, const float farClip) {
			float    SinFov;
			float    CosFov;
			MathHelper::SinCos(SinFov, CosFov, fovAngleY * 0.5f);

			float Height = CosFov / SinFov;
			float Width = Height / aspectRate;
			float fRange = farClip / (farClip - nearClip);

			Matrix4x4 output;
			output._11 = Width;
			output._12 = 0.0f;
			output._13 = 0.0f;
			output._14 = 0.0f;

			output._21 = 0.0f;
			output._22 = Height;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = 0.0f;
			output._33 = fRange;
			output._34 = -1.0f;

			output._41 = 0.0f;
			output._42 = 0.0f;
			output._43 = fRange * nearClip;
			output._44 = 0.0f;
			return output;
		}
		static Matrix4x4 OrthographicOffCenterLH(const float viewLeft, const float viewRight, const float viewBottom, const float viewTop, const float nearClip, const float farClip) {
			float reciprocalWidth = 1.0f / (viewRight - viewLeft);
			float reciprocalHeight = 1.0f / (viewTop - viewBottom);
			float fRange = 1.0f / (farClip - nearClip);

			Matrix4x4 output;
			output._11 = reciprocalWidth + reciprocalWidth;
			output._12 = 0.0f;
			output._13 = 0.0f;
			output._14 = 0.0f;

			output._21 = 0.0f;
			output._22 = reciprocalHeight + reciprocalHeight;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = 0.0f;
			output._33 = fRange;
			output._34 = 0.0f;

			output._41 = -(viewLeft + viewRight) * reciprocalWidth;
			output._42 = -(viewTop + viewBottom) * reciprocalHeight;
			output._43 = -fRange * nearClip;
			output._44 = 1.0f;
			return output;
		}
		static Matrix4x4 OrthographicOffCenterRH(const float viewLeft, const float viewRight, const float viewBottom, const float viewTop, const float nearClip, const float farClip) {
			float reciprocalWidth = 1.0f / (viewRight - viewLeft);
			float reciprocalHeight = 1.0f / (viewTop - viewBottom);
			float fRange = 1.0f / (nearClip - farClip);

			Matrix4x4 output;
			output._11 = reciprocalWidth + reciprocalWidth;
			output._12 = 0.0f;
			output._13 = 0.0f;
			output._14 = 0.0f;

			output._21 = 0.0f;
			output._22 = reciprocalHeight + reciprocalHeight;
			output._23 = 0.0f;
			output._24 = 0.0f;

			output._31 = 0.0f;
			output._32 = 0.0f;
			output._33 = fRange;
			output._34 = 0.0f;

			output._41 = -(viewLeft + viewRight) * reciprocalWidth;
			output._42 = -(viewTop + viewBottom) * reciprocalHeight;
			output._43 = fRange * nearClip;
			output._44 = 1.0f;

			return output;
		}
		~Matrix4x4() {}


		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(this->_11, this->_12, this->_13, this->_14);
			archive(this->_21, this->_22, this->_23, this->_24);
			archive(this->_31, this->_32, this->_33, this->_34);
			archive(this->_41, this->_42, this->_43, this->_44);
		}

#ifdef USE_DIRECTXMATH

		//DirectXMath‚Æ‚ÌŒÝŠ·
		inline Matrix4x4(const DirectX::XMMATRIX other)
		{
			DirectX::XMMATRIX tmp = other;
			DirectX::XMStoreFloat4x4((XMFLOAT4X4*)((void*)this), tmp);
		}
		inline Matrix4x4& operator=(const DirectX::XMMATRIX& other)
		{
			DirectX::XMStoreFloat4x4((XMFLOAT4X4*)((void*)this), other);
			return *this;
		}
		inline operator DirectX::XMMATRIX() const
		{
			auto output = DirectX::XMLoadFloat4x4((XMFLOAT4X4*)((void*)this));
			return output;
		}
#endif
		union
		{
			struct
			{
				float _11, _12, _13, _14;
				float _21, _22, _23, _24;
				float _31, _32, _33, _34;
				float _41, _42, _43, _44;
			};
			float m[4][4];
		};
	};

	struct Vector2 
	{
		float x;
		float y;
		explicit inline Vector2(float x, float y)
		{
			this->x = x;
			this->y = y;
		}

		explicit inline Vector2() {
			this->x = 0;
			this->y = 0;
		}
		inline Vector2(const Vector2& other) {
			this->x = other.x;
			this->y = other.y;
		}
		inline Vector2(const float value) {
			this->x = value;
			this->y = value;
		}

		inline ~Vector2() {}


		inline  float* GetData()
		{
			float output[2] = { x,y };
			return output;
		}
		inline  void GetData(float* out)
		{
			auto data = GetData();
			for (int i = 0; i < 2; i++) {
				out[i] = data[i];
			}
		}

		inline Vector2& operator +=(const Vector2& other)
		{
			*this = *this + other;
			return *this;
		}

		inline Vector2& operator +=(float value)
		{
			*this = *this + value;
			return *this;
		}
		inline Vector2& operator -=(const Vector2& other)
		{
			*this = *this - other;
			return *this;
		}

		inline Vector2& operator -=(float value)
		{
			*this = *this - value;
			return *this;
		}

		inline Vector2& operator *=(const Vector2& other)
		{
			*this = *this * other;
			return *this;
		}

		inline Vector2 operator /=(const Vector2& other)
		{
			*this = *this / other;
			return *this;
		}

		inline Vector2& operator *=(float value)
		{
			x *= value;
			y *= value;
			return *this;
		}

		inline Vector2& operator /=(float value)
		{
			x /= value;
			y /= value;
			return *this;
		}



		inline Vector2 operator +(float value) const
		{
			return Vector2(x + value, y + value);
		}
		inline Vector2 operator -(float value)const
		{
			return Vector2(x - value, y - value);
		}
		inline Vector2 operator *(float value)const
		{
			return Vector2(x * value, y * value);
		}
		inline Vector2 operator /(float value)const
		{
			return Vector2(x / value, y / value);
		}

		inline Vector2 operator +(const Vector2& other) const
		{
			return Vector2(x + other.x, y + other.y);
		}
		inline Vector2 operator -(const Vector2& other) const
		{
			return Vector2(x - other.x, y - other.y);
		}
		inline Vector2 operator *(const Vector2& other) const
		{
			return Vector2(x * other.x, y * other.y);
		}
		inline Vector2 operator /(const Vector2& other) const
		{
			return Vector2(x / other.x, y / other.y);
		}


		inline const Vector2 operator -() const
		{
			return  (*this) * -1;
		}

		inline bool operator==(const Vector2& other)
		{

			return (x == other.x && y == other.y);
		}
		inline bool operator!=(const Vector2& other)
		{

			return !(*this  == other);
		}

		inline float& operator [](const unsigned int idx)
		{
			return *(&x + idx);
		}


		inline float operator [](const unsigned int idx) const
		{
			return *(&x + idx);
		}
		inline operator Vector3() const;
		inline operator Vector4() const;


		inline Vector2& Floor(int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow(10, len - 1);
			x = std::floor(x);
			y = std::floor(y);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}

		inline Vector2& Round(int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow(10, len - 1);
			x = std::round(x);
			y = std::round(y);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}

		inline Vector2& Ceil(int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow(10, len - 1);
			x = std::ceil(x);
			y = std::ceil(y);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}

		inline Vector2 GetRound(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector2 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::round(x);
			output.y = std::round(y);
			output /= (float) std::pow(10, len - 1);
			return output;
		}
		inline Vector2 GetFloor(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector2 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::floor(x);
			output.y = std::floor(y);
			output /= (float) std::pow(10, len - 1);
			return output;
		}
		inline Vector2 GetCeil(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector2 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::ceil(x);
			output.y = std::ceil(y);
			output /= (float) std::pow(10, len - 1);
			return output;
		}

		inline float Dot(const Vector2& other) const
		{
			return this->x*other.x + this->y * other.y;
		}

		inline bool IsNaN() const
		{
			return isnan(x) || isnan(y);
		}

		inline bool IsInfinite() const
		{
			return isinf(x) || isinf(y);
		}

		inline float GetLengthSqr() const
		{
			return this->x * this->x + this->y * this->y;
		}

		inline float GetLength() const {
			return std::sqrt(GetLengthSqr());
		}

		inline float GetDistance(Vector2& other) const {
			auto i = std::sqrt((other.x - x) * (other.x - x) + (other.y - y) * (other.y - y));
			return i;
		}

		inline Vector2& ToRadian() {
			x *= (BM_PI / 180.0f);
			y *= (BM_PI / 180.0f);

			return *this;
		}

		inline Vector2 GetRadian() const {
			return Vector2(x * (BM_PI / 180.0f), y * (BM_PI / 180.0f));

		}
		inline Vector2& ToDegrees() {
			x *= (180.0f / BM_PI);
			y *= (180.0f / BM_PI);
			return *this;
		}

		inline Vector2 GetDegrees() const {
			return Vector2(x * (180.0f / BM_PI), y * (180.0f / BM_PI));

		}
		inline Vector2& Clamp(const Vector2& arg_max, const Vector2& arg_min) {
			x = max(arg_min.x, x);
			y = max(arg_min.y, y);

			x = min(arg_max.x, x);
			y = min(arg_max.y, y);
			return *this;
		}
		inline Vector2 GetClamp(const Vector2& arg_max, const Vector2& arg_min)const {
			Vector2 output;
			output.x = max(arg_min.x, x);
			output.y = max(arg_min.y, y);

			output.x = min(arg_max.x, x);
			output.y = min(arg_max.y, y);
			return *this;
		}
		inline void Normalize()
		{
			float length = GetLength();
			if(length)
				this->x /= length;
			if(length)
				this->y /= length;
		}
		inline Vector2 GetNormalize()const
		{
			Vector2 temp = *this;
			temp.Normalize();
			return temp;
		}
		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(x, y);
		}


#ifdef USE_DIRECTXMATH

		//DirectXMath‚Æ‚ÌŒÝŠ·
		inline operator DirectX::XMVECTOR() const
		{
			DirectX::XMVECTOR output = DirectX::XMLoadFloat2((XMFLOAT2*)((void*)this));
			return output;
		}
		inline operator DirectX::XMFLOAT2() const
		{
			return *(XMFLOAT2*)((void*)this);
		}
		inline Vector2& operator=(const DirectX::XMVECTOR& other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat2((XMFLOAT2*)((void*)this), temp);
			return *this;
		}
		inline Vector2(const DirectX::XMVECTOR other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat2((XMFLOAT2*)((void*)this), temp);
		}
#endif


	};

	struct  Vector3 
	{
		static Vector3 XAxis;
		static Vector3 YAxis;
		static Vector3 ZAxis;
		static Vector3 Zero;

		explicit inline Vector3(float x, float y, float z)
		{
			this->x = x;
			this->y = y;
			this->z = z;
		}
		inline Vector3(const Vector3& other)
		{
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
		}
		inline Vector3(float v)
		{
			this->x = v;
			this->y = v;
			this->z = v;
		}
		inline Vector3()
		{
			x = 0;
			y = 0;
			z = 0;
		}

		inline Vector2 GetVector2() {
			return Vector2(x, y);
		}

		inline ~Vector3()
		{
		}

		inline  void GetData(float out[3])
		{
			out[0] = x;
			out[1] = y;
			out[2] = z;
		}
		inline const float* GetData_const()const
		{
			return &x;
		}
		inline float* GetData()
		{
			return &x;
		}

		inline Vector3 operator=(const Vector3& other)
		{
			if (this != &other) {
				x = other.x;
				y = other.y;
				z = other.z;
			}
			return *this;
		}

		inline Vector3& operator +=(const Vector3& other)
		{
			*this = *this + other;
			return *this;
		}

		inline Vector3& operator +=(float value)
		{
			*this = *this + value;
			return *this;
		}
		inline Vector3& operator -=(const Vector3& other)
		{
			*this = *this - other;
			return *this;
		}

		inline Vector3& operator -=(float value)
		{
			*this = *this - value;
			return *this;
		}
		inline Vector3& operator *=(float value)
		{
			*this = *this * value;
			return *this;
		}
		inline Vector3& operator *=(const Matrix4x4& value)
		{
			*this = *this * value;
			return *this;
		}
		inline Vector3& operator *=(const Vector3& other) {
			*this = Vector3(other.x * x, other.y * y, other.z * z);
			return  *this;
		}

		inline Vector3& operator /=(float value)
		{
			*this = *this / value;
			return *this;
		}
		inline operator Vector4()const;
		inline operator Vector2()const {
			return Vector2(x, y);
		}

		inline Vector3 operator +(float value)const
		{
			return Vector3(this->x + value, this->y + value, this->z + value);
		}
		inline Vector3 operator -(float value)const
		{
			return Vector3(this->x - value, this->y - value, this->z - value);
		}

		inline Vector3 operator *(float value)const
		{
			return Vector3(this->x * value, this->y * value, this->z * value);
		}
		inline Vector3 operator /(float value)const
		{
			return Vector3(this->x / value, this->y / value, this->z / value);
		}

		inline Vector3 operator +(Vector3 other)const
		{
			return Vector3(this->x + other.x, this->y + other.y, this->z + other.z);
		}
		inline Vector3 operator -(const Vector3& other)const
		{
			return Vector3(x - other.x, y - other.y, z - other.z);
		}

		inline Vector3 operator *(const Vector3& other)const {

			return Vector3(other.x * x, other.y * y, other.z * z);
		}
		inline Vector3 operator /(const Vector3& other)const {

			return Vector3(other.x / x, other.y / y, other.z / z);
		}



		inline Vector3 operator *(const Matrix4x4& value) const
		{
			Vector3 temp = Vector3(value._11 * x + value._21 * y + value._31 * z + value._41,
				value._12 * x + value._22 * y + value._32 * z + value._42,
				value._13 * x + value._23 * y + value._33 * z + value._43);
			return temp;
		}
		inline const Vector3 operator -() const
		{
			return   (*this)* -1;
		}

		inline Vector3 operator*(const Quat& other);


		inline const bool operator==(const Vector3& other)const {
			if (other.x != this->x || other.y != this->y || other.z != this->z) {
				return false;
			}
			return true;
		}

		inline const bool operator!=(const Vector3& other)const {

			return !(*this == other);
		}

		inline float& operator [](const unsigned int idx)
		{
			return *(&x + idx);
		}


		inline float operator [](const unsigned int idx) const
		{
			return *(&x + idx);
		}


		inline Vector3& Floor(const int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow(10, len - 1);
			x = std::floor(x);
			y = std::floor(y);
			z = std::floor(z);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}

		inline Vector3& Round(const int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow( 10,len-1);
			x = std::round(x);
			y = std::round(y);
			z = std::round(z);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}

		inline Vector3& Ceil(const int len=1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float) std::pow(10, len - 1);
			x= std::ceil(x);
			y=std::ceil(y);
			z=std::ceil(z);
			*this /= (float) std::pow(10, len - 1);
			return *this;
		}


		inline Vector3 GetRound(const int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector3 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::round(x);
			output.y = std::round(y);
			output.z = std::round(z);
			output /= (float) std::pow(10, len - 1);
			return output;
		}
		inline Vector3 GetFloor(const int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector3 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::floor(x);
			output.y = std::floor(y);
			output.z = std::floor(z);
			output /= (float) std::pow(10, len - 1);
			return output;
		}
		inline Vector3 GetCeil(const int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector3 output;
			output *= (float) std::pow(10, len - 1);
			output.x = std::ceil(x);
			output.y = std::ceil(y);
			output.z = std::ceil(z);
			output /= (float) std::pow(10, len - 1);
			return output;
		}

		inline float Distance(const Vector3& arg_point)const {

			return sqrt((this->x - arg_point.x) * (this->x - arg_point.x) + (this->y - arg_point.y) * (this->y - arg_point.y) + (this->z - arg_point.z) * (this->z - arg_point.z));
			
		}

		inline Vector2 ToVector2()const {
			return Vector2(x,y);
		}


		inline bool IsNaN() const
		{
			return isnan(x) || isnan(y) || isnan(z);
		}

		inline Vector3 GetRemoveNan() {
			if (IsNaN()) {
				auto output = *this;

				if (output.x == nanf("")) {
					output.x = 0;
				}
				if (output.y == nanf("")) {
					output.y = 0;
				}
				if (output.z == nanf("")) {
					output.z = 0;
				}
			}
			return *this;
		}
		inline bool IsInfinite() const
		{
			return isinf(x) || isinf(y) || isinf(z);
		}
		inline bool IsZero() const
		{
			return (abs(x) <= FLT_MIN * 10) && (abs(y) <= FLT_MIN * 10) && (abs(z) <= FLT_MIN * 10);
		}
		inline bool IsMinEps() const
		{
			return (abs(x) <= 0.00025) && (abs(y) <= 0.00025) && (abs(z) <= 0.00025);
		}
		inline void RemoveEps() {
			if ((abs(x) <= 0.000000025)) {
				x = 0;
			}
			if ((abs(y) <= 0.000000025)) {
				y = 0;
			}
			if ((abs(z) <= 0.000000025)) {
				z = 0;
			}
		}

		inline const Vector3& ToDegrees() {
			x *= (180.0f / BM_PI);
			y *= (180.0f / BM_PI);
			z *= (180.0f / BM_PI);

			return *this;
		}
		inline  Vector3 GetDegrees() const{
			Vector3 output = Vector3(x * (180.0f / BM_PI), y * (180.0f / BM_PI), z * (180.0f / BM_PI));


			return output;
		}
		inline const Vector3& ToRadian() {
			x *= (BM_PI / 180.0f);
			y *= (BM_PI / 180.0f);
			z *= (BM_PI / 180.0f);

			return *this;
		}

		inline Vector3 GetRadian() const {
			return Vector3(x*(BM_PI/180.0f), y * (BM_PI / 180.0f), z * (BM_PI / 180.0f));

		}
		inline Vector3& Clamp(const Vector3& arg_max, const Vector3& arg_min) {
			x = max(arg_min.x, x);
			y = max(arg_min.y, y);
			z = max(arg_min.z, z);

			x = min(arg_max.x, x);
			y = min(arg_max.y, y);
			z = min(arg_max.z, z);
			return *this;
		}
		inline Vector3 GetClamp(const Vector3& arg_max, const Vector3& arg_min)const {
			Vector3 output;
			output.x = max(arg_min.x, x);
			output.y = max(arg_min.y, y);
			output.z = max(arg_min.z, z);

			output.x = min(arg_max.x, x);
			output.y = min(arg_max.y, y);
			output.z = min(arg_max.z, z);
			return *this;
		}
		inline Vector3 GetFreeze(bool arg_freezeX, bool arg_freezeY, bool arg_freezeZ)const {
			auto output = *this;
			if (arg_freezeX)
				output.x = 0;
			if (arg_freezeY)
				output.y = 0;
			if (arg_freezeZ)
				output.z = 0;

			return output;
		}

		inline float Dot(const Vector3& other) const
		{
			return this->x * other.x + this->y * other.y + this->z * other.z;
		}
		inline Vector3& Cross(const Vector3& other)
		{
			auto temp = *this;
			this->x = (temp.y * other.z) - (temp.z * other.y);
			this->y = (temp.z * other.x) - (temp.x * other.z);
			this->z = (temp.x * other.y) - (temp.y * other.x);
			return *this;
		}

		inline Vector3 GetCross(const Vector3& other)const
		{
			return Vector3((this->y * other.z) - (this->z * other.y), (this->z * other.x) - (this->x * other.z), (this->x * other.y) - (this->y * other.x));
		}
		inline float GetLengthSqr() const
		{
			return this->x * this->x + this->y * this->y + this->z * this->z;
		}


		inline float GetLength() const
		{
			return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z);
		}

		inline Vector3& Normalize()
		{
			float length = GetLength();
			if (length) {
				this->x /= length;
				this->y /= length;
				this->z /= length;
			}
			return *this;
		}

		inline Vector3 GetNormalize() const
		{
			Vector3 output = *this;
			output.Normalize();
			return output;
		}

		inline Vector3& Abs() {

			x = abs(x);
			y = abs(y);
			z = abs(z);

			return *this;
		}
		inline Vector3 GetAbs()const {
			Vector3 output;
			output.x = abs(x);
			output.y = abs(y);
			output.z = abs(z);

			return output;
		}
		inline Vector3& Scroll()
		{
			auto temp = *this;

			x = temp.z;
			y = temp.x;
			z = temp.y;
			return *this;
		}

		inline Vector3& Scroll_Reverse() {

			auto temp = *this;

			x = temp.y;
			y = temp.z;
			z = temp.x;
			return *this;
		}

		inline Vector3& Max(const Vector3& arg_other) {
			this->x = max(x, arg_other.x);
			this->y = max(y, arg_other.y);
			this->z = max(z, arg_other.z);
			return *this;
		}
		inline Vector3& Min(const Vector3& arg_other) {
			this->x = min(x, arg_other.x);
			this->y = min(y, arg_other.y);
			this->z = min(z, arg_other.z);
			return *this;
		}
		inline Vector3 GetMax(const Vector3& arg_other)const {
			Vector3 output;
			output.x = max(x, arg_other.x);
			output.y = max(y, arg_other.y);
			output.z = max(z, arg_other.z);

			return output;
		}
		inline Vector3 GetMin(const Vector3& arg_other)const {

			Vector3 output;
			output.x = min(x, arg_other.x);
			output.y = min(y, arg_other.y);
			output.z = min(z, arg_other.z);

			return output;
		}

		inline Vector3 GetReflect(const Vector3& other)const {
			Vector3 output;
			float s = 2.0f * (this->x * other.x + this->y * other.y + this->z * other.z);

			output.x = this->x - s * other.x;
			output.y = this->y - s * y;
			output.z = this->z - s * z;

			return output;
		}

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(x, y, z);
		}
#ifdef USE_DIRECTXMATH
		//DirectXMath‚Æ‚ÌŒÝŠ·
		inline operator DirectX::XMVECTOR() const
		{
			DirectX::XMVECTOR output = DirectX::XMLoadFloat3((XMFLOAT3*)((void*)this));
			return output;
		};
		inline Vector3& operator=(const DirectX::XMFLOAT3& other)
		{
			DirectX::XMVECTOR temp = DirectX::XMLoadFloat3(&other);
			DirectX::XMStoreFloat3((XMFLOAT3*)((void*)this), temp);
			return *this;
		}
		inline Vector3(const DirectX::XMVECTOR other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat3((XMFLOAT3*)((void*)this), temp);
		}
		inline Vector3& operator=(const DirectX::XMVECTOR& other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat3((XMFLOAT3*)((void*)this), temp);
			return *this;
		}
#endif
		


		float x, y, z;
	};

	struct Vector4 
	{
		explicit inline Vector4(float x, float y, float z, float w)
		{
			this->x = x;
			this->y = y;
			this->z = z;
			this->w = w;
		}
		explicit inline Vector4(const Vector3& xyz, float w)
		{
			this->x = xyz.x;
			this->y = xyz.y;
			this->z = xyz.z;
			this->w = w;
		}
		inline Vector4(const Vector4& other)
		{
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			this->w = other.w;
		}

		inline Vector4()
		{
			x = 0;y = 0;z = 0;w = 0;
		}

		inline  float* GetData() 
		{			
			return &(this->x);
		}
		inline ~Vector4() {};
		inline Vector4& operator +=(const Vector4& other) {
			*this = *this + other;
			return *this;
		};

		inline Vector4& operator -=(const Vector4& other)
		{
			*this = *this - other;
			return *this;
		}
		inline Vector4& operator *=(const Vector4& other) {
			*this = *this * other;
			return *this;
		};

		inline Vector4& operator /=(const Vector4& other)
		{
			*this = *this / other;
			return *this;
		}

		inline Vector4& operator +=(float value) {
			*this = *this + value;
			return *this;
		};

		inline Vector4& operator -=(float value)
		{
			*this = *this - value;
			return *this;
		};

		inline Vector4& operator *=(float value)
		{
			*this = *this * value;
			return *this;
		}

		inline Vector4& operator /=(float value)
		{
			*this = *this / value;
			return *this;
		}

		inline Vector4 operator +(float value)const {
			return Vector4(this->x + value, this->y + value, this->z + value, this->w + value);
		}
		inline Vector4 operator -(float value)const
		{
			return Vector4(this->x - value, this->y - value, this->z - value, this->w - value);
		};
		inline Vector4 operator *(float value)const {
			return Vector4(this->x * value, this->y * value, this->z * value, this->w * value);
		}
		inline Vector4 operator /(float value)const
		{
			return Vector4(this->x / value, this->y / value, this->z / value, this->w / value);
		};



		inline Vector4 operator +(const Vector4& other)const {
			return Vector4(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w);
		}
		inline Vector4 operator -(const Vector4& other)const {
			return Vector4(this->x - other.x, this->y - other.y, this->z - other.z, this->w - other.w);
		}
		inline Vector4 operator *(const Vector4& other)const {
			return Vector4(this->x * other.x, this->y * other.y, this->z * other.z, this->w * other.w);
		}
		inline Vector4 operator /(const Vector4& other)const {
			return Vector4(this->x / other.x, this->y / other.y, this->z / other.z, this->w / other.w);
		}


		inline Vector4 operator *(const Matrix4x4& other) const
		{
			Vector4 temp = Vector4(other._11 * this->x + other._21 * this->y + other._31 * this->z + other._41 * this->w,
				other._12 * this->x + other._22 * this->y + other._32 * this->z + other._42 * this->w,
				other._13 * this->x + other._23 * this->y + other._33 * this->z + other._43 * this->w,
				other._14 * this->x + other._24 * this->y + other._34 * this->z + other._44 * this->w
			);
			return temp;
		}


		inline const Vector4 operator -() const
		{
			return *this* - 1;
		}

		inline bool operator==(const Vector4& other)const {
			return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w;
		}

		inline bool operator!=(const Vector4& other)const {
			return !((*this) == other);
		}

		inline float& operator [](int idx)
		{
			return *(&x + idx);
		}
		inline float operator [](int idx) const
		{
			return *(&x + idx);
		}

		inline operator Vector3() const{
			return Vector3(x, y, z);
		}
		inline operator Vector2()const {
			return Vector2(x, y);
		}

		inline Vector4& Floor(int len = 1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float)std::pow(10, len - 1);
			x = std::floor(x);
			y = std::floor(y);
			z = std::floor(z);
			w = std::floor(w);
			*this /= (float)std::pow(10, len - 1);
			return *this;
		}
		inline Vector4& Round(int len = 1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float)std::pow(10, len - 1);
			x = std::round(x);
			y = std::round(y);
			z = std::round(z);
			w = std::round(w);
			*this /= (float)std::pow(10, len - 1);
			return *this;
		}
		inline Vector4& Ceil(int len = 1)
		{
			if (len <= 0) {
				return *this;
			}
			*this *= (float)std::pow(10, len - 1);
			x = std::ceil(x);
			y = std::ceil(y);
			z = std::ceil(z);
			w = std::ceil(w);
			*this /= (float)std::pow(10, len - 1);
			return *this;
		}

		inline Vector4 GetRound(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector4 output;
			output *= (float)std::pow(10, len - 1);
			output.x = std::round(x);
			output.y = std::round(y);
			output.z = std::round(z);
			output.w = std::round(w);
			output /= (float)std::pow(10, len - 1);
			return output;
		}
		inline Vector4 GetFloor(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector4 output;
			output *= (float)std::pow(10, len - 1);
			output.x = std::floor(x);
			output.y = std::floor(y);
			output.z = std::floor(z);
			output.w = std::floor(w);
			output /= (float)std::pow(10, len - 1);
			return output;
		}
		inline Vector4 GetCeil(int len = 1)const
		{
			if (len <= 0) {
				return *this;
			}
			Vector4 output;
			output *= (float)std::pow(10, len - 1);
			output.x = std::ceil(x);
			output.y = std::ceil(y);
			output.z = std::ceil(z);
			output.w = std::ceil(w);
			output /= (float)std::pow(10, len - 1);
			return output;
		}

		inline Vector3 GetVec3()const {
			return Vector3(x, y, z);
		}

		inline float Dot(const Vector4& other) const
		{
			return this->x * other.x + this->y * other.y + this->z * other.z + this->w * other.w;
		}

		inline bool IsNaN() const
		{
			return isnan(x) || isnan(y) || isnan(z) || isnan(w);
		}

		inline bool IsInfinite() const
		{
			return isinf(x) || isinf(y) || isinf(z) || isinf(w);
		}

		inline float GetLengthSqr() const
		{
			return this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;
		}
		inline Vector4& RemoveNan() {
			if (IsNaN()) {

				if (isnan((*this).x)) {
					(*this).x = 0;
				}
				if (isnan((*this).y)) {
					(*this).y = 0;
				}
				if (isnan((*this).z)) {
					(*this).z = 0;
				}
				if (isnan((*this).w)) {
					(*this).w = 0;
				}
			}
			return *this;
		}
		inline float GetLength() const
		{
			return std::sqrt(this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w);
		}

		inline const Vector4& ToDegrees() {
			x *= (180.0f / BM_PI);
			y *= (180.0f / BM_PI);
			z *= (180.0f / BM_PI);
			w *= (180.0f / BM_PI);

			return *this;
		}
		inline  Vector4 GetDegrees() const {
			Vector4 output = Vector4(x * (180.0f / BM_PI), y * (180.0f / BM_PI), z * (180.0f / BM_PI), w * (180.0f / BM_PI));


			return output;
		}
		inline const Vector4& ToRadian() {
			x *= (BM_PI / 180.0f);
			y *= (BM_PI / 180.0f);
			z *= (BM_PI / 180.0f);
			w *= (BM_PI / 180.0f);

			return *this;
		}

		inline Vector4 GetRadian() const {
			return Vector4(x * (BM_PI / 180.0f), y * (BM_PI / 180.0f), z * (BM_PI / 180.0f), w * (BM_PI / 180.0f));

		}

		inline Vector4& Clamp(const Vector4& arg_max, const Vector4& arg_min) {
			x = max(arg_min.x, x);
			y = max(arg_min.y, y);
			z = max(arg_min.z, z);
			w = max(arg_min.w, w);

			x = min(arg_max.x, x);
			y = min(arg_max.y, y);
			z = min(arg_max.z, z);
			w = min(arg_max.w, w);
			return *this;
		}
		inline Vector4 GetClamp(const Vector4& arg_max, const Vector4& arg_min)const {
			Vector4 output;
			output.x = max(arg_min.x, x);
			output.y = max(arg_min.y, y);
			output.z = max(arg_min.z, z);
			output.w = max(arg_min.w, w);

			output.x = min(arg_max.x, x);
			output.y = min(arg_max.y, y);
			output.z = min(arg_max.z, z);
			output.w = min(arg_max.w, w);
			return *this;
		}

		inline void Normalize()
		{
			auto length = GetLength();
			if (length) {
				x /= length;
				y /= length;
				z /= length;
				w /= length;
			}

		}
		inline Vector4 GetNormalize()const
		{
			Vector4 output = *this;
			output.Normalize();
			return output;

		}

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(x, y, z, w);
		}

#ifdef USE_DIRECTXMATH

		//DirectXMath‚Æ‚ÌŒÝŠ·«
		inline operator DirectX::XMVECTOR() const
		{
			DirectX::XMVECTOR output = DirectX::XMLoadFloat4((XMFLOAT4*)((void*)this));
			return output;
		}
		inline Vector4(const DirectX::XMVECTOR other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat4((XMFLOAT4*)((void*)this), temp);
		}
		inline Vector4(const DirectX::XMFLOAT4 other)
		{
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			this->w = other.w;
		}
		inline Vector4& operator=(const DirectX::XMVECTOR& other)
		{
			DirectX::XMVECTOR temp = other;
			DirectX::XMStoreFloat4((XMFLOAT4*)((void*)this), temp);
			return *this;
		};

#endif

		float x, y, z, w;
	};

	struct Quat : public Vector4
	{

		inline Quat() :Vector4()
		{
			Identity();
		}
		inline Quat(const Quat& quat) :
			Vector4(quat.x, quat.y, quat.z, quat.w)
		{
		}
		explicit inline Quat(float x, float y, float z, float w) :
			Vector4(x, y, z, w)
		{
		}

		explicit inline Quat(const Vector3& axis, float radians) :
			Vector4()
		{
			this->w = cos(radians * 0.5f);
			float sinedRad = sin(radians * 0.5f);
			this->x = axis.x * sinedRad;
			this->y = axis.y * sinedRad;
			this->z = axis.z * sinedRad;
		}

		explicit  inline Quat(float val) :
			Vector4()
		{
			x = val;
			y = val;
			z = val;
			w = val;
		}

		explicit inline Quat(const Matrix4x4& m) {
			*this = m.ToQuat();
		}


		inline Quat& operator =(const Quat& other)
		{
			if (this != &other) {
				x = other.x;
				y = other.y;
				z = other.z;
				w = other.w;
			}
			return *this;
		}




		inline Quat& SetXYZ(const Vector3& other)
		{
			x = other.x;
			y = other.y;
			z = other.z;
			return *this;
		}

		inline const Vector3 GetXYZ() const
		{
			return Vector3(x, y, z);
		}


		inline float& operator [](int idx)
		{
			return *(&x + idx);
		}


		inline float operator [](int idx) const
		{
			return *(&x + idx);
		}


		inline Quat operator +(const Quat& other) const
		{
			return Quat(this->x + other.x, this->y + other.y, this->z + other.z, this->w + other.w);
		}

		inline Quat operator -(const Quat& other) const
		{
			return Quat(this->x - other.x, this->y - other.y, this->z - other.z, this->w - other.w);
		}

		inline Quat operator *(const Quat& other) const
		{
			float px = this->x, py = this->y, pz = this->z, pw = this->w;
			float qx = other.x, qy = other.y, qz = other.z, qw = other.w;
			return Quat(qw*px -qz*py + qy*pz + qx*pw, qz*px + qw*py-qx*pz + qy*pw,
				-qy*px + qx*py + qw*pz + qz*pw, -qx*px-qy*py-qz*pz + qw*pw);
		}
		inline Quat operator /(const Quat& other) const
		{
			float ax = this->x, ay = this->y, az = this->z, aw = this->w;
			float bx = -other.x, by = -other.y, bz = -other.z, bw = -other.w;
			return Quat(ax * bw + aw * bx + ay * bz - az * by, ay * bw + aw * by + az * bx - ax * bz,
				az * bw + aw * bz + ax * by - ay * bx, aw * bw - ax * bx - ay * by - az * bz);
		}


		inline Quat& operator *=(const Quat& other) {
			float ax = this->x, ay = this->y, az = this->z, aw = this->w;
			float bx = other.x, by = other.y, bz = other.z, bw = other.w;
			this->x = (ax * bw + aw * bx + ay * bz - az * by);
			this->y = (ay * bw + aw * by + az * bx - ax * bz);
			this->z = (az * bw + aw * bz + ax * by - ay * bx);
			this->w=(aw * bw - ax * bx - ay * by - az * bz);
			return *this;
		}
		inline Quat& operator /=(const Quat& other) {
			float ax = this->x, ay = this->y, az = this->z, aw = this->w;
			float bx = -other.x, by = -other.y, bz = -other.z, bw = -other.w;
			this->x = (ax * bw + aw * bx + ay * bz - az * by);
			this->y = (ay * bw + aw * by + az * bx - ax * bz);
			this->z = (az * bw + aw * bz + ax * by - ay * bx);
			this->w = (aw * bw - ax * bx - ay * by - az * bz);
			return *this;
		}

		inline Quat operator *(float value) const
		{
			return Quat(this->x * value, this->y * value, this->z * value, this->w * value);
		}
		inline Quat operator /(float value) const
		{
			return Quat(this->x / value, this->y / value, this->z / value, this->w / value);
		}
		inline const Quat& operator *=(float value)
		{
			*this = *this * value;
			return *this;
		}
		inline const Quat& operator /=(float value)
		{
			*this = *this / value;
			return *this;
		}

		inline const Quat operator -() const
		{
			return   (*this) * -1;
		}
		inline bool operator==(const Quat& other)const {
			return this->x == other.x && this->y == other.y && this->z == other.z && this->w == other.w;
		}

		inline bool operator!=(const Quat& other)const {
			return !((*this) == other);
		}

		inline const Quat& CreateFromAxisRotate(const Vector3& axis, const  float radians) {
			this->w = cos(radians * 0.5f);
			float sinedRad = sin(radians * 0.5f);
			this->x = axis.x * sinedRad;
			this->y = axis.y * sinedRad;
			this->z = axis.z * sinedRad;
			return *this;
		}

		inline Matrix4x4 ToMatrix()const {
			return Matrix4x4(*this);
		}
		inline Quat& CreateFromEular(const Vector3& arg_eular) {

			Matrix4x4 rotationMatrix = Matrix4x4::RollZ(
				arg_eular.z
			) *
				Matrix4x4::RollY(
					arg_eular.y
				) *
				Matrix4x4::RollX(
					arg_eular.x
				);
			*this = rotationMatrix.ToQuat();
			return *this;
		}
		inline Quat& CreateFromEular_local_deg(const Vector3& arg_eular) {

			Matrix4x4 rotationMatrix = Matrix4x4::RollZ(
				(arg_eular.z) * (BM_PI / 180.0f)
			) *
				Matrix4x4::RollY(
					(arg_eular.y) * (BM_PI / 180.0f)
				) *
				Matrix4x4::RollX(
					(arg_eular.x) * (BM_PI / 180.0f)
				);
			*this = rotationMatrix.ToQuat();
			return *this;
		}
		inline Quat& CreateFromEular_deg(const Vector3& arg_eular) {

			Matrix4x4 rotationMatrix = Matrix4x4::RollX(
				(arg_eular.x) * (BM_PI / 180.0f)
			) *
				Matrix4x4::RollY(
					(arg_eular.y) * (BM_PI / 180.0f)
				) *
				Matrix4x4::RollZ(
					(arg_eular.z) * (BM_PI / 180.0f)
				);
			*this = rotationMatrix.ToQuat();
			return *this;
		}


		inline Quat& Normalize() {
			Vector4::Normalize();
			return *this;
		}

		inline float Dot(const Quat& other)const {
			return Vector4::Dot(other);
		}

		inline Quat& Conj(const Quat& other) {
			this->x = -other.x;
			this->y = -other.y;
			this->z = -other.z;
			this->w = other.w;
			return *this;
		}
		inline Quat& Conj() {
			this->x = -this->x;
			this->y = -this->y;
			this->z = -this->z;
			return *this;
		}
		inline  Quat& Identity() {
			this->x = 0;
			this->y = 0;
			this->z = 0;
			this->w = 1;
			return *this;
		}


		inline  Vector3 ToRotVec() const {
			Quat Temp = *this;
			Temp.Normalize();
			Matrix4x4 mt(Temp);
			Vector3 Rot;
			if (mt._32 == 1.0f) {
				Rot.x = BM_PI / 2.0f;
				Rot.y = 0;
				Rot.z = -atan2(mt._21, mt._11);
			}
			else if (mt._32 == -1.0f) {
				Rot.x = -BM_PI / 2.0f;
				Rot.y = 0;
				Rot.z = -atan2(mt._21, mt._11);
			}
			else {
				Rot.x = -asin(mt._32);
				Rot.y = -atan2(-mt._31, mt._33);
				Rot.z = atan2(mt._12, mt._11);
			}
			return Rot;
		}

		inline Quat& Inverse() {
			float LengthSq = this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;

			this->x = -this->x / LengthSq;
			this->y = -this->y / LengthSq;
			this->z = -this->z / LengthSq;
			this->w = this->w / LengthSq;
			return *this;
		}
		inline Quat GetInverse()const {
			Quat output;
			float LengthSq = this->x * this->x + this->y * this->y + this->z * this->z + this->w * this->w;

			output.x = -this->x / LengthSq;
			output.y = -this->y / LengthSq;
			output.z = -this->z / LengthSq;
			output.w = this->w / LengthSq;
			return output;
		}

#ifdef USE_DIRECTXMATH
		//DirectXMath‚Æ‚ÌŒÝŠ·
		inline Quat& operator=(const XMFLOAT4& other) {
			this->x = other.x;
			this->y = other.y;
			this->z = other.z;
			this->w = other.w;
			return *this;
		}
		explicit inline Quat(const XMFLOAT4& v) :
			Vector4(v)
		{
		}
		explicit inline Quat(const XMVECTOR& other) : Vector4()
		{
			XMVECTOR temp = other;
			XMStoreFloat4((XMFLOAT4*)((void*)this), temp);
		}
		inline operator XMVECTOR() const {
			return XMLoadFloat4((XMFLOAT4*)((void*)this));
		}
		inline Quat& operator=(const XMVECTOR& other) {
			XMVECTOR temp = other;
			XMStoreFloat4((XMFLOAT4*)((void*)this), temp);
			return *this;
		}
#endif
		
	};


	static Vector2 operator* (float value, const Vector2& other) {
		return Vector2(other.x * value, other.y * value);
	}
	static Vector3 operator* (float value, const Vector3& other) {
		return Vector3(other.x * value, other.y * value, other.z * value);
	}
	static Vector4 operator* (float value, const Vector4& other) {
		return Vector4(other.x * value, other.y * value, other.z * value, other.w * value);
	}
	static Quat operator* (float value, const Quat& other) {
		return Quat(other.x * value, other.y * value, other.z * value, other.w * value);
	}

	namespace MathHelper
	{

		static  Matrix4x4 GetLookAtRotation(const Vector3& arg_lookPos,const Vector3& arg_targetPos, const Vector3& arg_upAxis) {
			Vector3 z = ((Vector3)(arg_targetPos - arg_lookPos)).GetNormalize();
			Vector3 x = arg_upAxis.GetCross(z).GetNormalize();
			Vector3 y = z.GetCross(x).GetNormalize();

			auto out = Matrix4x4();
			out._11 = x.x; out._12 = x.y; out._13 = x.z;
			out._21 = y.x; out._22 = y.y; out._23 = y.z;
			out._31 = z.x; out._32 = z.y; out._33 = z.z;


			return out;
		}


		static Vector3 Slide(const Vector3& vec, const Vector3& normal)
		{
			float Len = vec.Dot( normal);
			Vector3 Contact = normal * Len;
			return (vec - Contact);
		}

		

		static Quat LearpQuat(const Quat& arg_firstQuat, const Quat& arg_secondQuat, const float pase) {
			Quat secQ = arg_secondQuat;
			if (arg_firstQuat.Dot(secQ) <= 0) {
				secQ = -secQ;
			}
			Quat out = Quat();
			const float len1 = arg_firstQuat.GetLength();
			const float len2 = arg_firstQuat.GetLength();

			if (len1 == 0.0f || len2 == 0.0f)
				return out;

			const float cos_val = (arg_firstQuat[0] * secQ[0] + arg_firstQuat[1] * secQ[1] + arg_firstQuat[2] * secQ[2] + arg_firstQuat[3] * secQ[3]) / (len1 * len2);


			if (abs(cos_val - 1.0f) < 0.001) {
				return arg_firstQuat;
			}
			const float w = acosf(cos_val);
			const float sin_w = sinf(w);
			const float sin_t_w = sinf(pase * w);
			const float sin_inv_t_w = sinf((1.0f - pase) * w);
			const float mult_q1 = sin_inv_t_w / sin_w;
			const float mult_q2 = sin_t_w / sin_w;


			for (int i = 0; i < 4; i++)
				out[i] = mult_q1 * arg_firstQuat[i] + mult_q2 * secQ[i];

			return out;
		}
		static Vector3 LarpPosition(const Vector3& arg_startPoint, const Vector3& arg_endPoint, const float t) {
			return arg_startPoint + (arg_endPoint - arg_startPoint) * t;

		}

		static Vector3 LarpPosition(const Vector3& arg_startPoint, const Vector3& arg_endPoint, const float xt, const float yt, const float zt) {
			return Vector3(arg_startPoint.x + (arg_endPoint.x - arg_startPoint.x) * xt, arg_startPoint.y + (arg_endPoint.y - arg_startPoint.y) * yt, arg_startPoint.z + (arg_endPoint.z - arg_startPoint.z) * zt);

		}

		static Vector3 GetMobiusPoint(const float arg_time,const float arg_radius) {
			Vector3 output;

			output.x = -(arg_radius * cos(arg_time) + 2) * sin(2 * arg_time);
			output.y = (arg_radius * cos(arg_time) + 2) * cos(2 * arg_time);
			output.z = arg_radius * sin(arg_time);

			return output;
		}
		static Vector3 GetMobiusNormal(const float arg_time, const float arg_radius) {
			if (arg_radius == 0) {

				Vector3 point1 = GetMobiusPoint(arg_time, arg_radius), point2 = GetMobiusPoint(arg_time, -1), point3 = GetMobiusPoint(arg_time + ToRadian(0.5f), arg_radius);

				return (point2 - point1).GetCross(point3 - point1).GetNormalize();
			}
			else {
				Vector3 point1 = GetMobiusPoint(arg_time, arg_radius), point2 = GetMobiusPoint(arg_time, 0), point3 = GetMobiusPoint(arg_time + ToRadian(0.5f), arg_radius);

				return (point2 - point1).GetCross(point3 - point1).GetNormalize();
			}
		}
	};



	inline Vector3 ButiEngine::Vector3::operator*(const Quat& other)
	{
		return (Vector3)(*this)*(other.ToMatrix());
	}
	inline ButiEngine::Vector3::operator Vector4()const {
		return Vector4(x, y, z, 1.0f);
	}

	inline ButiEngine::Vector2::operator Vector3() const
	{
		return Vector3(x, y, 0.0f);
	}
	inline ButiEngine::Vector2::operator Vector4() const
	{
		return Vector4(x, y, 0.0f,1.0f);
	}
	struct Line {
		Vector3 point;
		Vector3 velocity;
		Line() {}
		Line(const Vector3& p, const Vector3& v) :point(p), velocity(v) {}


		inline virtual Vector3 GetPoint(float t) const {
			return point + velocity * t;
		}

		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(point);
			archive(velocity);

		}
	};

	struct Segment : public Line {
		Segment() {}
		Segment(const Vector3& startPoint, const Vector3& endPoint) :Line(startPoint,((Vector3)( endPoint - startPoint)).GetNormalize()), endPos(endPoint) {}


		inline Vector3 GetEndPoint() const {
			return endPos;
		}
		inline Vector3 GetPoint(float t) const override {
			return point + (endPos - point) * t;
		}
		float Length()const {
			return ((Vector3)(endPos - point)).GetLength();
		}
		float LengthSqr()const {
			return ((Vector3)(endPos - point)).GetLengthSqr();
		}
		Vector3 endPos;
	};

	struct QuadraticBezierCurve {
	public:
		QuadraticBezierCurve(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC) :lineAB(pointA, pointB), lineBC(pointB, pointC) {}
		inline Vector3 GetPoint(float t) {
			quadratic.point = lineAB.GetPoint(t);
			quadratic.endPos = lineBC.GetPoint(t);

			return quadratic.GetPoint(t);
		}
	private:
		Segment lineAB;
		Segment lineBC;
		Segment quadratic;
	};
	struct CubicBezierCurve {
	public:
		CubicBezierCurve(const Vector3& pointA, const Vector3& pointB, const Vector3& pointC, const Vector3& pointD) :curveABC(pointA, pointB, pointC), curveBCD(pointB, pointC, pointD) {}

		inline Vector3 GetPoint(float t) {
			cubic.point = curveABC.GetPoint(t);
			cubic.endPos = curveBCD.GetPoint(t);

			return cubic.GetPoint(t);
		}
	private:
		QuadraticBezierCurve curveABC;
		QuadraticBezierCurve curveBCD;
		Segment cubic;
	};

	struct Line2D {
		Vector2 point;
		Vector2 velocity;
		Line2D() {}
		Line2D(const Vector2& p, const Vector2& v) :point(p), velocity(v) {}


		inline virtual Vector2 GetPoint(float t) const {
			return point + velocity * t;
		}
	};

	struct Segment2D : public Line2D {
		Segment2D() {}
		Segment2D(const Vector2& startPoint, const Vector2& endPoint) :Line2D(startPoint, endPoint - startPoint), endPos(endPoint) {}


		inline Vector2 GetEndPoint() const {
			return endPos;
		}
		inline Vector2 GetPoint(float t) const override {
			return point + (endPos - point) * t;
		}
		Vector2 endPos;
	};

	struct QuadraticBezierCurve2D {
	public:
		QuadraticBezierCurve2D() {}
		QuadraticBezierCurve2D(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC) :lineAB(pointA, pointB), lineBC(pointB, pointC) {}
		inline Vector2 GetPoint(float t) {
			quadratic.point = lineAB.GetPoint(t);
			quadratic.endPos = lineBC.GetPoint(t);

			return quadratic.GetPoint(t);
		}
		Segment2D lineAB;
		Segment2D lineBC;
	private:
		Segment2D quadratic;
	};
	struct CubicBezierCurve2D {
	public:
		CubicBezierCurve2D(const Vector2& pointA, const Vector2& pointB, const Vector2& pointC, const Vector2& pointD)
		{
			curveABC = QuadraticBezierCurve2D(pointA, pointB, pointC);
			curveBCD = QuadraticBezierCurve2D(pointB, pointC, pointD);
		}

		inline Vector2 GetPoint(float t) {
			cubic.point = curveABC.GetPoint(t);
			cubic.endPos = curveBCD.GetPoint(t);

			return cubic.GetPoint(t);
		}

		inline float GetDerivative(float t) {
			return (3.0f * curveABC.lineAB.point.x * t + 2.0f * curveABC.lineAB.endPos.x) * t + curveABC.lineBC.endPos.x;
		}

		inline float GetYFromNuton(float x) {
			float epsilon = 1e-5f; // è‡’l
			float x2, t0, t1, t2, d2, i;
			for (t2 = x, i = 0; i < 16; i++) {
				x2 = GetPoint(t2).x - x;
				if (abs(x2) < epsilon) {
					return GetPoint(t2).y;
				}
				d2 = GetDerivative(t2);
				if (abs(d2) < 1e-6f) {
					break;
				}
				t2 = t2 - x2 / d2;
			}
			t0 = 0.0f;
			t1 = 1.0f;
			t2 = x;
			if (t2 < t0) {
				return GetPoint(t0).y;
			}
			if (t2 > t1) {
				return GetPoint(t1).y;
			}
			while (t0 < t1) {
				x2 = GetPoint(t2).x;
				if (abs(x2 - x) < epsilon) {
					return GetPoint(t2).y;
				}
				if (x > x2) {
					t0 = t2;
				}
				else {
					t1 = t2;
				}
				t2 = (t1 - t0) * 0.5f + t0;
			}


			return  GetPoint(t2).y;
		}

		inline float GetYFromHalf(const float x, const char testCount = 12) {

			Vector2 a = curveABC.lineBC.endPos;
			Vector2 b = curveABC.lineAB.endPos;
			if (a.x == a.y && b.x == b.y) {
				return x;
			}
			float t = x;

			const float k0 = 1 + 3 * a.x - 3 * b.x;
			const float k1 = 3 * b.x - 6 * a.x;
			const float k2 = 3 * a.x;

			float epsilon = 0.0005f; // è‡’l

			for (int i = 0; i < testCount; i++) {
				auto ft = k0 * t * t * t + k1 * t * t + k2 * t - x;

				if (ft <= epsilon && ft >= -epsilon) {
					break;
				}
				t -= t / 2;
			}
			auto r = 1 - t;

			return t * t * t + 3 * t * t * r * b.y + 3 * t * r * r * a.y;
		}
	private:
		QuadraticBezierCurve2D curveABC;
		QuadraticBezierCurve2D curveBCD;
		Segment2D cubic;
	};


	struct SplineCurve {
	public:
		SplineCurve() {
			for (int i = 0; i < 6; i++)
				vec_points.push_back(Vector3(0, 0, 0));
			Initialize();
		}
		SplineCurve(const std::vector<Vector3>& arg_vec_points) {
			vec_points = arg_vec_points;
			Initialize();
		}
		SplineCurve(Vector3 start, Vector3 end, const std::vector<Vector3>& arg_vec_points) {
			vec_points.push_back(start);
			vec_points.push_back(start);

			std::copy(arg_vec_points.begin(), arg_vec_points.end(), std::back_inserter(vec_points));

			vec_points.push_back(end);
			vec_points.push_back(end);
			Initialize();
		}
		SplineCurve(Vector3 startAndEnd, std::vector<Vector3> arg_vec_points) {
			vec_points.push_back(startAndEnd);
			vec_points.push_back(startAndEnd);

			std::copy(arg_vec_points.begin(), arg_vec_points.end(), std::back_inserter(vec_points));

			vec_points.push_back(startAndEnd);
			vec_points.push_back(startAndEnd);
			Initialize();
		}
		bool IsLoop()const {
			return vec_points.at(0) == vec_points.at(vec_points.size()-1);
		}

		Vector3 GetPoint(float t) const{
			{
				UINT itr =(UINT) (t / unit);

				if (itr >= vec_points.size() - 3) {
					itr =(UINT) vec_points.size() - 4;
				}


				return CatmullRom((t - itr * unit) / unit, vec_points[itr], vec_points[(unsigned long long int)itr + 1], vec_points[(unsigned long long int)itr + 2], vec_points[(unsigned long long int)itr + 3]);

			}
			t += 0.01f;

			if (t > 1.0f) {
				t = 0;
			}
			return vec_points[vec_points.size() - 1];
		}

		bool ShowUI();

		void Initialize() {
			unit = 1.0f / (vec_points.size() - 3);
		}


		template<class Archive>
		void serialize(Archive& archive)
		{
			archive(vec_points);
			archive(unit);
		}

	private:
		static Vector3 CatmullRom(float t, Vector3 p0, Vector3 p1, Vector3 p2, Vector3 p3) {
			Vector3 a = -p0 + 3.0f * p1 - 3.0f * p2 + p3;
			Vector3 b = 2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3;
			Vector3 c = -p0 + p2;
			Vector3 d = 2.0f * p1;

			return 0.5f * ((a * t * t * t) + (b * t * t) + (c * t) + d);
		}

		std::vector<Vector3> vec_points;
		float unit = 0.0f;
	};

	////////////////////////////////////////////////////

	inline Vector4 ButiEngine::Matrix4x4::operator*(const Vector4& other)
	{

		Vector4 temp = Vector4(this->_11 * other.x + this->_21 * other.y + this->_31 * other.z + this->_41 * other.w,
			this->_12 * other.x + this->_22 * other.y + this->_32 * other.z + this->_42 * other.w,
			this->_13 * other.x + this->_23 * other.y + this->_33 * other.z + this->_43 * other.w,
			this->_14 * other.x + this->_24 * other.y + this->_34 * other.z + this->_44 * other.w
		);
		return temp;
	}
	inline Vector3 ButiEngine::Matrix4x4::operator*(const Vector3& other)
	{

		Vector3 temp = Vector3(this->_11 * other.x + this->_21 * other.y + this->_31 * other.z + this->_41,
			this->_12 * other.x + this->_22 * other.y + this->_32 * other.z + this->_42 ,
			this->_13 * other.x + this->_23 * other.y + this->_33 * other.z + this->_43 
		);
		return temp;
	}

	inline Vector4& Matrix4x4::operator[](const unsigned int idx)
	{
		return *((Vector4*) (&m[idx][0]));
	}

	inline Vector4 Matrix4x4::operator[](const unsigned int idx) const
	{
		return *((Vector4*)(&m[idx][0]));
	}

	inline ButiEngine::Matrix4x4::Matrix4x4(const Quat& other)
	{
		if (!other.IsNaN()) {

			float x2 = other.x + other.x, y2 = other.y + other.y, z2 = other.z + other.z;
			float xx = other.x * x2, xy = other.x * y2, xz = other.x * z2;
			float yy = other.y * y2, yz = other.y * z2, zz = other.z * z2;
			float wx = other.w * x2, wy = other.w * y2, wz = other.w * z2;
			this->_11 = 1 - (yy + zz);
			this->_21 = xy - wz;
			this->_31 = xz + wy;
			this->_41 = 0;
			this->_12 = xy + wz;
			this->_22 = 1 - (xx + zz);
			this->_32 = yz - wx;
			this->_42 = 0;
			this->_13 = xz - wy;
			this->_23 = yz + wx;
			this->_33 = 1 - (xx + yy);
			this->_34 = 0;
			this->_14 = 0;
			this->_24 = 0;
			this->_34 = 0;
			this->_44 = 1;
		}
		else {
			_11 = 1.0f;
			_12 = 0.0f;
			_13 = 0.0f;
			_14 = 0.0f;

			_21 = 0.0f;
			_22 = 1.0f;
			_23 = 0.0f;
			_24 = 0.0f;

			_31 = 0.0f;
			_32 = 0.0f;
			_33 = 1.0f;
			_34 = 0.0f;

			_41 = 0.0f;
			_42 = 0.0f;
			_43 = 0.0f;
			_44 = 1.0f;
		}
	}
	inline ButiEngine::Vector3 ButiEngine::Matrix4x4::GetPosition()const
	{
		return Vector3(_41, _42, _43);
	}
	inline ButiEngine::Vector3 ButiEngine::Matrix4x4::GetPosition_Transpose()const
	{
		return Vector3(_14, _24, _34);
	}
	inline void Matrix4x4::PositionFloor()
	{
		if (_41 > 0) {
			_41 = floor(_41);
		}
		else {
			_41 = ceil(_41);
		}
		if (_42 > 0) {
			_42 = floor(_42);
		}
		else {
			_42 = ceil(_42);
		}
		if (_43 > 0) {
			_43 = floor(_43);
		}
		else {
			_43 = ceil(_43);
		}
	}
	inline Matrix4x4 Matrix4x4::GetPositionFloor() const
	{
		Matrix4x4 output = *this;
		output.PositionFloor();
		return output;
	}
	inline void Matrix4x4::PositionFloor_transpose()
	{
		if (_14 > 0) {
			_14 = floor(_14);
		}
		else {
			_14 = ceil(_14);
		}
		if (_24 > 0) {
			_24 = floor(_24);
		}
		else {
			_24 = ceil(_24);
		}
		if (_34 > 0) {
			_34 = floor(_34);
		}
		else {
			_34 = ceil(_34);
		}
	}
	inline void Matrix4x4::PositionXFloor_transpose()
	{
		if (_14 > 0) {
			_14 = floor(_14);
		}
		else {
			_14 = ceil(_14);
		}
	}
	inline void Matrix4x4::PositionYFloor_transpose()
	{
		if (_24 > 0) {
			_24 = floor(_24);
		}
		else {
			_24 = ceil(_24);
		}
	}
	inline void Matrix4x4::PositionZFloor_transpose()
	{
		if (_34 > 0) {
			_34 = floor(_34);
		}
		else {
			_34 = ceil(_34);
		}
	}
	inline void Matrix4x4::PositionXYFloor_transpose()
	{
		if (_14 > 0) {
			_14 = floor(_14);
		}
		else {
			_14 = ceil(_14);
		}
		if (_24 > 0) {
			_24 = floor(_24);
		}
		else {
			_24 = ceil(_24);
		}
	}
	inline void Matrix4x4::PositionYZFloor_transpose()
	{
		if (_24 > 0) {
			_24 = floor(_24);
		}
		else {
			_24 = ceil(_24);
		}
		if (_34 > 0) {
			_34 = floor(_34);
		}
		else {
			_34 = ceil(_34);
		}
	}
	inline void Matrix4x4::PositionXZFloor_transpose()
	{
		if (_14 > 0) {
			_14 = floor(_14);
		}
		else {
			_14 = ceil(_14);
		}
		if (_34 > 0) {
			_34 = floor(_34);
		}
		else {
			_34 = ceil(_34);
		}
	}
	inline Matrix4x4 Matrix4x4::GetPositionFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionXFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionXFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionYFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionYFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionZFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionZFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionXYFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionXYFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionYZFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionYZFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetPositionXZFloor_transpose() const
	{
		Matrix4x4 output = *this;
		output.PositionXZFloor_transpose();
		return output;
	}
	inline Matrix4x4 Matrix4x4::GetOnlyRotation_transpose() const
	{
		Matrix4x4 output;
		output._14 = this->_14;
		output._24 = this->_24;
		output._34 = this->_34;
		output._44 = this->_44;

		return output;
	}
	inline void  ButiEngine::Matrix4x4::RemovePosition()
	{
		_41 = 0;
		_42 = 0;
		_43 = 0;
	}
	inline Matrix4x4 ButiEngine::Matrix4x4::Translate(const Vector3& arg_position)
	{
		Matrix4x4 output;
		output._41 = arg_position.x;
		output._42 = arg_position.y;
		output._43 = arg_position.z;
		return output;
	}
	inline Matrix4x4 ButiEngine::Matrix4x4::Scale(const Vector3& arg_scale)
	{
		Matrix4x4 output;
		output._11 = arg_scale.x;
		output._22 = arg_scale.y;
		output._33 = arg_scale.z;
		return output;
	}
	inline Quat ButiEngine::Matrix4x4::ToQuat() const
	{
		Quat output;
		float elem[4];
		elem[0] = this->_11 - this->_22 - this->_33 + 1.0f;
		elem[1] = -this->_11 + this->_22 - this->_33 + 1.0f;
		elem[2] = -this->_11 - this->_22 + this->_33 + 1.0f;
		elem[3] = this->_11 + this->_22 + this->_33 + 1.0f;

		unsigned biggestIndex = 0;
		for (int i = 1; i < 4; i++) {
			if (elem[i] > elem[biggestIndex])
				biggestIndex = i;
		}



		float v = sqrtf(elem[biggestIndex]) * 0.5f;
		output[biggestIndex] = v;
		float mult = 0.25f / v;

		switch (biggestIndex) {
		case 0: // x
			output[1] = (this->_12 + this->_21) * mult;
			output[2] = (this->_31 + this->_13) * mult;
			output[3] = (this->_23 - this->_32) * mult;
			break;
		case 1: // y
			output[0] = (this->_12 + this->_21) * mult;
			output[2] = (this->_23 + this->_32) * mult;
			output[3] = (this->_31 - this->_13) * mult;
			break;
		case 2: // z
			output[0] = (this->_31 + this->_13) * mult;
			output[1] = (this->_23 + this->_32) * mult;
			output[3] = (this->_12 - this->_21) * mult;
			break;
		case 3: // w
			output[0] = (this->_23 - this->_32) * mult;
			output[1] = (this->_31 - this->_13) * mult;
			output[2] = (this->_12 - this->_21) * mult;
			break;
		}

		return output;
	}

	inline Matrix4x4& Matrix4x4::CreateFromEuler(const Vector3& arg_euler)
	{
		*this = RollX(arg_euler.x) *RollY(arg_euler.y) *RollZ(arg_euler.z);
		return *this;
	}
	inline Matrix4x4& Matrix4x4::CreateFromEuler_local(const Vector3& arg_euler)
	{
		*this = RollZ(arg_euler.z) *RollY(arg_euler.y) *RollX(arg_euler.x);
		return *this;
	}

	static const float EulerXLimit = 90.0f / 180.0f * BM_PI;
	inline Vector3 Matrix4x4::GetEulerOneValue_local() const
	{

		Vector3 Rot;
		if (this->_31 == 1.0f) {
			Rot.x = atan2(-this->_32,this->_33);
			Rot.y = BM_PI / 2.0f;
			Rot.z = 0;
		}
		else if (this->_31 == -1.0f) {
			Rot.x = atan2(-this->_32,this->_33);
			Rot.y = -BM_PI / 2.0f;
			Rot.z = 0;
		}
		else {


			Rot.x = atan2(-this->_32, this->_33);
			Rot.y = asin(this->_31);
			Rot.z = -atan2(this->_21, this->_11);///-90~90
		}

		return Rot;


	}
	inline Vector3 Matrix4x4::GetEulerOneValue() const
	{
		Vector3 Rot;
		if (this->_13 == 1.0f) {
			Rot.x = atan2(this->_23 , this->_33);
			Rot.y = BM_PI / 2.0f;
			Rot.z = 0;
		}
		else if (this->_13 == -1.0f) {
			Rot.x = atan2(this->_23 , this->_33 );
			Rot.y = -BM_PI / 2.0f;
			Rot.z = 0;
		}
		else {


			Rot.x = atan2(this->_23 , this->_33);
			Rot.y = -asin(this->_13);
			Rot.z = atan2(this->_12 , this->_11 );///-90~90
		} 

		return Rot;
	}


	using Color=Vector4 ;
	using Point2D=Vector2;

	class Rectangle {
	public:
		float width;
		float height;
		float outerCircleRadius;
		Vector2 position;
		inline Rectangle() :width(0), height(0), position(Vector2()), outerCircleRadius(0) {};
		inline Rectangle(float arg_width, float  arg_height, Vector2 arg_position) :width(arg_width), height(arg_height), position(arg_position), outerCircleRadius(GetRectangleOuterCircleRadius((int)width, (int)height)) {};
		inline Rectangle(float arg_width, float  arg_height, Vector2 arg_position, float arg_outerCircleRadius) :width(arg_width), height(arg_height), position(arg_position), outerCircleRadius(arg_outerCircleRadius) {};
		inline Rectangle(float arg_width, float  arg_height, float x, float y) :width(arg_width), height(arg_height), position(Vector2(x, y)), outerCircleRadius(GetRectangleOuterCircleRadius((int)width, (int)height)) {};
		inline std::vector< Point2D> GetVertecies()
		{
			std::vector<Point2D> out{
				   Point2D(position.x - width / 2, position.y - height / 2),
				   Point2D(position.x + width / 2, position.y - height / 2),
				   Point2D(position.x + width / 2, position.y + height / 2),
				   Point2D(position.x - width / 2, position.y + height / 2)
			};

			return out;
		}


		inline bool IsContain(Point2D arg_point2D) {
			if (arg_point2D.x >= position.x - width / 2 && arg_point2D.x <= position.x + width / 2
				&& arg_point2D.y >= position.y - height / 2 && arg_point2D.y <= position.y + height / 2) {
				return true;
			}
			return false;
		}
		inline bool IsIntersects(std::shared_ptr< Rectangle> other) {

			/*auto d = other->position.GetDistance(position);
			if (other->position.GetDistance(position) > other->outerCircleRadius + outerCircleRadius) {
				return false;
			}*/
			auto otherVertecies = other->GetVertecies();
			for (auto i = 0; i < 4; i++)
			{
				if (IsContain(otherVertecies.at(i))) {
					return true;
				}
			}
			return false;
		}
		inline float GetTop() {

			return position.y - height / 2;
		}
		inline float GetBottom() {

			return position.y + height / 2;
		}
		inline float GetLeft() {
			return position.x - width / 2;
		}
		inline float GetRight() {

			return position.x + width / 2;
		}
		inline static float GetRectangleOuterCircleRadius(const int width, const int height) {
			auto aW = width; auto aH = height;
			auto s = (aW + aH);
			auto t = (s - aW) * (s - aH) * (s - aW) * (s - aH);
			auto u = (aW * aH * 2) * (aW * aH * 2) * (aW * aW + aH * aH);


			auto output = std::sqrt(u / t) / 4;
			return (float)output;
		}
		inline static float GetRectangleOuterCircleRadius(const Rectangle& ref_rect) {
			auto s = (ref_rect.width + ref_rect.height);
			auto t = (s - ref_rect.width) * (s - ref_rect.height) * (s - ref_rect.width) * (s - ref_rect.height);
			auto u = (ref_rect.width * ref_rect.height * 2) * (ref_rect.width * ref_rect.height * 2) * (ref_rect.width * ref_rect.width + ref_rect.height * ref_rect.height);
			auto output = std::sqrt(u / t) / 4;
			return output;
		}
	};


	void OutputCereal(const Vector2& v, const std::string& path);

	void InputCereal(Vector2& v, const std::string& path);

	void OutputCereal(const Vector3& v, const std::string& path);

	void InputCereal(Vector3& v, const std::string& path);

	void OutputCereal(const Vector4& v, const std::string& path);

	void InputCereal(Vector4& v, const std::string& path);

	void OutputCereal(const Matrix4x4& v, const std::string& path);

	void InputCereal(Matrix4x4& v, const std::string& path);

	void OutputCereal(const Line& v, const std::string& path);

	void InputCereal(Line& v, const std::string& path);



}

namespace std {
	static std::string to_string(const ButiEngine::Vector2& arg_v) {
		return std::to_string(arg_v.x) + "," + std::to_string(arg_v.y);
	}
	static std::string to_string(const ButiEngine::Vector3& arg_v) {
		return std::to_string(arg_v.x) + "," + std::to_string(arg_v.y) + "," + std::to_string(arg_v.z);
	}
	static std::string to_string(const ButiEngine::Vector4& arg_v) {
		return std::to_string(arg_v.x) + "," + std::to_string(arg_v.y) + "," + std::to_string(arg_v.z) + "," + std::to_string(arg_v.w);
	}
	static std::string to_string(const ButiEngine::Quat& arg_v) {
		return std::to_string(arg_v.x) + "," + std::to_string(arg_v.y) + "," + std::to_string(arg_v.z) + "," + std::to_string(arg_v.w);
	}
	static std::string to_string(const ButiEngine::Matrix4x4& arg_v) {
		return std::to_string(arg_v._11) + "," + std::to_string(arg_v._12) + "," + std::to_string(arg_v._13) + "," + std::to_string(arg_v._14) + "," +
			std::to_string(arg_v._21) + "," + std::to_string(arg_v._22) + "," + std::to_string(arg_v._23) + "," + std::to_string(arg_v._24) + "," +
			std::to_string(arg_v._31) + "," + std::to_string(arg_v._32) + "," + std::to_string(arg_v._33) + "," + std::to_string(arg_v._34) + "," +
			std::to_string(arg_v._41) + "," + std::to_string(arg_v._42) + "," + std::to_string(arg_v._43) + "," + std::to_string(arg_v._44);
	}
}

namespace StrConvert {
	template <typename T>
	static T ConvertString(const std::string& arg_str) {

		const char* _Ptr = arg_str.c_str();
		char* _Eptr;

		const long _Ans = _CSTD strtol(_Ptr, &_Eptr, 10);

		if (_Ptr == _Eptr) {
			//–³Œø‚È•ÏŠ·
			return 0;
		}


		return static_cast<T>(_Ans);
	}
	template <>
	static float ConvertString(const std::string& arg_str) {

		const char* _Ptr = arg_str.c_str();
		char* _Eptr;
		const float _Ans = _CSTD strtof(_Ptr, &_Eptr);

		if (_Ptr == _Eptr) {
			//–³Œø‚È•ÏŠ·
			return 0.00f;
		}
		return _Ans;
	}
	template <>
	static double ConvertString(const std::string& arg_str) {
		int& _Errno_ref = errno;
		const char* _Ptr = arg_str.c_str();
		char* _Eptr;
		_Errno_ref = 0;
		const double _Ans = _CSTD strtod(_Ptr, &_Eptr);

		if (_Ptr == _Eptr) {
			//–³Œø‚È•ÏŠ·
			return 0.00;
		}
		return _Ans;
	}
	template <>
	static ButiEngine::Vector2 ConvertString(const std::string& arg_str) {
		auto splited = std::vector<std::string>();
		int first = 0;
		int last = arg_str.find_first_of(",");
		if (last == std::string::npos){
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector2();
		}
		while (first < arg_str.size())
		{
			auto subString = arg_str.substr(first, last - first);
			splited.push_back(subString);
			first = last + 1;
			last = arg_str.find_first_of(",", first);
			if (last == std::string::npos) {
				last = arg_str.size();
			}
		}
		if (splited.size()<2) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector2();
		}


		const char* _Ptr; 
		char* _Eptr = nullptr;

		ButiEngine::Vector2 _Ans;

		for (int i = 0; i < 2; i++) {

			_Ptr = splited[i].c_str(); 
			_Eptr = nullptr;
			float f = _CSTD strtof(_Ptr, &_Eptr);
			if (_Ptr == _Eptr) {
				//–³Œø‚È•ÏŠ·
				return _Ans;
			}
			_Ans[i] = f;
		}

		return _Ans;
	}
	template <>
	static ButiEngine::Vector3 ConvertString(const std::string& arg_str) {
		auto splited = std::vector<std::string>();
		int first = 0;
		int last = arg_str.find_first_of(",");
		if (last == std::string::npos) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector3();
		}
		while (first < arg_str.size())
		{
			auto subString = arg_str.substr(first, last - first);
			splited.push_back(subString);
			first = last + 1;
			last = arg_str.find_first_of(",", first);
			if (last == std::string::npos) {
				last = arg_str.size();
			}
		}
		if (splited.size() < 3) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector3();
		}


		const char* _Ptr;
		char* _Eptr = nullptr;

		ButiEngine::Vector3 _Ans;

		for (int i = 0; i < 3; i++) {

			_Ptr = splited[i].c_str();
			_Eptr = nullptr;
			float f = _CSTD strtof(_Ptr, &_Eptr);
			if (_Ptr == _Eptr) {
				//–³Œø‚È•ÏŠ·
				return _Ans;
			}
			_Ans[i] = f;
		}



		return _Ans;
	}
	template <>
	static ButiEngine::Vector4 ConvertString(const std::string& arg_str) {
		auto splited = std::vector<std::string>();
		int first = 0;
		int last = arg_str.find_first_of(",");
		if (last == std::string::npos) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector4();
		}
		while (first < arg_str.size())
		{
			auto subString = arg_str.substr(first, last - first);
			splited.push_back(subString);
			first = last + 1;
			last = arg_str.find_first_of(",", first);
			if (last == std::string::npos) {
				last = arg_str.size();
			}
		}
		if (splited.size() < 4) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Vector4();
		}


		const char* _Ptr;
		char* _Eptr = nullptr;

		ButiEngine::Vector4 _Ans;

		for (int i = 0; i < 4; i++) {

			_Ptr = splited[i].c_str();
			_Eptr = nullptr;
			float f = _CSTD strtof(_Ptr, &_Eptr);
			if (_Ptr == _Eptr) {
				//–³Œø‚È•ÏŠ·
				return _Ans;
			}
			_Ans[i] = f;
		}


		return _Ans;
	}
	template <>
	static ButiEngine::Quat ConvertString(const std::string& arg_str) {
		auto splited = std::vector<std::string>();
		int first = 0;
		int last = arg_str.find_first_of(",");
		if (last == std::string::npos) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Quat();
		}
		while (first < arg_str.size())
		{
			auto subString = arg_str.substr(first, last - first);
			splited.push_back(subString);
			first = last + 1;
			last = arg_str.find_first_of(",", first);
			if (last == std::string::npos) {
				last = arg_str.size();
			}
		}
		if (splited.size() < 4) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Quat();		}


		const char* _Ptr;
		char* _Eptr = nullptr;

		ButiEngine::Quat _Ans;

		for (int i = 0; i < 2; i++) {

			_Ptr = splited[i].c_str();
			_Eptr = nullptr;
			float f = _CSTD strtof(_Ptr, &_Eptr);
			if (_Ptr == _Eptr) {
				//–³Œø‚È•ÏŠ·
				return _Ans;
			}
			_Ans[i] = f;
		}


		return _Ans;
	}
	template <>
	static ButiEngine::Matrix4x4 ConvertString(const std::string& arg_str) {

		auto splited = std::vector<std::string>();
		int first = 0;
		int last = arg_str.find_first_of(",");
		if (last == std::string::npos) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Matrix4x4();
		}
		while (first < arg_str.size())
		{
			auto subString = arg_str.substr(first, last - first);
			splited.push_back(subString);
			first = last + 1;
			last = arg_str.find_first_of(",", first);
			if (last == std::string::npos) {
				last = arg_str.size();
			}
		}
		if (splited.size() < 16) {
			//–³Œø‚È•ÏŠ·
			return ButiEngine::Matrix4x4();
		}
		ButiEngine::Matrix4x4 _Ans;
		for (int i = 0; i < 4; i++) {
			for (int j = 0; j < 4; j++) {

				int& _Errno_ref = errno;
				const char* _Ptr = splited[i*4+j].c_str();
				char* _Eptr;
				_Errno_ref = 0;
				float f = _CSTD strtof(_Ptr, &_Eptr);
				if (_Ptr == _Eptr) {
					//–³Œø‚È•ÏŠ·
					return _Ans;
				}
				_Ans[i][j] = f;
			}
		}


		return _Ans;
	}


}

#endif
