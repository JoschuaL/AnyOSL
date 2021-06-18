//
// Created by misha on 14/06/2021.
//

#ifndef OSL_ARTIC_H
#define OSL_ARTIC_H

#include <string>
#include <utility>
#include <iostream>

class ArticSource{
public:
    ArticSource(std::string indent_string): m_indent(0), m_indent_string(std::move(indent_string)), m_code(){};
    void add_source_with_indent(std::string code);
    void add_source(std::string code);
    void newline();
    int push_indent();
    int pop_indent();
    std::string get_code();
    void print();
    std::string pop_temp_characters(int num_chars = 1);
    void save_temp(std::string in);
    void save_temp_with_indent(std::string&& in);
    std::string&& pop_temp();
    ArticSource make_temp_source();
    void integrate_temp_source(ArticSource&& articSource);
private:
    int m_indent;
    std::string m_indent_string;
    std::string m_code;
    std::string m_buffer;
};

#endif  //OSL_ARTIC_H
