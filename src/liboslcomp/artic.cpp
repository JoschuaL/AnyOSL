//
// Created by misha on 14/06/2021.
//
#include "artic.h"

void
ArticSource::add_source_with_indent(std::string code)
{
    for(int i = 0; i < m_indent; i++){
        m_code += m_indent_string;
    }
    m_code += code;
}
void
ArticSource::add_source(std::string code)
{
    m_code += code;
}
int
ArticSource::push_indent()
{
    return m_indent++;
}
int
ArticSource::pop_indent()
{
    return m_indent--;
}
std::string
ArticSource::get_code()
{
    return m_code;
}
void
ArticSource::print()
{
    std::cout << m_code << std::endl;
}
void
ArticSource::newline()
{
    m_code += "\n";
}
std::string
ArticSource::pop_temp_characters(int num_chars)
{
    auto poped = "";
    for (int i = 0; i < num_chars; ++i) {
        poped += m_buffer[m_code.length() - 1];
        m_buffer.pop_back();
    }
    return poped;
}

void
ArticSource::save_temp_with_indent(std::string&& in)
{
    for(int i = 0; i < m_indent; i++){
        m_buffer += m_indent_string;
    }
    m_buffer += in;
}

void
ArticSource::save_temp(std::string in)
{
    m_buffer += in;
}

std::string&& ArticSource::pop_temp()
{
    return std::move(m_buffer);
}
ArticSource
ArticSource::make_temp_source()
{
    auto is = this->m_indent_string;
    auto a = ArticSource(is);
    a.m_indent = this->m_indent;
    return a;
}
void ArticSource::integrate_temp_source(ArticSource&& articSource) {
    this->save_temp(std::move(articSource.m_code));
}
