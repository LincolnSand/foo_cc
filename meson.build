project('foo_cc', 'cpp',
    version : '0.1',
    default_options : ['warning_level=3', 'cpp_std=c++17'])

add_global_arguments('-O0', language : 'cpp')
add_global_arguments('-Wall', language : 'cpp')
add_global_arguments('-Wextra', language : 'cpp')
add_global_arguments('-fsanitize=address', language : 'cpp')
add_global_arguments('--coverage', language : 'cpp')
add_global_arguments('-g', language : 'cpp')


inc = [include_directories('src')]

project_source_files = [
    'src/main.cpp',
    'src/io/file_io.cpp',
    'src/frontend/lexing/lexer.cpp',
    'src/frontend/parsing/parser_utils.cpp',
    'src/frontend/parsing/parser.cpp',
    'src/middle_end/typing/generate_typing.cpp',
    'src/backend/interpreter/compile_time_evaluator.cpp',
    'src/frontend/ast/ast_printer.cpp'
]

link_arguments = [
    '-fsanitize=address',
    '-static-libasan',
    '--coverage'
]

exe = executable('foo_cc',
    project_source_files,
    include_directories : inc,
    install : true,
    override_options: ['b_lundef=false'],
    link_args : link_arguments)

test('basic', exe)
