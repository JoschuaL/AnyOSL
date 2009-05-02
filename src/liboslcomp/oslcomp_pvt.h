/*****************************************************************************
 *
 *             Copyright (c) 2009 Sony Pictures Imageworks, Inc.
 *                            All rights reserved.
 *
 *  This material contains the confidential and proprietary information
 *  of Sony Pictures Imageworks, Inc. and may not be disclosed, copied or
 *  duplicated in any form, electronic or hardcopy, in whole or in part,
 *  without the express prior written consent of Sony Pictures Imageworks,
 *  Inc. This copyright notice does not imply publication.
 *
 *****************************************************************************/

#ifndef OSLCOMP_PVT_H
#define OSLCOMP_PVT_H

#include "OpenImageIO/ustring.h"

#include "oslcomp.h"
#include "ast.h"
#include "symtab.h"


class oslFlexLexer;
extern int oslparse ();


namespace OSL {
namespace pvt {


class ASTNode;


/// Intermediate Represenatation opcode
///
class IROpcode {
public:
    IROpcode (ustring op, ustring method, size_t firstarg, size_t nargs);
    const char *opname () const { return m_op.c_str(); }
    size_t firstarg () const { return (size_t)m_firstarg; }
    size_t nargs () const { return (size_t) m_nargs; }
    ustring method () const { return m_method; }
    void source (ustring sourcefile, int sourceline) {
        m_sourcefile = sourcefile;
        m_sourceline = sourceline;
    }
    ustring sourcefile () const { return m_sourcefile; }
    int sourceline () const { return m_sourceline; }

    /// Set the jump addresses (-1 means no jump)
    ///
    void set_jump (int jump0=-1, int jump1=-1, int jump2=-1) {
        m_jump[0] = jump0;
        m_jump[1] = jump1;
        m_jump[2] = jump2;
    }

    /// Return the i'th jump target address (-1 for none).
    ///
    int jump (int i) const { return m_jump[i]; }

    static const int max_jumps = 3; ///< Maximum jump targets an op can have

private:
    ustring m_op;                   ///< Name of opcode
    int m_firstarg;                 ///< Index of first argument
    int m_nargs;                    ///< Total number of arguments
    ustring m_method;               ///< Which param or method this code is for
    int m_jump[max_jumps];          ///< Jump addresses (-1 means none)
    ustring m_sourcefile;           ///< Source filename for this op
    int m_sourceline;               ///< Line of source code for this op
};


typedef std::vector<IROpcode> IROpcodeVec;



class OSLCompilerImpl : public OSL::OSLCompiler {
public:
    OSLCompilerImpl ();
    virtual ~OSLCompilerImpl ();

    /// Fully compile a shader located in 'filename', with the command-line
    /// options ("-I" and the like) in the options vector.
    virtual bool compile (const std::string &filename,
                          const std::vector<std::string> &options);

    /// The name of the file we're currently parsing
    ///
    ustring filename () const { return m_filename; }

    /// Set the name of the file we're currently parsing (should only
    /// be called by the lexer!)
    void filename (ustring f) { m_filename = f; }

    /// The line we're currently parsing
    ///
    int lineno () const { return m_lineno; }

    /// Set the line we're currently parsing (should only be called by
    /// the lexer!)
    void lineno (int l) { m_lineno = l; }

    /// Increment the line count
    ///
    int incr_lineno () { return ++m_lineno; }

    /// Return a pointer to the current lexer.
    ///
    oslFlexLexer *lexer() const { return m_lexer; }

    /// Error reporting
    ///
    void error (ustring filename, int line, const char *format, ...);

    /// Warning reporting
    ///
    void warning (ustring filename, int line, const char *format, ...);

    /// Have we hit an error?
    ///
    bool error_encountered () const { return m_err; }

    /// Has a shader already been defined?
    bool shader_is_defined () const { return m_shader; }

    /// Define the shader we're compiling with the given AST root.
    ///
    void shader (ASTNode::ref s) { m_shader = s; }

    /// Return the AST root of the main shader we're compiling.
    ///
    ASTNode::ref shader () const { return m_shader; }

