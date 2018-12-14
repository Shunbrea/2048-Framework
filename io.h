#pragma once
#include <string>
#include <sstream>
#include <iostream>

class input {
public:
	input(std::istream& in = std::cin) : in(in) {}
	input(const input&) = delete;
	operator bool() const { return bool(in); }
	operator std::string() { std::string line; operator>>(line); return line; }
	input& operator>>(std::string& line) {
		if (std::getline(in, line, '\n'))
			if (line.size() && line.back() == '\r') line.pop_back();
		return *this;
	}
	input& operator =(const input&) = delete;
private:
	std::istream& in;
};

class output {
public:
	output(const std::string& init = "", std::ostream& out = std::cout) : out(out), buf(init) {}
	output(const output&) = delete;
	~output() { out << buf.str() << std::flush; }
	template<typename type> output& operator<<(const type& v) { buf << v; return *this; }
	output& operator<<(std::ios_base& (*pf)(std::ios_base&)) { buf << pf; return *this; }
	output& operator<<(std::ostream& (*pf)(std::ostream&)) { buf << pf; return *this; }
	output& operator =(const output&) = delete;
private:
	std::ostream& out;
	std::stringstream buf;
};

class info : public output {
public:
	info(const std::string& init = "") : output(init, std::cerr) {}
	info(const info&) = delete;
	info& operator =(const info&) = delete;
};

class error : public output {
public:
	error(const std::string& init = "") : output(init, std::cerr) {}
	error(const error&) = delete;
	error& operator =(const error&) = delete;
};

