//
// Created by misha on 14/06/2021.
//
#include "artic.h"


OSL_NAMESPACE_ENTER


using pvt::ASTNode;
using pvt::TypeSpec;

std::string
op_to_string(int op_int)
{
    ASTNode::Operator op = static_cast<ASTNode::Operator>(op_int);
    switch (op) {
    case ASTNode::Nothing: NOT_IMPLEMENTED; break;
    case ASTNode::Decr: return "dec"; break;
    case ASTNode::Incr: return "inc"; break;
    case ASTNode::Assign: return "="; break;
    case ASTNode::Mul: return "mul"; break;
    case ASTNode::Div: return "div"; break;
    case ASTNode::Add: return "add"; break;
    case ASTNode::Sub: return "sub"; break;
    case ASTNode::Mod: return "mod"; break;
    case ASTNode::Equal: return "eq"; break;
    case ASTNode::NotEqual: return "neq"; break;
    case ASTNode::Greater: return "ge"; break;
    case ASTNode::Less: return "le"; break;
    case ASTNode::GreaterEqual: return "geq"; break;
    case ASTNode::LessEqual: return "leq"; break;
    case ASTNode::BitAnd: return "band"; break;
    case ASTNode::BitOr: return "bor"; break;
    case ASTNode::Xor: return "bxor"; break;
    case ASTNode::Compl: return "bcomp"; break;
    case ASTNode::And: return "land"; break;
    case ASTNode::Or: return "lor"; break;
    case ASTNode::Not: return "lnot"; break;
    case ASTNode::ShiftLeft: return "shiftl"; break;
    case ASTNode::ShiftRight: return "shiftr"; break;
    default: NOT_IMPLEMENTED; break;
    }
}

std::string
artic_type_string_to_string(TypeSpec typeSpec)
{
    auto tstring = artic_string(typeSpec);
    size_t pos;
    while ((pos = tstring.find('[')) != std::string::npos) {
        tstring.replace(pos, 1, "_");
    }

    while ((pos = tstring.find('<')) != std::string::npos) {
        tstring.replace(pos, 1, "_");
    }

    while ((pos = tstring.find('>')) != std::string::npos) {
        tstring.replace(pos, 1, "_");
    }
    return tstring;
}


const std::string
artic_string(TypeSpec typeSpec)
{
    if (typeSpec.is_array()) {
        std::string start = "";
        std ::string end  = "";
        start += "[";

        start += artic_string(typeSpec.elementtype());


        if (typeSpec.is_sized_array()) {
            end += ";";
            end += std::to_string(typeSpec.arraylength());
        }
        end += "]";
        return start + end;
    } else {
        if (typeSpec.is_closure()) {
            return "Closure";

        } else if (typeSpec.is_structure()) {
            return typeSpec.structspec()->name().string();
        } else {
            return artic_simpletype(typeSpec.simpletype());
        }
    }
}

const std::string
artic_simpletype(TypeDesc st)
{
    if (st.is_unknown()) {
        NOT_IMPLEMENTED;
    }
    std::string start = "";
    std::string end   = "";
    if (st.is_array()) {
        start += "[";
        if (st.is_sized_array()) {
            end += ";";
            end += std::to_string(st.arraylen);
        }
        end += "]";
    }

    if (st.elementtype().is_vec3(TypeDesc::FLOAT)) {
        auto ste = st.elementtype();
        if (ste == TypeDesc::TypeColor) {
            start += "Color";
        } else if (ste == TypeDesc::TypePoint) {
            start += "Point";
        } else if (ste == TypeDesc::TypeVector) {
            start += "Vector";
        } else if (ste == TypeDesc::TypeNormal) {
            start += "Normal";
        } else {
            NOT_IMPLEMENTED;
        }
    } else if (st == TypeDesc::TypeMatrix) {
        start += "Matrix";
    } else if (st == TypeDesc::TypeString) {
        start += "String";
    } else if (st.is_floating_point()) {
        start += "f32";
    } else {
        if (st.is_signed()) {
            start += "i";
        } else {
            start += "u";
        }
        start += "32";
    }
    return start + end;
}

template<typename ... Args>
void ArticSource::add_source_with_indent(const std::string& code, Args ... args)
{
    for (int i = 0; i < m_indent; i++) {
        m_code += m_indent_string;
    }
    add_source(code, args...);
}



template<typename ... Args>
void ArticSource::add_source(const std::string& code, Args ... args)
{
    m_code += code;
    add_source(args...);
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
    for (int i = 0; i < m_indent; i++) {
        m_buffer += m_indent_string;
    }
    m_buffer += in;
}

void
ArticSource::save_temp(std::string in)
{
    m_buffer += in;
}

std::string&&
ArticSource::pop_temp()
{
    return std::move(m_buffer);
}
ArticSource
ArticSource::make_temp_source()
{
    auto is    = this->m_indent_string;
    auto a     = ArticSource(is);
    a.m_indent = this->m_indent;
    return a;
}
void
ArticSource::integrate_temp_source(ArticSource&& articSource)
{
    this->save_temp(std::move(articSource.m_code));
}

