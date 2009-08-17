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
#include <libxml/parser.h>
#include <curl/curl.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>

#include "tinyxmlrpc.h"

namespace tinyxmlrpc {

static
const std::string base64_chars = 
	"ABCDEFGHIJKLMNOPQRSTUVWXYZ"
	"abcdefghijklmnopqrstuvwxyz"
	"0123456789+/";
#define is_base64(c) ( \
		isalnum((unsigned char)c) || \
		((unsigned char)c == '+') || \
		((unsigned char)c == '/'))

static
std::string base64_encode(unsigned char const* bytes_to_encode, unsigned int in_len) {
	std::string ret;
	int i = 0;
	int j = 0;
	unsigned char char_array_3[3] = {0};
	unsigned char char_array_4[4] = {0};

	while (in_len--) {
		char_array_3[i++] = *(bytes_to_encode++);
		if (i == 3) {
			char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
			char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
			char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
			char_array_4[3] = char_array_3[2] & 0x3f;

			for(i = 0; (i <4) ; i++)
				ret += base64_chars[char_array_4[i]];
			i = 0;
		}
	}

	if (i) {
		for(j = i; j < 3; j++)
			char_array_3[j] = '\0';

		char_array_4[0] = (char_array_3[0] & 0xfc) >> 2;
		char_array_4[1] = ((char_array_3[0] & 0x03) << 4) + ((char_array_3[1] & 0xf0) >> 4);
		char_array_4[2] = ((char_array_3[1] & 0x0f) << 2) + ((char_array_3[2] & 0xc0) >> 6);
		char_array_4[3] = char_array_3[2] & 0x3f;

		for (j = 0; (j < i + 1); j++)
			ret += base64_chars[char_array_4[j]];

		while((i++ < 3))
			ret += '=';
	}

	return ret;
}

static
std::string base64_decode(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4] = {0};
	unsigned char char_array_3[3] = {0};
	std::string ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret += char_array_3[i];
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret += char_array_3[j];
	}

	return ret;
}

