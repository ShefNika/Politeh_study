#include "MyString.h"
#include "AhoCorasik.h"
#include "MyStringError.h"

#include <cstring>  
#include <stdexcept>
#include <iostream> 
#include <fstream> 

using namespace std;

MyString::MyString() {
	len = 0;
	cap = 1;
	data = new char[cap];
	data[0] = '\0';
}

MyString::MyString(const char* source_str, int count) {
	if (source_str == nullptr) {
		len = 0;
		cap = 1;
		data = new char[cap];
		data[0] = '\0';
	}
	else {
		int source_len = strlen(source_str);
		len = min(source_len, count);
		cap = len + 1;
		data = new char[cap];
		memcpy(data, source_str, len); 
		data[len] = '\0';
	}
}

MyString::MyString(const std::string& source_str, int count) : MyString(source_str.c_str(), count){
}

MyString::MyString(const MyString& source_str, int count) : MyString(source_str.data, count) { 
}

MyString::MyString(const char* source_str) : MyString(source_str, strlen(source_str)) {
}

MyString::MyString(const MyString& source_str) : MyString(source_str, source_str.len) {
}

MyString::MyString(const std::string& source_str) : MyString(source_str, source_str.length()) {
}

MyString::MyString(int count, char ch) {
	if (count < 0) 
		throw invalid_argument("Count cannot be negative");
	else if (count == 0) 
		MyString();
	else {
		len = count;
		cap = len + 1;
		data = new char[cap];
		for (int i = 0; i < len; i++) {
			data[i] = ch;
		}
		data[len] = '\0';
	}
}

MyString::MyString(MyString&& source_str) noexcept {
	data = source_str.data;
	len = source_str.len;
	cap = source_str.cap;

	source_str.data = new char[1];  
	source_str.data[0] = '\0';
	source_str.len = 0;
	source_str.cap = 1;
}

MyString::MyString(int number) {
	if (number == 0) {
		len = 1;
		cap = 2;
		data = new char[cap];
		data[0] = '0';
		data[1] = '\0';
		return;
	}
	int length = 0;
	int negative = 0;
	if (number < 0) 
		negative = 1;
	int absnum = negative ? -number : number;
	int temp = absnum;
	while (absnum > 0) {
		absnum /= 10;
		length++;
	}
	len = length+ (negative ? 1 : 0);
	cap = len + 1;
	data = new char[cap];
	if (negative)
		data[0] = '-';
	int start = len - 1;
	do {
		data[start--] = '0' + (temp % 10);
		temp /= 10;
	} while (temp > 0);
	data[len] = '\0';
}


MyString::MyString(double number) {
	if (number == 0.0) {
		len = 3;
		cap = 4;
		data = new char[cap];
		data[0] = '0'; data[1] = '.'; data[2] = '0'; data[3] = '\0';
		return;
	}

	bool negative = number < 0.0;
	double absnum = (negative ? -number : number) + 1e-15;
	int int_part = static_cast<int>(absnum);
	double fractional = absnum - static_cast<double>(int_part);  
	MyString int_str(int_part);

	char frac_buffer[16] = { 0 };  
	int frac_count = 0;
	double temp_frac = fractional;
	while (frac_count < 15 && temp_frac > 0) {
		temp_frac *= 10.0;
		int digit = static_cast<int>(temp_frac);
		frac_buffer[frac_count++] = '0' + digit;
		temp_frac -= static_cast<double>(digit);
		if (temp_frac < 1e-11) 
			temp_frac = 0.0;
	}
	while (frac_count > 1 && frac_buffer[frac_count - 1] == '0') 
		frac_count--;
	bool has_fraction = (frac_count > 0);
	if (!has_fraction) {
		frac_count = 1;  
		frac_buffer[0] = '0';  
	}
	int total_len = int_str.size() + frac_count +1 + (negative ? 1 : 0);
	len = total_len;
	cap = len + 1;
	data = new char[cap];
	int start = 0;
	if (negative) 
		data[start++] = '-';
	memcpy(data + start, int_str.c_str(), int_str.size());  
	start += int_str.size();
	data[start++] = '.';
	memcpy(data + start, frac_buffer, frac_count);
	start += frac_count;
	data[len] = '\0';  
}


MyString::~MyString() {
	delete[] data;
	data = nullptr;
}