void ArticTranspiler::dispatch_node(ASTNode::ref n)
{
    auto node = n.get();
    switch (node->nodetype()) {
    case ASTNode::unknown_node:
        NOT_IMPLEMENTED;
        break;
    case ASTNode::shader_declaration_node:
        transpile_shader_declaration((ASTshader_declaration*) node);
        break;
    case ASTNode::function_declaration_node:
        transpile_function_declaration((ASTfunction_declaration*) node);
        break;
    case ASTNode::variable_declaration_node:
        transpile_variable_declaration((ASTvariable_declaration*) node);
        break;
    case ASTNode::compound_initializer_node:
        transpile_compound_initializer((ASTcompound_initializer*) node);
        break;
    case ASTNode::variable_ref_node:
        transpile_variable_ref((ASTvariable_ref*) node);
        break;
    case ASTNode::preincdec_node:
        transpile_preincdec((ASTpreincdec*) node);
        break;
    case ASTNode::postincdec_node:
        transpile_postincdec((ASTpostincdec*)node);
        break;
    case ASTNode::index_node:
        transpile_index((ASTindex*) node);
        break;
    case ASTNode::structselect_node:
        transpile_structureselection((ASTstructselect*) node);
        break;
    case ASTNode::conditional_statement_node:
        transpile_conditional_statement((ASTconditional_statement*) node);
        break;
    case ASTNode::loop_statement_node:
        transpile_loop_statement((ASTloop_statement*) node);
        break;
    case ASTNode::loopmod_statement_node:
        transpile_loopmod_statement((ASTloopmod_statement*) node);
        break;
    case ASTNode::return_statement_node:
        transpile_return_statement((ASTreturn_statement*) node);
        break;
    case ASTNode::binary_expression_node:
        transpile_binary_expression((ASTbinary_expression*) node);
        break;
    case ASTNode::unary_expression_node:
        transpile_unary_expression((ASTunary_expression*) node);
        break;
    case ASTNode::assign_expression_node:
        transpile_assign_expression((ASTassign_expression*) node);
        break;
    case ASTNode::ternary_expression_node:
        transpile_ternary_expression((ASTternary_expression *) node);
        break;
    case ASTNode::comma_operator_node:
        transpile_comma_operator((ASTcomma_operator*) node);
        break;
    case ASTNode::typecast_expression_node:
        transpile_typecast_expression((ASTtypecast_expression*) node);
        break;
    case ASTNode::type_constructor_node:
        transpile_type_constructor((ASTtype_constructor*) node);
        break;
    case ASTNode::function_call_node:
        transpile_function_call((ASTfunction_call*) node);
        break;
    case ASTNode::literal_node:
        transpile_literal_node((ASTliteral*) node);
        break;
    case ASTNode::_last_node:
        NOT_IMPLEMENTED;
        break;
    }
}
void
ArticTranspiler::transpile_shader_declaration(ASTshader_declaration* node)
{
    auto shadername = node->shadername().string();
    source.add_source_with_indent("struct ", shadername, "_in {\n");
    source.push_indent();
    std::vector<ASTvariable_declaration*> inputs  = {};
    std::vector<ASTvariable_declaration*> outputs = {};
    for (ASTNode::ref f = node->formals(); f; f = f->next()) {
        auto v = (ASTvariable_declaration*)f.get();
        if (v->is_output()) {
            outputs.push_back(v);
        }
        inputs.push_back(v);
        source.add_source_with_indent(v->name().string(), ": ",
                                      artic_string(v->typespec()), ",\n");
    }
    source.pop_indent();
    source.add_source_with_indent("}\n\n");

    source.add_source_with_indent("fn make_", shadername, "_in -> ", shadername,
                                  "_in {\n");
    source.push_indent();
    source.add_source_with_indent(shadername, "_in{\n");
    source.push_indent();
    for (auto v : inputs) {
        source.add_source_with_indent(v->name().string(), " = ");
        dispatch_node(v->init());
        source.add_source(",\n");
    }
    source.pop_indent();
    source.add_source_with_indent("}\n");
    source.pop_indent();
    source.add_source_with_indent("}\n\n");

    source.add_source_with_indent("struct ", shadername, "_out {\n");
    source.push_indent();
    for (auto v : outputs) {
        source.add_source_with_indent(v->name().string(), ": ",
                                      artic_string(v->typespec()), ",\n");
    }
    source.pop_indent();
    source.add_source_with_indent("}\n\n");

    source.add_source_with_indent("fn ", shadername, "_impl(in: ", shadername,
                                  "_in) -> ", shadername, "_out {\n");
    source.push_indent();
    for (auto v : inputs) {
        source.add_source_with_indent("let ", v->name().string(), " = in.",
                                      v->name().string(), ",\n");
    }

    transpile_statement_list(node->statements());

    source.add_source_with_indent(node->shadername().string(), "_out {\n");
    source.push_indent();
    for (auto v : outputs) {
        source.add_source_with_indent(v->name().string(), " = ",
                                      v->name().string(), ",\n");
    }
    source.pop_indent();
    source.add_source_with_indent("}\n");
    source.pop_indent();
    source.add_source_with_indent("}\n\n");
}

void ArticTranspiler::transpile_statement_list(ASTNode::ref node)
{
    while (node) {
        dispatch_node(node);
        node      = node->next();
    }
}
void
ArticTranspiler::transpile_function_declaration(ASTfunction_declaration* node)
{
    NOT_IMPLEMENTED;
}

void
ArticTranspiler::transpile_variable_declaration(ASTvariable_declaration* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_compound_initializer(ASTcompound_initializer* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_variable_ref(ASTvariable_ref* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_preincdec(ASTpreincdec* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_postincdec(ASTpostincdec* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_index(ASTindex* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_structureselection(ASTstructselect* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_conditional_statement(ASTconditional_statement* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_loop_statement(ASTloop_statement* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_loopmod_statement(ASTloopmod_statement* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_return_statement(ASTreturn_statement* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_binary_expression(ASTbinary_expression* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_unary_expression(ASTunary_expression* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_assign_expression(ASTassign_expression* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_ternary_expression(ASTternary_expression* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_comma_operator(ASTcomma_operator* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_typecast_expression(ASTtypecast_expression* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_type_constructor(ASTtype_constructor* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_function_call(ASTfunction_call* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_literal_node(ASTliteral* node)
{
    NOT_IMPLEMENTED;
}

OSL_NAMESPACE_EXIT