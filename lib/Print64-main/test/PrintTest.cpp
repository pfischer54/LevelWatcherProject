#include "Particle.h"
#include "Print64.h"

uint64_t testValues[] = {
	0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16, 17,
	0x100, 0x200, 0x800, 0xfff,
	0x7fff, 0x8000, 0x8ffff, 0xffff,
	0x7fffff, 0x800000, 0x8ffffff, 0xffffff,
	0x7fffffff, 0x80000000, 0x8ffffffff, 0xffffffff,
	0x017fffffff, 0x0180000000, 0x018ffffffff, 0x01ffffffff,
	0x7fffffffffffffff, 0x80000000ffffffff, 0x8fffffffffffffff, 0xfffffffffffffff,
};

const char *binaryStrings[] = {
	"0",
	"1",
	"10",
	"11", // 3
	"100",
	"101",
	"110",
	"111", // 7
	"1000",
	"1001",
	"1010",
	"1011",
	"1100",
	"1101",
	"1110",
	"1111" // 15
};

const char *base5strings[] = {
	"0", "1", "2", "3", "4", "10", "11", "12", "13", "14", "20"
};

int main(int argc, char *argv[]) {
	char expected[68];

	// Unsigned uint64_t decimal
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		snprintf(expected, sizeof(expected), "%llu", testValues[ii]);

		String s = toString(testValues[ii], 10);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%llu line=%u\n", ii, testValues[ii], __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}
	}

	// Signed int64_t
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		if (testValues[ii] > 0x7fffffffffffffff) {
			continue;
		}
		int64_t intValue = (int64_t)testValues[ii];

		snprintf(expected, sizeof(expected), "%lld", intValue);

		String s = toString(intValue);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%lld line=%u\n", ii, intValue, __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}

		// Test negative
		intValue = -intValue;

		snprintf(expected, sizeof(expected), "%lld", intValue);

		s = toString(intValue);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%lld line=%u\n", ii, intValue, __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}

	}

	// Hex 
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		snprintf(expected, sizeof(expected), "%llx", testValues[ii]);

		String s = toString(testValues[ii], 16);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%llx line=%u\n", ii, testValues[ii], __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}
	}

	// Octal
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		snprintf(expected, sizeof(expected), "%llo", testValues[ii]);

		String s = toString(testValues[ii], 8);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%llo line=%u\n", ii, testValues[ii], __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}
	}

	// Binary
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		if (ii >= (sizeof(binaryStrings) / sizeof(binaryStrings[0]))) {
			break;
		}
		const char *expected = binaryStrings[ii];

		String s = toString(testValues[ii], 2);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%llo line=%u\n", ii, testValues[ii], __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}

	}

	// Binary, just to test the conversion. These are too tedious to type in.
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		String s = toString(testValues[ii], 2);
	}

	// Base 5 is weird, but just to make sure the we don't crash or do something unexpected
	for(size_t ii = 0; ii < (sizeof(testValues) / sizeof(testValues[0])); ii++) {
		if (ii >= (sizeof(base5strings) / sizeof(base5strings[0]))) {
			break;
		}
		const char *expected = base5strings[ii];

		String s = toString(testValues[ii], 5);

		if (s.compareTo(expected)) {
			printf("test failed ii=%zu value=%llo line=%u\n", ii, testValues[ii], __LINE__);
			printf("expected: %s\n", expected);
			printf("got     : %s\n", s.c_str());
		}

	}


	return 0;
}
