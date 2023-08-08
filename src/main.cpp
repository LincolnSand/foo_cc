#include <iostream>
#include <cstring>

#include <io/file_io.hpp>
#include <frontend/lexing/lexer.hpp>
#include <frontend/parsing/parser.hpp>
#include <frontend/ast/ast_printer.hpp>
#include <middle_end/validation/validate_ast.hpp>
//#include <backend/x86_64/traverse_ast.hpp>


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
        lexer_t lexer(file_contents.c_str());
        std::vector<token_t> tokens_list = scan_all_tokens(lexer);
        parser_t parser(tokens_list);
        ast::program_t ast = parse(parser);
        validate_ast(ast);
        print_ast(ast);
        //std::string assembly_output = generate_asm(ast);
        //write_string_into_file(assembly_output, out_filename.c_str());
    } else {
        std::cout << "You require an input file\n";
    }

    return 0;
}
