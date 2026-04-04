#ifndef MYSTRING_H
#define MYSTRING_H
#include <iostream>

class MyString
{
public:
	MyString();
	MyString(const char* source_str); 
	MyString(const std::string& source_str);    
	MyString(const MyString& source_str);   
	MyString(const char* sourse_str, int count);
	MyString(const std::string& source_str, int count);
	MyString(const MyString& source_str, int count);
	MyString(int count, char ch);
	explicit MyString(int number);
	explicit MyString(double number);
	MyString(MyString&& source_str) noexcept;
	~MyString();

	void clear();
	void shrink_to_fit();
	const char* c_str() const { return data; }
	int size() const { return len; }
	int capacity() const { return cap; }
	bool empty() { return len == 0; }
	void erase(int index, int count);

	void insert(int index, int count, char ch);
	void insert(int index, const char* source_str);
	void insert(int index, const std::string& source_str);
	void insert(int index, const MyString& source_str);
	void insert(int index, const char* source_str, int count);
	void insert(int index, const std::string& source_str, int count);
	void insert(int index, const MyString& source_str, int count);
	void insert(int index, const char* source_str, int s_index, int count);
	void insert(int index, const std::string& source_str, int s_index, int count);
	void insert(int index, const MyString& source_str, int s_index, int count);

	void append(int count, char ch);
	void append(const char* source_str);
	void append(const std::string& source_str);
	void append(const MyString& source_str);
	void append(const char* source_str, int count);
	void append(const std::string& source_str, int count);
	void append(const MyString& source_str, int count);
	void append(const char* source_str,int s_index, int count);
	void append(const std::string& source_str, int s_index, int count);
	void append(const MyString& source_str, int s_index, int count);

	void replace(int index, int count, const char* source_str);
	void replace(int index, int count, const std::string& source_str);
	void replace(int index, int count, const MyString& source_str);
	void replace(int index, int count, const char* source_str, int s_count);
	void replace(int index, int count, const std::string& source_str, int s_count);
	void replace(int index, int count, const MyString& source_str, int s_count);
	void replace(int index, int count, const char* source_str,int s_index, int s_count);
	void replace(int index, int count, const std::string& source_str, int s_index, int s_count);
	void replace(int index, int count, const MyString& source_str, int s_index, int s_count);

	MyString substr(int index) const;
	MyString substr(int index, int count) const;

	int compare(const MyString& source_str) const;

	bool operator==(const MyString& soutce_str) const;
	bool operator!=(const MyString& soutce_str) const;
	bool operator<(const MyString& soutce_str) const;
	bool operator<=(const MyString& soutce_str) const;
	bool operator>(const MyString& soutce_str) const;
	bool operator>=(const MyString& soutce_str) const;

	char& operator[](int index);
	char& at(int index);
	
	MyString& operator=(const char* source_str);
	MyString& operator=(const std::string& source_str);
	MyString& operator=(const MyString& source_str);
	MyString& operator=(char ch);
	MyString& operator=(MyString&& other) noexcept;

	MyString& operator+=(const char* source_str);
	MyString& operator+=(const std::string& source_str);
	MyString& operator+=(const MyString& source_str);

	MyString operator+(const char* source_str) const;
	MyString operator+(const std::string& source_str) const;
	MyString operator+(const MyString& source_str) const;

	int find(const char* source_str) const;
	int find(const std::string& source_str) const;
	int find(const MyString& source_str) const;
	int find(const char* source_str, int index) const;
	int find(const std::string& source_str, int index) const;
	int find(const MyString& source_str, int index) const;

	/*int find_aho(const char* source_str) const;
	int find_aho(const std::string& source_str) const;
	int find_aho(const MyString& source_str) const;
	int find_aho(const char* source_str, int index) const;
	int find_aho(const std::string& source_str, int index) const;
	int find_aho(const MyString& source_str, int index) const;*/

	// friend - чтобы операторы имели доступ к private полям (тут MyString справа от оператора, поэтому надо)
	friend std::ofstream& operator<<(std::ofstream& ofs, const MyString& str);
	friend std::ifstream& operator>>(std::ifstream& ifs, MyString& str);

	int to_int() const;
	float to_float() const;

private:
	int len;
	int cap;
	char* data;
};

#endif