void MyString::clear() {
	data[0] = '\0';
	len = 0;
}

void MyString::shrink_to_fit() {
	if (cap > len + 1) {
		cap = len + 1;
		char* new_data = new char[cap];
		memcpy(new_data, data, len + 1);
		delete[] data;
		data = new_data;
	}
}

void MyString::erase(int index, int count) {
	if (index < 0 || index > len)
		throw out_of_range("Insert index out of range");
	if (count <= 0)
		throw invalid_argument("Count cannot be negative");
	int del = min(count, len - index);
	for (int i = index; i <= len - del; i++) 
		data[i] = data[i + del];
	len -= del;
	data[len] = '\0';
}

void MyString::insert(int index, const char* source_str, int s_index, int count) {
	if (index < 0 || index > len) 
		throw out_of_range("Insert index out of range");
	if (source_str == nullptr) 
		throw invalid_argument("Source string cannot be nullptr");
	if (count < 0) 
		throw invalid_argument("Count cannot be negative");
	if (source_str[0] == '\0')
		return;
	int source_len = strlen(source_str);
	if (s_index < 0 || s_index >= source_len) 
		throw std::out_of_range("Start index out of range in source string");
	if (s_index > source_len) {
		cout << "Error: check the arguments!" << endl;
		return;
	}
	int add_len = min(count, source_len - s_index);
	int new_len = len + add_len;
	if (new_len > cap - 1) {
		int new_capacity = new_len + 1;
		char* new_data = new char[new_capacity];
		memcpy(new_data, data, index);
		memcpy(new_data + index, source_str + s_index, add_len);
		memcpy(new_data + index + add_len, data + index, len - index);
		new_data[new_len] = '\0';
		delete[] data;
		data = new_data;
		cap = new_capacity;
		len = new_len;
	}
	else {
		memmove(data + index + count, data + index, len - index + 1);
		memcpy(data + index, source_str + s_index, add_len);
		len = new_len;
	}
}

void MyString::insert(int index, const std::string& source_str, int s_index, int count) {
	insert(index, source_str.c_str(), s_index, count);
}

void MyString::insert(int index, const MyString& source_str, int s_index, int count) {
	if (source_str.data == nullptr) 
		throw invalid_argument("Source MyString is empty");
	insert(index, source_str.data, s_index, count);
}

void MyString::insert(int index, const char* source_str) {
		insert(index, source_str, 0, strlen(source_str));
}

void MyString::insert(int index, const std::string& source_str) {
		insert(index, source_str.c_str(), 0, source_str.length());
}

void MyString::insert(int index, const MyString& source_str) {
	if(source_str.data == nullptr)
		throw invalid_argument("Source MyString is empty");
	insert(index, source_str.data, 0, source_str.len);
}

void MyString::insert(int index, const char* source_str, int count) {
		insert(index, source_str, 0, count);
}

void MyString::insert(int index, const std::string& source_str, int count) {
		insert(index, source_str.c_str(), 0, count);
}

void MyString::insert(int index, const MyString& source_str, int count) {
	if(source_str.data == nullptr)
		throw invalid_argument("Source MyString is empty");
	insert(index, source_str.data, 0, count);
}

void MyString::insert(int index, int count, char ch) {
	if (count < 0) 
		throw invalid_argument("Count cannot be negative");
	if (index < 0 || index > len)
		throw out_of_range("Insert index out of range");
	char* temp = new char[count + 1];
	for (int i = 0; i < count; i++) 
		temp[i] = ch;
	temp[count] = '\0';
	insert(index, temp, 0, count);
	delete[] temp;
}

void MyString::append(int count, char ch) {
	insert(len, count, ch);
}

void MyString::append(const char* source_str) {
	insert(len, source_str);
}

void MyString::append(const std::string& source_str) {
	insert(len, source_str);
}

void MyString::append(const MyString& source_str) {
	insert(len, source_str);
}

void MyString::append(const char* source_str, int count) {
	insert(len, source_str, count);
}

void MyString::append(const std::string& source_str, int count) {
	insert(len, source_str, count);
}

void MyString::append(const MyString& source_str, int count) {
	insert(len, source_str, count);
}

void MyString::append(const char* source_str, int s_index, int count) {
	insert(len, source_str, s_index, count);
}

