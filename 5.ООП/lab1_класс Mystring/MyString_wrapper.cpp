#include "MyString.h"
#include <pybind11/pybind11.h>

namespace py = pybind11;

PYBIND11_MODULE(MyString, m) {
    py::class_<MyString>(m, "MyString")
        //Конструкторы
        .def(py::init<>())
        .def(py::init<const char*>())
        .def(py::init<const std::string&>())
        .def(py::init<const MyString&>())
        .def(py::init<const char*, int>())
        .def(py::init<const std::string&, int>())
        .def(py::init<const MyString&, int>())
        .def(py::init<int, char>())

        //Базовые (без перегрузок)
        .def("clear", &MyString::clear)
        .def("shrink_to_fit", &MyString::shrink_to_fit)
        .def("size", &MyString::size)
        .def("capacity", &MyString::capacity)
        .def("empty", &MyString::empty)
        .def("c_str", &MyString::c_str)
        .def("erase", &MyString::erase)
        .def("compare", &MyString::compare)

        //Операторы сравнения
        .def("__eq__", &MyString::operator==)
        .def("__ne__", &MyString::operator!=)
        .def("__lt__", &MyString::operator<)
        .def("__le__", &MyString::operator<=)
        .def("__gt__", &MyString::operator>)
        .def("__ge__", &MyString::operator>=)

        // Оператор взятия значения по индексу и присваивания:
        .def("__getitem__", [](MyString& self, int index) -> char {return self[index];})
        .def("__setitem__", [](MyString& self, int index, char ch) {return self[index] = ch;})

        // Оператор присваивания (используется лямбда-функция для перегрузки)
        // [] - захват переменных, () - параметры фукнции, -> - указатель возвращаемого типа, {} - тело функции
        .def("__assign__", [](MyString& self, const char* source_str)  { 
            return self = source_str; 
        }, py::return_value_policy::reference_internal)
        .def("__assign__", [](MyString& self, const std::string& source_str)  { 
            return self = source_str; 
        }, py::return_value_policy::reference_internal)
        .def("__assign__", [](MyString& self, const MyString& source_str)  { 
            return self = source_str; 
        }, py::return_value_policy::reference_internal)
        .def("__assign__", [](MyString& self, char ch) { 
            return self = ch; 
        }, py::return_value_policy::reference_internal)

        // Оператор += конкатенация-расширение return_value_policy::reference_internal - возвращает ссылку
        .def("__iadd__", [](MyString& self, const char* source_str) { 
            return self += source_str; 
        }, py::return_value_policy::reference_internal)
        .def("__iadd__", [](MyString& self, const std::string& source_str) { 
            return self += source_str; 
        }, py::return_value_policy::reference_internal)
        .def("__iadd__", [](MyString& self, const MyString& source_str) { 
            return self += source_str; 
        }, py::return_value_policy::reference_internal)

        // Оператор конкатенации
        .def("__add__", [](const MyString& self, const char* source_str) { 
            return self + source_str; 
        })
        .def("__add__", [](const MyString& self, const std::string& source_str) { 
            return self + source_str; 
        })
        .def("__add__", [](const MyString& self, const MyString& source_str) { 
            return self + source_str; 
            })

            // Insert
        .def("insert", [](MyString& self, int index, int count, char ch) {self.insert(index, count, ch); })
        .def("insert", [](MyString& self, int index, const char* source_str) {self.insert(index, source_str); })
        .def("insert", [](MyString& self, int index, const std::string& source_str) {self.insert(index, source_str); })
        .def("insert", [](MyString& self, int index, const MyString& source_str) {self.insert(index, source_str); })
        .def("insert", [](MyString& self, int index, const char* source_str, int count) {self.insert(index, source_str, count); })
        .def("insert", [](MyString& self, int index, const std::string& source_str, int count) {self.insert(index, source_str, count); })
        .def("insert", [](MyString& self, int index, const MyString& source_str, int count) {self.insert(index, source_str, count); })
        .def("insert", [](MyString& self, int index, const char* source_str, int s_index, int count) {self.insert(index, source_str, s_index, count); })
        .def("insert", [](MyString& self, int index, const std::string& source_str, int s_index, int count) {self.insert(index, source_str, s_index, count); })
                .def("insert", [](MyString& self, int index, const MyString& source_str, int s_index, int count) {self.insert(index, source_str, s_index, count); })

        // Substr
        .def("substr", [](const MyString& self, int index) {return self.substr(index);})
        .def("substr", [](const MyString& self, int index, int count) {return self.substr(index, count);})

        //Replace
        .def("replace", [](MyString& self, int index, int count, const char* source_str) {self.replace(index, count, source_str);})
        .def("replace", [](MyString& self, int index, int count, const std::string& source_str) {self.replace(index, count, source_str);})
        .def("replace", [](MyString& self, int index, int count, const MyString& source_str) {self.replace(index, count, source_str);})
        .def("replace", [](MyString& self, int index, int count, const char* source_str, int s_count) {self.replace(index, count, source_str, s_count);})
        .def("replace", [](MyString& self, int index, int count, const std::string& source_str, int s_count) {self.replace(index, count, source_str, s_count);})
        .def("replace", [](MyString& self, int index, int count, const MyString& source_str, int s_count) {self.replace(index, count, source_str, s_count);})
        .def("replace", [](MyString& self, int index, int count, const char* source_str, int s_index, int s_count) {self.replace(index, count, source_str, s_index, s_count);})
        .def("replace", [](MyString& self, int index, int count, const std::string& source_str, int s_index, int s_count) {self.replace(index, count, source_str, s_index, s_count);})
        .def("replace", [](MyString& self, int index, int count, const MyString& source_str, int s_index, int s_count) {self.replace(index, count, source_str, s_index, s_count);})

         // Append
           .def("append", [](MyString& self, int count, char ch) {self.append(count, ch);})
           .def("append", [](MyString& self, const char* source_str) {self.append(source_str);})
           .def("append", [](MyString& self, const std::string& source_str) {self.append(source_str);})
           .def("append", [](MyString& self, const MyString& source_str) {self.append(source_str);})
           .def("append", [](MyString& self, const char* source_str, int count) {self.append(source_str, count);})
           .def("append", [](MyString& self, const std::string& source_str, int count) {self.append(source_str, count);})
           .def("append", [](MyString& self, const MyString& source_str, int count) {self.append(source_str, count);})
           .def("append", [](MyString& self, const char* source_str, int s_index, int count) {self.append(source_str, s_index, count);})
           .def("append", [](MyString& self, const std::string& source_str, int s_index, int count) {self.append(source_str, s_index, count);})
           .def("append", [](MyString& self, const MyString& source_str, int s_index, int count) {self.append(source_str, s_index, count);})
        
        // Find
        .def("find", [](const MyString& self, const char* source_str) {return self.find(source_str);})
        .def("find", [](const MyString& self, const std::string& source_str) {return self.find(source_str);})
        .def("find", [](const MyString& self, const MyString& source_str) {return self.find(source_str); })
        .def("find", [](const MyString& self, const char* source_str, int index) {return self.find(source_str, index);})
        .def("find", [](const MyString& self, const std::string& source_str, int index) {return self.find(source_str, index);})
        .def("find", [](const MyString& self, const MyString& source_str, int index) {return self.find(source_str, index);})

        //Магические
        .def("__len__", &MyString::size)
        .def("__str__", [](const MyString& s) { return std::string(s.c_str()); })
        .def("__repr__", [](const MyString& s) { return "MyString(\"" + std::string(s.c_str()) + "\")"; });
}