static
std::vector<char> base64_decode_binary(std::string const& encoded_string) {
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4] = {0};
	unsigned char char_array_3[3] = {0};
	std::vector<char> ret;

	while (in_len-- && ( encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_]; in_++;
		if (i ==4) {
			for (i = 0; i <4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j <4; j++)
			char_array_4[j] = 0;

		for (j = 0; j <4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret.push_back(char_array_3[j]);
	}

	return ret;
}

std::ostream& operator<<(std::ostream& os, value& v) {
	switch (v._type) {
	default:           break;
	case value::TypeBoolean:  os << v._value.asBool; break;
	case value::TypeInt:      os << v._value.asInt; break;
	case value::TypeDouble:   os << v._value.asDouble; break;
	case value::TypeString:   os << *v._value.asString; break;
	case value::TypeTime:
		{
			char buf[20];
			snprintf(buf, sizeof(buf)-1, "%4d%02d%02dT%02d:%02d:%02d",
				v._value.asTime->tm_year+1900,
				v._value.asTime->tm_mon,
				v._value.asTime->tm_mday,
				v._value.asTime->tm_hour,
				v._value.asTime->tm_min,
				v._value.asTime->tm_sec);
			buf[sizeof(buf)-1] = 0;
			os << buf;
			break;
		}
	case value::TypeBinary:
		{
			value::Binary::const_iterator itbinary;
			unsigned char *ptr = new unsigned char[v._value.asBinary->size()];
			int n;
			for(n = 0, itbinary = v._value.asBinary->begin(); itbinary != v._value.asBinary->end(); n++, itbinary++)
				ptr[n] = *itbinary;
			os << base64_encode((const unsigned char*)ptr, v._value.asBinary->size()).c_str();
			delete ptr;
			break;
		}
	case value::TypeArray:
		{
			int s = int(v._value.asArray->size());
			os << '{';
			for (int i=0; i < s; i++) {
				if (i > 0) os << ',';
					os << v._value.asArray->at(i);
			}
			os << '}';
			break;
		}
	case value::TypeStruct:
		{
			os << "[";
			value::Struct::const_iterator it;
			for (it = v._value.asStruct->begin(); it != v._value.asStruct->end(); it++)
			{
				if (it != v._value.asStruct->begin())
					os << ",";
				os << it->first << ":";
				value s = it->second;
				os << s;
			}
			os << "]";
			break;
		}
	case value::TypeException:
		{
			os << "[";
			os << v._value.asException->message;
			os << ",";
			os << v._value.asException->code;
			os << "]";
			break;
		}
	}

	return os;
}

value parse(xmlNodePtr pList) {
	value retVal;
	xmlNodePtr pNode;
	pNode = pList;
	while(pNode) {
		value ret;
		std::string strName = (char*)pNode->name;
		if (strName == "xml") {
			pNode = pNode->next;
			continue;
		}
		else
		if (strName == "methodCall")
			return parse(pNode->children);
		else
		if (strName == "methodResponse")
			return parse(pNode->children);
		else
		if (strName == "fault")
			return parse(pNode->children);
		else
		if (strName == "params")
			return parse(pNode->children);
		else
		if (strName == "param")
			return parse(pNode->children);
		else
		if (strName == "struct") {
			xmlNodePtr pMembers, pMember;
			pMembers = pNode->children;
			value::Struct valuestruct;
			while(pMembers) {
				if ("member" == (std::string)(char*)pMembers->name) {
					xmlNodePtr pName;
					std::string member_name;
					pName = pMembers->children;
					while(pName) {
						if ("name" == (std::string)(char*)pName->name) {
							member_name = (char*)pName->children->content;
							valuestruct[member_name] = parse(pMembers->children);
						}
						pName = pName->next;
					}
				}
				pMembers = pMembers->next;
			}
			ret = valuestruct;
		}
		else
		if (strName == "array") {
			xmlNodePtr pDatas;
			pDatas = pNode->children;
			while(pDatas) {
				if ("data" == (std::string)(char*)pDatas->name) {
					ret = parse(pDatas->children);
				}
				pDatas = pDatas->next;
			}
		}
		else
		if (strName == "value") {
			if (pNode->children) {
				/*
				if (pNode->children->type == XML_TEXT_NODE) {
					ret = (const char*)pNode->children->content;
				} else
				*/
				ret = parse(pNode->children);
			} else {
				ret = (const char*)pNode->children->content;
			}
		}
		else
		if (strName == "i4" || strName == "int")
			if (pNode->children) ret = (int)atol((char*)pNode->children->content);
			else ret = 0;
		else
		if (strName == "boolean")
			if (pNode->children) ret = (bool)(std::string((char*)pNode->children->content) == "true");
			else ret = false;
		else
		if (strName == "double")
			if (pNode->children) ret = (double)atof((char*)pNode->children->content);
			else ret = 0.0;
		else
		if (strName == "string")
			if (pNode->children) ret = (char*)pNode->children->content;
			else ret = "";
		else
		if (strName == "dateTime.iso8601") {
			struct tm tmTime = {0};
			if (pNode->children) {
				char num[64], val[16];
				strcpy(num, (char*)pNode->children->content);
				memset(val, 0, sizeof(val)); strncpy(val, num+ 0, 4);
				tmTime.tm_year = atol(val)-1900;
				memset(val, 0, sizeof(val)); strncpy(val, num+ 4, 2);
				tmTime.tm_mon= atol(val);
				memset(val, 0, sizeof(val)); strncpy(val, num+ 6, 2);
				tmTime.tm_mday = atol(val);
				memset(val, 0, sizeof(val)); strncpy(val, num+ 9, 2);
				tmTime.tm_hour = atol(val);
				memset(val, 0, sizeof(val)); strncpy(val, num+12, 2);
				tmTime.tm_min = atol(val);
				memset(val, 0, sizeof(val)); strncpy(val, num+15, 2);
				tmTime.tm_sec = atol(val);
				ret = tmTime;
			} else {
				ret = tmTime;
			}
		} else
		if (strName == "base64") {
			value::Binary valuebinary;
			valuebinary = base64_decode_binary((char*)pNode->children->content);
			ret = valuebinary;
		}

		if (ret.getType() != value::TypeInvalid) {
			if (retVal.getType() != value::TypeInvalid) {
				if (retVal.getType() == value::TypeArray)
					retVal[(int)retVal._value.asArray->size()] = ret;
				else {
					value::Array valuearray;
					valuearray.push_back(retVal);
					valuearray.push_back(ret);
					retVal = valuearray;
				}
			} else
				retVal = ret;
		}
		pNode = pNode->next;
	}
	return retVal;
}

void serialize(xmlNodePtr pValue, const value& param) {
	xmlDocPtr pDoc = pValue->doc;
	xmlNodePtr pArray, pData;
	xmlNodePtr pStruct;
	xmlNodePtr pSubValue;

	value::Binary valuebinary;
	value::Binary::const_iterator itbinary;

	value::Array valuearray;
	value::Array::const_iterator itarray;

	value::Struct valuestruct;
	value::Struct::const_iterator itstruct;

	char buf[64];
	unsigned char *ptr;
	struct tm *tmTime;
	int n;
	switch(param.getType()) {
	case value::TypeString:
		xmlNewTextChild(pValue, NULL, (xmlChar*)"string", (xmlChar*)param.getString().c_str());
		break;
	case value::TypeTime:
		tmTime = param.getTime();
		sprintf(buf, "%04d/%02d/%02d %02d:%02d:%02d",
			tmTime->tm_year+1900,
			tmTime->tm_mon,
			tmTime->tm_mday,
			tmTime->tm_hour,
			tmTime->tm_min,
			tmTime->tm_sec);
		xmlNewTextChild(pValue, NULL, (xmlChar*)"dateTime.iso8601", (xmlChar*)buf);
		break;
	case value::TypeInt:
		sprintf(buf, "%d", param.getInt());
		xmlNewTextChild(pValue, NULL, (xmlChar*)"i4", (xmlChar*)buf);
		break;
	case value::TypeDouble:
		sprintf(buf, "%f", param.getDouble());
		xmlNewTextChild(pValue, NULL, (xmlChar*)"double", (xmlChar*)buf);
		break;
	case value::TypeBoolean:
		xmlNewTextChild(pValue, NULL, (xmlChar*)"boolean", param.getBoolean() ? (xmlChar*)"true" : (xmlChar*)"false");
		break;
	case value::TypeBinary:
		valuebinary = param.getBinary();
		ptr = new unsigned char[valuebinary.size()];
		for(n = 0, itbinary = valuebinary.begin(); itbinary != valuebinary.end(); n++, itbinary++)
			ptr[n] = *itbinary;
		xmlNewTextChild(pValue, NULL, (xmlChar*)"base64", (xmlChar*)base64_encode((const unsigned char*)ptr, valuebinary.size()).c_str());
		delete ptr;
		break;
	case value::TypeArray:
		pArray = xmlNewChild(pValue, NULL, (xmlChar*)"array", NULL);
		pData = xmlNewChild(pArray, NULL, (xmlChar*)"data", NULL);
		for(itarray = param._value.asArray->begin(); itarray != param._value.asArray->end(); itarray++) {
			pSubValue = xmlNewChild(pData, NULL, (xmlChar*)"value", NULL);
			serialize(pSubValue, *itarray);
		}
		break;
	case value::TypeStruct:
		pStruct = xmlNewChild(pValue, NULL, (xmlChar*)"struct", NULL);
		for(itstruct = param._value.asStruct->begin(); itstruct != param._value.asStruct->end(); itstruct++) {
			xmlNodePtr pMember, pParam, pSubValue;
			pMember = xmlNewChild(pStruct, NULL, (xmlChar*)"member", NULL);
			xmlNewTextChild(pMember, NULL, (xmlChar*)"name", (xmlChar*)itstruct->first.c_str());
			pSubValue = xmlNewChild(pMember, NULL, (xmlChar*)"value", NULL);
			serialize(pSubValue, itstruct->second);
		}
		break;
	}
}

bool failed(std::string& strXml) {
	xmlDocPtr pDoc;
	xmlNodePtr pMethodResponse, pFault;
	bool ret = false;
	pDoc = xmlParseDoc((xmlChar*)strXml.c_str());
	pMethodResponse = pDoc->children;
	while(pMethodResponse) {
		if ("methodResponse" == (std::string)(char*)pMethodResponse->name) {
			pFault = pMethodResponse->children;
			while(pFault) {
				if ("fault" == (std::string)(char*)pFault->name) {
					ret = true;
					break;
				}
				pFault = pFault->next;
			}
			if (ret) break;
		}
		pMethodResponse = pMethodResponse->next;
	}
	xmlFreeDoc(pDoc);
	return ret;
}

bool failed(value& res) {
	if (res.getType() == value::TypeStruct) {
		if (res.hasMember("faultString") && res.hasMember("faultCode"))
			return true;
	} else
	if (res.getType() == value::TypeArray) {
		if (res.size() > 1 && res[0].getType() == value::TypeStruct) {
			if (res[0].hasMember("faultString") && res.hasMember("faultCode"))
				return true;
		}
	} else
	if (res.getType() == value::TypeException)
		return true;
	return false;
}

std::string extract_method_name(std::string& strXml) {
	xmlDocPtr pDoc;
	xmlNodePtr pMethodCall, pMethodName;
	std::string ret = "";
	pDoc = xmlParseDoc((xmlChar*)strXml.c_str());
	pMethodCall = pDoc->children;
	while(pMethodCall) {
		if ("methodCall" == (std::string)(char*)pMethodCall->name) {
			pMethodName = pMethodCall->children;
			while(pMethodName) {
				if ("methodName" == (std::string)(char*)pMethodName->name) {
					ret = (char*)pMethodName->children->content;
					break;
				}
				pMethodName = pMethodName->next;
			}
			if (ret != "") break;
		}
		pMethodCall = pMethodCall->next;
	}
	xmlFreeDoc(pDoc);
	return ret;
}

value parse(std::string& strXml) {
	xmlDocPtr pDoc;
	value res;
	pDoc = xmlParseDoc((xmlChar*)strXml.c_str());
	if (pDoc) {
		res = parse(pDoc->children);
		xmlFreeDoc(pDoc);
	} else
		res = strXml;
	return res;
}

std::string serialize(std::string method, std::vector<value>& requests) {
	xmlDocPtr pDoc;
	xmlNodePtr pNode;
	xmlNodePtr pMethodCall, pMethodName, pParams;
	xmlNodePtr pParam, pValue;

	pDoc = xmlNewDoc((xmlChar*)"1.0");
	pMethodCall = xmlNewNode(NULL, (xmlChar*)"methodCall");
	xmlDocSetRootElement(pDoc, pMethodCall);
	xmlNewTextChild(pMethodCall, NULL, (xmlChar*)"methodName", (xmlChar*)method.c_str());
	pParams = xmlNewChild(pMethodCall, NULL, (xmlChar*)"params", NULL);

	std::vector<value>::const_iterator it;
	for(it = requests.begin(); it != requests.end(); it++) {
		pParam = xmlNewChild(pParams, NULL, (xmlChar*)"param", NULL);
		pValue = xmlNewChild(pParam, NULL, (xmlChar*)"value", NULL);
		serialize(pValue, *it);
	}

	xmlChar* pstrXml;
	int bufsize;
	xmlDocDumpFormatMemoryEnc(pDoc, &pstrXml, &bufsize, "utf-8", 0);
	xmlFreeDoc(pDoc);
	std::string strXml = (char*)pstrXml;
	xmlFree(pstrXml);
	return strXml;
}

std::string parse(value& response) {
	xmlDocPtr pDoc;
	xmlNodePtr pNode;
	xmlNodePtr pFault, pMethodResponse, pParams;
	xmlNodePtr pParam, pValue;

	pDoc = xmlNewDoc((xmlChar*)"1.0");
	pMethodResponse = xmlNewNode(NULL, (xmlChar*)"methodResponse");
	xmlDocSetRootElement(pDoc, pMethodResponse);
	pParams = xmlNewChild(pMethodResponse, NULL, (xmlChar*)"params", NULL);

	pParam = xmlNewChild(pParams, NULL, (xmlChar*)"param", NULL);
	pValue = xmlNewChild(pParam, NULL, (xmlChar*)"value", NULL);
	serialize(pValue, response);

	xmlChar* pstrXml;
	int bufsize;
	xmlDocDumpFormatMemoryEnc(pDoc, &pstrXml, &bufsize, "utf-8", 0);
	xmlFreeDoc(pDoc);
	std::string strXml = (char*)pstrXml;
	xmlFree(pstrXml);
	return strXml;
}

std::string value::Exception::to_xml() {
	xmlDocPtr pDoc;
	xmlNodePtr pMethodResponse, pFault, pValue, pStruct, pMember;

	pDoc = xmlNewDoc((xmlChar*)"1.0");
	pMethodResponse = xmlNewNode(NULL, (xmlChar*)"methodResponse");
	xmlDocSetRootElement(pDoc, pMethodResponse);
	pFault = xmlNewChild(pMethodResponse, NULL, (xmlChar*)"fault", NULL);
	pValue = xmlNewChild(pFault, NULL, (xmlChar*)"value", NULL);
	pStruct = xmlNewChild(pValue, NULL, (xmlChar*)"struct", NULL);

	pMember = xmlNewChild(pStruct, NULL, (xmlChar*)"member", NULL);
	xmlNewTextChild(pMember, NULL, (xmlChar*)"name", (xmlChar*)"faultString");
	xmlNewTextChild(pMember, NULL, (xmlChar*)"value", (xmlChar*)this->message.c_str());

	pMember = xmlNewChild(pStruct, NULL, (xmlChar*)"member", NULL);
	xmlNewTextChild(pMember, NULL, (xmlChar*)"name", (xmlChar*)"faultCode");
	char buf[16];
	sprintf(buf, "%d", this->code);
	xmlNewTextChild(pMember, NULL, (xmlChar*)"value", (xmlChar*)buf);

	xmlChar* pstrXml;
	int bufsize;
	xmlDocDumpFormatMemoryEnc(pDoc, &pstrXml, &bufsize, "utf-8", 0);
	xmlFreeDoc(pDoc);
	std::string strXml = (char*)pstrXml;
	xmlFree(pstrXml);
	return strXml;
}

typedef struct {
    char* data;     // response data from server
    size_t size;    // response size of data
} MEMFILE;

MEMFILE*
memfopen() {
    MEMFILE* mf = (MEMFILE*) malloc(sizeof(MEMFILE));
    mf->data = NULL;
    mf->size = 0;
    return mf;
}

void
memfclose(MEMFILE* mf) {
    if (mf->data) free(mf->data);
    free(mf);
}

size_t
memfwrite(char* ptr, size_t size, size_t nmemb, void* stream) {
    MEMFILE* mf = (MEMFILE*) stream;
    int block = size * nmemb;
    if (!mf->data)
        mf->data = (char*) malloc(block);
    else
        mf->data = (char*) realloc(mf->data, mf->size + block);
    if (mf->data) {
        memcpy(mf->data + mf->size, ptr, block);
        mf->size += block;
    }
    return block;
}

char*
memfstrdup(MEMFILE* mf) {
    char* buf = (char*)malloc(mf->size + 1);
    memcpy(buf, mf->data, mf->size);
    buf[mf->size] = 0;
    return buf;
}

int post(std::string url, std::string method, std::string request, std::string& response, std::map<std::string, std::string>& headers) {
	CURL* curl = curl_easy_init();
	int ret = -1;
	if(curl) {
		response = "";
		struct curl_slist *headerlist=NULL;
  		char error[256];
		std::map<std::string, std::string>::iterator it;
		bool have_content_type = false;
		for (it = headers.begin(); it != headers.end(); it++) {
			std::string header = it->first + ": ";
			header += it->second;
			headerlist = curl_slist_append(headerlist, header.c_str());
			std::string key = it->first;
			std::transform(key.begin(), key.end(), key.begin(), ::tolower);
			if (key == "CONTENT-TYPE") have_content_type = true;
		}
		if (!have_content_type) headerlist = curl_slist_append(headerlist, "Content-Type: text/xml");
		MEMFILE* mf = memfopen();
		curl_easy_setopt(curl, CURLOPT_URL, url.c_str());
  		curl_easy_setopt(curl, CURLOPT_ERRORBUFFER, &error);
		curl_easy_setopt(curl, CURLOPT_HTTPHEADER, headerlist);
		curl_easy_setopt(curl, CURLOPT_POSTFIELDS, request.c_str());
		curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, request.size());
		curl_easy_setopt(curl, CURLOPT_POST, 1);
		curl_easy_setopt(curl, CURLOPT_WRITEDATA, mf);
		curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, memfwrite);
		CURLcode code = curl_easy_perform(curl);
  		if (code != CURLE_OK) {
			response = curl_easy_strerror(code);
			ret = -2;
		} else {
			int status = 200;
			if (curl_easy_getinfo(curl, CURLINFO_HTTP_CODE, &status) != CURLE_OK) {
				response = error;
			} else {
				if (status != 200) {
					response = std::string(mf->data, mf->size);
					response = extract_failt_message(response);
					ret = -3;
				} else {
					response = std::string(mf->data, mf->size);
					ret = 0;
				}
			}
		}
		memfclose(mf);
		curl_easy_cleanup(curl);
		curl_slist_free_all (headerlist);
	}
	return ret;
}