    /// Return a reference to the symbol table.
    ///
    SymbolTable &symtab () { return m_symtab; }

    /// Register a symbol
    ///
//    void add_function (Symbol *sym) { m_allfuncs.push_back (sym); }

    TypeSpec current_typespec () const { return m_current_typespec; }
    void current_typespec (TypeSpec t) { m_current_typespec = t; }
    bool current_output () const { return m_current_output; }
    void current_output (bool b) { m_current_output = b; }

    /// Given a pointer to a type code string that we use for argument
    /// checking ("p", "v", etc.) return the TypeSpec of the first type
    /// described by the string (UNKNOWN if it couldn't be recognized).
    /// If 'advance' is non-NULL, set *advance to the number of
    /// characters taken by the first code so the caller can advance
    /// their pointer to the next code in the string.
    TypeSpec type_from_code (const char *code, int *advance=NULL);

    /// Take a type code string (possibly containing many types)
    /// and turn it into a human-readable string.
    std::string typelist_from_code (const char *code);

    /// Emit a single IR opcode -- append one op to the list of
    /// intermediate code, returning the label (address) of the new op.
    int emitcode (const char *opname, size_t nargs, Symbol **args,
                  ASTNode *node);

    /// Return the label (opcode address) for the next opcode that will
    /// be emitted.
    int next_op_label () { return (int)m_ircode.size(); }

    /// Return a reference to a given IR opcode.
    ///
    IROpcode & ircode (int index) { return m_ircode[index]; }

    /// Specify that subsequent opcodes are for a particular method
    ///
    void codegen_method (ustring method) { m_codegenmethod = method; }

    /// Make a temporary symbol of the given type.
    ///
    Symbol *make_temporary (const TypeSpec &type);

    /// Make a constant string symbol
    ///
    Symbol *make_constant (ustring s);

    /// Make a constant int symbol
    ///
    Symbol *make_constant (int i);

    /// Make a constant float symbol
    ///
    Symbol *make_constant (float f);

    std::string output_filename (const std::string &inputfilename);

private:
    void initialize_globals ();
    void initialize_builtin_funcs ();

    void write_oso_file (const std::string &outfilename);
    void write_oso_const_value (const ConstantSymbol *sym) const;
    void write_oso_formal_default (const ASTvariable_declaration *node) const;
    void write_oso_symbol (const Symbol *sym) const;
    void write_oso_metadata (const ASTNode *metanode) const;
    void oso (const char *fmt, ...) const;

    ASTshader_declaration *shader_decl () const {
        return dynamic_cast<ASTshader_declaration *>(m_shader.get());
    }
    std::string retrieve_source (ustring filename, int line);

    oslFlexLexer *m_lexer;    ///< Lexical scanner
    ustring m_filename;       ///< Current file we're parsing
    int m_lineno;             ///< Current line we're parsing
    ASTNode::ref m_shader;    ///< The shader's syntax tree
    bool m_err;               ///< Has an error occurred?
    SymbolTable m_symtab;     ///< Symbol table
    TypeSpec m_current_typespec;  ///< Currently-declared type
    bool m_current_output;        ///< Currently-declared output status
//    SymbolList m_allfuncs;      ///< All function symbols, in decl order
    bool m_verbose;           ///< Verbose mode
    bool m_debug;             ///< Debug mode
    IROpcodeVec m_ircode;     ///< Generated IR code
    std::vector<Symbol *> m_opargs;  ///< Arguments for all instructions
    int m_next_temp;          ///< Next temporary symbol index
    int m_next_const;         ///< Next const symbol index
    std::vector<ConstantSymbol *> m_const_syms;  ///< All consts we've made
    FILE *m_osofile;          ///< Open .oso file for output
    FILE *m_sourcefile;       ///< Open file handle for retrieve_source
    ustring m_last_sourcefile;///< Last filename for retrieve_source
    int m_last_sourceline;    ///< Last line read for retrieve_source
    ustring m_codegenmethod;  ///< Current method we're generating code for
};


extern OSLCompilerImpl *oslcompiler;


}; // namespace pvt
}; // namespace OSL


#endif /* OSLCOMP_PVT_H */
