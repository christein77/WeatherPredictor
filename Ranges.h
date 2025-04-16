#pragma once
#include <array>
#include <cmath>

namespace Range {
	enum Level : unsigned {
		LOW = 0,
		MED = 1,
		HIGH = 2
	};
}

template<unsigned Size>
class Ranges {
public:
	unsigned size() const {
		return Size;
	}

	std::array<Range::Level, Size> getRanges() const {
		return ranges_;
	}

	const Range::Level& operator[](unsigned index) const {
		return ranges_[index];
	}

	Range::Level& operator[](unsigned index) {
		return ranges_[index];
	}

	Ranges(unsigned serial, unsigned base = 3) {
		for (unsigned i = 0; i < Size; i++) {
			ranges_[i] = (Range::Level)(serial % base);
			serial /= base;
		}
	}

	unsigned serial() const {
		unsigned ret = 0;
		for (unsigned i = 0; i < Size; i++) {
			ret += (unsigned)(std::pow(3, i)) * ranges_[i];
		}
		return ret;
	}

	Ranges() {
		for (auto& level : ranges_) {
			level = Range::LOW;
		}
	}
private:
	std::array<Range::Level, Size> ranges_;
};