void MyString::append(const std::string& source_str, int s_index, int count) {
	insert(len, source_str, s_index, count);
}

void MyString::append(const MyString& source_str, int s_index, int count) {
	insert(len, source_str, s_index, count);
}

void MyString::replace(int index, int count, const char* source_str, int s_index, int s_count) {
	erase(index, count);
	insert(index, source_str, s_index, s_count);
}

void MyString::replace(int index, int count, const std::string& source_str, int s_index, int s_count) {
	erase(index, count);
	insert(index, source_str, s_index, s_count);
}

void MyString::replace(int index, int count, const MyString& source_str, int s_index, int s_count) {
	erase(index, count);
	insert(index, source_str, s_index, s_count);
}

void MyString::replace(int index, int count, const char* source_str, int s_count) {
	replace(index, count, source_str, 0, s_count);
}

void MyString::replace(int index, int count, const std::string& source_str, int s_count) {
	replace(index, count, source_str, 0, s_count);
}

void MyString::replace(int index, int count, const MyString& source_str, int s_count) {
	replace(index, count, source_str, 0, s_count);
}

void MyString::replace(int index, int count, const char* source_str) {
	replace(index, count, source_str, 0, strlen(source_str));
}

void MyString::replace(int index, int count, const std::string& source_str) {
	replace(index, count, source_str, 0, source_str.length());
}

void MyString::replace(int index, int count, const MyString& source_str) {
	replace(index, count, source_str, 0, source_str.len);
}

MyString MyString::substr(int index, int count) const {
	if (index < 0 || index >= len) 
		throw out_of_range("Insert index out of range");
	if (count <= 0)
		throw invalid_argument("Caunt cannot be negative");
	int del = min(count, len - index);
	return MyString(data + index, del); 
}

MyString MyString::substr(int index) const {
	return substr(index, len - index);
}

int MyString::compare(const MyString& source_str) const {
	if (data == nullptr || source_str.data == nullptr)
		throw invalid_argument("Source MyString is empty");
	int result = strcmp(data, source_str.data);
	if (result < 0) return -1;
	if (result > 0) return 1;
	return 0;
}

bool MyString::operator>(const MyString& source_str) const { return compare(source_str) > 0; }
bool MyString::operator<(const MyString& source_str) const { return compare(source_str) < 0; }
bool MyString::operator>=(const MyString& source_str) const { return compare(source_str) >= 0; }
bool MyString::operator<=(const MyString& source_str) const { return compare(source_str) <= 0; }
bool MyString::operator!=(const MyString& source_str) const { return compare(source_str) != 0; }
bool MyString::operator==(const MyString& source_str) const { return compare(source_str) == 0; }

char& MyString::operator[](int index) {
	if (index < 0 || index >= len) 
		throw out_of_range("Index out of range");
	return data[index];
}

char& MyString::at(int index) {
	if(index < 0 || index >= len)
		throw MyStringError("Index out of range");
	return data[index];
}

MyString& MyString::operator=(const char* source_str) {
	clear(); 
	append(source_str); 
	return *this;
}

MyString& MyString::operator=(const std::string& source_str) {
	*this = source_str.c_str();
	return *this;
}

MyString& MyString::operator=(const MyString& source_str) {
	*this = source_str.c_str();
	return *this;
}

MyString& MyString::operator=(char ch) {
	clear();
	append(1, ch);
	return *this;
}

MyString& MyString::operator=(MyString&& source_str) noexcept {
	delete[] data;
	len = source_str.len;
	cap = source_str.cap;
	data = source_str.data;
		
	source_str.len = 0;
	source_str.cap = 1;
	source_str.data = new char[1];  
	source_str.data[0] = '\0';
	
	return *this;
}

MyString& MyString::operator+=(const char* source_str) {
	insert(len, source_str);
	return *this;
}

MyString& MyString::operator+=(const std::string& source_str) {
	insert(len, source_str);
	return *this;
}

MyString& MyString::operator+=(const MyString& source_str) {
	insert(len, source_str);
	return *this;
}

MyString MyString::operator+(const char* source_str) const {
	MyString result(*this); 
	result += source_str;   
	return result;
}

MyString MyString::operator+(const std::string& source_str) const {
	MyString result(*this); 
	result += source_str;   
	return result;
}

MyString MyString::operator+(const MyString& source_str) const {
	MyString result(*this); 
	result += source_str;   
	return result;
}

