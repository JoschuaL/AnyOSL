//
// Created by misha on 14/06/2021.
//
#include "artic.h"


OSL_NAMESPACE_ENTER


using pvt::ASTNode;
using pvt::TypeSpec;



std::string
artic_type_string_to_string(std::string in)
{
    auto tstring = in;
    size_t pos;
    while ((pos = tstring.find('[')) != std::string::npos) {
        tstring.replace(pos, 1, "_");
    }

    while ((pos = tstring.find(']')) != std::string::npos) {
        tstring.replace(pos, 1, "_");
    }

    while ((pos = tstring.find('*')) != std::string::npos) {
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
artic_string(TypeSpec typeSpec, int array_size)
{
    if (typeSpec.is_array()) {
        std::string start = "";
        std ::string end  = "";
        start += "[";

        start += artic_string(typeSpec.elementtype(), 0);
        end += "*";

        if (typeSpec.is_sized_array()) {
            end += std::to_string(typeSpec.arraylength());
        } else {
            if(array_size > 0){
                end += std::to_string(array_size);
            } else {
                return artic_string(typeSpec.elementtype(), 0) + "Array";
            }

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

int
get_array_size(ASTNode::ref node)
{
    auto typeSpec = node->typespec();
    if (typeSpec.is_sized_array()) {
        return typeSpec.arraylength();
    } else if (typeSpec.is_unsized_array()) {
        if (node->nodetype() == ASTNode::NodeType::compound_initializer_node) {
            int i     = 0;
            auto n = ((ASTcompound_initializer*)node.get())->initlist();
            while (n) {
                i++;
                n = n->next();
            }
            return i;
        } else if(node->nodetype() == ASTNode::NodeType::variable_ref_node){
            auto n = (ASTvariable_ref*) node.get();
            auto d = n->sym()->node();
            if(d->nodetype() == ASTNode::NodeType::variable_declaration_node){
                auto decl = (ASTvariable_declaration*) d;
                if(decl->init()){
                    return get_array_size(decl->init());
                } else {
                    NOT_IMPLEMENTED;
                }
            } else {
                NOT_IMPLEMENTED;
            }
        } else {
            NOT_IMPLEMENTED;
        }

    } else {
        NOT_IMPLEMENTED;
    }
    return 0;
}

const std::string
get_artic_type_string(ASTNode::ref node)
{
    auto typeSpec = node->typespec();
    if (typeSpec.is_unsized_array()) {
        switch (node->nodetype()) {
        case ASTNode::variable_declaration_node:
            return artic_string(
                node->typespec(),
                get_array_size(
                    ((ASTvariable_declaration*)node.get())->init()));
        default:
            return artic_string(typeSpec, 0);
            break;
        }
    } else {
        return artic_string(typeSpec, 0);
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
            end += "*";
            end += std::to_string(st.arraylen);
        }
        end += "]";
    }

    if (st.elementtype().is_vec3(TypeDesc::FLOAT)) {
        start += "Vector";
    } else if (st == TypeDesc::TypeMatrix) {
        start += "Matrix";
    } else if (st == TypeDesc::TypeString) {
        start += "String";
    } else if (st.is_floating_point()) {
        start += "f32";
    } else if (st.basetype == 1) {
        start += "()";
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

template<typename... Args>
void
ArticSource::add_source_with_indent(const std::string& code, Args... args)
{
    for (int i = 0; i < m_indent; i++) {
        m_code += m_indent_string;
    }
    add_source(code, args...);
}



template<typename... Args>
void
ArticSource::add_source(const std::string& code, Args... args)
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



void
ArticTranspiler::dispatch_node(ASTNode::ref n)
{
    auto node = n.get();
    switch (node->nodetype()) {
    case ASTNode::unknown_node: NOT_IMPLEMENTED; break;
    case ASTNode::shader_declaration_node:
        transpile_shader_declaration((ASTshader_declaration*)node);
        break;
    case ASTNode::function_declaration_node:
        transpile_function_declaration((ASTfunction_declaration*)node);
        break;
    case ASTNode::variable_declaration_node:
        transpile_variable_declaration((ASTvariable_declaration*)node);
        break;
    case ASTNode::compound_initializer_node:
        transpile_compound_initializer((ASTcompound_initializer*)node);
        break;
    case ASTNode::variable_ref_node:
        transpile_variable_ref((ASTvariable_ref*)node);
        break;
    case ASTNode::preincdec_node:
        transpile_preincdec((ASTpreincdec*)node);
        break;
    case ASTNode::postincdec_node:
        transpile_postincdec((ASTpostincdec*)node);
        break;
    case ASTNode::index_node: transpile_index((ASTindex*)node); break;
    case ASTNode::structselect_node:
        transpile_structureselection((ASTstructselect*)node);
        break;
    case ASTNode::conditional_statement_node:
        transpile_conditional_statement((ASTconditional_statement*)node);
        break;
    case ASTNode::loop_statement_node:
        transpile_loop_statement((ASTloop_statement*)node);
        break;
    case ASTNode::loopmod_statement_node:
        transpile_loopmod_statement((ASTloopmod_statement*)node);
        break;
    case ASTNode::return_statement_node:
        transpile_return_statement((ASTreturn_statement*)node);
        break;
    case ASTNode::binary_expression_node:
        transpile_binary_expression((ASTbinary_expression*)node);
        break;
    case ASTNode::unary_expression_node:
        transpile_unary_expression((ASTunary_expression*)node);
        break;
    case ASTNode::assign_expression_node:
        transpile_assign_expression((ASTassign_expression*)node);
        break;
    case ASTNode::ternary_expression_node:
        transpile_ternary_expression((ASTternary_expression*)node);
        break;
    case ASTNode::comma_operator_node:
        transpile_comma_operator((ASTcomma_operator*)node);
        break;
    case ASTNode::typecast_expression_node:
        transpile_typecast_expression((ASTtypecast_expression*)node);
        break;
    case ASTNode::type_constructor_node:
        transpile_type_constructor((ASTtype_constructor*)node);
        break;
    case ASTNode::function_call_node:
        transpile_function_call((ASTfunction_call*)node);
        break;
    case ASTNode::literal_node:
        transpile_literal_node((ASTliteral*)node);
        break;
    case ASTNode::_last_node: NOT_IMPLEMENTED; break;
    }
}
void
ArticTranspiler::transpile_shader_declaration(ASTshader_declaration* node)
{
    this->in_shader      = true;
    ShaderType st        = node->shadertype();
    std::string ststring = shadertypename(st);
    auto shadername      = node->shadername().string();
    source->add_source_with_indent("struct ", shadername, "_in {\n");
    source->push_indent();
    std::vector<ASTvariable_declaration*> inputs  = {};
    std::vector<ASTvariable_declaration*> outputs = {};
    for (ASTNode::ref f = node->formals(); f; f = f->next()) {
        auto v = (ASTvariable_declaration*)f.get();
        if (v->is_output()) {
            outputs.push_back(v);
        }
        inputs.push_back(v);
        source->add_source_with_indent(v->name().string(), ": ",
                                       get_artic_type_string(f), ",\n");
    }
    source->pop_indent();
    source->add_source_with_indent("}\n\n");

    source->add_source_with_indent("fn make_", shadername, "_in() -> ",
                                   shadername, "_in {\n");
    source->push_indent();
    for (auto v : inputs) {
        source->add_source_with_indent("let ", v->name().string(), ": ",
                                       get_artic_type_string(v), " = ");
        if (v->init()->nodetype() == ASTNode::NodeType::literal_node) {
            //TODO: wtf is happening here?
            auto lit = new ASTtype_constructor(v->typespec(), v->init().get());
            dispatch_node(lit);
            //delete lit;
        } else {
            dispatch_node(v->init());
        }
        source->add_source(";\n");
    }

    source->add_source_with_indent(shadername, "_in{\n");
    source->push_indent();
    for (auto v : inputs) {
        source->add_source_with_indent(v->name().string(), " = ",
                                       v->name().string(), ",\n");
    }
    source->pop_indent();
    source->add_source_with_indent("}\n");
    source->pop_indent();
    source->add_source_with_indent("}\n\n");

    source->add_source_with_indent("struct ", shadername, "_out {\n");
    source->push_indent();
    for (auto v : outputs) {
        source->add_source_with_indent(v->name().string(), ": ",
                                       get_artic_type_string(v), ",\n");
    }
    source->pop_indent();
    source->add_source_with_indent("}\n\n");

    source->add_source_with_indent("fn @", shadername,
                                   "_impl(arg_in: ", shadername,
                                   "_in, inout : shader_inout) -> (",
                                   shadername, "_out, shader_inout) {\n");
    source->push_indent();
    for (auto v : inputs) {
        source->add_source_with_indent("let ", v->is_output() ? "mut " : "",
                                       v->name().string(), " = arg_in.",
                                       v->name().string(), ";\n");
    }
    emit_shaderinout_copy();

    transpile_statement_list(node->statements());

    source->add_source_with_indent("(", node->shadername().string(),
                                   "_out {\n");
    source->push_indent();
    for (auto v : outputs) {
        source->add_source_with_indent(v->name().string(), " = ",
                                       v->name().string(), ",\n");
    }
    source->pop_indent();
    source->add_source_with_indent("},\n");
    source->add_source_with_indent("");
    emit_shaderinout_constructor();
    source->add_source(")\n");

    source->pop_indent();
    source->add_source_with_indent("}\n\n");
    this->in_shader = false;
}

void
ArticTranspiler::transpile_statement_list(ASTNode::ref node)
{
    while (node) {
        if (node->nodetype() != ASTNode::NodeType::loop_statement_node
            && node->nodetype()
                   != ASTNode::NodeType::conditional_statement_node) {
            source->add_source_with_indent("");
            dispatch_node(node);
            source->add_source(";\n");
        } else {
            dispatch_node(node);
        }
        node = node->next();
    }
}
void
ArticTranspiler::transpile_function_declaration(ASTfunction_declaration* node)
{
    bool is         = this->in_shader;
    this->in_shader = false;
    if (!node->is_builtin()) {
        source->add_source("fn @", node->func()->name().string());
        auto formal_node = node->formals();
        while (formal_node) {
            source->add_source("_", get_artic_type_string(formal_node));
        }
        source->add_source("__", get_artic_type_string(node), "(");
        formal_node = node->formals();
        while (formal_node) {
            auto decl = (ASTvariable_declaration*)formal_node.get();
            source->add_source(decl->name().string(), ": ",
                               decl->is_output() ? "&mut " : "",
                               decl->typespec().is_array() ? "&" : "",
                               get_artic_type_string(formal_node), ", ");
            if(decl->typespec().is_array()) {
                source->add_source("arraylength: fn() -> i32, ");
            }
            formal_node = formal_node->next();
        }
        source->add_source(", inout: shader_inout) ->",
                           get_artic_type_string(node), "{\n");
        source->push_indent();
        emit_shaderinout_copy();
        transpile_statement_list(node->statements());
        source->pop_indent();
        source->add_source("}\n\n");
    }
    this->in_shader = is;
}

void
ArticTranspiler::transpile_variable_declaration(ASTvariable_declaration* node)
{
    source->add_source("let mut ", node->name().string(), ": ",
                       get_artic_type_string(node));
    if (node->init()) {
        source->add_source(" = ");
        if (node->init()->nodetype()
            != ASTNode::NodeType::type_constructor_node) {
            auto cons = new ASTtype_constructor(node->typespec(),
                                                node->init().get());
            dispatch_node(cons);
        } else {
            dispatch_node(node->init());
        }
    }
}
void
ArticTranspiler::transpile_compound_initializer(ASTcompound_initializer* node)
{
    source->add_source("[");
    auto init_node = node->initlist();
    while (init_node) {
        dispatch_node(init_node);
        source->add_source(", ");
        init_node = init_node->next();
    }
    source->add_source("]");
}
void
ArticTranspiler::transpile_variable_ref(ASTvariable_ref* node)
{
    source->add_source(node->name().string());
}
void
ArticTranspiler::transpile_preincdec(ASTpreincdec* node)
{
    source->add_source("{");
    dispatch_node(node->var());
    source->add_source(" ", node->is_increment() ? "+" : "-", "= 1;");
    dispatch_node(node->var());
    source->add_source("}");
}
void
ArticTranspiler::transpile_postincdec(ASTpostincdec* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_index(ASTindex* node)
{
    auto lval = node->lvalue();

    if (lval->typespec().is_triple()) {
        source->add_source("index_Vector(");
        dispatch_node(lval);
        source->add_source(", ");
        dispatch_node(node->index());
        source->add_source(")");
    } else {
        dispatch_node(lval);
        source->add_source("[");
        dispatch_node(node->index());
        source->add_source("]");
    }
}
void
ArticTranspiler::transpile_structureselection(ASTstructselect* node)
{
    if (node->compindex()) {
        dispatch_node(node->compindex());
    } else {
        dispatch_node(node->lvalue());
        source->add_source(".", node->field().string());
    }
}

void
ArticTranspiler::transpile_conditional_statement(ASTconditional_statement* node)
{
    auto true_node  = node->truestmt();
    auto false_node = node->falsestmt();
    if (node->cond()->nodetype() == ASTNode::NodeType::binary_expression_node
        && ((ASTbinary_expression*)node->cond().get())->is_boolean_operator()) {
        source->add_source_with_indent("if(");
        dispatch_node(node->cond());
        source->add_source(") {\n");
        source->push_indent();
        transpile_statement_list(true_node);
        source->pop_indent();
        source->add_source_with_indent("}");
        if (false_node) {
            source->add_source(" else {\n");
            source->push_indent();
            transpile_statement_list(false_node);
            source->pop_indent();
            source->add_source_with_indent("}");
        }
        source->add_source("\n");
    } else {
        source->add_source_with_indent("if(make_bool_",
                                       get_artic_type_string(node->cond()),
                                       "(");
        dispatch_node(node->cond());
        source->add_source(")) {\n");
        source->push_indent();
        transpile_statement_list(true_node);
        source->pop_indent();
        source->add_source_with_indent("}");
        if (false_node) {
            source->add_source(" else {\n");
            source->push_indent();
            transpile_statement_list(false_node);
            source->pop_indent();
            source->add_source_with_indent("}");
        }
        source->add_source("\n");
    }
}
void
ArticTranspiler::transpile_loop_statement(ASTloop_statement* node)
{
    switch (node->get_looptype()) {
    case ASTloop_statement::LoopFor:
        source->add_source_with_indent("{\n");
        source->push_indent();
        //source->add_source_with_indent("let mut ");
        source->add_source_with_indent("");
        dispatch_node(node->init());
        source->add_source(";\n");
    case ASTloop_statement::LoopWhile:
        source->add_source_with_indent("while(");
        dispatch_node(node->cond());
        source->add_source(") {\n");
        break;
    case ASTloop_statement::LoopDo:
        source->add_source_with_indent("while({");
        break;
    }
    source->push_indent();
    transpile_statement_list(node->stmt());


    switch (node->get_looptype()) {
    case ASTloop_statement::LoopFor:
        source->add_source_with_indent("");
        dispatch_node(node->iter());
        source->add_source(";");
        source->pop_indent();
        source->add_source("\n");
        source->add_source_with_indent("}\n");
        source->pop_indent();
        source->add_source_with_indent("}\n");
        break;
    case ASTloop_statement::LoopWhile:
        source->pop_indent();
        source->add_source_with_indent("}\n");
        break;
    case ASTloop_statement::LoopDo:
        dispatch_node(node->cond());
        source->pop_indent();
        source->add_source_with_indent("}){}\n");
        break;
    }
}
void
ArticTranspiler::transpile_loopmod_statement(ASTloopmod_statement* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_return_statement(ASTreturn_statement* node)
{
    source->add_source("return ");
    dispatch_node(node->expr());
}
void
ArticTranspiler::transpile_binary_expression(ASTbinary_expression* node)
{
    auto left  = node->left();
    auto right = node->right();
    if (node->is_boolean_operator()) {
        source->add_source("(");
        dispatch_node(left);
        source->add_source(") ", node->opname(), " (");
        dispatch_node(right);
        source->add_source(")");
    } else {
        source->add_source(
            "ops_", artic_type_string_to_string(get_artic_type_string(left)),
            "().", node->opword(), "_", get_artic_type_string(right), "(");
        dispatch_node(left);
        source->add_source(", ");
        dispatch_node(right);
        source->add_source(")");
    }
}
void
ArticTranspiler::transpile_unary_expression(ASTunary_expression* node)
{
    source->add_source(node->opname(), "(");
    dispatch_node(node->expr());
    source->add_source(")");
}
void
ArticTranspiler::transpile_assign_expression(ASTassign_expression* node)
{
    auto ref  = (ASTvariable_ref*)node->var().get();
    auto decl = (ASTvariable_declaration*)ref->sym()->node();
    if (decl) {
        if (decl->is_output() && !this->in_shader) {
            source->add_source("*");
        }
    }
    dispatch_node(node->var());
    source->add_source(" = ");

    if (node->expr()->nodetype() == ASTNode::NodeType::literal_node) {
        auto lit = new ASTtype_constructor(node->typespec(), node->expr().get());
        dispatch_node(lit);
        //delete lit;
    } else {
        if(node->typespec() != node->expr()->typespec()){
            source->add_source("(");
            dispatch_node(node->expr());
            source->add_source(") as ", get_artic_type_string(node));
        } else {
            dispatch_node(node->expr());
        }

    }



}
void
ArticTranspiler::transpile_ternary_expression(ASTternary_expression* node)
{
    source->add_source("if (");
    dispatch_node(node->cond());
    source->add_source(") {");
    dispatch_node(node->trueexpr());
    source->add_source("} else {");
    dispatch_node(node->falseexpr());
    source->add_source("}");
}

void
ArticTranspiler::transpile_comma_operator(ASTcomma_operator* node)
{
    NOT_IMPLEMENTED;
}
void
ArticTranspiler::transpile_typecast_expression(ASTtypecast_expression* node)
{
    source->add_source("ops_", get_artic_type_string(node->expr()), "().as_",
                       get_artic_type_string(node), "(");
    dispatch_node(node->expr());
    source->add_source(")");
}
void
ArticTranspiler::transpile_type_constructor(ASTtype_constructor* node)
{
    if (node->typespec() == node->args()->typespec()
        && !node->args()->next()) {  // copy-constructor
        dispatch_node(node->args());
        return;
    } else if (node->typespec().is_float()
               || node->typespec()
                      .is_int()) {
        if(node->args()->nodetype() == ASTNode::NodeType::literal_node){
            dispatch_node(node->args()); // initializing trivially with literal
        } else {
            source->add_source("(");
            dispatch_node(node->args());
            source->add_source(") as ", get_artic_type_string(node));
        }

        return;
    } else if (node->typespec().is_closure()
               && node->args()->typespec().is_int()) {
        source->add_source("EMPTY_CLOSURE");
        return;
    }

    std::vector<ASTNode::ref> args = {};
    ASTNode::ref arg_node          = node->args();
    while (arg_node) {
        args.push_back(arg_node);
        auto next = arg_node->next();
        arg_node->detach_next();
        arg_node = next;
    }
    source->add_source(artic_string(node->typespec(), 0), "{");
    if (node->typespec().is_triple()
        && args.size() == 1) {  // init triple with one value
        args.push_back(args[0]);
        args.push_back(args[0]);
    }

    for (size_t i = 0; i < args.size(); ++i) {
        source->add_source(get_arg_name(node->typespec(), static_cast<int>(i)),
                           " = ");

        dispath_constructor_argument(node->typespec(), args[i], i);
        source->add_source(", ");
    }
    source->add_source("}");
}

void
ArticTranspiler::dispath_constructor_argument(TypeSpec ts, ASTNode::ref arg,
                                              int i)
{
    if (ts.is_triple()) {
        dispatch_node(arg);
    } else if (ts.is_structure()) {
        auto fs   = ts.structspec()->field(i).type;
        auto cons = ASTtype_constructor(fs, arg.get());
        transpile_type_constructor(&cons);
    } else {
        NOT_IMPLEMENTED;
    }
}


void
ArticTranspiler::transpile_function_call(ASTfunction_call* node)
{
    if (node->is_struct_ctr()) {
        auto constructor = ASTtype_constructor(node);
        transpile_type_constructor(&constructor);
    } else {
        std::vector<ASTNode::ref> args = {};
        auto arg_node                  = node->args();
        source->add_source(node->opname());
        while (arg_node) {
            args.push_back(arg_node);
            source->add_source("_", get_artic_type_string(arg_node));
            arg_node = arg_node->next();
        }
        source->add_source("__", get_artic_type_string(node), "(");

        auto func_node                   = node->user_function();
        ASTvariable_declaration* argnode = nullptr;
        if (func_node) {
            argnode = (ASTvariable_declaration*)func_node->formals().get();
        }
        for (auto arg : args) {
            if (argnode) {
                if (argnode->is_output()) {
                    source->add_source("&mut ");
                }
                argnode = (ASTvariable_declaration*)argnode->next().get();
            }
            if(arg->typespec().is_array()) {
                source->add_source("&");
            }
            dispatch_node(arg);
            source->add_source(", ");
            if(arg->typespec().is_array()) {
                source->add_source("||{", std::to_string(get_array_size(arg)), "}, ");
            }
        }
        emit_shaderinout_constructor();
        source->add_source(")");
    }
}
void
ArticTranspiler::transpile_literal_node(ASTliteral* node)
{
    if (node->typespec().is_int()) {
        source->add_source(std::to_string(node->intval()));
    } else if (node->typespec().is_float()) {
        source->add_source(std::to_string(node->floatval()));
    } else if (node->typespec().is_string()) {
        add_string_constant(node->strval());
        std::string sval = node->strval();
        if (sval.empty()) {
            sval = "empty_string";
        }
        source->add_source("String::", sval);
    } else {
        NOT_IMPLEMENTED;
    }
}


std::string
ArticTranspiler::get_arg_name(TypeSpec typeSpec, int argnum)
{
    if (typeSpec.is_triple()) {
        switch (argnum) {
        case 0: return "x";
        case 1: return "y";
        default: return "z";
        }
    } else if (typeSpec.is_structure()) {
        auto structSpec = typeSpec.structspec();
        OSL_ASSERT(argnum < structSpec->numfields());
        auto fieldSpec = structSpec->field(static_cast<int>(argnum));
        return fieldSpec.name.string();
    } else if (typeSpec.is_matrix()) {
        switch (argnum) {
        case 0: return "m1_n1";
        case 1: return "m1_n2";
        case 2: return "m1_n3";
        case 3: return "m1_n4";
        case 4: return "m2_n1";
        case 5: return "m2_n2";
        case 6: return "m2_n3";
        case 7: return "m2_n4";
        case 8: return "m3_n1";
        case 9: return "m3_n2";
        case 10: return "m3_n3";
        case 11: return "m3_n4";
        case 12: return "m4_n1";
        case 13: return "m4_n2";
        case 14: return "m4_n3";
        case 15: return "m4_n4";
        default: NOT_IMPLEMENTED;
        }
    } else if (typeSpec.is_float() || typeSpec.is_int()) {
        NOT_IMPLEMENTED;
    } else {
        NOT_IMPLEMENTED;
    }
}
void
ArticTranspiler::add_string_constant(const std::string& s)
{
    const_strings.insert(s);
}
void
ArticTranspiler::generate_struct_definition(TypeSpec typeSpec)
{
    if (typeSpec.is_structure()) {
        auto structSpec = typeSpec.structspec();
        source->add_source_with_indent("struct ", structSpec->name().string(),
                                       " {\n");
        source->push_indent();
        for (int i = 0; i < structSpec->numfields(); i++) {
            auto fieldSpec = structSpec->field(i);
            source->add_source_with_indent(fieldSpec.name.string(), ": ",
                                           artic_string(fieldSpec.type, 0),
                                           ",\n");
        }
        source->pop_indent();
        source->add_source_with_indent("}\n\n");
    }
}
void
ArticTranspiler::emit_shaderinout_copy()
{
    source->add_source_with_indent("let mut P = inout.P;\n");
    source->add_source_with_indent("let I = inout.I;\n");
    source->add_source_with_indent("let mut N = inout.N;\n");
    source->add_source_with_indent("let Ng = inout.Ng;\n");
    source->add_source_with_indent("let dPdu = inout.dPdu;\n");
    source->add_source_with_indent("let dPdv = inout.dPdv;\n");
    source->add_source_with_indent("let Ps = inout.Ps;\n");
    source->add_source_with_indent("let u = inout.u;\n");
    source->add_source_with_indent("let v = inout.v;\n");
    source->add_source_with_indent("let time = inout.time;\n");
    source->add_source_with_indent("let dtime = inout.dtime;\n");
    source->add_source_with_indent("let dPdtime = inout.dPdtime;\n");
    source->add_source_with_indent("let mut Ci = inout.Ci;\n");
}
void
ArticTranspiler::emit_shaderinout_constructor()
{
    source->add_source("shader_inout {\n");
    source->push_indent();
    source->add_source_with_indent("P = P,\n");
    source->add_source_with_indent("I = I,\n");
    source->add_source_with_indent("N = N,\n");
    source->add_source_with_indent("Ng = Ng,\n");
    source->add_source_with_indent("dPdu = dPdu,\n");
    source->add_source_with_indent("dPdv = dPdv,\n");
    source->add_source_with_indent("Ps = Ps,\n");
    source->add_source_with_indent("u = u,\n");
    source->add_source_with_indent("v = v,\n");
    source->add_source_with_indent("time = time,\n");
    source->add_source_with_indent("dtime = dtime,\n");
    source->add_source_with_indent("dPdtime = dPdtime,\n");
    source->add_source_with_indent("Ci = Ci,\n");
    source->pop_indent();
    source->add_source_with_indent("}");
}



OSL_NAMESPACE_EXIT