#ifndef _GNU_SOURCE
#define _GNU_SOURCE
#endif
#ifndef __USE_GNU
#define __USE_GNU
#endif


#include <iostream>
#include <cstring>
#include <stdexcept>

#include <io/file_io.hpp>
#include <frontend/lexing/lexer.hpp>
#include <frontend/parsing/parser.hpp>
#include <frontend/ast/ast_printer.hpp>
#include <middle_end/typing/type_checker.hpp>
//#include <backend/x86_64/traverse_ast.hpp>

#include <exception_stack_trace.hpp>


int main(int argc, char** argv) {
    std::string out_filename;
    if(argc == 2) {
        uint32_t i;
        for(i = 0; i < std::strlen(argv[1])-1 && (argv[1][i] != '.' || argv[1][i+1] == '/'); ++i);
        out_filename = std::string(argv[1], i) + std::string(".s");
    }
    else if(argc == 3) {
        out_filename = std::string(argv[2]);
    } else {
        std::cerr << "You require an input file\n";
    }

    if(argc > 1) {
        std::string file_contents = read_file_into_string(argv[1]);

        //try {
            lexer_t lexer(file_contents.c_str());
            std::vector<token_t> tokens_list = scan_all_tokens(lexer);

            parser_t parser(tokens_list);
            ast::validated_program_t ast = parse(parser);

            std::cout << "before type checking\n";
            print_validated_ast(ast);

            type_check(ast); // mutates `valid_ast`

            std::cout << "after type checking\n";
            print_validated_ast(ast);
        //} catch(const std::runtime_error &e) {
            // Temporary while we are fuzzing.
            // TODO: Implement proper error handling for compile errors.
            //return 0;
        //}

        //std::string assembly_output = generate_asm(ast);

        //write_string_into_file(assembly_output, out_filename.c_str());
    } else {
        std::cout << "You require an input file\n";
    }

    return 0;
}
