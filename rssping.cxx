#include "tinyxmlrpc.h"
#include <iostream>
#include <stdarg.h>

class vargs {
private:
	tinyxmlrpc::value::Array args_;
public:
	vargs& operator <<(tinyxmlrpc::value arg) {
		args_.push_back(arg);
		return *this;
	}
	tinyxmlrpc::value::Array& list() {
		return this->args_;
	}
	vargs& builder() {
		this->args_.clear();
		return *this;
	}
};

int main(int argc, char* argv[]) {
	try {
		tinyxmlrpc::value res;
		vargs args;

		res = tinyxmlrpc::call("http://api.my.yahoo.co.jp/RPC2",
				"weblogUpdates.ping",
				(args.builder() << "Big Sky" << "http://mattn.kaoriya.net/index.rss").list());
		std::cerr << res << std::endl;
	} catch(tinyxmlrpc::value::Exception& e) {
		std::cerr << e.message << std::endl;
	}

	return 0;
}

