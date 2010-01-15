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
	vargs& first() {
		this->args_.clear();
		return *this;
	}
};

int main(int argc, char* argv[]) {
	if (argc != 4) return -1;

	std::string endpoint = argv[1];
	std::string user = argv[2];
	std::string pass = argv[3];

	try {
		tinyxmlrpc::value::Array req;
		tinyxmlrpc::value res;
		tinyxmlrpc::value::Struct entry;
		vargs args;

		res = tinyxmlrpc::call(endpoint,
				"metaWeblog.getRecentPosts",
				(args.first() << "1" << user << pass << 3 << true).list());
		if (!failed(res)) {
			for(int n = 0; n < res.size(); n++) {
				std::vector<std::string> members = res[n].listMembers();
				std::vector<std::string>::const_iterator it;
				std::cout << "{" << std::endl;
				for(it = members.begin(); it != members.end(); it++) {
					std::string val = res[n][*it].to_str();
					std::cout << "  " << it->c_str() << "=" << val.c_str() << std::endl;
				}
				std::cout << "}" << std::endl;
			}
		} else {
			std::cerr << res << std::endl;
		}

		entry["title"] = "hasegawaにいじめられた";
		entry["description"] = "今日は、wassrでhasegawaにいじめられた。悲しかった。";
		entry["dateCreated"] = "";
		res = tinyxmlrpc::call(
				endpoint,
				"metaWeblog.newPost",
				(args.first() << "1" << user << pass << entry << 1).list());
		if (!failed(res)) {
			std::cout << "result:" << res << std::endl;
		} else 
			std::cerr << res << std::endl;
	} catch(tinyxmlrpc::value::Exception& e) {
		std::cerr << e.message << std::endl;
	}

	return 0;
}

