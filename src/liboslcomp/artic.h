//
// Created by misha on 14/06/2021.
//

#ifndef OSL_ARTIC_H
#define OSL_ARTIC_H

#include <iostream>
#include <string>
#include <utility>
#include "ast.h"

OSL_NAMESPACE_ENTER


#define NOT_IMPLEMENTED                  \
    do {                                 \
        std::cerr << "NOT IMPLEMENTED "; \
        OSL_ASSERT(false);               \
        exit(3);                         \
    } while (0)


using namespace pvt;

std::string
artic_type_string_to_string(TypeSpec typeSpec);

const std::string
artic_string(TypeSpec typeSpec);

const std::string
artic_simpletype(TypeDesc typedesc);


class ArticSource {
public:
    ArticSource(std::string indent_string)
        : m_indent(0), m_indent_string(std::move(indent_string)), m_code() {};

    template <typename ... Args>
    void add_source_with_indent(const std::string& code, Args ... args);



    template <typename ... Args>
    void add_source(const std::string& code, Args ... args);
    inline void add_source(){};





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


class ArticTranspiler {
    ArticTranspiler(ArticSource source) : source(source) {}

public:



private:
    void dispatch_node(ASTNode::ref);

    void transpile_shader_declaration(ASTshader_declaration* node);

    void transpile_function_declaration(ASTfunction_declaration* node);

    void transpile_variable_declaration(ASTvariable_declaration* node);

    void transpile_compound_initializer(ASTcompound_initializer* node);

    void transpile_variable_ref(ASTvariable_ref* node);

    void transpile_preincdec(ASTpreincdec* node);

    void transpile_postincdec(ASTpostincdec* node);

    void transpile_index(ASTindex* node);

    void transpile_structureselection(ASTstructselect* node);

    void transpile_conditional_statement(ASTconditional_statement* node);

    void transpile_loop_statement(ASTloop_statement* node);

    void transpile_loopmod_statement(ASTloopmod_statement* node);

    void transpile_return_statement(ASTreturn_statement* node);

    void transpile_binary_expression(ASTbinary_expression* node);

    void transpile_unary_expression(ASTunary_expression* node);

    void transpile_assign_expression(ASTassign_expression* node);

    void transpile_ternary_expression(ASTternary_expression* node);

    void transpile_comma_operator(ASTcomma_operator* node);

    void transpile_typecast_expression(ASTtypecast_expression* node);

    void transpile_type_constructor(ASTtype_constructor* node);

    void transpile_function_call(ASTfunction_call* node);

    void transpile_literal_node(ASTliteral* node);

    void transpile_statement_list(ASTNode::ref node);



    ArticSource source;
};

OSL_NAMESPACE_EXIT


#endif  //OSL_ARTIC_H