std::string extract_failt_message(std::string& strXml) {
	std::string ret;
	ret = strXml;
	ret.erase(std::remove(ret.begin(), ret.end(), '\r'), ret.end());
	std::replace(ret.begin(), ret.end(), '\t', ' ');
	while (true) {
		int iPos1 = ret.find_first_of('<');
		int iPos2 = ret.find_first_of('>');
		if (iPos1 == -1 || iPos2 == -1) break;
		ret.erase(iPos1, iPos2-iPos1+1);
		if (ret[iPos1] == '\n' || ret[iPos1] == ' ') {
			iPos2 = ret.substr(iPos1).find_first_not_of("\t \n");
			ret.erase(iPos1, iPos2);
		} else
		if (iPos1 > 0 && ret[iPos1-1] != '\n' && ret[iPos1-1] != ' ')
			ret.insert(iPos1, " ");
	}
	if (!ret.empty()) {
		if (ret[ret.size()-1] == '\n')
			ret.resize(ret.size()-1);
	} else
		ret = "Unknown Error";

	return ret;
}

value::Binary binary_fromfile(std::string filename) {
#ifdef _WIN32
	struct _stat statbuf = {0};
#else
	struct stat statbuf = {0};
#endif
	if (stat(filename.c_str(), &statbuf) != -1) {
		tinyxmlrpc::value::Binary valuebinary;
		FILE *fp = fopen(filename.c_str(), "rb");
		char *ptr = new char[statbuf.st_size];
		fread(ptr, statbuf.st_size, 1, fp);
		fclose(fp);
		valuebinary.reserve(statbuf.st_size);
		for(int n = 0; n < statbuf.st_size; n++)
			valuebinary.push_back(ptr[n]);
		delete[] ptr;
		return valuebinary;
	}
	value empty;
	return empty;
}

bool binary_tofile(std::string filename, value::Binary valuebinary) {
	FILE *fp = fopen(filename.c_str(), "wb");
	if (!fp) return false;
	tinyxmlrpc::value::Binary::const_iterator it;
	for(it = valuebinary.begin(); it != valuebinary.end(); it++) {
		char c = *it;
		fwrite(&c, 1, 1, fp);
	}
	fclose(fp);
	return true;
}

const value call(std::string url, std::string method, std::vector<value>& requests) {
	std::map<std::string, std::string> headers;
	return call(url, method, requests, headers);
}

const value call(std::string url, std::string method, std::vector<value>& requests, std::map<std::string, std::string>& headers) {
	int result = 0;
	std::string response;
	result = post(url, method, serialize(method, requests), response, headers);
	if (result == 0)
		return parse(response);
	else
		return new value::Exception(response, result);
}

}
