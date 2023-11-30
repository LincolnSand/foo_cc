project('foo_cc', 'cpp',
    version : '0.1',
    #default_options : ['warning_level=3', 'cpp_std=c++17'])
    default_options : ['warning_level=0', 'cpp_std=c++17'])

add_global_arguments('-O0', language : 'cpp')
#add_global_arguments('-Wall', language : 'cpp')
#add_global_arguments('-Wextra', language : 'cpp')
add_global_arguments('-fsanitize=address', language : 'cpp')
add_global_arguments('--coverage', language : 'cpp')
add_global_arguments('-g', language : 'cpp')
add_global_arguments('-g', language : 'c')
add_global_arguments('-ggdb', language : 'cpp')
add_global_arguments('-ggdb', language : 'c')



inc = [include_directories('src')] + [include_directories('debug')]

project_source_files = [
    'debug/exception_stack_trace.cpp',

    'src/io/file_io.cpp',

    'src/frontend/lexing/lexer.cpp',

    'src/frontend/parsing/parser_utils.cpp',
    'src/frontend/parsing/parser.cpp',

    'src/middle_end/typing/generate_typing.cpp',
    'src/middle_end/typing/type_checker.cpp',

    'src/backend/interpreter/compile_time_evaluator.cpp',

    'src/backend/x86_64/compile_operators.cpp',
    'src/backend/x86_64/traverse_ast.cpp',
    'src/backend/x86_64/traverse_ast_helpers.cpp',

    'src/frontend/ast/ast_printer.cpp'
]

link_arguments = [
    #'-rdynamic',

    '-fsanitize=address',
    '-static-libasan',
    '--coverage'
]

exe = executable('foo_cc',
    ['src/main.cpp'] + project_source_files,
    include_directories : inc,
    install : true,
    override_options: ['b_lundef=false'],
    link_args : link_arguments)


tests_src = [
    'tests/compile_time/variant_adapter.cpp',
    'tests/runtime/utils_common_test.cpp'
]

tests_inc = [

]

gtest_proj = subproject('gtest')
gtest_dep = gtest_proj.get_variable('gtest_main_dep')
gmock_dep = gtest_proj.get_variable('gmock_dep')

test_exe = executable(
    'gtest-all',
    project_source_files + tests_src,
    include_directories : inc + tests_inc,
    dependencies : [gtest_dep, gmock_dep],
    link_args : link_arguments)

test('gtest tests', test_exe)
