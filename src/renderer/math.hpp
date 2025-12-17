#ifndef MATH_HPP
#define MATH_HPP

#include <array>
#include <print>
#include <initializer_list>
#include <algorithm>
#include <iostream>
#include <cassert>

template <typename T>
class Vec2 {
public:
  std::array<T, 2> vector;
  Vec2() = default;
  Vec2(T t_x, T t_y) : vector({t_x, t_y}) {}

  void print() const {std::print("[0] = {0}, [1] = {1}\n", vector.at(0), vector.at(1));}

  [[nodiscard]] Vec2<T> operator+(const Vec2<T> &other) const {
	return {vector.at(0) + other.vector.at(0),
			vector.at(1) + other.vector.at(1)};
  }
  
  [[nodiscard]] Vec2<T> operator-(const Vec2<T> &other) const
  {
		return {vector.at(0) - other.vector.at(0),
			vector.at(1) - other.vector.at(1)};
  }
  
  [[nodiscard]] Vec2<T> operator*(const float S) const {
	return {static_cast<T>(S * vector.at(0)),
			static_cast<T>(S * vector.at(1))};
  }
  
  [[nodiscard]] float operator&(const Vec2<T> &other) const
  {
	return (vector.at(0) * other.vector.at(0)) + (vector.at(1) * other.vector.at(1));
  }
  
};


template <typename T>
class Vec3 {
public:
  std::array<T, 3> vector;
  Vec3() = default;
  Vec3(T t_x, T t_y, T t_z) : vector({t_x, t_y, t_z}) {}

  void print() const {std::print("[0] = {0}, [1] = {1}, [2] = {2}\n", vector.at(0), vector.at(1), vector.at(2));}

  T x() const {return vector.at(0);}
  T y() const {return vector.at(1);}
  T z() const {return vector.at(2);}

  [[nodiscard]] float mag() const
  {
	return std::hypot(vector.at(0), vector.at(1), vector.at(2));
  }

  [[nodiscard]] Vec3<T> operator+(const Vec3<T> &other) const {
	return {vector.at(0) + other.vector.at(0),
			vector.at(1) + other.vector.at(1),
			vector.at(2) + other.vector.at(2)};
  }

   [[nodiscard]] Vec3<T> operator^(const Vec3<T> &other) const {
	 return {y() * other.z() - z() * other.y(),
			 z() * other.x() - x() * other.z(),
			 x() * other.y() - y() * other.x()};
  }
  
  [[nodiscard]] Vec3<T> operator-(const Vec3<T> &other) const
  {
		return {vector.at(0) - other.vector.at(0),
				vector.at(1) - other.vector.at(1),
		        vector.at(2) - other.vector.at(2)};
  }
  
  [[nodiscard]] Vec3<T> operator*(const float S) const {
	return {static_cast<T>(S * vector.at(0)),
			static_cast<T>(S * vector.at(1)),
	        static_cast<T>(S * vector.at(2))};
  }
  
  [[nodiscard]] float operator&(const Vec3<T> &other) const
  {
	return (vector.at(0) * other.vector.at(0)) +
	       (vector.at(1) * other.vector.at(1)) +
	       (vector.at(2) * other.vector.at(2));
  }


  
};

template <typename T, std::size_t Rows, std::size_t Cols>
class Matrix {

public:
  std::array<T, Rows * Cols> data;
  
  Matrix(const std::initializer_list<T> &init_list) {
	assert(init_list.size() == Rows * Cols);
	std::copy(init_list.begin(), init_list.end(), data.begin());
  }
  Matrix() {data.fill(0);}

  void print() const {
	for(std::size_t i = 0; i < Rows; ++i){
	  for(std::size_t j = 0; j < Cols; ++j){
		std::cout << data.at((i*Cols) + j);
		if (j < (Cols - 1)) {std::cout << ", ";}
	  }
	  std::cout << std::endl;
	}
  }

  [[nodiscard]] Matrix operator+(Matrix<T, Rows, Cols> &other) {
	Matrix<T, Rows, Cols> res {};
	for (size_t i = 0; i < data.size(); ++i){
	  res.data[i] = other.data[i] + data[i];
	}
	return res;
  }

  [[nodiscard]] Matrix operator-(Matrix<T, Rows, Cols> &other) {
	Matrix<T, Rows, Cols> res {};
	for (size_t i = 0; i < data.size(); ++i){
	  res.data[i] = other.data[i] - data[i];
	}
	return res;
  }

  [[nodiscard]] Vec3<T> operator*(const Vec3<T> &vec)
  {
	Vec3<T> result;
	T partial_sum {};
	for(size_t i = 0; i < 3; ++i)
	  {
		partial_sum = 0;
		for(size_t j = 0; j < Cols; ++j)
		  {
			partial_sum += get(i,j) * vec.vector.at(j);
		  }
		result.vector.at(i) = partial_sum;
	  }
	return result;
  }

  T& get (size_t i, size_t j) {
	return data.at((i*Cols) + j);
  }
  
  void Tr() {
	for(std::size_t i = 0; i < Rows; ++i){
	  for(std::size_t j = i+1; j < Cols; ++j){
		//std::cout << get(i,j) << std::endl;
		std::swap(get(i,j), get(j,i));
	  }
	}
  }

  
  

};

#endif // MATH_HPP
