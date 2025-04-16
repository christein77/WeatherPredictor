#pragma once
#include <vector>
#include <iostream>

class Matrix {
public:
	Matrix(unsigned rows, unsigned columns) :
	columns_(columns), rows_(rows), data_(columns * rows, 0) {}

	void display() const {
		for (unsigned r = 1; r <= rows(); r++) {
			for (unsigned c = 1; c <= columns(); c++) {
				std::cout << operator()(r, c) << " ";
			}
			std::cout << "\n";
		}
	}

	// set up so that (1,1) returns first element
	// according to matrix convention
	float& operator()(unsigned r, unsigned c) {
		return data_[(r - 1) * columns_ + c - 1];
	}

	float operator()(unsigned r, unsigned c) const {
		return data_[(r - 1) * columns_ + c - 1];
	}

	Matrix operator*(const Matrix& other) const {
		Matrix ret(this->rows_, other.columns());

		for (unsigned row = 1; row <= rows_; row++) {
			for (unsigned col = 1; col <= other.columns(); col++) {
				float total = 0.f;
				for (unsigned i = 1; i <= columns_; i++) {
					total += operator()(row, i) * other(i, col);
				}
				ret(row, col) = total;
			}
		}

		return ret;
	}

	unsigned rows() const {
		return rows_;
	}

	unsigned columns() const {
		return columns_;
	}

	bool isStochastic() const {
		for (unsigned c = 1; c <= columns(); c++) {
			float total = 0.f;
			for (unsigned r = 1; r <= rows(); r++) {
				total += operator()(r, c);
			}
			if (abs(total - 1) > .0001f) {
				std::cout << "Column total: " << total << "\n";
				return false;
			}
		}
		return true;
	}

	void makeStochastic() {

		for (unsigned col = 1; col <= columns(); col++) {

			float total = 0;

			for (unsigned row = 1; row <= rows(); row++) {
				total += operator()(row, col);
			}

			if (total == 0) {
				float newVal = 1.f / rows();
				for (unsigned row = 1; row <= rows(); row++) {
					operator()(row, col) = newVal;
				}
			} else {
				for (unsigned row = 1; row <= rows(); row++) {
					operator()(row, col) /= total;
				}
			}
		}
	}

private:
	unsigned columns_ = 0;
	unsigned rows_ = 0;

	std::vector<float> data_;
};
