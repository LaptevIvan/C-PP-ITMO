#include "bucket_storage.hpp"

int main() {
	BucketStorage<int> b;
	int n = 3;
	for (int i = 0; i < n; ++i) {
		b.insert(i);
	}
	for (auto it = b.begin(); it < b.end(); ++it) {
		std::cout << *it << std::endl;
	}
	return 0;
}
