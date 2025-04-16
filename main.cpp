#include "Matrix.h"
#include "Ranges.h"
#include <array>
#include <iostream>
#include <iomanip>
#include <fstream>
#include <cmath>
#include <algorithm>
#include <map>

std::string value(const std::string& str, unsigned& startIndex, char delim = ',') {

	unsigned nextIndex = str.find_first_of(delim, startIndex + 1);
	std::string ret = str.substr(startIndex, nextIndex - startIndex);
	startIndex = nextIndex + 1;
	return ret;
}

constexpr unsigned NUM_COLUMNS = 5;

typedef std::array<float, NUM_COLUMNS> DataPoint;

void loadDataPoints(std::vector<DataPoint>& dataPoints) {
	std::fstream file;
	file.open("1950WeatherDat.csv", std::ios::in);

	if (file.is_open() == false) {
		std::cout << "Could not open file in loadDataPoints\n";
	}

	std::string line;

	dataPoints.clear();
	for (unsigned i = 0; i < 4; i++) {
		std::getline(file, line);
	}

	while (std::getline(file, line)) {
		dataPoints.push_back({});
		auto& dp = dataPoints.back();
		unsigned startIndex = 0;
		value(line, startIndex);
		for (unsigned i = 0; i < NUM_COLUMNS; i++) {
			dp[i] = std::stof(value(line, startIndex));
		}
	}
	std::cout << dataPoints.size() << "\n";
}

void populateRanges(
	const std::vector<DataPoint>& dataPoints,
	std::vector<Ranges<NUM_COLUMNS>>& ranges) {

	ranges.resize(dataPoints.size());

	const std::vector<std::pair<float, float>> thresholds = {
		{ 5, 25 }, // Temp in C
		{ 25, 65 }, // humidity
		{ 40, 70 }, // cloud cover
		{ 8, 15 }, // wind speed
		{ .02, .3 } // rain
	};

	for (unsigned i = 0; i < dataPoints.size(); i++) {
		auto& dp = dataPoints[i];
		for (unsigned j = 0; j < NUM_COLUMNS; j++) {
			if (dp[j] > thresholds[j].second) {
				ranges[i][j] = Range::Level::HIGH;
			} else if (dp[j] > thresholds[j].first) {
				ranges[i][j] = Range::Level::MED;
			} else {
				ranges[i][j] = Range::Level::LOW;
			}
		}
	}
}

void loadMatrix(const std::vector<Ranges<NUM_COLUMNS>>& ranges, Matrix& m) {
	for (unsigned i = 1; i < ranges.size(); i++) {
		unsigned row = ranges[i - 1].serial();
		unsigned col = ranges[i].serial();

		m(row + 1, col + 1)++;
	}
}

void displayRange(const Ranges<NUM_COLUMNS>& range) {
	const std::vector<std::string> labels = {
		"Temp",
		"Humidity",
		"Cloud Cover",
		"Wind Speed",
		"Rain"
	};

	for (unsigned i = 0; i < range.size(); i++) {
		std::cout << labels[i] << ":\t";

		switch(range[i]) {
		case Range::Level::HIGH:
			std::cout << "High\n";
			break;
		case Range::Level::MED:
			std::cout << "Medium\n";
			break;
		case Range::Level::LOW:
			std::cout << "Low\n";
			break;
		default: break;
		}
	}
}

unsigned getUnsigned(unsigned max) {
	int temp;
	do {
		std::cin >> temp;
	} while (temp < 0 || temp > max);

	return (unsigned)temp;
}

Matrix getUserInput() {

	std::vector<std::string> labels = {
		"Temp:\t",
		"Humidity:\t",
		"Cloud Cover:\t",
		"Wind Speed:\t",
		"Rain:\t"
	};

	Ranges<NUM_COLUMNS> r;
	std::cout << "Answer with 0 (Below Avg), 1(Avg), or 2(Above Avg)\n";

	for (unsigned i = 0; i < labels.size(); i++) {
		std::cout << labels[i];
		r[i] = (Range::Level)getUnsigned(2);
	}
	std::cout << "\n";

	Matrix ret(std::pow(3, NUM_COLUMNS), 1);
	ret(r.serial(), 1)++;

	return ret;
}

int main() {

	std::vector<DataPoint> dataPoints;
	loadDataPoints(dataPoints);

	std::vector<Ranges<NUM_COLUMNS>> ranges;
	populateRanges(dataPoints, ranges);

	constexpr unsigned size = std::pow(3, NUM_COLUMNS);
	Matrix stochastic(size, size);

	loadMatrix(ranges, stochastic);

	stochastic.makeStochastic();

	if (stochastic.isStochastic() == false) {
		std::cout << "Failed to make Matrix stochastic!\n";
		return 0;
	}

	auto input = getUserInput();

	auto result = stochastic * input;
	struct FinalValue {
		float likelihood = 0;
		Ranges<NUM_COLUMNS> ranges;
	};

	struct Levels {
		float high = 0;
		float med = 0;
		float low = 0;
	};

	std::vector<Levels> levels(NUM_COLUMNS);

	std::vector<FinalValue> finalValues;
	for (unsigned row = 1; row <= result.rows(); row++) {
		float likelihood = result(row, 1);
		auto r = Ranges<NUM_COLUMNS>(row - 1);
		for (unsigned i = 0; i < NUM_COLUMNS; i++) {
			switch(r[i]) {
			case Range::Level::HIGH:
				levels[i].high += likelihood;
				break;
			case Range::Level::MED:
				levels[i].med += likelihood;
				break;
			case Range::Level::LOW:
				levels[i].low += likelihood;
				break;
			default: break;
			}
		}

		if (likelihood == 0) continue;
		FinalValue fv;
		fv.likelihood = likelihood;
		fv.ranges = Ranges<NUM_COLUMNS>(row - 1);
		finalValues.push_back(fv);
	}

	const std::vector<std::string> labels {
		"Temp:        ",
		"Humidity:    ",
		"Cloud Cover: ",
		"Wind Speed:  ",
		"Rain:        "
	};

	for (unsigned i = 0; i < NUM_COLUMNS; i++) {
		float likelihood = 0;
		Range::Level level = Range::Level::LOW;
		std::cout << labels[i] << "\t";
		if (levels[i].high > likelihood) {
			likelihood = levels[i].high;
			level = Range::Level::HIGH;
		}

		if (levels[i].med > likelihood) {
			likelihood = levels[i].med;
			level = Range::Level::MED;
		}

		if (levels[i].low > likelihood) {
			likelihood = levels[i].low;
			level = Range::Level::LOW;
		}

		switch(level) {
		case Range::Level::HIGH: std::cout << "High\t"; break;
		case Range::Level::MED: std::cout << "Med\t"; break;
		case Range::Level::LOW: std::cout << "Low\t"; break;
		default: break;
		}
		std::cout << 100 * likelihood << "%\n";
	}

	std::sort(finalValues.begin(), finalValues.end(), [](const auto& a, const auto& b) {
		return a.likelihood > b.likelihood;
	});

	unsigned totalLikelihoods = 0;

	std::vector<DataPoint> isolatedProbabilities(3);

	for (const auto& fv : finalValues) {
		if (fv.likelihood < .01) continue;
		totalLikelihoods++;
		//std::cout << fv.likelihood * 100 << "%\n";
		//displayRange(fv.ranges);
		//std::cout << "\n";
	}

	if (totalLikelihoods == 0) {
		std::cout << "Idk, it's anyone's game, man.\n";
	}
}