//int MyString::find(const char* source_str, int index) const {
//	if (source_str == nullptr || data == nullptr)
//		throw invalid_argument("Source string cannot be nullptr");
//	if (index < 0 || index >= len)
//		throw out_of_range("Index out of range");
//	char* ptr = strstr(data + index, source_str);
//	return ((ptr != nullptr) ? (ptr-data) : -1);
//}
//
//int MyString::find(const MyString& source_str, int index) const {
//    return find(source_str.data, index);
//}
//
//int MyString::find(const std::string& source_str, int index) const {
//	return find(source_str.c_str(), index);
//}
//
//int MyString::find(const char* source_str) const {
//	return find(source_str, 0);
//}
//
//int MyString::find(const std::string& source_str) const {
//	return find(source_str, 0);
//}
//
//int MyString::find(const MyString& source_str) const {
//	return find(source_str, 0);
//}

int MyString::find(const char* source_str, int index) const {
	if (source_str == nullptr || data == nullptr)
		throw invalid_argument("Source string cannot be nullptr");
	if (index < 0 || index >= len)
		throw out_of_range("Index out of range");
	Aho aho;
	aho.add_string(source_str);
	aho.prepare();
	const char* text_start = data + index;
	int ost_len = len - index;

	int pos = aho.find_first(text_start, ost_len, 0); 
	if (pos == -1) return -1;
	return pos + index;  
}

int MyString::find(const std::string& source_str, int index) const {
	return find(source_str.c_str(), index);
}

int MyString::find(const MyString& source_str, int index) const {
	return find(source_str.data, index);
}

int MyString::find(const char* source_str) const {
	return find(source_str, 0);
}

int MyString::find(const std::string& source_str) const {
	return find(source_str, 0);
}

int MyString::find(const MyString& source_str) const {
	return find(source_str, 0);
}

std::ofstream& operator<<(std::ofstream& ofs, const MyString& str) {
	if (!ofs.is_open()) 
		throw std::runtime_error("File is not open");
	if (str.data != nullptr && str.len > 0) {
		ofs.write(str.data, str.len);
		ofs << '\n';
		if (ofs.fail()) 
			throw std::runtime_error("Failed to write to file");
	}
	return ofs;
}

std::ifstream& operator>>(std::ifstream& ifs, MyString& str) {
	if (!ifs.is_open())
		throw std::runtime_error("Error in opening file");
	const int buffer_size = 256;
	char buffer[buffer_size] = { 0 };  
	str.clear();
	if (ifs.getline(buffer, buffer_size, '\n')) 
		str = buffer;
	else 
		throw std::runtime_error("Error in reading file");
	return ifs;
}

int MyString::to_int() const {
	if (len == 0) 
		throw MyStringError("Invalid integer conversion: empty string");
	int result = 0;
	bool negative = false;
	int i = 0;
	if (data[0] == '-') {
		negative = true;
		i = 1;
	}
	for (; i < len; ++i) {
		char ch = data[i];
		if (ch < '0' || ch > '9') 
			throw MyStringError("Invalid integer conversion: non-digit character");
		result = result * 10 + (ch - '0');
	}
	return negative ? -result : result;
}

float MyString::to_float() const {
	if (len == 0)
		throw MyStringError("Invalid float conversion: empty string");
	int dot_index = -1;
	for (int i = 0; i < len; ++i) {
		if (data[i] == '.') {
			if (dot_index != -1) 
				throw MyStringError("Invalid float conversion: multiple decimal points");
			dot_index = i;
		}
	}
	if (dot_index == -1) 
		throw MyStringError("Invalid float conversion: not decimal points");
	if (dot_index == 0 || dot_index == len - 1) 
		throw MyStringError("Invalid float conversion: decimal point at edge");
	MyString int_part_str(data, dot_index);
	int int_part = int_part_str.to_int();
	MyString frac_part_str(data + dot_index + 1, len - dot_index - 1);
	int frac_part = frac_part_str.to_int();
	float result = static_cast<float>(int_part);
	if (frac_part > 0) {
		float fractional = static_cast<float>(frac_part);
		for (int i = 0; i < frac_part_str.size(); ++i) 
			fractional /= 10.0f;
		result = int_part < 0 ? result - fractional : result + fractional;
	}
	return result;
}






