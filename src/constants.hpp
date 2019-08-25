#pragma once
/*
	Addresses issues: #65, #69
*/


template<typename T>
constexpr T pi() { return static_cast<T>(3.141592653589793238); }

// there's no ideal value, it is scene dependent
constexpr const float OFFSET_EPSILON = 0.0001f;