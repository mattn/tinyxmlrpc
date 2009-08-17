/* Copyright 2009 by Yasuhiro Matsumoto
 * modification, are permitted provided that the following conditions are met:
 * 
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
 * REGENTS OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#ifndef _TINYXMLRPC_H_
#define _TINYXMLRPC_H_

#include <vector>
#include <map>
#include <string>
#include <ostream>
#include <algorithm>

namespace tinyxmlrpc {

class value {
public:
	typedef std::vector<char> Binary;
	typedef std::vector<value> Array;
	typedef std::map<std::string, value> Struct;
	class Exception {
	public:
		std::string message;
		int code;
		Exception(std::string message_, int code_) {
			message = message_;
			code = code_;
		}
		std::string to_xml();
	};
	enum Type {
	  TypeInvalid, TypeBoolean, TypeInt, TypeDouble, TypeTime,
	  TypeString, TypeBinary, TypeList, TypeArray, TypeStruct,
	  TypeException
	};
protected:
	typedef union {
		bool			asBool;
		int				asInt;
		double			asDouble;
		struct tm*		asTime;
		std::string*	asString;
		Binary*			asBinary;
		Array*			asArray;
		Struct*			asStruct;
		Exception*		asException;
	} Value;
public:
	Type _type;
	Value _value;
	char* operator=(char* x) {
		invalidate();
		_value.asString = new std::string(x);
		_type = TypeString;
		return (char*)_value.asString;
	}
	const char* operator=(const char* x) {
		invalidate();
		_value.asString = new std::string(x);
		_type = TypeString;
		return (char*)_value.asString;
	}
	std::string operator=(std::string x) {
		invalidate();
		_value.asString = new std::string(x);
		_type = TypeString;
		return (char*)_value.asString;
	}
	bool operator=(bool x) {
		invalidate();
		_value.asBool = x;
		_type = TypeBoolean;
		return _value.asBool;
	}
	int operator=(int x) {
		invalidate();
		_value.asInt = x;
		_type = TypeInt;
		return _value.asInt;
	}
	double operator=(double x) {
		invalidate();
		_value.asDouble = x;
		_type = TypeDouble;
		return _value.asDouble;
	}
	Exception* operator=(Exception* x) {
		invalidate();
		_value.asException = new Exception(x->message, x->code);
		_type = TypeException;
		return _value.asException;
	}
	value() {
		_type = TypeInvalid;
	}
	~value() {
		invalidate();
	}
	value(const char* _string) {
		_value.asString = new std::string();
		*_value.asString = _string;
		_type = TypeString;
	}
	value(std::string _string) {
		_value.asString = new std::string();
		*_value.asString = _string;
		_type = TypeString;
	}
	value(bool _bool) {
		_value.asBool = _bool;
		_type = TypeBoolean;
	}
	value(int _int) {
		_value.asInt = _int;
		_type = TypeInt;
	}
	value(long _long) {
		_value.asInt = _long;
		_type = TypeInt;
	}
	value(double _double) {
		_value.asDouble = _double;
		_type = TypeDouble;
	}
	value(struct tm _tm) {
		_value.asTime = new struct tm(_tm);
		_type = TypeTime;
	}
	value(Binary& _binary) {
		_value.asBinary = new Binary(_binary);
		_type = TypeBinary;
	}
	value(Exception* _exception) {
		_value.asException = _exception;
		_type = TypeException;
	}
    Type const &getType() const {
		return _type;
	}
	bool getBoolean() const {
		return _value.asBool;
	}
	int getInt() const {
		return _value.asInt;
	}
	double getDouble() const {
		return _value.asDouble;
	}
	struct tm* getTime() const {
		return _value.asTime;
	}
	std::string getString() const {
		return *(_value.asString);
	}
	Binary getBinary() const {
		return *(_value.asBinary);
	}
	bool hasMember(const std::string& name) const {
		return _type == TypeStruct && _value.asStruct->find(name) != _value.asStruct->end();
	}
	std::vector<std::string> listMembers() const {
		std::vector<std::string> ret;
		Struct::const_iterator it;
		if (_type == TypeStruct) {
			for(it = _value.asStruct->begin(); it != _value.asStruct->end(); it++)
				ret.push_back(it->first);
		}
		return ret;
	}
	Exception getException() const {
		return *(_value.asException);
	}
	size_t size() const {
		switch(_type) {
		case TypeString:
			return int(_value.asString->size());
		case TypeBinary:
			return int(_value.asBinary->size());
		case TypeArray: 
			return int(_value.asArray->size());
		case TypeStruct:
			return int(_value.asStruct->size());
		default:
			break;
		}
		return 0;
	}
	std::string to_str() const {
		std::string ret;
		static char buf[256];
		value::Array::const_iterator itarray;
		value::Struct::const_iterator itstruct;
		switch(_type) {
		case TypeString:
			ret = *_value.asString;
			break;
		case TypeInt:
			sprintf(buf, "%d", _value.asInt);
			ret = buf;
			break;
		case TypeTime:
			sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d",
				_value.asTime->tm_year+1900,
				_value.asTime->tm_mon,
				_value.asTime->tm_mday,
				_value.asTime->tm_hour,
				_value.asTime->tm_min,
				_value.asTime->tm_sec);
			ret = buf;
			break;
		case TypeBoolean:
			ret = _value.asBool ? "true" : "false";
			break;
		case TypeArray:
			ret += "[";
			for(itarray = _value.asArray->begin(); itarray != _value.asArray->end(); itarray++) {
				if (itarray != _value.asArray->begin())
					ret += ", ";
				ret += itarray->to_str();
			}
			ret += "]";
			break;
		case TypeStruct:
			ret += "[";
			for(itstruct = _value.asStruct->begin(); itstruct != _value.asStruct->end(); itstruct++) {
				if (itstruct != _value.asStruct->begin())
					ret += ", ";
				ret += itstruct->first.c_str();
				ret += "=\"";
				ret += itstruct->second.to_str();
				ret += "\"";
			}
			ret += "]";
			break;
		}
		return ret;
	}

	operator int&() { return _value.asInt; }
	operator struct tm*() { return _value.asTime; }
	operator const char*() { return _value.asString->c_str(); }
	operator std::string&() { return *_value.asString; }
	operator Binary&() { return *_value.asBinary; }
	operator Array&() { return *_value.asArray; }
	operator Struct&() { return *_value.asStruct; }
	operator Exception&() { return *_value.asException; }

	value(value const& rhs) {
		_type = TypeInvalid;
		*this = rhs;
	}
	value& operator=(value const& rhs) {
		if (this != &rhs) {
			invalidate();
			_type = rhs._type;
			switch (_type) {
			case TypeBoolean:	_value.asBool = rhs._value.asBool; break;
			case TypeInt:		_value.asInt = rhs._value.asInt; break;
			case TypeDouble:	_value.asDouble = rhs._value.asDouble; break;
			case TypeTime:		_value.asTime = new struct tm(*rhs._value.asTime); break;
			case TypeString:	_value.asString = new std::string(*rhs._value.asString); break;
			case TypeBinary:	_value.asBinary = new Binary(*rhs._value.asBinary); break;
			case TypeArray:		_value.asArray = new Array(*rhs._value.asArray); break;
			case TypeStruct:	_value.asStruct = new Struct(*rhs._value.asStruct); break;
			case TypeException:	_value.asException = new Exception(*rhs._value.asException); break;
			default:
				_value.asBinary = 0;
				_value.asException = 0;
				break;
			}
		}
		return *this;
	}
	value(Array& _array) {
		invalidate();
		_value.asArray = new Array(_array);
		_type = TypeArray;
	}
	value(Struct& _struct) {
		invalidate();
		_value.asStruct = new Struct(_struct);
		_type = TypeStruct;
	}

    value const& operator[](int i) const { assertArray(i+1); return _value.asArray->at(i); }
    value& operator[](int i)             { assertArray(i+1); return _value.asArray->at(i); }

    value& operator[](std::string const& k) { assertStruct(); return (*_value.asStruct)[k]; }
    value& operator[](const char* k) { assertStruct(); std::string s(k); return (*_value.asStruct)[s]; }

	static bool tmEq(struct tm* const& t1, struct tm* const& t2) {
	return
		t1->tm_sec == t2->tm_sec && t1->tm_min == t2->tm_min &&
		t1->tm_hour == t2->tm_hour && t1->tm_mday == t1->tm_mday &&
		t1->tm_mon == t2->tm_mon && t1->tm_year == t2->tm_year;
	}

	bool operator==(value const& other) const
	{
		if (_type != other._type)
			return false;

		switch (_type) {
		case TypeBoolean:  return ( !_value.asBool && !other._value.asBool) ||
					( _value.asBool && other._value.asBool);
		case TypeInt:      return _value.asInt == other._value.asInt;
		case TypeDouble:   return _value.asDouble == other._value.asDouble;
		case TypeTime:     return tmEq(_value.asTime, other._value.asTime);
		case TypeString:   return *_value.asString == *other._value.asString;
		case TypeBinary:   return *_value.asBinary == *other._value.asBinary;
		case TypeArray:    return *_value.asArray == *other._value.asArray;
		case TypeStruct:
			{
				if (_value.asStruct->size() != other._value.asStruct->size())
					return false;

				Struct::const_iterator it1=_value.asStruct->begin();
				Struct::const_iterator it2=other._value.asStruct->begin();
				while (it1 != _value.asStruct->end()) {
					const value& v1 = it1->second;
					const value& v2 = it2->second;
					if ( ! (v1 == v2))
						return false;
					it1++;
					it2++;
				}
				return true;
			}
		case TypeException:
			return (_value.asException->code == other._value.asException->code
				&& _value.asException->message == other._value.asException->message);
		default: break;
		}
		return true;
	}

	bool operator!=(value const& other) const
	{
		return !(*this == other);
	}

    void clear() {
		invalidate();
	}

protected:
	void invalidate() {
		if (_type == TypeTime)
			delete _value.asTime;
		if (_type == TypeString)
			delete _value.asString;
		if (_type == TypeBinary)
			delete _value.asBinary;
		if (_type == TypeArray)
			delete _value.asArray;
		if (_type == TypeStruct)
			delete _value.asStruct;
		if (_type == TypeException)
			delete _value.asException;
		_type = TypeInvalid;
	}
    void assertArray(int size) const {
		if (_type != TypeArray)
		  throw Exception("type error: expected an array", 4);
		else if (int(_value.asArray->size()) < size)
		  throw Exception("range error: array index too large", 4);
	}
	void assertArray(int size)
	{
		if (_type == TypeInvalid) {
			_type = TypeArray;
			_value.asArray = new Array(size);
		} else if (_type == TypeArray) {
			if (int(_value.asArray->size()) < size)
			_value.asArray->resize(size);
		} else
			throw Exception("type error: expected an array", 4);
	}
	void assertStruct()
	{
		if (_type == TypeInvalid) {
			_type = TypeStruct;
			_value.asStruct = new Struct();
		}
	}
};

std::ostream& operator<<(std::ostream& os, value& v);
bool failed(value& res);
std::string extract_method_name(std::string& strXml);
std::string extract_failt_message(std::string& strXml);
value parse(std::string& strXml);
std::string serialize(std::string method, std::vector<value>& requests);
std::string serialize(value& response);
const value call(std::string url, std::string method, std::vector<value>& requests, std::map<std::string, std::string>& headers);
const value call(std::string url, std::string method, std::vector<value>& requests);
value::Binary binary_fromfile(std::string filename);
bool binary_tofile(std::string filename, value::Binary binary);

}

#endif /* _TINYXMLRPC_H_ */
